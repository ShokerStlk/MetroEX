#pragma once
#include "MetroTypes.h"

struct Pak {
    CharString      name;
    StringArray     mappings;
    size_t          idx;
    MyArray<size_t>   files;
};

class VFXReader {
public:
    VFXReader();
    ~VFXReader();

    bool                LoadFromFile(const fs::path& filePath);
    bool                ExtractFile(const size_t fileIdx, BytesArray& content);

    const CharString    GetSelfName() const;

    size_t              FindFile(const CharString& fileName, const MetroFile* inFolder = nullptr) const;
    const MetroFile&    GetRootFolder() const;
    const MetroFile*    GetParentFolder(const size_t fileIdx) const;
    const MetroFile&    GetFile(const size_t idx) const;
    size_t              CountFilesInFolder(const size_t idx) const;

private:
    size_t              Decompress(const BytesArray& compressedData, BytesArray& uncompressedData);

    CharString          mFileName;
    fs::path            mBasePath;
    MyArray<Pak>          mPaks;
    MyArray<MetroFile>    mFiles;
    MyArray<size_t>       mFolders;
};
