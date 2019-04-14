#include "mycommon.h"
#include "dds_utils.h"
#include "bc7decomp.h"
#include "bc7enc16.h"
#define STB_DXT_IMPLEMENTATION
#include "stb_dxt.h"

void DDS_DecodeColorBlock(uint8_t* dest, const size_t w, const size_t h, const size_t xOff, const size_t yOff, const bool isBC3, const uint8_t* src) {
    uint8_t colors[4][3];

    const uint16_t c0 = rcast<const uint16_t*>(src)[0];
    const uint16_t c1 = rcast<const uint16_t*>(src)[1];

    // Extract the two stored colors
    colors[0][0] = ((c0 >> 11) & 0x1F) << 3;
    colors[0][1] = ((c0 >> 5) & 0x3F) << 2;
    colors[0][2] = (c0 & 0x1F) << 3;

    colors[1][0] = ((c1 >> 11) & 0x1F) << 3;
    colors[1][1] = ((c1 >> 5) & 0x3F) << 2;
    colors[1][2] = (c1 & 0x1F) << 3;

    // compute the other two colors
    if (c0 > c1 || isBC3) {
        for (size_t i = 0; i < 3; ++i) {
            colors[2][i] = (2 * colors[0][i] + colors[1][i] + 1) / 3;
            colors[3][i] = (colors[0][i] + 2 * colors[1][i] + 1) / 3;
        }
    } else {
        for (size_t i = 0; i < 3; ++i) {
            colors[2][i] = (colors[0][i] + colors[1][i] + 1) >> 1;
            colors[3][i] = 0;
        }
    }

    src += 4;
    for (size_t y = 0; y < h; ++y) {
        uint8_t* dst = dest + yOff * y;
        uint32_t indexes = src[y];
        for (size_t x = 0; x < w; ++x) {
            const uint32_t index = indexes & 0x3;
            dst[0] = colors[index][0];
            dst[1] = colors[index][1];
            dst[2] = colors[index][2];
            indexes >>= 2;

            dst += xOff;
        }
    }
}

void DDS_DecodeBC2AlphaBlock(uint8_t* dest, const size_t w, const size_t h, const size_t xOff, const size_t yOff, const uint8_t* src) {
    for (size_t y = 0; y < h; ++y) {
        uint8_t* dst = dest + yOff * y;
        uint32_t alpha = rcast<const uint32_t*>(src)[y];
        for (size_t x = 0; x < w; ++x) {
            *dst = (alpha & 0xF) * 17;
            alpha >>= 4;
            dst += xOff;
        }
    }
}

void DDS_DecodeBC3AlphaBlock(uint8_t* dest, const size_t w, const size_t h, const size_t xOff, const size_t yOff, const uint8_t* src) {
    const uint8_t a0 = src[0];
    const uint8_t a1 = src[1];
    uint64_t alpha = *rcast<const uint64_t*>(src) >> 16;

    for (size_t y = 0; y < h; ++y) {
        uint8_t* dst = dest + yOff * y;
        for (size_t x = 0; x < w; x++) {
            const uint32_t k = scast<uint32_t>(alpha & 0x7);
            if (0 == k) {
                *dst = a0;
            } else if (1 == k) {
                *dst = a1;
            } else if (a0 > a1) {
                *dst = ((8 - k) * a0 + (k - 1) * a1) / 7;
            } else if (k >= 6) {
                *dst = (k == 6) ? 0 : 255;
            } else {
                *dst = ((6 - k) * a0 + (k - 1) * a1) / 5;
            }

            alpha >>= 3;
            dst += xOff;
        }
        if (w < 4) {
            alpha >>= (3 * (4 - w));
        }
    }
}



void DDS_DecompressBC1(const void* inputBlocks, void* outPixels, const size_t width, const size_t height) {
    const size_t sx = (width < 4) ? width : 4;
    const size_t sy = (height < 4) ? height : 4;

    const uint8_t* src = rcast<const uint8_t*>(inputBlocks);
    uint8_t* dest = rcast<uint8_t*>(outPixels);

    for (size_t y = 0; y < height; y += 4) {
        for (size_t x = 0; x < width; x += 4) {
            uint8_t* dst = dest + (y * width + x) * 3;
            DDS_DecodeColorBlock(dst, sx, sy, 3, width * 3, false, src);
            src += 8;
        }
    }
}

