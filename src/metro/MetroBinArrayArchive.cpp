#include "MetroBinArrayArchive.h"
#include "MetroBinArchive.h"

MetroBinArrayArchive::ChunkData::ChunkData(const MemStream& fileStream) {
    // Save offset from parent stream
    offset = fileStream.GetCursor();

    // Create chunk
    MemStream& chunkStream = fileStream.Substream(fileStream.Remains());
    index = chunkStream.ReadTyped<uint32_t>();
    size = chunkStream.ReadTyped<uint32_t>();
    binName = chunkStream.ReadStringZ();
    binSize = size - (binName.length() + 0x1 /*null terminator*/);

    binArchive = std::make_shared<MetroBinArchive>(
        binName,
        chunkStream.Substream(binSize),
        MetroBinArchive::kHeaderNotExist //TODO: Is it always true?
    );
}

MetroBinArrayArchive::MetroBinArrayArchive(const CharString& name, const MemStream& _binStream, const CharString& _headerAlias) {
    // Get raw memory stream
    mFileName = name;
    mFileStream = MemStream(_binStream);
    mHeaderVersion = kFileVersionNotExist;

    // Save cursor position
    size_t cursorAtStart = mFileStream.GetCursor();

    // Search for header
    mHeaderAlias = _headerAlias;
    mIsHeaderExist = false;

    if (!_headerAlias.empty()) {
        for (int i = 0; i < 4; i++) {
            char s = mFileStream.ReadTyped<char>();
            if (s != _headerAlias[i]) {
                break;
            }
            else if (i == 3) {
                mIsHeaderExist = true;
            }
        }
    }

    // Search for file version
    if (mIsHeaderExist) {
        mHeaderVersion = mFileStream.ReadTyped<uint16_t>();
    }
    else {
        mFileStream.SetCursor(cursorAtStart);
    }

    // Count elements, prepare array
    size_t numBins = mFileStream.ReadTyped<uint32_t>();
    assert(numBins > 0);
    mChunks.reserve(numBins);

    // Read all .bin-s
    size_t realNumBins = 0;
    while (!mFileStream.Ended()) {
        realNumBins++;
        ChunkData& chunkData = mChunks.emplace_back(mFileStream);
        mFileStream.SkipBytes(chunkData.GetTotalChunkSize());
    }

    assert(realNumBins == numBins);

    // Restore cursor position
    mFileStream.SetCursor(cursorAtStart);
}
