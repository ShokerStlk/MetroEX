#pragma once
#include "mycommon.h"

struct MetroCompression {
    enum {
        Type_Unknown    = 0,
        Type_LZ4        = 1
    };

    static size_t DecompressStream(const void* compressedData, const size_t compressedSize, void* uncompressedData, const size_t uncompressedSize);
    static size_t DecompressBlob(const void* compressedData, const size_t compressedSize, void* uncompressedData, const size_t uncompressedSize);

    static size_t CompressStream(const void* data, const size_t dataLength, BytesArray& compressed);
    static size_t CompressBlob(const void* data, const size_t dataLength, BytesArray& compressed);
};
