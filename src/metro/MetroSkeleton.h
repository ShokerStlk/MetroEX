#pragma once
#include "MetroTypes.h"

class MetroSkeleton {
public:
    MetroSkeleton();
    ~MetroSkeleton();

    bool                LoadFromData(const uint8_t* data, const size_t dataLength);

    size_t              GetNumBones() const;
    const quat&         GetBoneRotation(const size_t idx) const;
    const vec3&         GetBonePosition(const size_t idx) const;
    mat4                GetBoneTransform(const size_t idx) const;
    mat4                GetBoneFullTransform(const size_t idx) const;
    const size_t        GetBoneParentIdx(const size_t idx) const;
    const CharString&   GetBoneName(const size_t idx) const;

private:
    void                ReadSubChunks(MemStream& stream);

private:
    Array<MetroBone>    mBones;
    StringArray         mStrings;
};
