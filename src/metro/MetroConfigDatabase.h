#pragma once
#include "mycommon.h"

class MetroConfigsDatabase {
public:
    struct ConfigInfo {
        uint32_t    name_crc;
        CharString  name_decrypted;
        size_t      offset;
        size_t      length;
    };

    MetroConfigsDatabase();
    ~MetroConfigsDatabase();

    bool        LoadFromData(const void* data, const size_t length);
    
    ConfigInfo* FindFile(uint32_t nameCRC32);
    ConfigInfo* FindFile(CharString name);

    ConfigInfo* GetFileByIdx(size_t chunkIdx);

    MyArray<uint8_t>    mData;
    MyArray<ConfigInfo> mConfigsChunks;

    uint32_t mStatsTotalDecryptedNames;
    uint32_t mStatsTotalEncryptedNames;
};
