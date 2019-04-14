#include "MetroMotion.h"

PACKED_STRUCT_BEGIN
struct MotionDataHeader {   // size = 48 bytes
    Bitset256   bonesMask;
    uint16_t    numLocators;
    uint16_t    numXforms;
    uint32_t    totalSize;
    uint64_t    unknown_0;
} PACKED_STRUCT_END;

enum MotionChunks {
    MC_HeaderChunk  = 0x00000000,
    MC_InfoChunk    = 0x00000001,
    MC_DataChunk    = 0x00000009,
};

MetroMotion::MetroMotion(const CharString& name)
    : mName(name)
    // header
    , mVersion(0)
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

bool MetroMotion::LoadFromData(MemStream& stream) {
    bool result = false;

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
                mNumLocators = stream.ReadTyped<uint16_t>();
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

    result = this->LoadInternal();

    mMotionsData.resize(0);

    return result;
}

void MetroMotion::SetPath(const CharString& path) {
    mPath = path;
}

const CharString& MetroMotion::GetName() const {
    return mName;
}

const CharString& MetroMotion::GetPath() const {
    return mPath;
}

size_t MetroMotion::GetBonesCRC() const {
    return mBonesCRC;
}

size_t MetroMotion::GetNumBones() const {
    return mNumBones;
}

size_t MetroMotion::GetNumLocators() const {
    return mNumLocators;
}

size_t MetroMotion::GetNumKeys() const {
    return mNumKeys;
}

float MetroMotion::GetMotionTimeInSeconds() const {
    return scast<float>(mNumKeys) / 30.0f;
}

bool MetroMotion::IsBoneAnimated(const size_t boneIdx) const {
    const MotionDataHeader* hdr = rcast<const MotionDataHeader*>(mMotionsData.data());

    const bool bonePresent = mAffectedBones.IsPresent(boneIdx);
    const bool motionHasThisBone = hdr->bonesMask.IsPresent(boneIdx);

    return bonePresent && motionHasThisBone;
}

quat MetroMotion::GetBoneRotation(const size_t boneIdx, const size_t key) const {
    quat result(1.0f, 0.0f, 0.0f, 0.0f);

    const AttributeCurve& curve = mBonesRotations[boneIdx];
    if (!curve.points.empty()) {
        if (curve.points.size() == 1) { // constant value
            result = *rcast<const quat*>(&curve.points.front().value);
        } else {
            const float timing = scast<float>(key) / 30.0f;

            const size_t numPoints = curve.points.size();
            size_t pointA = numPoints, pointB = numPoints;
            for (size_t i = 0; i < numPoints; ++i) {
                const auto& p = curve.points[i];
                if (p.time >= timing) {
                    pointB = i;
                    break;
                }
            }

            if (pointB == numPoints) {
                pointB--;
                pointA = pointB;
            } else if (pointB == 0) {
                pointA = 0;
            } else {
                pointA = pointB - 1;
            }

            const auto& pA = curve.points[pointA];

            if (pointA == pointB) {
                result = *rcast<const quat*>(&pA.value);
            } else {
                const auto& pB = curve.points[pointB];
                const float t = (timing - pA.time) / (pB.time - pA.time);

                result = QuatSlerp(*rcast<const quat*>(&pA.value), *rcast<const quat*>(&pB.value), t);
            }
        }
    }

    return result;
}

vec3 MetroMotion::GetBonePosition(const size_t boneIdx, const size_t key) const {
    vec3 result(0.0f);

    const AttributeCurve& curve = mBonesPositions[boneIdx];
    if (!curve.points.empty()) {
        if (curve.points.size() == 1) { // constant value
            result = *rcast<const vec3*>(&curve.points.front().value);
        } else {
            const float timing = scast<float>(key) / 30.0f;

            const size_t numPoints = curve.points.size();
            size_t pointA = numPoints, pointB = numPoints;
            for (size_t i = 0; i < numPoints; ++i) {
                const auto& p = curve.points[i];
                if (p.time >= timing) {
                    pointB = i;
                    break;
                }
            }

            if (pointB == numPoints) {
                pointB--;
                pointA = pointB;
            } else if (pointB == 0) {
                pointA = 0;
            } else {
                pointA = pointB - 1;
            }

            const auto& pA = curve.points[pointA];

            if (pointA == pointB) {
                result = pA.value;
            } else {
                const auto& pB = curve.points[pointB];
                const float t = (timing - pA.time) / (pB.time - pA.time);

                result = Lerp(*rcast<const vec3*>(&pA.value), *rcast<const vec3*>(&pB.value), t);
            }
        }
    }

    return result;
}



enum class AttribCurveType : uint8_t {
    Invalid         = 0,
    Uncompressed    = 1,    // raw float values
    OneValue        = 2,    // constant value, no curve
    Unknown_3       = 3,
    CompressedPos   = 4,    // quantized position, scale + offset + u16 values
    CompressedQuat  = 5,    // quantized quaternion (xyz, we restore w), s16_snorm values
    Unknown_6       = 6,
    Empty           = 7     // no curve, why not just filter it out with mask ???
};