void DDS_DecompressBC2(const void* inputBlocks, void* outPixels, const size_t width, const size_t height) {
    const size_t sx = (width < 4) ? width : 4;
    const size_t sy = (height < 4) ? height : 4;

    const uint8_t* src = rcast<const uint8_t*>(inputBlocks);
    uint8_t* dest = rcast<uint8_t*>(outPixels);

    for (size_t y = 0; y < height; y += 4) {
        for (size_t x = 0; x < width; x += 4) {
            uint8_t* dst = dest + (y * width + x) * 4;
            
            DDS_DecodeBC2AlphaBlock(dst + 3, sx, sy, 4, width * 4, src);
            src += 8;

            DDS_DecodeColorBlock(dst, sx, sy, 4, width * 4, false, src);
            src += 8;
        }
    }
}

void DDS_DecompressBC3(const void* inputBlocks, void* outPixels, const size_t width, const size_t height) {
    const size_t sx = (width < 4) ? width : 4;
    const size_t sy = (height < 4) ? height : 4;

    const uint8_t* src = rcast<const uint8_t*>(inputBlocks);
    uint8_t* dest = rcast<uint8_t*>(outPixels);

    for (size_t y = 0; y < height; y += 4) {
        for (size_t x = 0; x < width; x += 4) {
            uint8_t* dst = dest + (y * width + x) * 4;

            DDS_DecodeBC3AlphaBlock(dst + 3, sx, sy, 4, width * 4, src);
            src += 8;

            DDS_DecodeColorBlock(dst, sx, sy, 4, width * 4, true, src);
            src += 8;
        }
    }
}

void DDS_DecompressBC7(const void* inputBlocks, void* outPixels, const size_t width, const size_t height) {
    const size_t sx = (width < 4) ? width : 4;
    const size_t sy = (height < 4) ? height : 4;

    const uint8_t* src = rcast<const uint8_t*>(inputBlocks);
    uint8_t* dest = rcast<uint8_t*>(outPixels);

    uint8_t pixelsBlock[16 * 4] = { 0 };

    for (size_t y = 0; y < height; y += 4) {
        for (size_t x = 0; x < width; x += 4) {
            const uint32_t modeMask = ~0u; // all of them
            detexDecompressBlockBPTC(src, modeMask, 0, pixelsBlock);
            src += 16;

            uint8_t* dst = dest + (y * width + x) * 4;
            for (size_t i = 0; i < 4; ++i) {
                std::memcpy(dst, &pixelsBlock[i * 16], 16);
                dst += (width * 4);
            }
        }
    }
}

void DDS_CompressBC3(const void* inputRGBA, void* outBlocks, const size_t width, const size_t height) {
    const size_t sx = (width < 4) ? width : 4;
    const size_t sy = (height < 4) ? height : 4;

    const uint8_t* srcPtr = rcast<const uint8_t*>(inputRGBA);
    uint8_t* dst = rcast<uint8_t*>(outBlocks);

    uint8_t pixelsBlock[16 * 4] = { 0 };

    for (size_t y = 0; y < height; y += 4) {
        for (size_t x = 0; x < width; x += 4) {
            const uint8_t* src = srcPtr + (y * width + x) * 4;
            for (size_t i = 0; i < 4; ++i) {
                std::memcpy(&pixelsBlock[i * 16], src, 16);
                src += (width * 4);
            }

            stb_compress_dxt_block(dst, pixelsBlock, 1, STB_DXT_HIGHQUAL);

            dst += 16;
        }
    }
}

