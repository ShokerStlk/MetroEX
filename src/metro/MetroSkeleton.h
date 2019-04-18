#pragma once
#include "MetroTypes.h"

class MetroReflectionReader;

struct ParentMapped {   // 48 bytes
    CharString  parent_bone;
    CharString  self_bone;
    quat        q;
    vec3        t;
    vec3        s;

    void Serialize(MetroReflectionReader& s);
};

struct MetroBone {      // 38 bytes
    static const size_t InvalidIdx = kInvalidValue;

    CharString  name;
    CharString  parent;
    quat        q;
    vec3        t;
    uint8_t     bp;
    uint8_t     bpf;

    void Serialize(MetroReflectionReader& s);
};

class MetroSkeleton {
public:
    MetroSkeleton();
    ~MetroSkeleton();

    bool                    LoadFromData(MemStream& stream);

    size_t                  GetNumBones() const;
    const quat&             GetBoneRotation(const size_t idx) const;
    const vec3&             GetBonePosition(const size_t idx) const;
    mat4                    GetBoneTransform(const size_t idx) const;
    mat4                    GetBoneFullTransform(const size_t idx) const;
    const size_t            GetBoneParentIdx(const size_t idx) const;
    const CharString&       GetBoneName(const size_t idx) const;

    const CharString&       GetMotionsStr() const;

private:
    void                    DeserializeSelf(MetroReflectionReader& reader);

private:
    uint32_t                ver;
    uint32_t                crc;
    CharString              pfnn;
    bool                    has_as;
    CharString              motions;
    CharString              source_info;
    CharString              parent_skeleton;
    MyArray<ParentMapped>   parent_bone_maps;
    MyArray<MetroBone>      bones;

    StringArray             mStringsDict;
};
