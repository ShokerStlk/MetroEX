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

void MetroTextureAliasInfo::Serialize(MetroReflectionReader & s) {
    METRO_READ_MEMBER(s, src);
    METRO_READ_MEMBER(s, dst);
}

MetroTexturesDatabase::MetroTexturesDatabase() {

}
MetroTexturesDatabase::~MetroTexturesDatabase() {

}

bool MetroTexturesDatabase::LoadFromData(MemStream& stream) {
    size_t numEntries = stream.ReadTyped<uint32_t>();
    if (numEntries == MakeFourcc<'A','V','E','R'>()) {
        stream.SkipBytes(2);
        numEntries = stream.ReadTyped<uint32_t>();
    }

    mPool.resize(numEntries);
    mDatabase.reserve(numEntries);

    for (size_t i = 0; i < numEntries; ++i) {
        const size_t idx = stream.ReadTyped<uint32_t>();
        const size_t size = stream.ReadTyped<uint32_t>();

        MemStream subStream = stream.Substream(size);

        CharString name = subStream.ReadStringZ();
        const uint8_t flags = subStream.ReadTyped<uint8_t>();

        MetroReflectionReader reader(subStream, flags);

        MetroTextureInfo* texInfo = &mPool[i];
        texInfo->name = name;
        reader >> *texInfo;

        HashString hashStr(texInfo->name);
        mDatabase.insert({ hashStr, texInfo });

        stream.SkipBytes(size);
    }

    return true;
}

bool MetroTexturesDatabase::LoadAliasesFromData(MemStream& stream) {
    MetroBinArchive bin("texture_aliases.bin", stream, MetroBinArchive::kHeaderDoAutoSearch);
    MetroReflectionReader reader = bin.ReflectionReader();

    MyArray<MetroTextureAliasInfo> texture_aliases;
    METRO_READ_STRUCT_ARRAY_MEMBER(reader, texture_aliases);

    if (!texture_aliases.empty()) {
        mAliases.reserve(texture_aliases.size());
        for (const auto& alias : texture_aliases) {
            mAliases.insert({ HashString(alias.src), HashString(alias.dst) });
        }
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
    static HashString emptyString;

    auto it = mAliases.find(name);
    if (it == mAliases.end()) {
        return emptyString;
    }

    return it->second;
}

const CharString& MetroTexturesDatabase::GetSourceName(const HashString& name) const {
    const HashString& alias = this->GetAlias(name);

    const MetroTextureInfo* mti = this->GetInfoByName((alias.hash == 0) ? name : alias);
    return (mti == nullptr) ? kEmptyString : mti->source_name;
}

const CharString& MetroTexturesDatabase::GetBumpName(const HashString& name) const {
    const HashString& alias = this->GetAlias(name);

    const MetroTextureInfo* mti = this->GetInfoByName((alias.hash == 0) ? name : alias);
    return (mti == nullptr) ? kEmptyString : mti->bump_name;
}
