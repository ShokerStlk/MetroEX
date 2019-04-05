#include "MetroConfigDatabase.h"
#include "MetroConfigNames.h"
#include "hashing.h"

#include <fstream>

MetroConfigsDatabase::MetroConfigsDatabase() {

}
MetroConfigsDatabase::~MetroConfigsDatabase() {

}

bool MetroConfigsDatabase::LoadFromData(const void* data, const size_t length) {
    MemStream stream(data, length);

    mStatsTotalDecryptedNames = 0;
    mStatsTotalEncryptedNames = 0;

    while (!stream.Ended()) {
        const uint32_t nameCrc = stream.ReadTyped<uint32_t>();
        const uint32_t bodySize = stream.ReadTyped<uint32_t>();

        ConfigNamesDB* cfgNamesDB = ConfigNamesDB::Get();

        ConfigInfo ci;
        ci.name_crc = nameCrc;
        ci.name_decrypted = cfgNamesDB->FindByCRC32(nameCrc);
        ci.offset = stream.GetCursor();
        ci.length = bodySize;

        if (ci.name_decrypted.empty()) {
            mStatsTotalEncryptedNames++;
        }
        else {
            mStatsTotalDecryptedNames++;
        }

        mConfigsChunks.push_back(ci);

#if 0
        // Extract materials.bin from configs.bin to user disk 
        if (ci.name_decrypted == "content\\scripts\\materials.bin") {
            std::ofstream file("materials.bin", std::ofstream::binary);
            if (file.good()) {
                file.write((char*)stream.GetDataAtCursor(), bodySize);
                file.flush();
            }
        }
#endif

        stream.SkipBytes(bodySize);
    }

    mData.resize(length);
    memcpy(mData.data(), data, length);

    return !mConfigsChunks.empty();
}

MetroConfigsDatabase::ConfigInfo* MetroConfigsDatabase::FindFile(uint32_t nameCRC32)
{
    for (ConfigInfo& ci : mConfigsChunks) {
        if (ci.name_crc == nameCRC32)
        {
            return &ci;
        }
    }

    return nullptr;
}

MetroConfigsDatabase::ConfigInfo* MetroConfigsDatabase::FindFile(CharString name)
{
    for (ConfigInfo& ci : mConfigsChunks) {
        if (ci.name_decrypted == name)
        {
            return &ci;
        }
    }

    return nullptr;
}

MetroConfigsDatabase::ConfigInfo* MetroConfigsDatabase::GetFileByIdx(size_t chunkIdx)
{
    assert(chunkIdx < mConfigsChunks.size());
    return &mConfigsChunks[chunkIdx];
}
