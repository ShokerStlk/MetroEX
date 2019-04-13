#include "MetroBinArchive.h"
#include "MetroReflection.h"

MetroBinArchive::MetroBinArchive(const CharString& name, const MemStream& _binStream, size_t _headerSize) {
    // Get raw memory stream
    mFileName = name;
    mFileStream = MemStream(_binStream);

    // Search for header
    mIsHeaderExist = false;
    mHeaderSize = 0;

    if (_headerSize != kHeaderNotExist) {
        if (_headerSize == kHeaderDoAutoSearch) {
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
            mHeaderSize = _headerSize;
        }
    }

    // Read .bin flags
    mFileStream.SetCursor(GetOffsetBinFlags());
    mBinFlags = mFileStream.ReadTyped<uint8_t>();

    // Read chunks
    if (this->HasChunks()) {
        while (!mFileStream.Ended()) {
            mChunks.push_back({
                mFileStream.GetCursor(),
                mFileStream.ReadTyped<uint32_t>(),
                mFileStream.ReadTyped<uint32_t>()
            });

            mFileStream.SkipBytes(mChunks.back().GetChunkSize());
        }

        // Mark RefStrings chunk
        if (this->HasRefStrings()) {
            ChunkData& lastChunkData = this->GetLastChunk();
            lastChunkData.SetIsStringTable(true);
        }
    }

    // Restore cursor position
    mFileStream.SetCursor(0);
}

StringArray MetroBinArchive::ReadStringTable() const {
    StringArray result;

    if (this->HasRefStrings()) {
        MemStream& rawStream = GetRawStreamCopy();

        // Read strings
        const ChunkData& lastChunk = GetLastChunk();
        assert(lastChunk.IsStringTable());

        rawStream.SetCursor(lastChunk.GetChunkDataOffset());

        const size_t numStrings = rawStream.ReadTyped<uint32_t>();
        result.resize(numStrings);

        for (CharString& s : result) {
            s = rawStream.ReadStringZ();
        }
    }

    return result;
}

MetroReflectionReader MetroBinArchive::ReturnReflectionReader(const size_t offset) const {
    MemStream rawStream = this->GetRawStreamCopy();
    rawStream.SetCursor(offset);

    MetroReflectionReader reader = MetroReflectionReader(rawStream);
    reader.SetOptions(this->HasDebugInfo(), this->HasRefStrings());

    return std::move(reader);
}
