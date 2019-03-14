#include "MetroConfigDatabase.h"
#include "hashing.h"

#include <fstream>

MetroConfigDatabase::MetroConfigDatabase() {

}
MetroConfigDatabase::~MetroConfigDatabase() {

}

bool MetroConfigDatabase::LoadFromData(const void* data, const size_t length) {
    MemStream stream(data, length);

    const uint32_t materialsCrc = Hash_CalculateCRC32("content\\scripts\\materials.bin");

    while (!stream.Ended()) {
        const uint32_t nameCrc = stream.ReadTyped<uint32_t>();
        const uint32_t bodySize = stream.ReadTyped<uint32_t>();

        ConfigInfo ci;
        ci.offset = stream.GetCursor();
        ci.length = bodySize;

        mConfigs.insert({ nameCrc, ci });

        if (materialsCrc == nameCrc) {
            std::ofstream file("materials.bin", std::ofstream::binary);
            if (file.good()) {
                file.write((char*)stream.GetDataAtCursor(), bodySize);
                file.flush();
            }
        }

        stream.SkipBytes(bodySize);
    }

    mData.resize(length);
    memcpy(mData.data(), data, length);

    return !mConfigs.empty();
}
