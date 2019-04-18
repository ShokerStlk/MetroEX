#include "MetroBinArchive.h"
#include "MetroReflection.h"

MetroBinArchive::MetroBinArchive(const CharString& name, const MemStream& stream, const size_t headerSize) {
    // Get raw memory stream
    mFileName = name;
    mFileStream = stream;

    // Search for header
    mHeaderSize = 0;

    if (headerSize != kHeaderNotExist) {
        if (headerSize == kHeaderDoAutoSearch) {
            // Try to autosearch first chunk location, to get flags and header size
            // TODO: tricky and unreliable method, may fail
            size_t firstChunkPos = 0;

            // Try to search for .bin flags
            size_t _maxOffsetToSeek = 0x10;

            for (size_t offs = 0; offs <= _maxOffsetToSeek; offs++) {
                mFileStream.SetCursor(offs);
                uint32_t value = mFileStream.ReadTyped<uint32_t>();
                if (value == 1) { // looking for chunkIdx == 1
                    firstChunkPos = offs;
                    break;
                }
            }

            // Restore cursor position
            mFileStream.SetCursor(0);

            // Set header size
            mHeaderSize = firstChunkPos - 0x1 /*bin flags*/;
        } else {
            mHeaderSize = headerSize;
        }
    }

    // Read .bin flags
    mFileStream.SetCursor(GetOffsetBinFlags());
    mBinFlags = mFileStream.ReadTyped<uint8_t>();

    // Read chunks
    if (TestBit(mBinFlags, MetroReflectionFlags::StringsTable)) {
        while (!mFileStream.Ended()) {
            const size_t chunkId = mFileStream.ReadTyped<uint32_t>();
            const size_t chunkSize = mFileStream.ReadTyped<uint32_t>();
            mChunks.push_back({
                chunkId,
                mFileStream.GetCursor(),
                chunkSize
            });

            mFileStream.SkipBytes(chunkSize);
        }

        // read strings table chunk
        if (this->GetNumChunks() == 2) {
            this->ReadStringsTable();
        }
    }

    // Restore cursor position
    mFileStream.SetCursor(0);
}

MetroReflectionReader MetroBinArchive::ReflectionReader() const {
    MetroReflectionReader result;

    if (this->HasChunks()) {
        const ChunkData& chunk = this->GetFirstChunk();

        result = MetroReflectionReader(mFileStream.Substream(chunk.offset, chunk.size), mBinFlags);
    } else {
        result = MetroReflectionReader(mFileStream.Substream(1, mFileStream.Length() - 1), mBinFlags);
    }

    if (!mSTable.data.empty()) {
        result.SetSTable(&mSTable);
    }

    return std::move(result);
}

void MetroBinArchive::ReadStringsTable() {
    const ChunkData& stableChunk = this->GetLastChunk();

    MemStream stream = mFileStream.Substream(stableChunk.offset, stableChunk.size);
    const size_t numStrings = stream.ReadTyped<uint32_t>();

    const size_t dataSize = stream.Remains();
    mSTable.data.resize(dataSize);
    stream.ReadToBuffer(mSTable.data.data(), dataSize);
    mSTable.strings.resize(numStrings);

    const char* s = mSTable.data.data();
    for (size_t i = 0; i < numStrings; ++i) {
        mSTable.strings[i] = s;
        while (*s++);
    }
}
