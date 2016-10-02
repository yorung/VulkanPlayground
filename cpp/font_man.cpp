#include "stdafx.h"

#define TEX_W		512
#define TEX_H		512

FontMan fontMan;

struct FontVertex {
	Vec2 pos;
	Vec2 coord;
};

static Vec2 fontVertAlign[] =
{
	Vec2(0, 0),
	Vec2(1, 0),
	Vec2(0, 1),
	Vec2(1, 1),
};

FontMan::FontMan()
{
	ClearCache();
}

FontMan::~FontMan()
{
	Destroy();
}

void FontMan::ClearCache()
{
	caches.clear();
	curX = curY = curLineMaxH = 0;
}

static InputElement elements[] = {
	CInputElement(0, AFF_R32G32_FLOAT, 0),
	CInputElement(1, AFF_R32G32_FLOAT, 8),
};

void FontMan::Create()
{
	Destroy();
	if (!texSrc.Create(TEX_W, TEX_H))
	{
		assert(0);
		return;
	}
	texture = afCreateTexture2D(AFF_R8G8B8A8_UNORM, IVec2(TEX_W, TEX_H), nullptr);
	afSetTextureName(texture, __FUNCTION__);

	VkDevice device = deviceMan.GetDevice();

	VkDescriptorSetLayout layouts[] = { deviceMan.commonTextureDescriptorSetLayout };
	const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, arrayparam(layouts) };
	afHandleVKError(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	pipeline = deviceMan.CreatePipeline("font", pipelineLayout, arrayparam(elements), BM_ALPHA, DSM_DISABLE, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
//	renderStates.Create("font", arrayparam(elements), BM_ALPHA, DSM_DISABLE, CM_DISABLE, arrayparam(samplers), PT_TRIANGLELIST);
	quadListVertexBuffer.Create(sizeof(FontVertex), SPRITE_MAX);
}

void FontMan::Destroy()
{
	afSafeDeleteTexture(texture);
	VkDevice device = deviceMan.GetDevice();
	afSafeDeleteVk(vkDestroyPipeline, device, pipeline);
	afSafeDeleteVk(vkDestroyPipelineLayout, device, pipelineLayout);
	texSrc.Destroy();
	quadListVertexBuffer.Destroy();
	ClearCache();
}

bool FontMan::Build(const CharSignature& signature)
{
	DIB	dib;
	CharCache cache;
	MakeFontBitmap("Gulim", signature, dib, cache.desc);
	int remainX = texSrc.getW() - curX;
	if (remainX < dib.getW()) {
		curX = 0;
		curY += curLineMaxH;
		curLineMaxH = 0;
	//	aflog("FontMan::Build() new line\n");
	}
	int remainY = texSrc.getH() - curY;
	if (remainY < dib.getH()) {
	//	aflog("FontMan::Build() font texture is full!\n");
		return false;
	}
	curLineMaxH = std::max(curLineMaxH, dib.getH());

	cache.srcPos = Vec2((float)curX, (float)curY);
	if (dib.getW() > 0 && dib.getH() > 0) {
		dib.Blt(texSrc, curX, curY);
		dirty = true;
	}

	//char codestr[128];
	//snprintf(codestr, dimof(codestr), "%04x %c", signature.code, signature.code < 0x80 ? signature.code : 0x20);
	//aflog("FontMan::Build() curX=%d curY=%d dib.getW()=%d dib.getH()=%d code=%s\n", curX, curY, dib.getW(), dib.getH(), codestr);

	curX += (int)std::ceil(cache.desc.srcWidth.x);
	caches[signature] = cache;
	return true;
}

bool FontMan::Cache(const CharSignature& sig)
{
	assert(sig.code >= 0 && sig.code <= 0xffff);
	Caches::iterator it = caches.find(sig);
	if (it != caches.end())
	{
		return true;
	}
	return Build(sig);
}

void FontMan::FlushToTexture()
{
	if (!dirty) {
		return;
	}
//	aflog("FontMan::FlushToTexture flushed\n");
	dirty = false;
	TexDesc desc;
	desc.size = IVec2(TEX_W, TEX_H);
	afWriteTexture(texture, desc, texSrc.ReferPixels());
}

void FontMan::Render()
{
	if (!numSprites) {
		return;
	}
	for (int i = 0; i < numSprites; i++) {
		Cache(charSprites[i].signature);
	}
	FlushToTexture();

	Vec2 scrSize = systemMisc.GetScreenSize();
	static FontVertex verts[4 * SPRITE_MAX];
	for (int i = 0; i < numSprites; i++) {
		CharSprite& cs = charSprites[i];
		Caches::iterator it = caches.find(cs.signature);
		if (it == caches.end()) {
			aflog("something wrong");
			continue;
		}
		const CharCache& cc = it->second;
		for (int j = 0; j < (int)dimof(fontVertAlign); j++) {
			verts[i * 4 + j].pos = (((cs.pos + cc.desc.distDelta + fontVertAlign[j] * cc.desc.srcWidth)) * Vec2(2, 2)) / scrSize + Vec2(-1, -1);
			verts[i * 4 + j].coord = (cc.srcPos + fontVertAlign[j] * cc.desc.srcWidth) / Vec2(TEX_W, TEX_H);
		}
	}

	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	quadListVertexBuffer.Write(verts, 4 * numSprites * sizeof(FontVertex));
	quadListVertexBuffer.Apply();
	afBindTexture(pipelineLayout, texture, 0);
	afDrawIndexed(numSprites * 6);
	numSprites = 0;
}

void FontMan::DrawChar(Vec2& pos, const CharSignature& sig)
{
	if (!texture) {
		return;
	}

	if (numSprites >= SPRITE_MAX) {
		return;
	}
	Cache(sig);

	if (sig.code != 32) {	// whitespace
		CharSprite& cs = charSprites[numSprites++];
		cs.signature = sig;
		cs.pos = pos;
	}

	Caches::iterator it = caches.find(sig);
	if (it != caches.end()) {
		pos.x += it->second.desc.step;
	}
}

Vec2 FontMan::MeasureString(int fontSize, const char *text)
{
	int len = (int)strlen(text);
	Vec2 pos(0, 0);
	Vec2 size(0, 0);
	for (int i = 0; i < len; i++)
	{
		CharSignature sig;
		sig.code = text[i];
		sig.fontSize = fontSize;
		Cache(sig);
		Caches::iterator it = caches.find(sig);
		if (it != caches.end()) {
			size.y = std::max(size.y, it->second.desc.srcWidth.y);
			size.x = pos.x + it->second.desc.distDelta.x + it->second.desc.srcWidth.x;
			pos.x += it->second.desc.step;
		}
	}
	return size;
}

void FontMan::DrawString(Vec2 pos, int fontSize, const wchar_t *text)
{
	int len = (int)wcslen(text);
	for (int i = 0; i < len; i++)
	{
		CharSignature sig;
		sig.code = text[i];
		sig.fontSize = fontSize;
		DrawChar(pos, sig);
	}
}

void FontMan::DrawString(Vec2 pos, int fontSize, const char *text)
{
	int len = (int)strlen(text);
	for (int i = 0; i < len; i++)
	{
		CharSignature sig;
		sig.code = text[i];
		sig.fontSize = fontSize;
		DrawChar(pos, sig);
	}
}
