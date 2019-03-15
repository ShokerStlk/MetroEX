#pragma once
#include "MetroTypes.h"

class VFXReader;
class MetroTexturesDatabase;
class MetroSkeleton;

class MetroModel {
public:
    MetroModel();
    ~MetroModel();

    bool                LoadFromData(const uint8_t* data, const size_t length, VFXReader* vfxReader, const size_t fileIdx);
    bool                SaveAsOBJ(const fs::path& filePath, VFXReader* vfxReader, MetroTexturesDatabase* database);
    bool                SaveAsFBX(const fs::path& filePath, VFXReader* vfxReader, MetroTexturesDatabase* database);

    const AABBox&       GetBBox() const;
    const vec4&         GetBSphere() const;
    size_t              GetNumMeshes() const;
    const MetroMesh*    GetMesh(const size_t idx) const;

private:
    void                ReadSubChunks(MemStream& stream);
    void                LoadLinkedMeshes(const StringArray& links);

private:
    AABBox              mBBox;
    vec4                mBSphere;
    MyArray<MetroMesh*>   mMeshes;
    MetroSkeleton*      mSkeleton;

    // these are temp pointers, invalid after loading
    MetroMesh*          mCurrentMesh;
    VFXReader*          mVFXReader;
    size_t              mThisFileIdx;
};
