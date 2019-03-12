#include "MetroSkeleton.h"
#include "MetroReflection.h"

enum SkeletonChunks : size_t {
    SC_SelfData             = 0x00000001,
    SC_StringsDictionary    = 0x00000002,
};


void MetroBone::Serialize(MetroReflectionReader& s) {
    METRO_READ_MEMBER(s, name);
    METRO_READ_MEMBER(s, parent);
    METRO_READ_MEMBER(s, q);
    METRO_READ_MEMBER(s, t);
    METRO_READ_MEMBER(s, bp);
    METRO_READ_MEMBER(s, bpf);
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
        return this->GetBoneTransform(idx) * this->GetBoneFullTransform(parentIdx);
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
    if (name.ref != RefString::InvalidRef) {
        return mStringsDict[name.ref];
    } else {
        return name.str;
    }
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
    METRO_READ_MEMBER(s, pfnn);
    METRO_READ_MEMBER(s, has_as);
    METRO_READ_MEMBER(s, motions);
    METRO_READ_MEMBER(s, source_info);
    METRO_READ_MEMBER(s, parent_skeleton);
    METRO_READ_STRUCT_ARRAY_MEMBER(s, parent_bone_maps);
    METRO_READ_STRUCT_ARRAY_MEMBER(s, bones);

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
