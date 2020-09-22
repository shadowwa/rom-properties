/***************************************************************************
 * ROM Properties Page shell extension. (librptexture)                     *
 * dds_structs.h: DirectDraw Surface texture format data structures.       *
 *                                                                         *
 * Copyright (c) 2017-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBRPTEXTURE_FILEFORMAT_DDS_STRUCTS_H__
#define __ROMPROPERTIES_LIBRPTEXTURE_FILEFORMAT_DDS_STRUCTS_H__

#include <stdint.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * References:
 * - https://msdn.microsoft.com/en-us/library/windows/desktop/bb943990(v=vs.85).aspx
 * - https://msdn.microsoft.com/en-us/library/windows/desktop/bb943992(v=vs.85).aspx
 * - https://msdn.microsoft.com/en-us/library/windows/desktop/bb943982(v=vs.85).aspx (DDS_HEADER)
 * - https://msdn.microsoft.com/en-us/library/windows/desktop/bb943983(v=vs.85).aspx (DDS_HEADER_DX10)
 * - https://msdn.microsoft.com/en-us/library/windows/desktop/bb943984(v=vs.85).aspx (DDS_PIXELFORMAT)
 * - https://github.com/Microsoft/DirectXTK/wiki/XboxDDSTextureLoader (DDS_HEADER_XBOX)
 * - https://github.com/Microsoft/DirectXTex
 */

// NOTE: This header may conflict with the official DirectX SDK.

/**
 * DirectDraw Surface: Pixel format.
 * Reference: https://msdn.microsoft.com/en-us/library/windows/desktop/bb943984(v=vs.85).aspx
 *
 * All fields are in little-endian.
 */
typedef struct _DDS_PIXELFORMAT {
	uint32_t dwSize;		// [0x000]
	uint32_t dwFlags;		// [0x004] See DDS_PIXELFORMAT_FLAGS
	uint32_t dwFourCC;		// [0x008] See DDS_PIXELFORMAT_FOURCC
	uint32_t dwRGBBitCount;		// [0x00C]
	uint32_t dwRBitMask;		// [0x010]
	uint32_t dwGBitMask;		// [0x014]
	uint32_t dwBBitMask;		// [0x018]
	uint32_t dwABitMask;		// [0x01C]
} DDS_PIXELFORMAT;
ASSERT_STRUCT(DDS_PIXELFORMAT, 8*sizeof(uint32_t));

// dwFlags
typedef enum {
	DDPF_ALPHAPIXELS	= 0x1,
	DDPF_ALPHA		= 0x2,
	DDPF_FOURCC		= 0x4,
	DDPF_RGB		= 0x40,
	DDPF_YUV		= 0x200,
	DDPF_LUMINANCE		= 0x20000,
} DDS_PIXELFORMAT_FLAGS;

// dwFourCC
typedef enum {
	DDPF_FOURCC_DXT1	= 'DXT1',
	DDPF_FOURCC_DXT2	= 'DXT2',
	DDPF_FOURCC_DXT3	= 'DXT3',
	DDPF_FOURCC_DXT4	= 'DXT4',
	DDPF_FOURCC_DXT5	= 'DXT5',

	// BC4
	DDPF_FOURCC_ATI1	= 'ATI1',
	DDPF_FOURCC_BC4U	= 'BC4U',
	DDPF_FOURCC_BC4S	= 'BC4S',	// TODO: BC4 with signed values?

	// BC5
	DDPF_FOURCC_ATI2	= 'ATI2',
	DDPF_FOURCC_BC5U	= 'BC5U',
	DDPF_FOURCC_BC5S	= 'BC5S',	// TODO: BC5 with signed values?

	// PVRTC
	DDPF_FOURCC_PTC2	= 'PTC2',	// PVRTC 2bpp (RGBA)
	DDPF_FOURCC_PTC4	= 'PTC4',	// PVRTC 4bpp (RGBA)

	DDPF_FOURCC_DX10	= 'DX10',
	DDPF_FOURCC_XBOX	= 'XBOX',
} DDS_PIXELFORMAT_FOURCC;

