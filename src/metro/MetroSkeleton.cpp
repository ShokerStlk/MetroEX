#include "MetroSkeleton.h"
#include "MetroBinArchive.h"
#include "MetroReflection.h"

enum SkeletonChunks : size_t {
    SC_SelfData             = 0x00000001,
    SC_StringsDictionary    = 0x00000002,
};


void ParentMapped::Serialize(MetroReflectionReader& reader) {
    METRO_READ_MEMBER(reader, parent_bone);
    METRO_READ_MEMBER(reader, self_bone);
    METRO_READ_MEMBER(reader, q);
    METRO_READ_MEMBER(reader, t);
    METRO_READ_MEMBER(reader, s);
}

void MetroBone::Serialize(MetroReflectionReader& reader) {
    METRO_READ_MEMBER(reader, name);
    METRO_READ_MEMBER(reader, parent);
    METRO_READ_MEMBER(reader, q);
    METRO_READ_MEMBER(reader, t);
    METRO_READ_MEMBER(reader, bp);
    METRO_READ_MEMBER(reader, bpf); // if skeleton version > 18
}


MetroSkeleton::MetroSkeleton() {

}
MetroSkeleton::~MetroSkeleton() {

}

bool MetroSkeleton::LoadFromData(MemStream& stream) {
    bool result = false;

    MetroBinArchive bin(kEmptyString, stream, MetroBinArchive::kHeaderDoAutoSearch);
    MetroReflectionReader reader = bin.ReflectionReader();
    if (reader.Good()) {
        this->DeserializeSelf(reader);
        result = !this->bones.empty();
    }

    return result;
}

size_t MetroSkeleton::GetNumBones() const {
    return this->bones.size();
}

const quat& MetroSkeleton::GetBoneRotation(const size_t idx) const {
    return this->bones[idx].q;
}

const vec3& MetroSkeleton::GetBonePosition(const size_t idx) const {
    return this->bones[idx].t;
}

mat4 MetroSkeleton::GetBoneTransform(const size_t idx) const {
    mat4 result = MatFromQuat(this->bones[idx].q);
    result[3] = vec4(this->bones[idx].t, 1.0f);

    return result;
}

mat4 MetroSkeleton::GetBoneFullTransform(const size_t idx) const {
    const size_t parentIdx = this->GetBoneParentIdx(idx);
    if (parentIdx == MetroBone::InvalidIdx) {
        return this->GetBoneTransform(idx);
    } else {
        return this->GetBoneFullTransform(parentIdx) * this->GetBoneTransform(idx);
    }
}

const size_t MetroSkeleton::GetBoneParentIdx(const size_t idx) const {
    size_t result = MetroBone::InvalidIdx;

    const CharString& parentName = this->bones[idx].parent;
    for (size_t i = 0; i < this->bones.size(); ++i) {
        if (this->bones[i].name == parentName) {
            result = i;
            break;
        }
    }

    return result;
}

const CharString& MetroSkeleton::GetBoneName(const size_t idx) const {
    return this->bones[idx].name;
}

const CharString& MetroSkeleton::GetMotionsStr() const {
    return this->motions;
}


void MetroSkeleton::DeserializeSelf(MetroReflectionReader& reader) {
    MetroReflectionReader skeletonReader = reader.OpenSection("skeleton");
    if (skeletonReader.Good()) {
        METRO_READ_MEMBER(skeletonReader, ver);
        METRO_READ_MEMBER(skeletonReader, crc);
        // if version < 15 - read facefx (CharString)
        METRO_READ_MEMBER(skeletonReader, pfnn); // if version > 16
        if (this->ver > 20) {
            METRO_READ_MEMBER(skeletonReader, has_as); // if version > 20
        }
        METRO_READ_MEMBER(skeletonReader, motions);
        METRO_READ_MEMBER(skeletonReader, source_info); // if version > 12
        METRO_READ_MEMBER(skeletonReader, parent_skeleton); // if version > 13
        METRO_READ_STRUCT_ARRAY_MEMBER(skeletonReader, parent_bone_maps); // if version > 13
        METRO_READ_STRUCT_ARRAY_MEMBER(skeletonReader, bones);
    }
    reader.CloseSection(skeletonReader);

    //#NOTE_SK: fix-up bones transforms by swizzling them back
    for (auto& b : bones) {
        b.q = MetroSwizzle(b.q);
        b.t = MetroSwizzle(b.t);
    }

    //#TODO_SK:
    // locators
    // aux_bones
    // procedural
}
