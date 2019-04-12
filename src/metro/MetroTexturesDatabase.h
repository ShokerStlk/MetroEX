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
    RefString       source_name;
    vec4            surf_xform;
    uint32_t        format;
    uint32_t        width;
    uint32_t        height;
    bool            animated;
    bool            draft;
    bool            override_avg_color;
    color4f         avg_color;
    RefString       shader_name;    // choose
    RefString       gamemtl_name;   // choose
    uint32_t        priority;
    bool            streamable;
    float           bump_height;
    uint8_t         displ_type;
    float           displ_height;
    float           parallax_height_mul;
    bool            mipmapped;
    float           reflectivity;
    bool            treat_as_metal;
    RefString       det_name;       // choose
    float           det_scale_u;
    float           det_scale_v;
    float           det_intensity;
    color4f         aux_params;
    color4f         aux_params_1;
    // !!! Optional fields !!!
    MyArray<uint8_t>  lum;
    MyArray<float>    sph_coefs;
    ///////////////////////////
    RefString       bump_name;      // choose
    RefString       aux0_name;      // choose
    RefString       aux1_name;      // choose
    RefString       aux2_name;      // choose
    RefString       aux3_name;      // choose
    RefString       aux4_name;      // choose
    RefString       aux5_name;      // choose
    RefString       aux6_name;      // choose
    RefString       aux7_name;      // choose

    void Serialize(MetroReflectionReader& s);
};

struct MetroTextureAliasInfo {
    uint32_t    unknown0;   // always 0 ???
    uint32_t    unknown1;   // always 9 ???
    uint8_t     flags;      // always 4 ???
    RefString   name;
    RefString   alias;

    void Serialize(MetroReflectionReader& s);
};

class MetroReflectionReader;

class MetroTexturesDatabase {
public:
    MetroTexturesDatabase();
    ~MetroTexturesDatabase();

    bool                    LoadFromData(MemStream& stream);
    bool                    LoadAliasesFromData(MemStream& stream);

    const MetroTextureInfo* GetInfoByName(const HashString& name) const;
    const HashString&       GetAlias(const HashString& name) const;
    const CharString&       GetSourceName(const HashString& name) const;
    const CharString&       GetBumpName(const HashString& name) const;

private:
    MyArray<MetroTextureInfo>             mPool;
    MyDict<HashString, MetroTextureInfo*> mDatabase;
    MyDict<HashString, MetroTextureInfo*> mDiffuseTextures;
    MyDict<HashString, MetroTextureInfo*> mNormalmapTextures;

    MyDict<HashString, HashString>        mAliases;
};