/**
 * DirectDraw Surface: NVTT header.
 * This is present within the DDS header if the DDS was created by
 * nVidia Texture Tools.
 *
 * Reference: https://github.com/castano/nvidia-texture-tools/blob/9489aed825c6a0a931dfdd75e8ab6f97292b31a7/src/nvimage/DirectDrawSurface.cpp#L511
 *
 * All fields are in little-endian.
 */
#define NVTT_MAGIC 'NVTT'
typedef struct _DDS_NVTT_Header {
	uint32_t dwNvttReserved[9];	// [0x000]
	uint32_t dwNvttMagic;		// [0x024] 'NVTT'
	// TODO: Separate uint8_t values for major/minor/revision?
	uint32_t dwNvttVersion;		// [0x028] Version number.
					//         (major << 16) | (minor << 8) | (revision)
} DDS_NVTT_Header;
ASSERT_STRUCT(DDS_NVTT_Header, 11*sizeof(uint32_t));

/**
 * DirectDraw Surface: File header.
 * This does NOT include the 'DDS ' magic.
 * Reference: https://msdn.microsoft.com/en-us/library/windows/desktop/bb943982(v=vs.85).aspx
 *
 * All fields are in little-endian.
 */
#define DDS_MAGIC 'DDS '
typedef struct _DDS_HEADER {
	uint32_t dwSize;		// [0x000]
	uint32_t dwFlags;		// [0x004] See DDS_HEADER_FLAGS
	uint32_t dwHeight;		// [0x008]
	uint32_t dwWidth;		// [0x00C]
	uint32_t dwPitchOrLinearSize;	// [0x010]
	uint32_t dwDepth;		// [0x014]
	uint32_t dwMipMapCount;		// [0x018]
	union {
		uint32_t dwReserved1[11];	// [0x01C]
		DDS_NVTT_Header nvtt;		// [0x01C] NVTT header
	};
	DDS_PIXELFORMAT ddspf;		// [0x048]
	uint32_t dwCaps;		// [0x068]
	uint32_t dwCaps2;		// [0x06C]
	uint32_t dwCaps3;		// [0x070]
	uint32_t dwCaps4;		// [0x074]
	uint32_t dwReserved2;		// [0x078]
} DDS_HEADER;
ASSERT_STRUCT(DDS_HEADER, 124);

// dwFlags
typedef enum {
	DDSD_CAPS		= 0x1,
	DDSD_HEIGHT		= 0x2,
	DDSD_WIDTH		= 0x4,
	DDSD_PITCH		= 0x8,
	DDSD_PIXELFORMAT	= 0x1000,
	DDSD_MIPMAPCOUNT	= 0x20000,
	DDSD_LINEARSIZE		= 0x80000,
	DDSD_DEPTH		= 0x800000,
} DDS_HEADER_FLAGS;

// dwCaps
typedef enum {
	DDSCAPS_COMPLEX		= 0x8,
	DDSCAPS_MIPMAP		= 0x400000,
	DDSCAPS_TEXTURE		= 0x1000,
} DDS_HEADER_CAPS;

// dwCaps2
typedef enum {
	DDSCAPS2_CUBEMAP		= 0x200,
	DDSCAPS2_CUBEMAP_POSITIVEX	= 0x400,
	DDSCAPS2_CUBEMAP_NEGATIVEX	= 0x800,
	DDSCAPS2_CUBEMAP_POSITIVEY	= 0x1000,
	DDSCAPS2_CUBEMAP_NEGATIVEY	= 0x2000,
	DDSCAPS2_CUBEMAP_POSITIVEZ	= 0x4000,
	DDSCAPS2_CUBEMAP_NEGATIVEZ	= 0x8000,
	DDSCAPS2_VOLUME			= 0x200000,
} DDS_HEADER_CAPS2;

/**
 * DirectX 10 data format enum.
 * Reference: https://msdn.microsoft.com/en-us/library/windows/desktop/bb173059(v=vs.85).aspx
 */
