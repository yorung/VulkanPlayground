#pragma once

struct TexDesc
{
	IVec2 size;
	int arraySize = 1;
	bool isCubeMap = false;
};

struct ViewDesc
{
	IVec2 screenSize;
	Mat matView;
	Mat matProj;
};

enum RenderStateFlags : uint32_t
{
	AFRS_NONE = 0,
	AFRS_CULL_CCW = 0x1,
	AFRS_CULL_CW = 0x2,
	AFRS_WIREFRAME = 0x4,
	AFRS_ALPHA_BLEND = 0x8,
	AFRS_DEPTH_ENABLE = 0x10,
	AFRS_DEPTH_CLOSEREQUAL_READONLY = 0x20,
	AFRS_PRIMITIVE_TRIANGLESTRIP = 0x0,	// default
	AFRS_PRIMITIVE_TRIANGLELIST = 0x40,
	AFRS_PRIMITIVE_LINELIST = 0x80,
	AFRS_PRIMITIVE_PATCHLIST = 0x100,
	AFRS_OFFSCREEN_RENDER_TARGET_R8G8B8A8_UNORM = 0x200,
	AFRS_OFFSCREEN_RENDER_TARGET_R16G16B16A16_FLOAT = 0x400,
	AFRS_OFFSCREEN_RENDER_TARGET_R32_FLOAT = 0x800,
	AFRS_AUTO_DEPTH_STENCIL = 0x1000,	// default
	AFRS_DEPTH_STENCIL_D32_FLOAT = 0x2000,
};

inline bool afIsRenderFlagsRenderToSwapchainSurface(uint32_t flags)
{
	return !(flags & (AFRS_OFFSCREEN_RENDER_TARGET_R8G8B8A8_UNORM | AFRS_OFFSCREEN_RENDER_TARGET_R16G16B16A16_FLOAT | AFRS_OFFSCREEN_RENDER_TARGET_R32_FLOAT));
}

enum AFBufferType : uint32_t
{
	AFBT_VERTEX,
	AFBT_VERTEX_CPUWRITE,
	AFBT_INDEX,
	AFBT_CONSTANT,
	AFBT_CONSTANT_CPUWRITE,
};

inline bool afIsBufferTypeAllowsCpuWrite(AFBufferType bufferType)
{
	switch (bufferType)
	{
	case AFBT_CONSTANT_CPUWRITE:
	case AFBT_VERTEX_CPUWRITE:
		return true;
	}
	return false;
}

enum TextureFlags : uint32_t
{
	AFTF_CPU_WRITE = 0x01,
	AFTF_SRV = 0x02,
	AFTF_DSV = 0x04,
	AFTF_RTV = 0x08,
};

enum SamplerType
{
	AFST_POINT_WRAP,
	AFST_POINT_CLAMP,
	AFST_LINEAR_WRAP,
	AFST_LINEAR_CLAMP,
	AFST_MIPMAP_WRAP,
	AFST_MIPMAP_CLAMP,
	AFST_ANISOTROPIC_WRAP,
	AFST_ANISOTROPIC_CLAMP,
	AFST_DEPTH_ANISOTROPIC,
	AFST_MAX
};

enum SetRenderTargetFlags : uint32_t
{
	AFSRTF_CLEAR_COLOR = 0x01,
	AFSRTF_CLEAR_DEPTH_STENCIL = 0x02,
	AFSRTF_CLEAR_ALL = AFSRTF_CLEAR_COLOR | AFSRTF_CLEAR_DEPTH_STENCIL,
};
