#include "VFXReader.h"
#include <fstream>


VFXReader::VFXReader() {

}

VFXReader::~VFXReader() {

}

bool VFXReader::LoadFromFile(const fs::path& filePath) {
    bool result = false;

    std::ifstream file(filePath, std::ifstream::binary);
    if (file.good()) {
        BytesArray fileData;

        file.seekg(0, std::ios::end);
        fileData.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(rcast<char*>(fileData.data()), fileData.size());
        file.close();

        MemStream stream(fileData.data(), fileData.size());

        auto readCharStringXored = [&stream]() -> CharString {
            CharString result;

            const uint16_t stringHeader = stream.ReadTyped<uint16_t>();
            const size_t stringLen = (stringHeader & 0xFF);
            const char xorMask = scast<char>((stringHeader >> 8) & 0xFF);

            result.reserve(stringLen);
            for (size_t i = 1; i < stringLen; ++i) {
                const char ch = stream.ReadTyped<char>();
                result.push_back(ch ^ xorMask);
            }

            stream.ReadTyped<char>(); // terminating null

            return result;
        };


        const uint32_t version = stream.ReadTyped<uint32_t>();
        if (version == 3) { // Metro: Exodus
            const uint32_t num1 = stream.ReadTyped<uint32_t>();
            CharString someString = stream.ReadStringZ();
            stream.SkipBytes(16); // ???
            const uint32_t numVFS = stream.ReadTyped<uint32_t>();
            const uint32_t numFiles = stream.ReadTyped<uint32_t>();
            const uint32_t numPatches = stream.ReadTyped<uint32_t>();

            mPaks.resize(numVFS);
            for (Pak& pak : mPaks) {
                pak.name = stream.ReadStringZ();

                const uint32_t numMappings = stream.ReadTyped<uint32_t>();
                pak.mappings.resize(numMappings);
                for (auto& m : pak.mappings) {
                    m = stream.ReadStringZ();
                }

                pak.idx = stream.ReadTyped<uint32_t>();
            }

            mFiles.resize(numFiles);
            size_t fileIdx = 0;
            for (MetroFile& mf : mFiles) {
                const uint16_t entryType = stream.ReadTyped<uint16_t>();

                mf.idx = fileIdx;
                mf.type = scast<MetroFile::FileType>(entryType);
                switch (mf.type) {
                    case MetroFile::FT_File:
                    case MetroFile::FT_File2: {
                        mf.pakIdx = stream.ReadTyped<uint16_t>();
                        mf.offset = stream.ReadTyped<uint32_t>();
                        mf.sizeUncompressed = stream.ReadTyped<uint32_t>();
                        mf.sizeCompressed = stream.ReadTyped<uint32_t>();
                        mf.name = readCharStringXored();
                    } break;

                    case MetroFile::FT_Dir:
                    case MetroFile::FT_Dir2: {
                        mf.numFiles = stream.ReadTyped<uint16_t>();
                        mf.firstFile = stream.ReadTyped<uint32_t>();
                        mf.name = readCharStringXored();

                        mFolders.push_back(fileIdx);
                    } break;

                    default:
                        assert(false);
                        break;
                }

                ++fileIdx;
            }

            mBasePath = filePath.parent_path();
            mFileName = filePath.filename().string();
            result = true;
        }
    }

    return result;
}

bool VFXReader::ExtractFile(const size_t fileIdx, BytesArray& content) {
    bool result = false;

    const MetroFile& mf = mFiles[fileIdx];
    const Pak& pak = mPaks[mf.pakIdx];

    fs::path pakPath = mBasePath / pak.name;
    std::ifstream file(pakPath, std::ifstream::binary);
    if (file.good()) {
        file.seekg(mf.offset);

        BytesArray fileContent(mf.sizeCompressed);
        file.read(rcast<char*>(fileContent.data()), mf.sizeCompressed);

        BytesArray uncompressedData;

        if (mf.sizeCompressed == mf.sizeUncompressed) {
            uncompressedData.swap(fileContent);
            result = true;
        } else {
            uncompressedData.resize(mf.sizeUncompressed);
            const size_t decompressResult = this->Decompress(fileContent, uncompressedData);
            if (decompressResult == mf.sizeUncompressed) {
                result = true;
            }
        }

        if (result) {
            content.swap(uncompressedData);
        }
    }

    return result;
}

const CharString VFXReader::GetSelfName() const {
    return mFileName;
}

size_t VFXReader::FindFile(const CharString& fileName, const MetroFile* inFolder) const {
    size_t result = MetroFile::InvalidFileIdx;

    CharString::size_type slashPos = fileName.find_first_of('\\'), lastSlashPos = 0;
    size_t folderIdx = 0;
    const MetroFile* folder = (inFolder == nullptr) ? &mFiles.front() : inFolder;
    while (slashPos != CharString::npos) {
        CharString name = fileName.substr(lastSlashPos, slashPos - lastSlashPos);
        for (size_t idx = folder->firstFile; idx < (folder->firstFile + folder->numFiles); ++idx) {
            const MetroFile& mf = mFiles[idx];
            if (name == mf.name) {
                folderIdx = idx;
                break;
            }
        }

        if (folderIdx == 0) { // failed to find
            return result;
        }

        lastSlashPos = slashPos + 1;
        slashPos = fileName.find_first_of('\\', lastSlashPos);
        folder = &mFiles[folderIdx];
    }

    CharString name = fileName.substr(lastSlashPos);

    for (size_t idx = folder->firstFile; idx < (folder->firstFile + folder->numFiles); ++idx) {
        const MetroFile& mf = mFiles[idx];
        if (name == mf.name) {
            result = idx;
            break;
        }
    }

    return result;
}

