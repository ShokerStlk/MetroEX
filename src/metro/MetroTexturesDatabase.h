#pragma once
#include "mycommon.h"
#include "mymath.h"

class MetroReflectionReader;

struct MetroTextureInfo {
    enum class TextureType : uint8_t {
        Diffuse             = 0,
        Detail_diffuse      = 1,
        Cubemap             = 2,
        Cubemap_hdr         = 3,        // has sph_coefs (fp32_array), seems to be SH1
        Terrain             = 4,
        Bumpmap             = 5,
        Diffuse_va          = 6,
        Arbitrary4          = 7,
        Normalmap           = 8,
        Normalmap_alpha     = 9,
        Normalmap_detail    = 0x0A,
        Unknown_01          = 0x0B,
        Unknown_has_lum     = 0x0C,     // has lum (u8_array)
        Instance            = 0x40
    };

    enum class DisplType : uint8_t {
        Simple   = 0,
        Parallax = 1,
        Displace = 2
    };

    CharString      name;           // no verification!
    uint8_t         flags;          // no verification!
    uint32_t        type;
    uint8_t         texture_type;
    CharString      source_name;
    vec4            surf_xform;
    uint32_t        format;
    uint32_t        width;
    uint32_t        height;
    bool            animated;
    bool            draft;
    bool            override_avg_color;
    color4f         avg_color;
    CharString      shader_name;    // choose
    CharString      gamemtl_name;   // choose
    uint32_t        priority;
    bool            streamable;
    float           bump_height;
    uint8_t         displ_type;
    float           displ_height;
    float           parallax_height_mul;
    bool            mipmapped;
    float           reflectivity;
    bool            treat_as_metal;
    CharString      det_name;       // choose
    float           det_scale_u;
    float           det_scale_v;
    float           det_intensity;
    color4f         aux_params;
    color4f         aux_params_1;
    // !!! Optional fields !!!
    Array<uint8_t>  lum;
    Array<float>    sph_coefs;
    ///////////////////////////
    CharString      bump_name;      // choose
    CharString      aux0_name;      // choose
    CharString      aux1_name;      // choose
    CharString      aux2_name;      // choose
    CharString      aux3_name;      // choose
    CharString      aux4_name;      // choose
    CharString      aux5_name;      // choose
    CharString      aux6_name;      // choose
    CharString      aux7_name;      // choose

    void Serialize(MetroReflectionReader& s);
};

class MetroTexturesDatabase {
public:
    MetroTexturesDatabase();
    ~MetroTexturesDatabase();

    bool                    LoadFromData(const void* data, const size_t length);
    bool                    LoadAliasesFromData(const void* data, const size_t length);

    const MetroTextureInfo* GetInfoByName(const HashString& name) const;
    const HashString&       GetAlias(const HashString& name) const;
    const CharString&       GetSourceName(const HashString& name) const;
    const CharString&       GetBumpName(const HashString& name) const;

private:
    Array<MetroTextureInfo>             mPool;
    Dict<HashString, MetroTextureInfo*> mDatabase;
    Dict<HashString, MetroTextureInfo*> mDiffuseTextures;
    Dict<HashString, MetroTextureInfo*> mNormalmapTextures;

    Dict<HashString, HashString>        mAliases;
};
