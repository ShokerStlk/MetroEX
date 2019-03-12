#pragma once
#include "mycommon.h"
#include "mymath.h"

struct MetroFile {
    static const size_t InvalidFileIdx = ~0;

    enum FileType {
        FT_File = 0,
        FT_Dir = 8,
        FT_Dir2 = 12,
        FT_File2 = 16
    };

    bool IsFile() const {
        return this->type == FT_File || this->type == FT_File2;
    }

    // common fields
    FileType    type;
    CharString  name;

    // file fields
    size_t      pakIdx;
    size_t      offset;
    size_t      sizeUncompressed;
    size_t      sizeCompressed;

    // dir fields
    size_t      firstFile;
    size_t      numFiles;
};


PACKED_STRUCT_BEGIN
struct MetroFace {
    uint16_t a, b, c;
} PACKED_STRUCT_END;

PACKED_STRUCT_BEGIN
struct MetroVertex {
    vec3        pos;
    uint8_t     bones[4];
    vec3        normal;
    uint8_t     weights[4];
    vec2        uv0;
    vec2        uv1;
} PACKED_STRUCT_END;

struct MetroMesh {
    bool                skinned;
    vec3                scales;
    AABBox              bbox;
    size_t              type;
    size_t              idx;
    StringArray         materials;
    Array<MetroFace>    faces;
    Array<MetroVertex>  vertices;
    BytesArray          bonesRemap;
};


// all these vertices are 32 bytes each
PACKED_STRUCT_BEGIN
struct VertexStatic {
    vec3     pos;
    uint32_t normal;
    uint32_t aux0;
    uint32_t aux1;
    vec2     uv;
} PACKED_STRUCT_END;

PACKED_STRUCT_BEGIN
struct VertexSkinned {
    int16_t  pos[4];
    uint32_t normal;
    uint32_t aux0;
    uint32_t aux1;
    uint8_t  bones[4];
    uint8_t  weights[4];
    int16_t  uv[2];
} PACKED_STRUCT_END;

PACKED_STRUCT_BEGIN
struct VertexLevel {
    vec3     pos;
    uint32_t normal;
    uint32_t aux0;
    uint32_t aux1;
    int16_t  uv0[2];
    int16_t  uv1[2];
} PACKED_STRUCT_END;


static vec3 DecodeNormal(const uint32_t n) {
    const float div = 1.0f / 127.0f;
    const float x = (((n & 0x00ff0000) >> 16) * div) - 1.0f;
    const float y = (((n & 0x0000ff00) >> 8) * div) - 1.0f;
    const float z = (((n & 0x000000ff) >> 0) * div) - 1.0f;

    return vec3(x, y, z);
}

template <typename T>
inline MetroVertex ConvertVertex(const T&) {
    assert(false);
}

template <>
inline MetroVertex ConvertVertex<VertexStatic>(const VertexStatic& v) {
    MetroVertex result = {};

    result.pos = v.pos;
    result.normal = DecodeNormal(v.normal);
    result.uv0 = v.uv;

    return result;
}

template <>
inline MetroVertex ConvertVertex<VertexSkinned>(const VertexSkinned& v) {
    const float posDequant = 1.0f / 32767.0f;
    const float uvDequant = 1.0f / 2048.0f;

    MetroVertex result = {};

    result.pos = vec3(scast<float>(v.pos[0]) * posDequant,
                      scast<float>(v.pos[1]) * posDequant,
                      scast<float>(v.pos[2]) * posDequant);
    *rcast<uint32_t*>(result.bones) = *rcast<const uint32_t*>(v.bones);
    result.normal = DecodeNormal(v.normal);
    *rcast<uint32_t*>(result.weights) = *rcast<const uint32_t*>(v.weights);
    result.uv0 = vec2(scast<float>(v.uv[0]) * uvDequant,
                      scast<float>(v.uv[1]) * uvDequant);
    return result;
}

template <>
inline MetroVertex ConvertVertex<VertexLevel>(const VertexLevel& v) {
    const float uvDequant = 1.0f / 1024.0f;

    MetroVertex result = {};

    result.pos = v.pos;
    result.normal = DecodeNormal(v.normal);
    result.uv0 = vec2(scast<float>(v.uv0[0]) * uvDequant,
                      scast<float>(v.uv0[1]) * uvDequant);
    result.uv1 = vec2(scast<float>(v.uv1[0]) * uvDequant,
                      scast<float>(v.uv1[1]) * uvDequant);
    return result;
}


enum class MetroBodyPart : size_t {
    Invalid     = 0,
    Generic     = 1,
    Head        = 2,
    Stomach     = 3,
    Leftarm     = 4,
    Rightarm    = 5,
    Leftleg     = 6,
    Rightleg    = 7,
    Chest       = 8,
    Gear        = 9
};

struct MetroBone {
    static const size_t InvalidIdx = ~0;

    size_t          name;
    size_t          parentName;
    quat            bindQ;
    vec3            bindT;
    MetroBodyPart   bodyPart;
};
