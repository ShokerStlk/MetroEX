#include "MetroTexturesDatabase.h"
#include "MetroReflection.h"
#include "MetroTypes.h"
#include "MetroBinArrayArchive.h"
#include "MetroBinArchive.h"


void MetroTextureInfo::Serialize(MetroReflectionReader& s) {
    METRO_READ_MEMBER(s, type);
    METRO_READ_MEMBER(s, texture_type);
    METRO_READ_MEMBER(s, source_name);
    METRO_READ_MEMBER(s, surf_xform);
    METRO_READ_MEMBER(s, format);
    METRO_READ_MEMBER(s, width);
    METRO_READ_MEMBER(s, height);
    METRO_READ_MEMBER(s, animated);
    METRO_READ_MEMBER(s, draft);
    METRO_READ_MEMBER(s, override_avg_color);
    METRO_READ_MEMBER(s, avg_color);
    METRO_READ_MEMBER_CHOOSE(s, shader_name);
    METRO_READ_MEMBER_CHOOSE(s, gamemtl_name);
    METRO_READ_MEMBER(s, priority);
    METRO_READ_MEMBER(s, streamable);
    METRO_READ_MEMBER(s, bump_height);
    METRO_READ_MEMBER(s, displ_type);
    METRO_READ_MEMBER(s, displ_height);
    METRO_READ_MEMBER(s, parallax_height_mul);
    METRO_READ_MEMBER(s, mipmapped);
    METRO_READ_MEMBER(s, reflectivity);
    METRO_READ_MEMBER(s, treat_as_metal);
    METRO_READ_MEMBER_CHOOSE(s, det_name);
    METRO_READ_MEMBER(s, det_scale_u);
    METRO_READ_MEMBER(s, det_scale_v);
    METRO_READ_MEMBER(s, det_intensity);
    METRO_READ_MEMBER(s, aux_params);
    METRO_READ_MEMBER(s, aux_params_1);

    // !!! Optional fields !!!
    if (this->texture_type == scast<uint8_t>(TextureType::Cubemap_hdr)) {
        METRO_READ_ARRAY_MEMBER(s, sph_coefs);
    } else if (this->texture_type == scast<uint8_t>(TextureType::Unknown_has_lum)) {
        METRO_READ_ARRAY_MEMBER(s, lum);
    }
    ///////////////////////////

    METRO_READ_MEMBER_CHOOSE(s, bump_name);
    METRO_READ_MEMBER_CHOOSE(s, aux0_name);
    METRO_READ_MEMBER_CHOOSE(s, aux1_name);
    METRO_READ_MEMBER_CHOOSE(s, aux2_name);
    METRO_READ_MEMBER_CHOOSE(s, aux3_name);
    METRO_READ_MEMBER_CHOOSE(s, aux4_name);
    METRO_READ_MEMBER_CHOOSE(s, aux5_name);
    METRO_READ_MEMBER_CHOOSE(s, aux6_name);
    METRO_READ_MEMBER_CHOOSE(s, aux7_name);
}

void MetroTextureAliasInfo::Serialize(MetroReflectionReader & s)
{
    METRO_READ_MEMBER(s, unknown0); // always 0 ???
    METRO_READ_MEMBER(s, unknown1); // always 9 ???
    METRO_READ_MEMBER(s, flags);    // always 4 ???
    METRO_READ_MEMBER(s, name);
    METRO_READ_MEMBER(s, alias);
}

MetroTexturesDatabase::MetroTexturesDatabase() {

}
MetroTexturesDatabase::~MetroTexturesDatabase() {

}

bool MetroTexturesDatabase::LoadFromData(MemStream& stream) {
    MetroBinArrayArchive binArchive("textures_handles_storage.bin", stream, MakeFourcc<'A','V','E','R'>());

    const size_t numEntries = binArchive.GetBinCnt();

    mDatabase.reserve(numEntries);
    mPool.resize(numEntries);

    for (size_t i = 0; i < numEntries; ++i) {
        const MetroBinArrayArchive::ChunkData& chunk = binArchive.GetChunkByIdx(i);
        const MetroBinArchive& bin = chunk.GetBinArchive();
        assert(bin.HasChunks() == false);
        
        MetroReflectionReader reader = bin.ReturnReflectionReader(bin.GetOffsetFirstDataBegin());

        MetroTextureInfo* texInfo = &mPool[i];
        texInfo->name   = bin.GetFileName();
        texInfo->flags  = bin.GetFlags();
        reader >> *texInfo;

        HashString hashStr(texInfo->name);

        mDatabase.insert({ hashStr, texInfo });

        if (texInfo->texture_type == scast<uint8_t>(MetroTextureInfo::TextureType::Diffuse)) {
            mDiffuseTextures.insert({ hashStr, texInfo });
        } else if (texInfo->texture_type == scast<uint8_t>(MetroTextureInfo::TextureType::Normalmap)) {
            mNormalmapTextures.insert({ hashStr, texInfo });
        }
    }

    return true;
}

bool MetroTexturesDatabase::LoadAliasesFromData(MemStream& stream) {
    MetroBinArchive bin("texture_aliases.bin", stream, MetroBinArchive::kHeaderDoAutoSearch);

    assert(bin.HasRefStrings());
    StringArray strings = bin.ReadStringTable();

    const size_t dataOffset = bin.GetFirstChunk().GetChunkDataOffset();
    MetroReflectionReader reader = bin.ReturnReflectionReader(dataOffset);

    reader.GetStream().SkipBytes(9); // skip unkn data

    uint32_t numAliases = 0;
    METRO_READ_MEMBER(reader, numAliases);
    mAliases.reserve(numAliases);

    for (size_t i = 0; i < scast<size_t>(numAliases); ++i) {
        MetroTextureAliasInfo texAliasInfo;
        reader >> texAliasInfo;

        mAliases.insert({ HashString(strings[texAliasInfo.name.ref]), HashString(strings[texAliasInfo.alias.ref]) });
    }

    return true;
}

const MetroTextureInfo* MetroTexturesDatabase::GetInfoByName(const HashString& name) const {
    const auto it = mDatabase.find(name);
    if (it == mDatabase.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}

const HashString& MetroTexturesDatabase::GetAlias(const HashString& name) const {
    static HashString empty;

    auto it = mAliases.find(name);
    if (it == mAliases.end()) {
        return empty;
    }

    return it->second;
}

const CharString& MetroTexturesDatabase::GetSourceName(const HashString& name) const {
    static CharString emptyStr;

    const HashString& alias = this->GetAlias(name);

    const MetroTextureInfo* mti = this->GetInfoByName((alias.hash == 0) ? name : alias);
    return (mti == nullptr) ? emptyStr : mti->source_name.str;
}

const CharString& MetroTexturesDatabase::GetBumpName(const HashString& name) const {
    static CharString emptyStr;

    const HashString& alias = this->GetAlias(name);

    const MetroTextureInfo* mti = this->GetInfoByName((alias.hash == 0) ? name : alias);
    return (mti == nullptr) ? emptyStr : mti->bump_name.str;
}
