#include "MetroSkeleton.h"
#include "hashing.h"

enum SkeletonChunks : size_t {
    SC_Bones        = 0x00000001,
    SC_BonesNames   = 0x00000002,
};


PACKED_STRUCT_BEGIN
struct SkeletonHeader {
    uint32_t    version;        // always 21 ???
    uint32_t    bonesCRC;
    uint8_t     unknown0[21];
    uint32_t    numBones;
} PACKED_STRUCT_END;


MetroSkeleton::MetroSkeleton() {

}
MetroSkeleton::~MetroSkeleton() {

}

bool MetroSkeleton::LoadFromData(const uint8_t* data, const size_t dataLength) {
    bool result = false;

    MemStream stream(data, dataLength);

    const size_t version = stream.ReadTyped<uint8_t>();
    if (version == 0x14) {
        this->ReadSubChunks(stream);
        result = !mBones.empty();
    }

    return result;
}

size_t MetroSkeleton::GetNumBones() const {
    return mBones.size();
}

const quat& MetroSkeleton::GetBoneRotation(const size_t idx) const {
    return mBones[idx].bindQ;
}

const vec3& MetroSkeleton::GetBonePosition(const size_t idx) const {
    return mBones[idx].bindT;
}

mat4 MetroSkeleton::GetBoneTransform(const size_t idx) const {
    mat4 result = MatFromQuat(mBones[idx].bindQ);
    result[3] = vec4(mBones[idx].bindT, 1.0f);
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

    const size_t parentName = mBones[idx].parentName;
    for (size_t i = 0; i < mBones.size(); ++i) {
        if (mBones[i].name == parentName) {
            result = i;
            break;
        }
    }

    return result;
}

const CharString& MetroSkeleton::GetBoneName(const size_t idx) const {
    return mStrings[mBones[idx].name];
}



void MetroSkeleton::ReadSubChunks(MemStream& stream) {
    while (!stream.Ended()) {
        const size_t chunkId = stream.ReadTyped<uint32_t>();
        const size_t chunkSize = stream.ReadTyped<uint32_t>();
        const size_t chunkEnd = stream.GetCursor() + chunkSize;

        switch (chunkId) {
            case SC_Bones: {
                SkeletonHeader header;
                stream.ReadStruct(header);

                mBones.resize(header.numBones);
                for (MetroBone& bone : mBones) {
                    bone.name = stream.ReadTyped<uint32_t>();
                    bone.parentName = stream.ReadTyped<uint32_t>();
                    stream.ReadStruct(bone.bindQ);
                    stream.ReadStruct(bone.bindT);
                    bone.bodyPart = scast<MetroBodyPart>(stream.ReadTyped<uint16_t>());
                }
            } break;

            case SC_BonesNames: {
                const size_t numStrings = stream.ReadTyped<uint32_t>();
                mStrings.resize(numStrings);

                for (CharString& s : mStrings) {
                    s = stream.ReadStringZ();
                }
            } break;
        }

        stream.SetCursor(chunkEnd);
    }
}
