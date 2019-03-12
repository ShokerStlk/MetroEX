#pragma once
#include "mycommon.h"

struct MetroSound {
    static bool SaveDataAsOGG(const void* data, const size_t dataLen, const fs::path& outPath);
    static bool SaveDataAsWAV(const void* data, const size_t dataLen, const fs::path& outPath);
};
