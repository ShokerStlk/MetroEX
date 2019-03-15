#pragma once
#include "MetroTypes.h"

class MetroReflectionReader;

struct ParentMapped {   // 48 bytes
    RefString   parent_bone;
    RefString   self_bone;
    quat        q;
    vec3        t;
    vec3        s;

    void Serialize(MetroReflectionReader& s);
};

struct MetroBone {      // 38 bytes
    static const size_t InvalidIdx = ~0;

    RefString   name;
    RefString   parent;
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

    bool                LoadFromData(const uint8_t* data, const size_t dataLength);

    size_t              GetNumBones() const;
    const quat&         GetBoneRotation(const size_t idx) const;
    const vec3&         GetBonePosition(const size_t idx) const;
    mat4                GetBoneTransform(const size_t idx) const;
    mat4                GetBoneFullTransform(const size_t idx) const;
    const size_t        GetBoneParentIdx(const size_t idx) const;
    const CharString&   GetBoneName(const size_t idx) const;

private:
    void                DeserializeSelf(MemStream& stream, const uint8_t flags);
    void                ReadSubChunks(MemStream& stream, const uint8_t flags);

private:
    uint32_t            ver;                // always 21 ???
    uint32_t            crc;
    RefString           pfnn;               // string
    bool                has_as;
    RefString           motions;            // string
    RefString           source_info;        // string
    RefString           parent_skeleton;    // string
    MyArray<ParentMapped> parent_bone_maps;
    MyArray<MetroBone>    bones;

    StringArray         mStringsDict;
};
