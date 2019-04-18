#pragma once
// generic .bin with header, flags and single data

#include "mycommon.h"
#include "MetroTypes.h"

class MetroReflectionReader;

class MetroBinArchive
{
public:
    static const size_t kHeaderNotExist = 0;
    static const size_t kHeaderDoAutoSearch = kInvalidValue;

    struct ChunkData {
        size_t  id;
        size_t  offset;
        size_t  size;
    };

    MetroBinArchive(const CharString& name, const MemStream& stream, const size_t headerSize);

    inline bool HasChunks() const {
        return !mChunks.empty();
    }

    inline const CharString& GetFileName() const {
        return mFileName;
    }

    inline bool IsHeaderExist() const {
        return mHeaderSize > 0;
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
        return this->GetOffsetBinFlags() + 1 /*bin flags*/;
    }

    // Get stream cursor to first data position (after bin flags, chunk idx, chunk size)
    inline size_t GetOffsetFirstDataBegin() const {
        if (this->HasChunks()) {
            return this->GetFirstChunk().offset;
        }

        return this->GetOffsetBinFlags() + 1 /*bin flags*/;
    }

    inline size_t GetNumChunks() const {
        return mChunks.size();
    }

    inline const ChunkData& GetChunk(size_t chunkIdx) {
        return mChunks[chunkIdx];
    }

    inline const ChunkData& GetFirstChunk() const {
        return mChunks.front();
    }

    inline const ChunkData& GetLastChunk() const {
        return mChunks.back();
    }


    MetroReflectionReader   ReflectionReader() const;

private:
    void                    ReadStringsTable();

private:
    CharString              mFileName;
    MemStream               mFileStream;    // <!> cursor of mFileStream must be always at 0
    size_t                  mHeaderSize;    // size of any data before .bin flags
    uint8_t                 mBinFlags;      // MetroBinFlags

    MyArray<ChunkData>      mChunks;
    StringsTable            mSTable;
};

