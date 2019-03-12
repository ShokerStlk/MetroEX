#include "MetroTexturesDatabase.h"
#include "MetroReflection.h"
#include "MetroTypes.h"


void MetroTextureInfo::Serialize(MetroReflectionReader& s) {
    METRO_READ_MEMBER_NO_VERIFY(s, name);
    METRO_READ_MEMBER_NO_VERIFY(s, flags);

    const bool hasDebugInfo = TestBit(flags, MetroBinFlags::HasDebugInfo);
    const bool stringsAsRef = TestBit(flags, MetroBinFlags::RefStrings);
    s.SetOptions(hasDebugInfo, stringsAsRef);

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

    s.SetOptions();
}


MetroTexturesDatabase::MetroTexturesDatabase() {

}
MetroTexturesDatabase::~MetroTexturesDatabase() {

}

bool MetroTexturesDatabase::LoadFromData(const void* data, const size_t length) {
    MemStream stream(data, length);

    const size_t numEntries = stream.ReadTyped<uint32_t>();
    mDatabase.reserve(numEntries);
    mPool.resize(numEntries);

    for (size_t i = 0; i < numEntries; ++i) {
        const size_t idx = stream.ReadTyped<uint32_t>();
        const size_t size = stream.ReadTyped<uint32_t>();

        MetroReflectionReader reader(stream.Substream(size));

        MetroTextureInfo* texInfo = &mPool[i];
        reader >> *texInfo;

        HashString hashStr(texInfo->name);

        mDatabase.insert({ hashStr, texInfo });

        if (texInfo->texture_type == scast<uint8_t>(MetroTextureInfo::TextureType::Diffuse)) {
            mDiffuseTextures.insert({ hashStr, texInfo });
        } else if (texInfo->texture_type == scast<uint8_t>(MetroTextureInfo::TextureType::Normalmap)) {
            mNormalmapTextures.insert({ hashStr, texInfo });
        }

        stream.SkipBytes(size);
    }

    return true;
}

PACKED_STRUCT_BEGIN
struct TextureAlias {
    uint32_t    unknown0;   // always 0 ???
    uint32_t    unknown1;   // always 9 ???
    uint8_t     flags;      // always 4 ???
    uint32_t    name;
    uint32_t    alias;
} PACKED_STRUCT_END;

bool MetroTexturesDatabase::LoadAliasesFromData(const void* data, const size_t length) {
    bool result = false;

    MemStream stream(data, length);

    const size_t flags = stream.ReadTyped<uint8_t>();
    if (flags != 4) {
        return false;
    }

    // Bindings chunk
    size_t chunkId = stream.ReadTyped<uint32_t>();
    size_t chunkSize = stream.ReadTyped<uint32_t>();
    size_t chunkEnd = stream.GetCursor() + chunkSize;

    if (chunkId != 1) { // pairs
        return false;
    }

    stream.SkipBytes(9);
    const size_t numAliases = stream.ReadTyped<uint32_t>();

    Array<TextureAlias> aliases(numAliases);
    stream.ReadToBuffer(aliases.data(), numAliases * sizeof(TextureAlias));

    stream.SetCursor(chunkEnd);

    // strings chunk
    chunkId = stream.ReadTyped<uint32_t>();
    chunkSize = stream.ReadTyped<uint32_t>();

    if (chunkId != 2) { // strings
        return false;
    }

    const size_t numStrings = stream.ReadTyped<uint32_t>();
    Array<CharString> strings(numStrings);
    for (CharString& s : strings) {
        s = stream.ReadStringZ();
    }

    mAliases.reserve(numAliases);
    for (const TextureAlias& ta : aliases) {
        mAliases.insert({ HashString(strings[ta.name]), HashString(strings[ta.alias]) });
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
