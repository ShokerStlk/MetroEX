#pragma once
#include "MetroTypes.h"

class VFXReader;
class MetroTexturesDatabase;
class MetroSkeleton;
class MetroMotion;

class MetroModel {
public:
    MetroModel();
    ~MetroModel();

    bool                    LoadFromData(MemStream& stream, VFXReader* vfxReader, const size_t fileIdx);
    bool                    SaveAsOBJ(const fs::path& filePath, VFXReader* vfxReader, MetroTexturesDatabase* database);
    bool                    SaveAsFBX(const fs::path& filePath, VFXReader* vfxReader, MetroTexturesDatabase* database, const bool withAnims);

    bool                    IsAnimated() const;
    const AABBox&           GetBBox() const;
    const vec4&             GetBSphere() const;
    size_t                  GetNumMeshes() const;
    const MetroMesh*        GetMesh(const size_t idx) const;

    const MetroSkeleton*    GetSkeleton() const;
    size_t                  GetNumMotions() const;
    const MetroMotion*      GetMotion(const size_t idx) const;

private:
    void                    ReadSubChunks(MemStream& stream);
    void                    LoadLinkedMeshes(const StringArray& links);
    void                    LoadMotions();

private:
    AABBox                  mBBox;
    vec4                    mBSphere;
    MyArray<MetroMesh*>     mMeshes;
    MetroSkeleton*          mSkeleton;
    MyArray<MetroMotion*>   mMotions;

    // these are temp pointers, invalid after loading
    MetroMesh*              mCurrentMesh;
    VFXReader*              mVFXReader;
    size_t                  mThisFileIdx;
};
