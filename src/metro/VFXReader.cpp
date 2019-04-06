#include "VFXReader.h"
#include <fstream>

#define LZ4_DISABLE_DEPRECATE_WARNINGS
#include "lz4.h"


static const size_t kVFXVersionExodus   = 3;
static const size_t kVFXCompressionLZ4  = 1;

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


        const size_t version = stream.ReadTyped<uint32_t>();
        const size_t compressionType = stream.ReadTyped<uint32_t>();
        if (version == kVFXVersionExodus && compressionType == kVFXCompressionLZ4) { // Metro Exodus and LZ4
            mContentVersion = stream.ReadStringZ();
            stream.SkipBytes(16); // guid, seems to be static across the game
            const size_t numVFS = stream.ReadTyped<uint32_t>();
            const size_t numFiles = stream.ReadTyped<uint32_t>();
            const size_t unknown_0 = stream.ReadTyped<uint32_t>();

            mPaks.resize(numVFS);
            for (Pak& pak : mPaks) {
                pak.name = stream.ReadStringZ();

                const uint32_t numStrings = stream.ReadTyped<uint32_t>();
                pak.someStrings.resize(numStrings);
                for (auto& s : pak.someStrings) {
                    s = stream.ReadStringZ();
                }

                pak.chunk = stream.ReadTyped<uint32_t>();
            }

            mFiles.resize(numFiles);
            size_t fileIdx = 0;
            for (MetroFile& mf : mFiles) {
                mf.idx = fileIdx;
                mf.flags = stream.ReadTyped<uint16_t>();

                if (mf.IsFile()) {
                    mf.pakIdx = stream.ReadTyped<uint16_t>();
                    mf.offset = stream.ReadTyped<uint32_t>();
                    mf.sizeUncompressed = stream.ReadTyped<uint32_t>();
                    mf.sizeCompressed = stream.ReadTyped<uint32_t>();
                    mf.name = readCharStringXored();
                } else {
                    mf.numFiles = stream.ReadTyped<uint16_t>();
                    mf.firstFile = stream.ReadTyped<uint32_t>();
                    mf.name = readCharStringXored();

                    mFolders.push_back(fileIdx);
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

MyArray<size_t> VFXReader::GetAllFolders() const {
    return mFolders;
}

const MetroFile* VFXReader::GetFolder(const CharString& folderPath, const MetroFile* inFolder) const {
    CharString::size_type slashPos = folderPath.find_first_of('\\'), lastSlashPos = 0;
    const MetroFile* folder = (inFolder == nullptr) ? &mFiles.front() : inFolder;
    while (slashPos != CharString::npos) {
        size_t folderIdx = MetroFile::InvalidFileIdx;

        CharString name = folderPath.substr(lastSlashPos, slashPos - lastSlashPos);
        for (size_t idx = folder->firstFile; idx < (folder->firstFile + folder->numFiles); ++idx) {
            const MetroFile& mf = mFiles[idx];
            if (name == mf.name) {
                folderIdx = idx;
                break;
            }
        }

        if (folderIdx == MetroFile::InvalidFileIdx) { // failed to find
            return nullptr;
        }

        lastSlashPos = slashPos + 1;
        slashPos = folderPath.find_first_of('\\', lastSlashPos);
        folder = &mFiles[folderIdx];
    }

    return folder;
}

size_t VFXReader::FindFile(const CharString& fileName, const MetroFile* inFolder) const {
    size_t result = MetroFile::InvalidFileIdx;

    const MetroFile* folder = (inFolder == nullptr) ? &mFiles.front() : inFolder;

    CharString::size_type lastSlashPos = fileName.find_last_of('\\');
    if (lastSlashPos != CharString::npos) {
        lastSlashPos++;
        folder = this->GetFolder(fileName.substr(0, lastSlashPos), inFolder);
    } else {
        lastSlashPos = 0;
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

MyArray<size_t> VFXReader::FindFilesInFolder(const size_t folderIdx, const CharString& extension, const bool withSubfolders) {
    MyArray<size_t> result;

    const MetroFile& folder = mFiles[folderIdx];
    if (!folder.IsFile()) {
        for (size_t idx = folder.firstFile; idx < (folder.firstFile + folder.numFiles); ++idx) {
            const MetroFile& mf = mFiles[idx];

            if (!mf.IsFile()) {
                const MyArray<size_t>& v = this->FindFilesInFolder(mf.idx, extension, withSubfolders);
                result.insert(result.end(), v.begin(), v.end());
            } else {
                if (StrEndsWith(mf.name, extension)) {
                    result.push_back(mf.idx);
                }
            }
        }
    }

    return std::move(result);
}

MyArray<size_t> VFXReader::FindFilesInFolder(const CharString& folder, const CharString& extension, const bool withSubfolders) {
    MyArray<size_t> result;

    const MetroFile* folderPtr = this->GetFolder(folder);
    if (folderPtr) {
        result = this->FindFilesInFolder(folderPtr->idx, extension, withSubfolders);
    }

    return std::move(result);
}



size_t VFXReader::Decompress(const BytesArray& compressedData, BytesArray& uncompressedData) {
    size_t result = 0;

    MemStream stream(compressedData.data(), compressedData.size());
    char* dst = rcast<char*>(uncompressedData.data());

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
