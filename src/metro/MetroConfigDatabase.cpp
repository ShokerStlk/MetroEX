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

        ConfigInfo ci;
        ci.nameCRC = nameCrc;
        ci.nameStr = ConfigNamesDB::Get().FindByCRC32(nameCrc);
        ci.offset = stream.GetCursor();
        ci.length = bodySize;

        if (ci.nameStr.empty()) {
            mStatsTotalEncryptedNames++;
        } else {
            mStatsTotalDecryptedNames++;
        }

        mConfigsChunks.emplace_back(ci);

        stream.SkipBytes(bodySize);
    }

    mData.resize(length);
    memcpy(mData.data(), data, length);

    return !mConfigsChunks.empty();
}

const MetroConfigsDatabase::ConfigInfo* MetroConfigsDatabase::FindFile(const uint32_t nameCRC) const {
    auto it = std::find_if(mConfigsChunks.begin(), mConfigsChunks.end(), [nameCRC](const ConfigInfo& ci)->bool {
        return ci.nameCRC == nameCRC;
    });

    return (it == mConfigsChunks.end()) ? nullptr : &(*it);
}

const MetroConfigsDatabase::ConfigInfo* MetroConfigsDatabase::FindFile(const CharString& name) const {
    auto it = std::find_if(mConfigsChunks.begin(), mConfigsChunks.end(), [&name](const ConfigInfo& ci)->bool {
        return ci.nameStr == name;
    });

    return (it == mConfigsChunks.end()) ? nullptr : &(*it);
}

size_t MetroConfigsDatabase::GetNumFiles() const {
    return mConfigsChunks.size();
}

const MetroConfigsDatabase::ConfigInfo& MetroConfigsDatabase::GetFileByIdx(const size_t chunkIdx) const {
    return mConfigsChunks[chunkIdx];
}
