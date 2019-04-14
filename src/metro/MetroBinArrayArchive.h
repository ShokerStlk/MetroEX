#pragma once
// .bin with fixed array of another (non-fixed size) .bin-s

#include "mycommon.h"
#include "MetroBinIArchive.h"

class MetroBinArchive;

class MetroBinArrayArchive : IMetroBinArchive
{
public:
    static const uint16_t kFileVersionNotExist = ~0;

    struct ChunkData {
    private:
        uint32_t         index;
        size_t           offset;
        size_t           size;
        CharString       binName;
        size_t           binSize;

        std::shared_ptr<MetroBinArchive> binArchive;

    public:
        ChunkData(const MemStream& fileStream);

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

        inline const CharString& GetBinName() const {
            return binName;
        }

        inline size_t GetBinSize() const {
            return binSize;
        }

        inline MetroBinArchive& GetBinArchive() {
            return *binArchive;
        }

        inline const MetroBinArchive& GetBinArchive() const {
            return *binArchive;
        }
    };

    MetroBinArrayArchive(const CharString& name, const MemStream& _binStream, const uint32_t _headerAlias);

    virtual bool IsBinArrayArchive() override { return true; };

    inline ChunkData& GetChunkByIdx(size_t idx) {
        assert(idx < this->GetBinCnt());
        return mChunks[idx];
    }

    inline const ChunkData& GetChunkByIdx(size_t idx) const {
        assert(idx < this->GetBinCnt());
        return mChunks[idx];
    }

    inline MetroBinArchive& GetBinByIdx(size_t idx) {
        return this->GetChunkByIdx(idx).GetBinArchive();
    }

    inline const MetroBinArchive& GetBinByIdx(size_t idx) const {
        assert(idx < this->GetBinCnt());
        return this->GetChunkByIdx(idx).GetBinArchive();
    }

    inline size_t GetBinCnt() const {
        return mChunks.size();
    }

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

    inline const CharString& GetFileName() const {
        return mFileName;
    }

    inline uint16_t GetHeaderVersion() const {
        return mHeaderVersion;
    }

    inline const CharString& GetHeaderAlias() const {
        return mHeaderAlias;
    }

    inline bool IsHeaderExist() const {
        return mIsHeaderExist;
    }

private:
    CharString  mFileName;
    MemStream   mFileStream; // <!> cursor of mFileStream must be always at 0
    uint16_t    mHeaderVersion;
    CharString  mHeaderAlias;
    bool        mIsHeaderExist;

    MyArray<ChunkData> mChunks; // Array of chunks with .bin-s inside
};

