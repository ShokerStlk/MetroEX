#pragma once
#include "mycommon.h"
#include "mymath.h"

class MetroReflectionReader;

struct MetroMaterialsCommonOptions {
    //0
    CharString  tex_name_0;
    uint16_t    tex_frame_width_0;
    uint16_t    tex_frame_height_0;
    //1
    CharString  tex_name_1;
    uint16_t    tex_frame_width_1;
    uint16_t    tex_frame_height_1;
    //2
    CharString  tex_name_2;
    uint16_t    tex_frame_width_2;
    uint16_t    tex_frame_height_2;
    //3
    CharString  tex_name_3;
    uint16_t    tex_frame_width_3;
    uint16_t    tex_frame_height_3;

    void Serialize(MetroReflectionReader& reader);
};

struct MetroVehicleMaterial {
    struct SurfaceDesc {
        uint8_t     type;
        CharString  name;

        void Serialize(MetroReflectionReader& reader);
    };

    MyArray<SurfaceDesc>    tire_types;
    MyArray<SurfaceDesc>    surface_types;
    MyArray<float>          coefs;

    void Serialize(MetroReflectionReader& reader);
};

struct MetroMaterial {
    CharString  name;
    float       ph_friction;
    float       ph_damping;
    float       ph_spring;
    float       cl_dmg_factor;
    float       vs_transp_factor;
    uint32_t    dbg_color;
    uint8_t     veh_surf_type;

    void Serialize(MetroReflectionReader& reader);
};

class MetroMaterialsDatabase {
public:
    MetroMaterialsDatabase();
    ~MetroMaterialsDatabase();

    bool    LoadFromData(MemStream& stream);

private:
    MetroMaterialsCommonOptions common;
    MetroVehicleMaterial        vehicle_materials;
    MyArray<MetroMaterial>      materials;
};
