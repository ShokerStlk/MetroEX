#pragma once

#include "dds_defs.h"

void DDS_DecompressBC1(const void* inputBlocks, void* outPixels, const size_t width, const size_t height);
void DDS_DecompressBC2(const void* inputBlocks, void* outPixels, const size_t width, const size_t height);
void DDS_DecompressBC3(const void* inputBlocks, void* outPixels, const size_t width, const size_t height);
//void DDS_DecompressBC4(const void* inputBlocks, void* outPixels, const size_t width, const size_t height);
//void DDS_DecompressBC5(const void* inputBlocks, void* outPixels, const size_t width, const size_t height);
//void DDS_DecompressBC6H(const void* inputBlocks, void* outPixels, const size_t width, const size_t height);
void DDS_DecompressBC7(const void* inputBlocks, void* outPixels, const size_t width, const size_t height);

void DDS_CompressBC3(const void* inputRGBA, void* outBlocks, const size_t width, const size_t height);

size_t DDS_GetCompressedSizeBC7(const size_t width, const size_t height, const size_t numMips);
void   DDS_MakeDX10Headers(DDSURFACEDESC2& desc, DDS_HEADER_DXT10& dx10hdr, const size_t w, const size_t h, const size_t numMips, const bool isCube = false);
void   DDS_MakeDX9Header(DDSURFACEDESC2& desc, const size_t w, const size_t h, const size_t numMips);
