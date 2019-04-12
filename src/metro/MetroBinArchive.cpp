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
        }
        else {
            mHeaderSize = _headerSize;
        }
    }

    // Read .bin flags
    mFileStream.SetCursor(GetOffsetBinFlags());
    mBinFlags = mFileStream.ReadTyped<uint8_t>();

    // Read chunks
    if (HasChunks()) {
        while (!mFileStream.Ended()) {
            size_t      chunkOffset = mFileStream.GetCursor();
            uint32_t    chunkIndex  = mFileStream.ReadTyped<uint32_t>();
            uint32_t    chunkSize   = mFileStream.ReadTyped<uint32_t>();
            ChunkData&  chunkData   = mChunks.emplace_back(chunkOffset, chunkIndex, chunkSize);

            mFileStream.SkipBytes(chunkData.GetChunkSize());
        }

        // Mark RefStrings chunk
        if (HasRefStrings()) {
            ChunkData& lastChunkData = GetLastChunk();
            lastChunkData.SetIsStringTable(true);
        }
    }

    // Restore cursor position
    mFileStream.SetCursor(0);
}

MyArray<CharString> MetroBinArchive::ReadStringTable() const {
    MyArray<CharString> stringsTablel(0);
    stringsTablel.clear();

    if (HasRefStrings() == false) {
        return stringsTablel;
    }

    MemStream& rawStream = GetRawStreamCopy();

    // Read strings
    const ChunkData& lastChunk = GetLastChunk();
    assert(lastChunk.IsStringTable());

    rawStream.SetCursor(lastChunk.GetChunkDataOffset());

    const size_t numStrings = rawStream.ReadTyped<uint32_t>();
    stringsTablel.reserve(numStrings);

    for (size_t i = 0; i < numStrings; i++) {
        CharString strVal = rawStream.ReadStringZ();
        stringsTablel.push_back(strVal);
    }

    // Return value
    return stringsTablel;
}

MetroReflectionReader MetroBinArchive::ReturnReflectionReader(size_t offset) const {
    MemStream& rawStream = GetRawStreamCopy();
    rawStream.SetCursor(offset);

    MetroReflectionReader reader = MetroReflectionReader(rawStream);
    reader.SetOptions(HasDebugInfo(), HasRefStrings());

    return reader;
}