typedef enum {
	DXGI_FORMAT_UNKNOWN			= 0,
	DXGI_FORMAT_R32G32B32A32_TYPELESS	= 1,
	DXGI_FORMAT_R32G32B32A32_FLOAT		= 2,
	DXGI_FORMAT_R32G32B32A32_UINT		= 3,
	DXGI_FORMAT_R32G32B32A32_SINT		= 4,
	DXGI_FORMAT_R32G32B32_TYPELESS		= 5,
	DXGI_FORMAT_R32G32B32_FLOAT		= 6,
	DXGI_FORMAT_R32G32B32_UINT		= 7,
	DXGI_FORMAT_R32G32B32_SINT		= 8,
	DXGI_FORMAT_R16G16B16A16_TYPELESS	= 9,
	DXGI_FORMAT_R16G16B16A16_FLOAT		= 10,
	DXGI_FORMAT_R16G16B16A16_UNORM		= 11,
	DXGI_FORMAT_R16G16B16A16_UINT		= 12,
	DXGI_FORMAT_R16G16B16A16_SNORM		= 13,
	DXGI_FORMAT_R16G16B16A16_SINT		= 14,
	DXGI_FORMAT_R32G32_TYPELESS		= 15,
	DXGI_FORMAT_R32G32_FLOAT		= 16,
	DXGI_FORMAT_R32G32_UINT			= 17,
	DXGI_FORMAT_R32G32_SINT			= 18,
	DXGI_FORMAT_R32G8X24_TYPELESS		= 19,
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT	= 20,
	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS	= 21,
	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT	= 22,
	DXGI_FORMAT_R10G10B10A2_TYPELESS	= 23,
	DXGI_FORMAT_R10G10B10A2_UNORM		= 24,
	DXGI_FORMAT_R10G10B10A2_UINT		= 25,
	DXGI_FORMAT_R11G11B10_FLOAT		= 26,
	DXGI_FORMAT_R8G8B8A8_TYPELESS		= 27,
	DXGI_FORMAT_R8G8B8A8_UNORM		= 28,
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB		= 29,
	DXGI_FORMAT_R8G8B8A8_UINT		= 30,
	DXGI_FORMAT_R8G8B8A8_SNORM		= 31,
	DXGI_FORMAT_R8G8B8A8_SINT		= 32,
	DXGI_FORMAT_R16G16_TYPELESS		= 33,
	DXGI_FORMAT_R16G16_FLOAT		= 34,
	DXGI_FORMAT_R16G16_UNORM		= 35,
	DXGI_FORMAT_R16G16_UINT			= 36,
	DXGI_FORMAT_R16G16_SNORM		= 37,
	DXGI_FORMAT_R16G16_SINT			= 38,
	DXGI_FORMAT_R32_TYPELESS		= 39,
	DXGI_FORMAT_D32_FLOAT			= 40,
	DXGI_FORMAT_R32_FLOAT			= 41,
	DXGI_FORMAT_R32_UINT			= 42,
	DXGI_FORMAT_R32_SINT			= 43,
	DXGI_FORMAT_R24G8_TYPELESS		= 44,
	DXGI_FORMAT_D24_UNORM_S8_UINT		= 45,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS	= 46,
	DXGI_FORMAT_X24_TYPELESS_G8_UINT	= 47,
	DXGI_FORMAT_R8G8_TYPELESS		= 48,
	DXGI_FORMAT_R8G8_UNORM			= 49,
	DXGI_FORMAT_R8G8_UINT			= 50,
	DXGI_FORMAT_R8G8_SNORM			= 51,
	DXGI_FORMAT_R8G8_SINT			= 52,
	DXGI_FORMAT_R16_TYPELESS		= 53,
	DXGI_FORMAT_R16_FLOAT			= 54,
	DXGI_FORMAT_D16_UNORM			= 55,
	DXGI_FORMAT_R16_UNORM			= 56,
	DXGI_FORMAT_R16_UINT			= 57,
	DXGI_FORMAT_R16_SNORM			= 58,
	DXGI_FORMAT_R16_SINT			= 59,
	DXGI_FORMAT_R8_TYPELESS			= 60,
	DXGI_FORMAT_R8_UNORM			= 61,
	DXGI_FORMAT_R8_UINT			= 62,
	DXGI_FORMAT_R8_SNORM			= 63,
	DXGI_FORMAT_R8_SINT			= 64,
	DXGI_FORMAT_A8_UNORM			= 65,
	DXGI_FORMAT_R1_UNORM			= 66,
	DXGI_FORMAT_R9G9B9E5_SHAREDEXP		= 67,
	DXGI_FORMAT_R8G8_B8G8_UNORM		= 68,
	DXGI_FORMAT_G8R8_G8B8_UNORM		= 69,
	DXGI_FORMAT_BC1_TYPELESS		= 70,
	DXGI_FORMAT_BC1_UNORM			= 71,
	DXGI_FORMAT_BC1_UNORM_SRGB		= 72,
	DXGI_FORMAT_BC2_TYPELESS		= 73,
	DXGI_FORMAT_BC2_UNORM			= 74,
	DXGI_FORMAT_BC2_UNORM_SRGB		= 75,
	DXGI_FORMAT_BC3_TYPELESS		= 76,
	DXGI_FORMAT_BC3_UNORM			= 77,
	DXGI_FORMAT_BC3_UNORM_SRGB		= 78,
	DXGI_FORMAT_BC4_TYPELESS		= 79,
	DXGI_FORMAT_BC4_UNORM			= 80,
	DXGI_FORMAT_BC4_SNORM			= 81,
	DXGI_FORMAT_BC5_TYPELESS		= 82,
	DXGI_FORMAT_BC5_UNORM			= 83,
	DXGI_FORMAT_BC5_SNORM			= 84,
	DXGI_FORMAT_B5G6R5_UNORM		= 85,
	DXGI_FORMAT_B5G5R5A1_UNORM		= 86,
	DXGI_FORMAT_B8G8R8A8_UNORM		= 87,
	DXGI_FORMAT_B8G8R8X8_UNORM		= 88,
	DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM	= 89,
	DXGI_FORMAT_B8G8R8A8_TYPELESS		= 90,
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB		= 91,
	DXGI_FORMAT_B8G8R8X8_TYPELESS		= 92,
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB		= 93,
	DXGI_FORMAT_BC6H_TYPELESS		= 94,
	DXGI_FORMAT_BC6H_UF16			= 95,
	DXGI_FORMAT_BC6H_SF16			= 96,
	DXGI_FORMAT_BC7_TYPELESS		= 97,
	DXGI_FORMAT_BC7_UNORM			= 98,
	DXGI_FORMAT_BC7_UNORM_SRGB		= 99,
	DXGI_FORMAT_AYUV			= 100,
	DXGI_FORMAT_Y410			= 101,
	DXGI_FORMAT_Y416			= 102,
	DXGI_FORMAT_NV12			= 103,
	DXGI_FORMAT_P010			= 104,
	DXGI_FORMAT_P016			= 105,
	DXGI_FORMAT_420_OPAQUE			= 106,
	DXGI_FORMAT_YUY2			= 107,
	DXGI_FORMAT_Y210			= 108,
	DXGI_FORMAT_Y216			= 109,
	DXGI_FORMAT_NV11			= 110,
	DXGI_FORMAT_AI44			= 111,
	DXGI_FORMAT_IA44			= 112,
	DXGI_FORMAT_P8				= 113,
	DXGI_FORMAT_A8P8			= 114,
	DXGI_FORMAT_B4G4R4A4_UNORM		= 115,

	// Xbox One formats.
	// Reference: https://github.com/Microsoft/DirectXTex/blob/master/DirectXTex/DirectXTexP.h
	XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT	= 116,
	XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT	= 117,
	XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT	= 118,
	XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS	= 119,
	XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT	= 120,

	// Windows 10 formats.
	// Reference: https://github.com/Microsoft/DirectXTex/blob/master/DirectXTex/DirectXTexP.h
	DXGI_FORMAT_P208			= 130,
	DXGI_FORMAT_V208			= 131,
	DXGI_FORMAT_V408			= 132,
	DXGI_FORMAT_FORCE_UINT			= 0xffffffff,

	// Additional Xbox One formats.
	// Reference: https://github.com/Microsoft/DirectXTex/blob/master/DirectXTex/DirectXTexP.h
	XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM	= 189,
	XBOX_DXGI_FORMAT_R4G4_UNORM			= 190,

	// NOTE: These are NOT actual DXGI format values.
	// These are here because DirectDrawSurface converts FourCC to DXGI_FORMAT,
	// so we need fake DXGI values for PVRTC.
	DXGI_FORMAT_FAKE_START			= 248,
	DXGI_FORMAT_FAKE_PVRTC_2bpp		= DXGI_FORMAT_FAKE_START + 1,
	DXGI_FORMAT_FAKE_PVRTC_4bpp		= DXGI_FORMAT_FAKE_START + 2,
	DXGI_FORMAT_FAKE_END			= DXGI_FORMAT_FAKE_PVRTC_4bpp,
} DXGI_FORMAT;

