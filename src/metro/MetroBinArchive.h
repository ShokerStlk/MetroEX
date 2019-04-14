#pragma once
// generic .bin with header, flags and single data

#include "mycommon.h"
#include "MetroTypes.h"
#include "MetroBinIArchive.h"

class MetroReflectionReader;

class MetroBinArchive : IMetroBinArchive
{
public:
    static const size_t kHeaderNotExist = 0;
    static const size_t kHeaderDoAutoSearch = kInvalidValue;

    struct ChunkData {
    private:
        size_t  index;
        size_t  offset;
        size_t  size;
        bool    isStringTable;

    public:
        ChunkData(const size_t _offset, const size_t _index, const size_t _size)
            : index(_index)
            , offset(_offset)
            , size(_size)
            , isStringTable(false) {
        }

        inline size_t GetChunkStartOffset() const {
            return offset;
        }

        inline size_t GetChunkEndOffset() const {
            return this->GetChunkDataOffset() + size;
        }

        inline size_t GetChunkDataOffset() const {
            return this->GetChunkStartOffset() + 0x4 /*chunk id*/ + 0x4 /*chunk size*/;
        }

        inline size_t GetTotalChunkSize() const {
            return this->GetChunkEndOffset() - this->GetChunkStartOffset();
        }

        inline size_t GetChunkIdx() const {
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

    bool IsBinArchive() override { return true; };

    virtual const MemStream& GetRawStream() const override {
        return mFileStream;
    }

    virtual MemStream& GetRawDangerStream() override { // use carefully
        return mFileStream;
    }

    virtual MemStream GetRawStreamCopy() override {
        return MemStream(mFileStream);
    }

    virtual MemStream GetRawStreamCopy() const override {
        return MemStream(mFileStream);
    }

    inline bool HasChunks() const {
        return this->HasRefStrings();
    }

    inline bool HasRefStrings() const {
        return TestBit(mBinFlags, MetroBinFlags::RefStrings);
    }

    inline bool HasDebugInfo() const {
        return TestBit(mBinFlags, MetroBinFlags::HasDebugInfo);
    }

    inline const CharString& GetFileName() const {
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

    inline void SetFlag(const uint8_t flag, const bool toSet) {
        mBinFlags = toSet ? SetBit(mBinFlags, flag) : RemoveBit(mBinFlags, flag);
    }

    // Get stream cursor to bin flags position
    inline size_t GetOffsetBinFlags() const {
        return mHeaderSize;
    }

    // Get stream cursor to first chunk position (after bin flags, before chunk idx, chunk size)
    inline size_t GetOffsetFirstChunkBegin() const {
        assert(this->HasChunks());
        return this->GetOffsetBinFlags() + 0x1 /*bin flags*/;
    }

    // Get stream cursor to first data position (after bin flags, chunk idx, chunk size)
    inline size_t GetOffsetFirstDataBegin() const {
        if (this->HasChunks()) {
            return this->GetOffsetFirstChunkBegin() + 0x4 /*chunk id*/ + 0x4 /*chunk size*/;
        }

        return this->GetOffsetBinFlags() + 0x1 /*bin flags*/;
    }

    inline size_t GetChunkCount() const {
        return mChunks.size();
    }

    inline ChunkData& GetChunkByNum(size_t chunkIdx) { // chunkIdx from 1 to ...
        assert(chunkIdx > 0 && chunkIdx <= this->GetChunkCount());
        return mChunks[chunkIdx - 1];
    }

    inline const ChunkData& GetChunkByNum(size_t chunkIdx) const { // chunkIdx from 1 to ...
        assert(chunkIdx > 0 && chunkIdx <= this->GetChunkCount());
        return mChunks[chunkIdx - 1];
    }

    inline ChunkData& GetFirstChunk() {
        assert(this->HasChunks());
        return this->GetChunkByNum(1);
    }

    inline const ChunkData& GetFirstChunk() const {
        assert(this->HasChunks());
        return this->GetChunkByNum(1);
    }

    inline ChunkData& GetLastChunk() {
        assert(this->HasChunks());
        const size_t cnt = this->GetChunkCount();
        return this->GetChunkByNum(cnt);
    }

    inline const ChunkData& GetLastChunk() const {
        assert(this->HasChunks());
        const size_t cnt = this->GetChunkCount();
        return this->GetChunkByNum(cnt);
    }

    StringArray             ReadStringTable() const;
    MetroReflectionReader   ReturnReflectionReader(const size_t offset = 0) const;

private:
    CharString  mFileName;
    MemStream   mFileStream; // <!> cursor of mFileStream must be always at 0
    bool        mIsHeaderExist;
    size_t      mHeaderSize; // size of any data before .bin flags
    uint8_t     mBinFlags; // MetroBinFlags

    MyArray<ChunkData> mChunks;
};

