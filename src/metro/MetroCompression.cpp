#include "MetroCompression.h"

#define LZ4_DISABLE_DEPRECATE_WARNINGS
#include "lz4.h"
#include "lz4hc.h"


size_t MetroCompression::DecompressStream(const void* compressedData, const size_t compressedSize, void* uncompressedData, const size_t uncompressedSize) {
    size_t result = 0;

    MemStream stream(compressedData, compressedSize);
    char* dst = rcast<char*>(uncompressedData);

    size_t outCursor = 0;
    while (!stream.Ended()) {
        const size_t blockSize = stream.ReadTyped<uint32_t>();
        const size_t blockUncompressedSize = stream.ReadTyped<uint32_t>();

        const char* src = rcast<const char*>(stream.GetDataAtCursor());

        const int nbRead = LZ4_decompress_fast_withPrefix64k(src, dst + outCursor, scast<int>(blockUncompressedSize));
        const int nbCompressed = scast<int>(blockSize - 8);
        if (nbRead < scast<int>(nbCompressed)) {
            // ooops, error :(
            return 0;
        }

        outCursor += blockUncompressedSize;

        stream.SkipBytes(nbCompressed);
    }

    result = outCursor;

    return result;
}

size_t MetroCompression::DecompressBlob(const void* compressedData, const size_t compressedSize, void* uncompressedData, const size_t uncompressedSize) {
    const int result = LZ4_decompress_safe(rcast<const char*>(compressedData), rcast<char*>(uncompressedData), scast<int>(compressedSize), scast<int>(uncompressedSize));
    return (result > 0 ? scast<size_t>(result) : 0);
}

size_t MetroCompression::CompressStream(const void* data, const size_t dataLength, BytesArray& compressed) {
    const size_t kMaxBlockSize = 0x30000;

    const size_t maxCompressedBlock = scast<size_t>(LZ4_compressBound(scast<int>(kMaxBlockSize)));
    BytesArray temp(maxCompressedBlock);
    char* dst = rcast<char*>(temp.data());

    size_t result = 0, bytesLeft = dataLength;

    MemStream inStream(data, dataLength);
    MemWriteStream outStream;

    while (bytesLeft) {
        const size_t blockSize = std::min<size_t>(kMaxBlockSize, bytesLeft);
        const char* src = rcast<const char*>(inStream.GetDataAtCursor());

        size_t packedSize = 0;

        const int lz4Result = LZ4_compress_HC(src, dst, scast<int>(blockSize), scast<int>(maxCompressedBlock), LZ4HC_CLEVEL_MAX);
        if (lz4Result <= 0) {
            // ooops, error :(
            return 0;
        } else {
            packedSize = scast<size_t>(lz4Result);
        }

        outStream.Write(scast<uint32_t>(packedSize + 8));
        outStream.Write(scast<uint32_t>(blockSize));
        outStream.Write(dst, packedSize);

        bytesLeft -= blockSize;
        result += blockSize;
    }

    outStream.buffer.swap(compressed);

    return result;
}

size_t MetroCompression::CompressBlob(const void* data, const size_t dataLength, BytesArray& compressed) {
    size_t result = 0;

    const size_t maxCompressedBlock = scast<size_t>(LZ4_compressBound(scast<int>(dataLength)));
    compressed.resize(maxCompressedBlock);

    const char* src = rcast<const char*>(data);
    char* dst = rcast<char*>(compressed.data());

    const int lz4Result = LZ4_compress_HC(src, dst, scast<int>(dataLength), scast<int>(maxCompressedBlock), LZ4HC_CLEVEL_MAX);
    if (lz4Result <= 0) {
        compressed.resize(0);
    } else {
        result = scast<size_t>(lz4Result);
        compressed.resize(result);
    }

    return result;
}