/**
 * DirectX 10 resource dimension enum.
 * Reference: https://msdn.microsoft.com/en-us/library/windows/desktop/bb172411(v=vs.85).aspx
 */
typedef enum {
	D3D10_RESOURCE_DIMENSION_UNKNOWN	= 0,
	D3D10_RESOURCE_DIMENSION_BUFFER		= 1,
	D3D10_RESOURCE_DIMENSION_TEXTURE1D	= 2,
	D3D10_RESOURCE_DIMENSION_TEXTURE2D	= 3,
	D3D10_RESOURCE_DIMENSION_TEXTURE3D	= 4,
} D3D10_RESOURCE_DIMENSION;

/**
 * DirectDraw Surface: DX10 header.
 * This is present after DDS_HEADER if ddspf.dwFourCC == 'DX10'.
 * Reference: https://msdn.microsoft.com/en-us/library/windows/desktop/bb943983(v=vs.85).aspx
 *
 * All fields are in little-endian.
 */
typedef struct _DDS_HEADER_DXT10 {
	DXGI_FORMAT dxgiFormat;
	D3D10_RESOURCE_DIMENSION resourceDimension;
	uint32_t miscFlag;	// See DDS_DXT10_MISC_FLAG.
	uint32_t arraySize;
	uint32_t miscFlags2;	// See DDS_DXT10_MISC_FLAGS2.
} DDS_HEADER_DXT10;
ASSERT_STRUCT(DDS_HEADER_DXT10, 5*sizeof(uint32_t));

