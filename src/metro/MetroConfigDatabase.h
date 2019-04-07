#pragma once
#include "mycommon.h"

class MetroConfigsDatabase {
public:
    struct ConfigInfo {
        uint32_t    nameCRC;
        CharString  nameStr;
        size_t      offset;
        size_t      length;
    };

    MetroConfigsDatabase();
    ~MetroConfigsDatabase();

    bool                LoadFromData(const void* data, const size_t length);

    const ConfigInfo*   FindFile(const uint32_t nameCRC) const;
    const ConfigInfo*   FindFile(const CharString& name) const;

    size_t              GetNumFiles() const;
    const ConfigInfo&   GetFileByIdx(const size_t chunkIdx) const;

private:
    MyArray<uint8_t>    mData;
    MyArray<ConfigInfo> mConfigsChunks;

    size_t              mStatsTotalDecryptedNames;
    size_t              mStatsTotalEncryptedNames;
};
