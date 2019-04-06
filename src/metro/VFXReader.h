#pragma once
#include "MetroTypes.h"

struct Pak {
    CharString      name;
    StringArray     someStrings;
    size_t          chunk;
    MyArray<size_t> files;
};

class VFXReader {
public:
    VFXReader();
    ~VFXReader();

    bool                LoadFromFile(const fs::path& filePath);
    bool                ExtractFile(const size_t fileIdx, BytesArray& content);

    const CharString    GetSelfName() const;
    MyArray<size_t>     GetAllFolders() const;

    const MetroFile*    GetFolder(const CharString& folderPath, const MetroFile* inFolder = nullptr) const;
    size_t              FindFile(const CharString& fileName, const MetroFile* inFolder = nullptr) const;
    const MetroFile&    GetRootFolder() const;
    const MetroFile*    GetParentFolder(const size_t fileIdx) const;
    const MetroFile&    GetFile(const size_t idx) const;
    size_t              CountFilesInFolder(const size_t idx) const;

    MyArray<size_t>     FindFilesInFolder(const size_t folderIdx, const CharString& extension, const bool withSubfolders = true);
    MyArray<size_t>     FindFilesInFolder(const CharString& folder, const CharString& extension, const bool withSubfolders = true);

private:
    size_t              Decompress(const BytesArray& compressedData, BytesArray& uncompressedData);

    CharString          mContentVersion;
    CharString          mFileName;
    fs::path            mBasePath;
    MyArray<Pak>        mPaks;
    MyArray<MetroFile>  mFiles;
    MyArray<size_t>     mFolders;
};