bool MetroMotion::LoadInternal() {
    bool result = false;

    if (!mMotionsData.empty() && mMotionsData.size() > mMotionsOffsetsSize) {
        const uint8_t* ptr = mMotionsData.data();

        const MotionDataHeader* hdr = rcast<const MotionDataHeader*>(ptr);
        const uint32_t* offsetsTable = rcast<const uint32_t*>(ptr + sizeof(MotionDataHeader));

        mBonesRotations.resize(mNumBones);
        mBonesPositions.resize(mNumBones);

        for (size_t boneIdx = 0, flatIdx = 0; boneIdx < mNumBones; ++boneIdx) {
            const bool bonePresent = mAffectedBones.IsPresent(boneIdx);
            const bool motionHasThisBone = hdr->bonesMask.IsPresent(boneIdx);

            if (bonePresent && motionHasThisBone) {
                const size_t offsetQ = offsetsTable[flatIdx * 3 + 0];
                const size_t offsetT = offsetsTable[flatIdx * 3 + 1];

                this->ReadAttributeCurve(ptr + offsetQ, mBonesRotations[boneIdx], 4);
                this->ReadAttributeCurve(ptr + offsetT, mBonesPositions[boneIdx], 3);

                ++flatIdx;
            }
        }

        result = true;
    }

    return result;
}

void MetroMotion::ReadAttributeCurve(const uint8_t* curveData, AttributeCurve& curve, const size_t attribSize) {
    const uint32_t curveHeader = *rcast<const uint32_t*>(curveData);

    const size_t numPoints = scast<size_t>(curveHeader & 0xFFFF);
    const AttribCurveType ctype = scast<AttribCurveType>((curveHeader >> 16) & 0xF);

    assert(attribSize <= 4);

    curveData += 4;

    if (ctype == AttribCurveType::Empty) {
        curve.points.clear();
    } else if (ctype == AttribCurveType::OneValue) {
        curve.points.resize(1);
        auto& p = curve.points.back();
        p.time = 0.0f;
        memcpy(&p.value, curveData, attribSize * sizeof(float));

        p.value = MetroSwizzle(p.value);
    } else if (ctype == AttribCurveType::Unknown_3 || ctype == AttribCurveType::Unknown_6) {
        assert(false);
    } else {
        curve.points.resize(numPoints);

        switch (ctype) {
            case AttribCurveType::Uncompressed: {
                const float* timingsPtr = rcast<const float*>(curveData);
                const float* valuesPtr = rcast<const float*>(curveData + (numPoints * sizeof(float)));

                for (auto& p : curve.points) {
                    p.time = *timingsPtr;
                    memcpy(&p.value, valuesPtr, attribSize * sizeof(float));

                    p.value = MetroSwizzle(p.value);

                    timingsPtr++;
                    valuesPtr += attribSize;
                }
            } break;

            case AttribCurveType::CompressedPos: {
                const float timingScale = 1.0f / *rcast<const float*>(curveData);
                curveData += 4;

                const vec3 scale = rcast<const vec3*>(curveData)[0];
                const vec3 offset = rcast<const vec3*>(curveData)[1];
                curveData += sizeof(vec3[2]);

                const uint16_t* timingsPtr = rcast<const uint16_t*>(curveData);
                const uint16_t* valuesPtr = rcast<const uint16_t*>(curveData + (numPoints * sizeof(uint16_t)));

                for (auto& p : curve.points) {
                    p.time = scast<float>(*timingsPtr) * timingScale;

                    p.value.x = scast<float>(valuesPtr[0]) * scale.x + offset.x;
                    p.value.y = scast<float>(valuesPtr[1]) * scale.y + offset.y;
                    p.value.z = scast<float>(valuesPtr[2]) * scale.z + offset.z;

                    p.value = MetroSwizzle(p.value);

                    timingsPtr++;
                    valuesPtr += 3;
                }
            } break;

            case AttribCurveType::CompressedQuat: {
                const float normFactor = 0.0000215805f;
                //const float normFactor = 1.0f / (65535.0f / sqrt(2.0f));

                const float timingScale = 1.0f / *rcast<const float*>(curveData);
                curveData += 4;

                const uint16_t* timingsPtr = rcast<const uint16_t*>(curveData);
                const int16_t* valuesPtr = rcast<const int16_t*>(curveData + (numPoints * sizeof(uint16_t)));

                for (auto& p : curve.points) {
                    p.time = scast<float>(*timingsPtr) * timingScale;

                    const int permutation = (valuesPtr[1] & 1) | (2 * (valuesPtr[0] & 1));
                    const int wsign = (valuesPtr[2] & 1);

                    const float qx = scast<float>(valuesPtr[0]) * normFactor;
                    const float qy = scast<float>(valuesPtr[1]) * normFactor;
                    const float qz = scast<float>(valuesPtr[2]) * normFactor;
                    const float t = 1.0f - (qx * qx) - (qy * qy) - (qz * qz);
                    const float qw = (t < 0.0f) ? 0.0f : (wsign ? -std::sqrtf(t) : std::sqrtf(t));

                    switch (permutation) {
                        case 0: p.value = vec4(qw, qx, qy, qz); break;
                        case 1: p.value = vec4(qx, qw, qy, qz); break;
                        case 2: p.value = vec4(qx, qy, qw, qz); break;
                        case 3: p.value = vec4(qx, qy, qz, qw); break;
                    }

                    p.value = MetroSwizzle(p.value);

                    //*rcast<quat*>(&p.value) = Normalize(*rcast<quat*>(&p.value));

                    timingsPtr++;
                    valuesPtr += 3;
                }
            } break;
        }
    }
}