void DDS_CompressBC7(const void* inputRGBA, void* outBlocks, const size_t width, const size_t height) {
    static bool sNeedInitBc7Comp = true;

    if (sNeedInitBc7Comp) {
        bc7enc16_compress_block_init();
        sNeedInitBc7Comp = false;
    }

    bc7enc16_compress_block_params params;
    bc7enc16_compress_block_params_init(&params);
    bc7enc16_compress_block_params_init_perceptual_weights(&params);

    const size_t sx = (width < 4) ? width : 4;
    const size_t sy = (height < 4) ? height : 4;

    const uint8_t* srcPtr = rcast<const uint8_t*>(inputRGBA);
    uint8_t* dst = rcast<uint8_t*>(outBlocks);

    uint8_t pixelsBlock[16 * 4] = { 0 };

    for (size_t y = 0; y < height; y += 4) {
        for (size_t x = 0; x < width; x += 4) {
            const uint8_t* src = srcPtr + (y * width + x) * 4;
            for (size_t i = 0; i < 4; ++i) {
                std::memcpy(&pixelsBlock[i * 16], src, 16);
                src += (width * 4);
            }

            bc7enc16_compress_block(dst, pixelsBlock, &params);

            dst += 16;
        }
    }
}


size_t DDS_GetCompressedSizeBC7(const size_t width, const size_t height, const size_t numMips) {
    size_t w = width;
    size_t h = height;
    size_t result = 0;

    for (size_t i = 0; i < numMips; ++i) {
        w = std::max<size_t>(4, w);
        h = std::max<size_t>(4, h);

        result += ((w / 4) * (h / 4)) * 16;

        w /= 2;
        h /= 2;
    }

    return result;
}


void DDS_MakeDX10Headers(DDSURFACEDESC2& desc, DDS_HEADER_DXT10& dx10hdr, const size_t w, const size_t h, const size_t numMips, const bool isCube) {
    memset(&desc, 0, sizeof(desc));
    memset(&dx10hdr, 0, sizeof(dx10hdr));

    desc.dwSize = sizeof(DDSURFACEDESC2);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    desc.dwWidth = scast<uint32_t>(w);
    desc.dwHeight = scast<uint32_t>(h);
    desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    desc.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
    desc.ddpfPixelFormat.dwFourCC = PIXEL_FMT_FOURCC('D', 'X', '1', '0');
    desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;

    if (numMips > 1) {
        desc.dwFlags |= DDSD_MIPMAPCOUNT;
        desc.ddsCaps.dwCaps |= DDSCAPS_COMPLEX;
        desc.dwMipMapCount = scast<uint32_t>(numMips);
    }

    if (isCube) {
        desc.ddsCaps.dwCaps2 = DDSCAPS2_CUBEMAP |
                               DDSCAPS2_CUBEMAP_POSITIVEX |
                               DDSCAPS2_CUBEMAP_NEGATIVEX |
                               DDSCAPS2_CUBEMAP_POSITIVEY |
                               DDSCAPS2_CUBEMAP_NEGATIVEY |
                               DDSCAPS2_CUBEMAP_POSITIVEZ |
                               DDSCAPS2_CUBEMAP_NEGATIVEZ;
    }

    dx10hdr.dxgiFormat = isCube ? DXGI_FORMAT_BC6H_UF16 : DXGI_FORMAT_BC7_UNORM;
    dx10hdr.resourceDimension = D3D10_RESOURCE_DIMENSION_TEXTURE2D;
    dx10hdr.miscFlag = isCube ? 4 : 0;
    dx10hdr.arraySize = 1;
    dx10hdr.miscFlags2 = 0;
}

void DDS_MakeDX9Header(DDSURFACEDESC2& desc, const size_t w, const size_t h, const size_t numMips) {
    memset(&desc, 0, sizeof(desc));

    desc.dwSize = sizeof(DDSURFACEDESC2);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    desc.dwWidth = scast<uint32_t>(w);
    desc.dwHeight = scast<uint32_t>(h);
    desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    desc.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
    desc.ddpfPixelFormat.dwFourCC = PIXEL_FMT_DXT5;
    desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;

    if (numMips) {
        desc.dwFlags |= DDSD_MIPMAPCOUNT;
        desc.ddsCaps.dwCaps |= DDSCAPS_MIPMAP;
        desc.dwMipMapCount = scast<uint32_t>(numMips);
    }
}
