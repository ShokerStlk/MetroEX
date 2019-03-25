#include "MetroMotion.h"

enum MotionChunks {
    MC_HeaderChunk  = 0x00000000,
    MC_InfoChunk    = 0x00000001,
    MC_DataChunk    = 0x00000009,
};

MetroMotion::MetroMotion()
    // header
    : mVersion(0)
    , mBonesCRC(0)
    , mNumBones(0)
    // info
    , mFlags(0)
    , mSpeed(0.0f)
    , mAccrue(0.0f)
    , mFalloff(0.0f)
    , mNumKeys(0)
    , mJumpFrame(0)
    , mLandFrame(0)
    , mAffectedBones()
    , mMotionsDataSize(0)
    , mMotionsOffsetsSize(0)
    , mHighQualityBones()
{

}
MetroMotion::~MetroMotion() {

}

bool MetroMotion::LoadFromData(const void* data, const size_t dataLength) {
    bool result = false;

    MemStream stream(data, dataLength);

    while (!stream.Ended()) {
        const size_t chunkId = stream.ReadTyped<uint32_t>();
        const size_t chunkSize = stream.ReadTyped<uint32_t>();
        const size_t chunkEnd = stream.GetCursor() + chunkSize;

        switch (chunkId) {
            case MC_HeaderChunk: {
                mVersion = stream.ReadTyped<uint32_t>();

                //#TODO_SK: check bones info against skeleton !
                mBonesCRC = stream.ReadTyped<uint32_t>();
                mNumBones = stream.ReadTyped<uint16_t>();
            } break;

            case MC_InfoChunk: {
                mFlags = stream.ReadTyped<uint16_t>();

                mSpeed = stream.ReadTyped<float>();
                mAccrue = stream.ReadTyped<float>();
                mFalloff = stream.ReadTyped<float>();

                mNumKeys = stream.ReadTyped<uint32_t>();
                mJumpFrame = stream.ReadTyped<uint16_t>();
                mLandFrame = stream.ReadTyped<uint16_t>();

                stream.ReadStruct(mAffectedBones);

                mMotionsDataSize = stream.ReadTyped<uint32_t>();
                mMotionsOffsetsSize = stream.ReadTyped<uint32_t>();

                stream.ReadStruct(mHighQualityBones);
            } break;

            case MC_DataChunk: {
                assert(chunkSize == mMotionsDataSize);
                if (chunkSize != mMotionsDataSize) {
                    return false;
                }

                mMotionsData.resize(mMotionsDataSize);
                stream.ReadToBuffer(mMotionsData.data(), mMotionsData.size());
            } break;
        }

        stream.SetCursor(chunkEnd);
    }

    return result;
}
