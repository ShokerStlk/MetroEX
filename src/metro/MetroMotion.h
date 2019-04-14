#pragma once
#include "MetroTypes.h"

struct AttributeCurve {
    struct AttribPoint {
        float   time;
        vec4    value;
    };

    MyArray<AttribPoint> points;
};

class MetroMotion {
public:
    MetroMotion(const CharString& name = "");
    ~MetroMotion();

    bool                    LoadFromData(MemStream& stream);
    void                    SetPath(const CharString& path);
    const CharString&       GetName() const;
    const CharString&       GetPath() const;
    

    size_t                  GetBonesCRC() const;
    size_t                  GetNumBones() const;
    size_t                  GetNumLocators() const;
    size_t                  GetNumKeys() const;
    float                   GetMotionTimeInSeconds() const;

    bool                    IsBoneAnimated(const size_t boneIdx) const;
    quat                    GetBoneRotation(const size_t boneIdx, const size_t key) const;
    vec3                    GetBonePosition(const size_t boneIdx, const size_t key) const;

//private:
    bool                    LoadInternal();
    void                    ReadAttributeCurve(const uint8_t* curveData, AttributeCurve& curve, const size_t attribSize);

//private:
    CharString              mName;
    CharString              mPath;

    // header
    size_t                  mVersion;
    size_t                  mBonesCRC;
    size_t                  mNumBones;
    size_t                  mNumLocators;
    // info
    size_t                  mFlags;
    float                   mSpeed;
    float                   mAccrue;
    float                   mFalloff;
    size_t                  mNumKeys;
    size_t                  mJumpFrame;
    size_t                  mLandFrame;
    Bitset256               mAffectedBones;
    size_t                  mMotionsDataSize;
    size_t                  mMotionsOffsetsSize;
    Bitset256               mHighQualityBones;
    // data
    BytesArray              mMotionsData;
    // curves
    MyArray<AttributeCurve> mBonesRotations;
    MyArray<AttributeCurve> mBonesPositions;
};
