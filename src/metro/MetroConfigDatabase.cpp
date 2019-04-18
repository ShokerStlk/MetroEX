#include "MetroConfigDatabase.h"
#include "MetroConfigNames.h"
#include "hashing.h"

#include <fstream>

MetroConfigsDatabase::MetroConfigsDatabase() {

}
MetroConfigsDatabase::~MetroConfigsDatabase() {

}

bool MetroConfigsDatabase::LoadFromData(MemStream& stream) {
    bool result = false;

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

    if (!mConfigsChunks.empty()) {
        mStream = stream.Clone();
        result = true;
    }

    return result;
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

MemStream MetroConfigsDatabase::GetFileStream(const CharString& name) const {
    auto i = this->FindFile(name);
    return (i == nullptr) ? MemStream() : mStream.Substream(i->offset, i->length);
}

size_t MetroConfigsDatabase::GetNumFiles() const {
    return mConfigsChunks.size();
}

const MetroConfigsDatabase::ConfigInfo& MetroConfigsDatabase::GetFileByIdx(const size_t chunkIdx) const {
    return mConfigsChunks[chunkIdx];
}
