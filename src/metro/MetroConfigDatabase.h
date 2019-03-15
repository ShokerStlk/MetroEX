#pragma once
#include "mycommon.h"

class MetroConfigDatabase {
public:
    MetroConfigDatabase();
    ~MetroConfigDatabase();

    bool    LoadFromData(const void* data, const size_t length);

private:
    struct ConfigInfo {
        size_t  offset;
        size_t  length;
    };

    MyArray<uint8_t>              mData;
    MyDict<uint32_t, ConfigInfo>  mConfigs;
};