// miscFlag
typedef enum {
	DDS_RESOURCE_MISC_TEXTURECUBE	= 0x4,
} DDS_DXT10_MISC_FLAG;

// miscFlags2
typedef enum {
	DDS_ALPHA_MODE_UNKNOWN		= 0x0,
	DDS_ALPHA_MODE_STRAIGHT		= 0x1,
	DDS_ALPHA_MODE_PREMULTIPLIED	= 0x2,
	DDS_ALPHA_MODE_OPAQUE		= 0x3,
	DDS_ALPHA_MODE_CUSTOM		= 0x4,
} DDS_DXT10_MISC_FLAGS2;

/**
 * Xbox One variant. (FourCC: 'XBOX')
 *
 * NOTE: XBOX DDS files have DDS_HEADER_DXT10
 * right before DDS_HEADER_XBOX.
 *
 * All fields are in little-endian.
 */
typedef struct _DDS_HEADER_XBOX {
	uint32_t tileMode;		// See DDS_XBOX_TILE_MODE. [TODO]
	uint32_t baseAlignment;
	uint32_t dataSize;
	uint32_t xdkVer;		// _XDK_VER
} DDS_HEADER_XBOX;
ASSERT_STRUCT(DDS_HEADER_XBOX, 4*sizeof(uint32_t));

#ifdef __cplusplus
}
#endif

#endif /* __ROMPROPERTIES_LIBRPTEXTURE_FILEFORMAT_DDS_STRUCTS_H__ */
