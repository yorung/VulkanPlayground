#include "stdafx.h"

#include <random>

float Random()
{
	static std::mt19937 seed{ std::random_device()() };
	return std::uniform_real_distribution<float>(0.0, 1.0)(seed);
}

double GetTime()
{
	static auto start = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(now - start).count();
}

void _afVerify(const char* file, const char* func, int line, const char* command, bool ok)
{
	if (ok)
	{
		return;
	}
	aflog("afVerify Fatal: %s %d %s %s", file, line, func, command);
	*(uint32_t*)(4) = 1;	// crash
}


IBOID afCreateTiledPlaneIBO(int numTiles, int* numIndies)
{
	const int numVert = numTiles + 1;

	std::vector<AFIndex> indi;
	for (int y = 0; y < numTiles; y++) {
		if (y != 0) {
			indi.push_back(y * numVert);
		}
		indi.push_back(y * numVert);
		for (int x = 0; x < numTiles; x++) {
			indi.push_back((y + 1) * numVert + x);
			indi.push_back(y * numVert + x + 1);
		}
		indi.push_back((y + 1) * numVert + numVert - 1);
		if (y != numTiles - 1) {
			indi.push_back((y + 1) * numVert + numVert - 1);
		}
	}

	if (numIndies) {
		*numIndies = (int)indi.size();
	}

	return afCreateIndexBuffer((int)indi.size(), &indi[0]);
}

VBOID afCreateTiledPlaneVBO(int numTiles)
{
	std::vector<Vec2> v;
	for (int y = 0; y <= numTiles; y++) {
		for (int x = 0; x <= numTiles; x++) {
			v.push_back((Vec2)IVec2(x, y) / (float)numTiles * 2 - Vec2(1, 1));
		}
	}
	return afCreateVertexBuffer((int)v.size() * sizeof(v[0]), &v[0]);
}

struct DDSHeader {
	uint32_t h3[3];
	int h, w;
	uint32_t h2[2];
	int mipCnt;
	uint32_t h13[13];
	uint32_t fourcc, bitsPerPixel, rMask, gMask, bMask, aMask, caps1, caps2;
	bool IsCubeMap() const { return caps2 == 0xFE00; }
	int GetArraySize() const { return IsCubeMap() ? 6 : 1; }
	int GetMipCnt() const { return std::max(mipCnt, 1); }
};

static void bitScanForward(uint32_t* result, uint32_t mask)
{
	//	DWORD dwd;
	//	_BitScanForward(&dwd, mask);
	//	*result = dwd;

	for (int i = 0; i < 32; i++) {
		if (mask & (1 << i)) {
			*result = i;
			return;
		}
	}
	*result = 0;
}

static void ArrangeRawDDS(void* img, int size)
{
	const DDSHeader* hdr = (DDSHeader*)img;
	uint32_t rShift, gShift, bShift, aShift;
	bitScanForward(&rShift, hdr->rMask);
	bitScanForward(&gShift, hdr->gMask);
	bitScanForward(&bShift, hdr->bMask);
	bitScanForward(&aShift, hdr->aMask);
	std::for_each((uint32_t*)img + 128 / 4, (uint32_t*)img + size / 4, [&](uint32_t& im) {
		im = ((hdr->aMask & im) >> aShift << 24) + ((hdr->bMask & im) >> bShift << 16) + ((hdr->gMask & im) >> gShift << 8) + ((hdr->rMask & im) >> rShift);
		if (hdr->aMask == 0) {
			im |= 0xff000000;
		}
	});
}

static SRVID LoadDDSTexture(const char* name, TexDesc& texDesc)
{
	int size;
	void* img = LoadFile(name, &size);
	if (!img) {
		aflog("LoadDDSTexture failed! %s", name);
		return SRVID();
	}
	const DDSHeader* hdr = (DDSHeader*)img;

	AFFormat format = AFF_INVALID;
	int(*pitchCalcurator)(int, int) = nullptr;
	switch (hdr->fourcc) {
	case 0x31545844: //'1TXD':
		format = AFF_BC1_UNORM;
		pitchCalcurator = [](int w, int h) { return ((w + 3) / 4) * ((h + 3) / 4) * 8; };
		break;
	case 0x33545844: //'3TXD':
		format = AFF_BC2_UNORM;
		pitchCalcurator = [](int w, int h) { return ((w + 3) / 4) * ((h + 3) / 4) * 16; };
		break;
	case 0x35545844: //'5TXD':
		format = AFF_BC3_UNORM;
		pitchCalcurator = [](int w, int h) { return ((w + 3) / 4) * ((h + 3) / 4) * 16; };
		break;
	default:
		ArrangeRawDDS(img, size);
		format = AFF_R8G8B8A8_UNORM;
		pitchCalcurator = [](int w, int h) { return w * h * 4; };
		break;
	}
	texDesc.size.x = hdr->w;
	texDesc.size.y = hdr->h;
	texDesc.arraySize = hdr->GetArraySize();
	texDesc.isCubeMap = hdr->IsCubeMap();

	int arraySize = hdr->GetArraySize();
	int mipCnt = hdr->GetMipCnt();
	std::vector<AFTexSubresourceData> r;
	int offset = 128;
	for (int a = 0; a < arraySize; a++) {
		for (int m = 0; m < mipCnt; m++) {
			int w = std::max(1, hdr->w >> m);
			int h = std::max(1, hdr->h >> m);
			int size = pitchCalcurator(w, h);
			r.push_back({ (char*)img + offset, (uint32_t)pitchCalcurator(w, 1), (uint32_t)size });
			offset += size;
		}
	}

	SRVID srv = afCreateTexture2D(format, texDesc, mipCnt, &r[0]);
	assert(srv);
	free(img);
	return srv;
}

SRVID afLoadTexture(const char* name, TexDesc& desc)
{
	desc = TexDesc();
	size_t len = strlen(name);
	SRVID tex;
	if (len > 4 && !stricmp(name + len - 4, ".dds"))
	{
		tex = LoadDDSTexture(name, desc);
	}
	else
	{
		tex = LoadTextureViaOS(name, desc.size);
	}
	afSetTextureName(tex, name);
	return tex;
}

IBOID afCreateQuadListIndexBuffer(int numQuads)
{
	std::vector<AFIndex> indi;
	int numIndi = numQuads * 6;
	indi.resize(numIndi);
	for (int i = 0; i < numIndi; i++)
	{
		static int tbl[] = { 0, 1, 2, 1, 3, 2 };
		int rectIdx = i / 6;
		int vertIdx = i % 6;
		indi[i] = rectIdx * 4 + tbl[vertIdx];
	}
	return afCreateIndexBuffer(numIndi, &indi[0]);
}
