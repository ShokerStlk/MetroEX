#pragma once
// generic .bin with header, flags and single data

#include "mycommon.h"
#include "MetroTypes.h"
#include "MetroBinIArchive.h"

class MetroReflectionReader;

class MetroBinArchive : public IMetroBinArchive
{
public:
    static const size_t kHeaderNotExist = 0;
    static const size_t kHeaderDoAutoSearch = ~0;

    struct ChunkData {
    private:
        uint32_t    index;
        size_t      offset;
        size_t      size;
        bool        isStringTable;

    public:
        ChunkData(size_t _offset, uint32_t _index, size_t _size) : isStringTable(false) {
            offset  = _offset;
            index   = _index;
            size    = _size;
        }

        inline size_t GetChunkStartOffset() const {
            return offset;
        }

        inline size_t GetChunkEndOffset() const {
            return GetChunkDataOffset() + size;
        }

        inline size_t GetChunkDataOffset() const {
            return GetChunkStartOffset() + 0x4 /*chunk id*/ + 0x4 /*chunk size*/;
        }

        inline size_t GetTotalChunkSize() const {
            return GetChunkEndOffset() - GetChunkStartOffset();
        }

        inline uint32_t GetChunkIdx() const {
            return index;
        }

        inline size_t GetChunkOffset() const {
            return offset;
        }

        inline size_t GetChunkSize() const {
            return size;
        }

        inline bool IsStringTable() const {
            return isStringTable;
        }

        inline void SetIsStringTable(bool value) {
            isStringTable = value;
        }
    };

    MetroBinArchive(const CharString& name, const MemStream& _binStream, size_t _headerSize);

    virtual bool IsBinArchive() { return true; };

    inline virtual const MemStream& GetRawStream() const {
        return mFileStream;
    }

    inline virtual MemStream& GetRawDangerStream() { // use carefully
        return mFileStream;
    }

    inline virtual MemStream GetRawStreamCopy() {
        return MemStream(mFileStream);
    }

    inline virtual MemStream GetRawStreamCopy() const {
        return MemStream(mFileStream);
    }

    inline bool HasChunks() const {
        if (HasRefStrings()) {
            return true;
        }

        return false;
    }

    inline bool HasRefStrings() const {
        return TestBit(mBinFlags, MetroBinFlags::RefStrings);
    }

    inline bool HasDebugInfo() const {
        return TestBit(mBinFlags, MetroBinFlags::HasDebugInfo);
    }

    inline virtual const CharString& GetFileName() const override {
        return mFileName;
    }

    inline bool IsHeaderExist() const {
        return mIsHeaderExist;
    }

    inline size_t GetHeaderSize() const {
        return mHeaderSize;
    }

    inline uint8_t GetFlags() const {
        return mBinFlags;
    }

    inline constexpr void SetFlag(uint8_t flag, bool value) {
        if (value == true) {
            SetBit(mBinFlags, flag);
        }
        else {
            RemoveBit(mBinFlags, flag);
        }
    }

    // Get stream cursor to bin flags position
    inline size_t GetOffsetBinFlags() const {
        return mHeaderSize;
    }

    // Get stream cursor to first chunk position (after bin flags, before chunk idx, chunk size)
    inline size_t GetOffsetFirstChunkBegin() const {
        assert(HasChunks());
        return GetOffsetBinFlags() + 0x1 /*bin flags*/;
    }

    // Get stream cursor to first data position (after bin flags, chunk idx, chunk size)
    inline size_t GetOffsetFirstDataBegin() const {
        if (HasChunks()) {
            return GetOffsetFirstChunkBegin() + 0x4 /*chunk id*/ + 0x4 /*chunk size*/;
        }

        return GetOffsetBinFlags() + 0x1 /*bin flags*/;
    }

    inline size_t GetChunkCount() const {
        return mChunks.size();
    }

    inline ChunkData& GetChunkByNum(size_t chunkIdx) { // chunkIdx from 1 to ...
        assert(chunkIdx > 0 && chunkIdx <= GetChunkCount());
        return mChunks[chunkIdx - 1];
    }

    inline const ChunkData& GetChunkByNum(size_t chunkIdx) const { // chunkIdx from 1 to ...
        assert(chunkIdx > 0 && chunkIdx <= GetChunkCount());
        return mChunks[chunkIdx - 1];
    }

    inline ChunkData& GetFirstChunk() {
        assert(HasChunks());
        return GetChunkByNum(1);
    }

    inline const ChunkData& GetFirstChunk() const {
        assert(HasChunks());
        return GetChunkByNum(1);
    }

    inline ChunkData& GetLastChunk() {
        assert(HasChunks());
        size_t cnt = GetChunkCount();
        return GetChunkByNum(cnt);
    }

    inline const ChunkData& GetLastChunk() const {
        assert(HasChunks());
        size_t cnt = GetChunkCount();
        return GetChunkByNum(cnt);
    }

    MyArray<CharString>     ReadStringTable() const;
    MetroReflectionReader   ReturnReflectionReader(size_t offset = 0) const;

private:
    CharString  mFileName;
    MemStream   mFileStream; // <!> cursor of mFileStream must be always at 0
    bool        mIsHeaderExist;
    size_t      mHeaderSize; // size of any data before .bin flags
    uint8_t     mBinFlags; // MetroBinFlags

    MyArray<ChunkData> mChunks;
};

