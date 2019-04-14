#pragma once
#include "mycommon.h"
#include "mymath.h"

class MetroReflectionReader;

struct MetroTextureInfo {
    enum class TextureType : uint8_t {
        Diffuse             = 0,
        Detail_diffuse      = 1,
        Cubemap             = 2,
        Cubemap_hdr         = 3,        //#NOTE_SK: has sph_coefs (fp32_array), seems to be SH1
        Terrain             = 4,
        Bumpmap             = 5,
        Diffuse_va          = 6,
        Arbitrary4          = 7,
        Normalmap           = 8,
        Normalmap_alpha     = 9,
        Normalmap_detail    = 0x0A,
        Unknown_01          = 0x0B,
        Unknown_has_lum     = 0x0C,     //#NOTE_SK: has lum (u8_array)
        Instance            = 0x40
    };

    enum class PixelFormat : uint32_t {
        Invalid                 = ~0u,
        BC1                     = 0,
        BC3                     = 1,
        RGBA8_UNORM             = 2,
        RGBA8_SNORM             = 3,
        BC6H                    = 4,
        BC7                     = 5,
        RG8_UNORM               = 6,
        RG8_SNORM               = 7,
        DEPTH_32F_S8            = 8,
        DEPTH_32F               = 9,
        R32_F                   = 10,
        RGBA16_F                = 11,
        RG16_F                  = 12,
        RGBA16_U                = 13,
        R8_UNORM                = 14,
        R8_U                    = 15,
        RGB10_UNORM_A2_UNORM    = 16,
        RGB10_SNORM_A2_UNORM    = 17,   //#NOTE_SK: this format is unavailable on PC for DirectX, tho available on Vulkan and XBox
        R11G11B10_F             = 18,
        R16_UNORM               = 19,
        R32_U                   = 20,
        RGBA32_F                = 21,
        PPS                     = 22,   //#NOTE_SK: have no clue wtf is this
        BGRA8_UNORM             = 23
    };

    enum class DisplType : uint8_t {
        Simple   = 0,
        Parallax = 1,
        Displace = 2
    };

    CharString      name;           //#NOTE_SK: no verification!
    uint8_t         flags;          //#NOTE_SK: no verification!
    uint32_t        type;           //#NOTE_SK: TextureType enum
    uint8_t         texture_type;   //#NOTE_SK: seems to be same as type
    RefString       source_name;
    vec4            surf_xform;
    uint32_t        format;         //#NOTE_SK: PixelFormat enum
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
    uint8_t         displ_type;     //#NOTE_SK: DisplType enum
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