const MetroFile& VFXReader::GetRootFolder() const {
    return mFiles.front();
}

const MetroFile* VFXReader::GetParentFolder(const size_t fileIdx) const {
    const MetroFile* result = nullptr;

    for (const size_t idx : mFolders) {
        const MetroFile* folder = &mFiles[idx];
        if (fileIdx >= folder->firstFile && fileIdx < (folder->firstFile + folder->numFiles)) {
            result = folder;
            break;
        }
    }

    return result;
}

const MetroFile& VFXReader::GetFile(const size_t idx) const {
    return mFiles[idx];
}

size_t VFXReader::CountFilesInFolder(const size_t idx) const {
    size_t result = 0;

    const MetroFile& folder = mFiles[idx];
    for (size_t idx = folder.firstFile; idx < (folder.firstFile + folder.numFiles); ++idx) {
        const MetroFile& mf = mFiles[idx];
        if (mf.IsFile()) {
            result++;
        } else {
            result += this->CountFilesInFolder(idx);
        }
    }

    return result;
}



static const int32_t kDecCounts[8] = { 0, 3, 2, 3, 0, 0, 0, 0 };
static const int32_t kDecOffsets[8] = { 0, 0, 0, -1, 0, 1, 2, 3 };

size_t VFXReader::Decompress(const BytesArray& compressedData, BytesArray& uncompressedData) {
    size_t result = 0;

    MemStream stream(compressedData.data(), compressedData.size());
    uint8_t* dst = uncompressedData.data();

    uint32_t num1 = 0;
    uint32_t num2 = 0;
    uint32_t destinationIdx = 0;

    while (!stream.Ended()) {
        const uint32_t num3 = stream.ReadTyped<uint32_t>();
        const uint32_t num4 = stream.ReadTyped<uint32_t>();

        num1 += num3;
        num2 += num4;

        uint32_t num5 = destinationIdx + num4;

        for (;;) {
            uint32_t num6 = stream.ReadTyped<uint8_t>();
            uint32_t num7 = num6 >> 4;
            if (num7 == 0xF) {
                uint8_t num8 = 0;
                do {
                    num8 = stream.ReadTyped<uint8_t>();
                    num7 += num8;
                } while (num8 == 0xFF);
            }

            stream.ReadToBuffer(dst + destinationIdx, num7);

            destinationIdx += num7;

            if (destinationIdx < num5) {
                const uint32_t index1 = stream.ReadTyped<uint16_t>();

                uint32_t num8 = num6 & 0xF;
                if (num8 == 0xF) {
                    uint8_t num9 = 0;
                    do {
                        num9 = stream.ReadTyped<uint8_t>();
                        num8 += num9;
                    } while (num9 == 0xFF);
                }

                uint32_t sourceIndex1 = destinationIdx - index1;
                uint32_t destinationIndex2 = destinationIdx;
                uint32_t sourceIndex2 = 0;
                uint32_t destinationIndex3 = 0;
                if (index1 >= 8) {
                    std::memmove(dst + destinationIndex2, dst + sourceIndex1, 8);
                    sourceIndex2 = sourceIndex1 + 8;
                    destinationIndex3 = destinationIndex2 + 8;
                } else {
                    dst[destinationIndex2 + 0] = dst[sourceIndex1 + 0];
                    dst[destinationIndex2 + 1] = dst[sourceIndex1 + 1];
                    dst[destinationIndex2 + 2] = dst[sourceIndex1 + 2];
                    dst[destinationIndex2 + 3] = dst[sourceIndex1 + 3];

                    int num9 = sourceIndex1 + 4;
                    int destinationIndex4 = destinationIndex2 + 4;
                    int index2 = destinationIndex4 - num9;
                    int sourceIndex3 = num9 - kDecCounts[index2];

                    std::memmove(dst + destinationIndex4, dst + sourceIndex3, 4);

                    sourceIndex2 = sourceIndex3 - kDecOffsets[index1];
                    destinationIndex3 = destinationIndex4 + 4;
                }

                int num10 = destinationIndex3 - 4 + num8;
                if (num10 < num5 - 8) {
                    for (; destinationIndex3 < num10; destinationIndex3 += 8) {
                        std::memmove(dst + destinationIndex3, dst + sourceIndex2, 8);
                        sourceIndex2 += 8;
                    }
                } else {
                    for (; destinationIndex3 < num10; ++destinationIndex3) {
                        dst[destinationIndex3] = dst[sourceIndex2];
                        ++sourceIndex2;
                    }
                }
                destinationIdx = num10;
            } else {
                break;
            }
        }
    }

    result = destinationIdx;

    return result;
}
