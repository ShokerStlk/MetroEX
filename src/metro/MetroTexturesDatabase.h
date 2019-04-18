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
        Normalmap_detail    = 10,
        Unknown_01          = 11,
        Unknown_has_lum     = 12,     //#NOTE_SK: has lum (u8_array)
        Instance            = 64
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

    CharString          name;           //#NOTE_SK: transient member !!!

    uint32_t            type;           //#NOTE_SK: TextureType enum
    uint8_t             texture_type;   //#NOTE_SK: seems to be same as type
    CharString          source_name;
    vec4                surf_xform;
    uint32_t            format;         //#NOTE_SK: PixelFormat enum
    uint32_t            width;
    uint32_t            height;
    bool                animated;
    bool                draft;
    bool                override_avg_color;
    color4f             avg_color;
    CharString          shader_name;    // choose
    CharString          gamemtl_name;   // choose
    uint32_t            priority;
    bool                streamable;
    float               bump_height;
    uint8_t             displ_type;     //#NOTE_SK: DisplType enum
    float               displ_height;
    float               parallax_height_mul;
    bool                mipmapped;
    float               reflectivity;
    bool                treat_as_metal;
    CharString          det_name;       // choose
    float               det_scale_u;
    float               det_scale_v;
    float               det_intensity;
    color4f             aux_params;
    color4f             aux_params_1;
    // !!! Optional fields !!!
    MyArray<uint8_t>    lum;
    MyArray<float>      sph_coefs;
    ///////////////////////////
    CharString          bump_name;      // choose
    CharString          aux0_name;      // choose
    CharString          aux1_name;      // choose
    CharString          aux2_name;      // choose
    CharString          aux3_name;      // choose
    CharString          aux4_name;      // choose
    CharString          aux5_name;      // choose
    CharString          aux6_name;      // choose
    CharString          aux7_name;      // choose

    void Serialize(MetroReflectionReader& s);
};

struct MetroTextureAliasInfo {
    CharString  src;
    CharString  dst;

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
    MyDict<HashString, HashString>        mAliases;
};
