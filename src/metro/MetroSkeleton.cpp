#include "MetroSkeleton.h"
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

bool MetroSkeleton::LoadFromData(const uint8_t* data, const size_t dataLength) {
    bool result = false;

    MemStream stream(data, dataLength);

    const uint8_t flags = stream.ReadTyped<uint8_t>();
    const bool hasDebugInfo = TestBit(flags, MetroBinFlags::HasDebugInfo);

    if (hasDebugInfo) {
        this->DeserializeSelf(stream, flags);
    } else {
        this->ReadSubChunks(stream, flags);
    }

    result = !this->bones.empty();

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

    const RefString& parentName = this->bones[idx].parent;
    for (size_t i = 0; i < this->bones.size(); ++i) {
        if (this->bones[i].name == parentName) {
            result = i;
            break;
        }
    }

    return result;
}

const CharString& MetroSkeleton::GetBoneName(const size_t idx) const {
    const RefString& name = this->bones[idx].name;
    return name.IsValidRef() ? mStringsDict[name.ref] : name.str;
}

const CharString& MetroSkeleton::GetMotionsStr() const {
    return this->motions.IsValidRef() ? mStringsDict[this->motions.ref] : this->motions.str;
}


void MetroSkeleton::DeserializeSelf(MemStream& stream, const uint8_t flags) {
    const bool hasDebugInfo = TestBit(flags, MetroBinFlags::HasDebugInfo);
    const bool stringsAsRef = TestBit(flags, MetroBinFlags::RefStrings);

    MetroReflectionReader s(stream, hasDebugInfo, stringsAsRef);

    if (hasDebugInfo) {
        //#NOTE_SK: temporary hack to skip section header
        const CharString& sectionName = s.BeginSection();
        assert(sectionName == "skeleton");
    }

    METRO_READ_MEMBER(s, ver);
    METRO_READ_MEMBER(s, crc);
    // if version < 15 - read facefx (RefString)
    METRO_READ_MEMBER(s, pfnn); // if version > 16
    if (this->ver > 20) {
        METRO_READ_MEMBER(s, has_as); // if version > 20
    }
    METRO_READ_MEMBER(s, motions);
    METRO_READ_MEMBER(s, source_info); // if version > 12
    METRO_READ_MEMBER(s, parent_skeleton); // if version > 13
    METRO_READ_STRUCT_ARRAY_MEMBER(s, parent_bone_maps); // if version > 13
    METRO_READ_STRUCT_ARRAY_MEMBER(s, bones);

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

void MetroSkeleton::ReadSubChunks(MemStream& stream, const uint8_t flags) {
    while (!stream.Ended()) {
        const size_t chunkId = stream.ReadTyped<uint32_t>();
        const size_t chunkSize = stream.ReadTyped<uint32_t>();
        const size_t chunkEnd = stream.GetCursor() + chunkSize;

        switch (chunkId) {
            case SC_SelfData: {
                this->DeserializeSelf(stream.Substream(chunkSize), flags);
            } break;

            case SC_StringsDictionary: {
                const size_t numStrings = stream.ReadTyped<uint32_t>();
                mStringsDict.resize(numStrings);

                for (CharString& s : mStringsDict) {
                    s = stream.ReadStringZ();
                }
            } break;
        }

        stream.SetCursor(chunkEnd);
    }
}
