#pragma once
#include "mycommon.h"

class MetroSound {
public:
    MetroSound();
    ~MetroSound();

    bool    LoadFromData(MemStream& stream);
    bool    GetWAVE(BytesArray& waveData);

    bool    SaveAsOGG(const fs::path& outPath);
    bool    SaveAsWAV(const fs::path& outPath);

private:
    MyArray<uint8_t>    mData;
};
