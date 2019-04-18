#include "MetroMaterialsDatabase.h"
#include "MetroReflection.h"
#include "MetroBinArchive.h"

void MetroMaterialsCommonOptions::Serialize(MetroReflectionReader& reader) {
    //0
    METRO_READ_MEMBER_CHOOSE(reader, tex_name_0);
    METRO_READ_MEMBER(reader, tex_frame_width_0);
    METRO_READ_MEMBER(reader, tex_frame_height_0);
    //1
    METRO_READ_MEMBER_CHOOSE(reader, tex_name_1);
    METRO_READ_MEMBER(reader, tex_frame_width_1);
    METRO_READ_MEMBER(reader, tex_frame_height_1);
    //2
    METRO_READ_MEMBER_CHOOSE(reader, tex_name_2);
    METRO_READ_MEMBER(reader, tex_frame_width_2);
    METRO_READ_MEMBER(reader, tex_frame_height_2);
    //3
    METRO_READ_MEMBER_CHOOSE(reader, tex_name_3);
    METRO_READ_MEMBER(reader, tex_frame_width_3);
    METRO_READ_MEMBER(reader, tex_frame_height_3);
}

void MetroVehicleMaterial::SurfaceDesc::Serialize(MetroReflectionReader& reader) {
    METRO_READ_MEMBER(reader, type);
    METRO_READ_MEMBER(reader, name);
}

void MetroVehicleMaterial::Serialize(MetroReflectionReader& reader) {
    METRO_READ_STRUCT_ARRAY_MEMBER(reader, tire_types);
    METRO_READ_STRUCT_ARRAY_MEMBER(reader, surface_types);
    //#NOTE_SK: this is same logic as in game
    //          I know it looks weird but what can I do ?
    coefs.resize(tire_types.size() * surface_types.size());
    if (!coefs.empty()) {
        size_t idx = 0;
        MetroReflectionReader coefsReader = reader.OpenSection("coefs");
        for (const auto& st : surface_types) {
            MetroReflectionReader tireCoefsReader = coefsReader.OpenSection(st.name, true);
            for (const auto& tt : tire_types) {
                tireCoefsReader.VerifyTypeInfo(tt.name, MetroTypeGetAlias<float>());
                tireCoefsReader >> coefs[idx++];
            }
            coefsReader.CloseSection(tireCoefsReader);
        }
        reader.CloseSection(coefsReader);
    }
}

void MetroMaterial::Serialize(MetroReflectionReader& reader) {
    METRO_READ_MEMBER(reader, name);
    METRO_READ_MEMBER(reader, ph_friction);
    METRO_READ_MEMBER(reader, ph_damping);
    METRO_READ_MEMBER(reader, ph_spring);
    METRO_READ_MEMBER(reader, cl_dmg_factor);
    METRO_READ_MEMBER(reader, vs_transp_factor);
    METRO_READ_MEMBER(reader, dbg_color);
    METRO_READ_MEMBER(reader, veh_surf_type);
}


MetroMaterialsDatabase::MetroMaterialsDatabase() {

}
MetroMaterialsDatabase::~MetroMaterialsDatabase() {

}

bool MetroMaterialsDatabase::LoadFromData(MemStream& stream) {
    MetroBinArchive bin("materials.bin", stream, MetroBinArchive::kHeaderDoAutoSearch);
    MetroReflectionReader reader = bin.ReflectionReader();

    METRO_READ_STRUCT_MEMBER(reader, common);
    METRO_READ_STRUCT_MEMBER(reader, vehicle_materials);
    METRO_READ_STRUCT_ARRAY_MEMBER(reader, materials);

    return true;
}
