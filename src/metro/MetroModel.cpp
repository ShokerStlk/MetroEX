#include "MetroModel.h"
#include "VFXReader.h"
#include "MetroTexturesDatabase.h"
#include "MetroSkeleton.h"
#include "MetroMotion.h"

#define FBXSDK_NEW_API
#define FBXSDK_SHARED
#include "fbxsdk.h"

#pragma comment (lib, "libfbxsdk.lib")

#include <fstream>
#include <sstream>

enum ModelChunks {
    MC_HeaderChunk          = 0x00000001,
    MC_MaterialsChunk       = 0x00000002,
    MC_VerticesChunk        = 0x00000003,
    MC_FacesChunk           = 0x00000004,
    MC_SkinnedVerticesChunk = 0x00000005,

    MC_SubMeshesChunk       = 0x00000009,

    MC_Lod_1_Chunk          = 0x0000000B,   // 11
    MC_Lod_2_Chunk          = 0x0000000C,   // 12

    MC_MeshesInline         = 0x0000000F,   // 15
    MC_MeshesLinks          = 0x00000010,   // 16

    MC_SkeletonLink         = 0x00000014,   // 20
    MC_SkeletonInline       = 0x00000018,   // 24
};

static const size_t kMetroModelMaxMaterials = 4;


PACKED_STRUCT_BEGIN
struct MdlHeader {          // size = 64
    uint8_t     version;
    uint8_t     type;
    uint16_t    index;
    AABBox      bbox;
    vec4        bsphere;
    uint32_t    checkSum;
    uint32_t    flags;
    vec3        scales;
} PACKED_STRUCT_END;

PACKED_STRUCT_BEGIN
struct MetroOBB {           // size = 60
    mat3    matrix;
    vec3    offset;
    vec3    hsize;
} PACKED_STRUCT_END;



MetroModel::MetroModel()
    : mSkeleton(nullptr)
    , mCurrentMesh(nullptr)
    , mVFXReader(nullptr)
    , mThisFileIdx(MetroFile::InvalidFileIdx)
{
}
MetroModel::~MetroModel() {
    std::for_each(mMeshes.begin(), mMeshes.end(), [](MetroMesh* mesh) { delete mesh; });
    std::for_each(mMotions.begin(), mMotions.end(), [](MetroMotion* motion) { delete motion; });
    MySafeDelete(mSkeleton);
}

bool MetroModel::LoadFromData(const uint8_t* data, const size_t length, VFXReader* vfxReader, const size_t fileIdx) {
    bool result = false;

    mVFXReader = vfxReader;
    mThisFileIdx = fileIdx;

    MemStream stream(data, length);
    this->ReadSubChunks(stream);

    this->LoadMotions();

    result = !mMeshes.empty();

    return result;
}

bool MetroModel::SaveAsOBJ(const fs::path& filePath, VFXReader* vfxReader, MetroTexturesDatabase* database) {
    bool result = false;

    const bool exportWithMats = (vfxReader != nullptr);

    std::ofstream file(filePath, std::ofstream::binary);
    if (file.good()) {
        CharString matName = filePath.filename().string();
        matName[matName.size() - 3] = 'm';
        matName[matName.size() - 2] = 't';
        matName[matName.size() - 1] = 'l';

        std::ostringstream stringBuilder;
        stringBuilder << "# Generated from Metro Exodus model file" << std::endl;
        stringBuilder << "# using MetroEX tool made by iOrange, 2019" << std::endl << std::endl;

        if (exportWithMats) {
            stringBuilder << "mtllib " << matName << std::endl << std::endl;
        }

        size_t lastIdx = 0;
        for (size_t i = 0; i < mMeshes.size(); ++i) {
            const MetroMesh* mesh = mMeshes[i];
            if (!mesh->vertices.empty() && !mesh->faces.empty()) {
                for (const MetroVertex& v : mesh->vertices) {
                    stringBuilder << "v " << v.pos.z << ' ' << v.pos.y << ' ' << v.pos.x << std::endl;
                }
                stringBuilder << "# " << mesh->vertices.size() << " vertices" << std::endl << std::endl;

                for (const MetroVertex& v : mesh->vertices) {
                    stringBuilder << "vt " << v.uv0.x << ' ' << (1.0f - v.uv0.y) << std::endl;
                }
                stringBuilder << "# " << mesh->vertices.size() << " texcoords" << std::endl << std::endl;

                for (const MetroVertex& v : mesh->vertices) {
                    stringBuilder << "vn " << v.normal.z << ' ' << v.normal.y << ' ' << v.normal.x << std::endl;
                }
                stringBuilder << "# " << mesh->vertices.size() << " normals" << std::endl << std::endl;

                stringBuilder << "g Mesh_" << i << std::endl;
                if (exportWithMats) {
                    stringBuilder << "usemtl " << "Material_" << i << std::endl;
                }

                for (const MetroFace& f : mesh->faces) {
                    const size_t a = f.c + lastIdx + 1;
                    const size_t b = f.b + lastIdx + 1;
                    const size_t c = f.a + lastIdx + 1;

                    stringBuilder << "f " << a << '/' << a << '/' << a <<
                                      ' ' << b << '/' << b << '/' << b <<
                                      ' ' << c << '/' << c << '/' << c << std::endl;
                }
                stringBuilder << "# " << mesh->faces.size() << " faces" << std::endl << std::endl;

                lastIdx += mesh->vertices.size();
            }
        }

        const CharString& str = stringBuilder.str();
        file.write(str.c_str(), str.length());
        file.flush();

        if (exportWithMats) {
            fs::path modelFolder = filePath.parent_path();

            CharString matPath = filePath.string();
            matPath[matPath.size() - 3] = 'm';
            matPath[matPath.size() - 2] = 't';
            matPath[matPath.size() - 1] = 'l';

            std::ofstream mtlFile(matPath, std::ofstream::binary);
            if (mtlFile.good()) {
                std::ostringstream mtlBuilder;
                mtlBuilder << "# Generated for Metro Exodus model file" << std::endl;
                mtlBuilder << "# using MetroEX tool made by iOrange, 2019" << std::endl << std::endl;

                for (size_t i = 0; i < mMeshes.size(); ++i) {
                    const MetroMesh* mesh = mMeshes[i];
                    if (!mesh->vertices.empty() && !mesh->faces.empty()) {
                        mtlBuilder << "newmtl " << "Material_" << i << std::endl;

                        const CharString& textureName = mesh->materials.front();

                        const CharString& sourceName = database->GetSourceName(textureName);
                        const CharString& bumpName = database->GetSourceName(textureName);

                        CharString textureTgaName = fs::path(sourceName).filename().string() + ".tga";

                        mtlBuilder << "Kd 1 1 1" << std::endl;
                        mtlBuilder << "Ke 0 0 0" << std::endl;
                        mtlBuilder << "Ns 1000" << std::endl;
                        mtlBuilder << "illum 2" << std::endl;
                        mtlBuilder << "map_Ka " << textureTgaName << std::endl;
                        mtlBuilder << "map_Kd " << textureTgaName << std::endl;

                        if (!bumpName.empty()) {
                            CharString bumpTgaName = fs::path(bumpName).filename().string() + "_nm.tga";
                            mtlBuilder << "bump " << bumpTgaName << std::endl;
                            mtlBuilder << "map_bump " << bumpTgaName << std::endl;
                        }

                        mtlBuilder << std::endl;
                    }
                }

                const CharString& mtlStr = mtlBuilder.str();
                mtlFile.write(mtlStr.c_str(), mtlStr.length());
                mtlFile.flush();
            }
        }

        result = true;
    }

    return result;
}

struct ClusterInfo {
    MyArray<int>      vertexIdxs;
    MyArray<float>    weigths;
};
void CollectClusters(const MetroMesh* mesh, const MetroSkeleton* skeleton, MyArray<ClusterInfo>& clusters) {
    const size_t numBones = skeleton->GetNumBones();
    clusters.resize(numBones);

    for (size_t i = 0; i < numBones; ++i) {
        ClusterInfo& cluster = clusters[i];

        for (size_t j = 0; j < mesh->vertices.size(); ++j) {
            const MetroVertex& v = mesh->vertices[j];

            for (size_t k = 0; k < 4; ++k) {
                const size_t rawIdx = v.bones[k];
                if (rawIdx < mesh->bonesRemap.size()) {
                    const size_t mappedBoneIdx = mesh->bonesRemap[rawIdx];
                    if (mappedBoneIdx == i && v.weights[k]) {
                        cluster.vertexIdxs.push_back(scast<int>(j));
                        cluster.weigths.push_back(scast<float>(v.weights[k]) * (1.0f / 255.0f));
                    }
                }
            }
        }
    }
}


static FbxVector4 MetroVecToFbxVec(const vec3& v) {
    return FbxVector4(v.z, v.y, v.x);
}

static FbxVector4 MetroRotToFbxRot(const quat& q) {
    vec3 euler = QuatToEuler(q);
    return FbxVector4(Rad2Deg(euler.z), Rad2Deg(euler.y), Rad2Deg(euler.x));
}


static FbxNode* CreateFBXSkeleton(FbxScene* scene, const MetroSkeleton* skeleton, MyArray<FbxNode*>& boneNodes) {
    const size_t numBones = skeleton->GetNumBones();
    boneNodes.reserve(numBones);

    for (size_t i = 0; i < numBones; ++i) {
        const size_t parentIdx = skeleton->GetBoneParentIdx(i);
        const CharString& name = skeleton->GetBoneName(i);

        FbxSkeleton* attribute = FbxSkeleton::Create(scene, name.c_str());
        if (MetroBone::InvalidIdx == parentIdx) {
            attribute->SetSkeletonType(FbxSkeleton::eRoot);
        } else {
            attribute->SetSkeletonType(FbxSkeleton::eLimbNode);
        }

        FbxNode* node = FbxNode::Create(scene, name.c_str());
        node->SetNodeAttribute(attribute);

        boneNodes.push_back(node);
    }

    FbxNode* rootNode = nullptr;
    for (size_t i = 0; i < numBones; ++i) {
        FbxNode* node = boneNodes[i];
        const size_t parentIdx = skeleton->GetBoneParentIdx(i);

        const quat& bindQ = skeleton->GetBoneRotation(i);
        const vec3& bindT = skeleton->GetBonePosition(i);

        node->LclTranslation.Set(MetroVecToFbxVec(bindT));
        node->LclRotation.Set(MetroRotToFbxRot(bindQ));

        if (MetroBone::InvalidIdx != parentIdx) {
            boneNodes[parentIdx]->AddChild(node);
        } else {
            rootNode = node;
        }
    }

    return rootNode;
}

// The curve code doesn't differentiate between angles and other data, so an interpolation from 179 to -179
// will cause the bone to rotate all the way around through 0 degrees.  So here we make a second pass over the
// rotation tracks to convert the angles into a more interpolation-friendly format.
static void CorrectAnimTrackInterpolation(MyArray<FbxNode*>& boneNodes, FbxAnimLayer* animLayer) {
    for (FbxNode* bone : boneNodes) {
        FbxAnimCurveNode* rotCurveNode = bone->LclRotation.GetCurveNode(animLayer);
        if (rotCurveNode) {
            //#NOTE_SK: just because fucking FBX doesn't allow us to use quaternions for rotations
            //          we'll get angles "clicking" issues
            //          this is the only way I found to fight this issue
            FbxAnimCurveFilterUnroll unrollFilter;
            unrollFilter.SetForceAutoTangents(true);
            unrollFilter.Apply(*rotCurveNode);
        }
    }
}

static void AddAnimTrackToScene(FbxScene* scene, const MetroMotion* motion, const CharString& animName, MyArray<FbxNode*>& skelNodes) {
    FbxAnimStack* animStack = FbxAnimStack::Create(scene, animName.c_str());
    FbxAnimLayer* animLayer = FbxAnimLayer::Create(scene->GetFbxManager(), "Base_Layer");
    animStack->AddMember(animLayer);

    const double animFPS = 30.0f;

    FbxTime startTime, stopTime;
    startTime.SetGlobalTimeMode(FbxTime::eFrames30);
    stopTime.SetGlobalTimeMode(FbxTime::eFrames30);

    startTime.SetSecondDouble(0.0);

    // kinda hack to get animation duration
    const double animDuration = scast<double>(motion->GetMotionTimeInSeconds());
    stopTime.SetSecondDouble(animDuration);

    FbxTimeSpan animTimeSpan;
    animTimeSpan.Set(startTime, stopTime);
    animStack->SetLocalTimeSpan(animTimeSpan);


    FbxTime keyTime;
    int keyIndex;

    for (size_t i = 0; i < skelNodes.size(); ++i) {
        FbxNode* boneNode = skelNodes[i];

        if (motion->IsBoneAnimated(i)) {
            const auto& posCurve = motion->mBonesPositions[i];
            const auto& rotCurve = motion->mBonesRotations[i];

            boneNode->LclRotation.GetCurveNode(animLayer, true);
            boneNode->LclTranslation.GetCurveNode(animLayer, true);

            FbxAnimCurve* offsetCurve[3] = {
                boneNode->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X, true),
                boneNode->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y, true),
                boneNode->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z, true)
            };

            offsetCurve[0]->KeyModifyBegin();
            offsetCurve[1]->KeyModifyBegin();
            offsetCurve[2]->KeyModifyBegin();

            if (!posCurve.points.empty()) {
                if (posCurve.points.size() == 1) {
                    FbxVector4 fv = MetroVecToFbxVec(vec3(posCurve.points.front().value));

                    keyTime.SetSecondDouble(0.0);

                    for (int k = 0; k < 3; ++k) {
                        keyIndex = offsetCurve[k]->KeyAdd(keyTime);
                        offsetCurve[k]->KeySetValue(keyIndex, scast<float>(fv[k]));
                        offsetCurve[k]->KeySetInterpolation(keyIndex, FbxAnimCurveDef::eInterpolationLinear);
                    }
                } else {
                    for (auto& pt : posCurve.points) {
                        FbxVector4 fv = MetroVecToFbxVec(vec3(pt.value));

                        keyTime.SetSecondDouble(scast<double>(pt.time));

                        for (int k = 0; k < 3; ++k) {
                            keyIndex = offsetCurve[k]->KeyAdd(keyTime);
                            offsetCurve[k]->KeySetValue(keyIndex, scast<float>(fv[k]));
                            offsetCurve[k]->KeySetInterpolation(keyIndex, FbxAnimCurveDef::eInterpolationCubic);
                        }
                    }
                }
            }

            offsetCurve[0]->KeyModifyEnd();
            offsetCurve[1]->KeyModifyEnd();
            offsetCurve[2]->KeyModifyEnd();

            FbxAnimCurve* rotationCurve[3] = {
                boneNode->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X, true),
                boneNode->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y, true),
                boneNode->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z, true)
            };

            rotationCurve[0]->KeyModifyBegin();
            rotationCurve[1]->KeyModifyBegin();
            rotationCurve[2]->KeyModifyBegin();

            if (!rotCurve.points.empty()) {
                if (rotCurve.points.size() == 1) {
                    FbxVector4 fv = MetroRotToFbxRot(*rcast<const quat*>(&rotCurve.points.front().value));

                    keyTime.SetSecondDouble(0.0);

                    for (int k = 0; k < 3; ++k) {
                        keyIndex = rotationCurve[k]->KeyAdd(keyTime);
                        rotationCurve[k]->KeySetValue(keyIndex, scast<float>(fv[k]));
                        rotationCurve[k]->KeySetInterpolation(keyIndex, FbxAnimCurveDef::eInterpolationLinear);
                    }
                } else {
                    for (auto& pt : rotCurve.points) {
                        FbxVector4 fv = MetroRotToFbxRot(*rcast<const quat*>(&pt.value));

                        keyTime.SetSecondDouble(scast<double>(pt.time));

                        for (int k = 0; k < 3; ++k) {
                            keyIndex = rotationCurve[k]->KeyAdd(keyTime);
                            rotationCurve[k]->KeySetValue(keyIndex, scast<float>(fv[k]));
                            rotationCurve[k]->KeySetInterpolation(keyIndex, FbxAnimCurveDef::eInterpolationCubic);
                        }
                    }
                }
            }

            rotationCurve[0]->KeyModifyEnd();
            rotationCurve[1]->KeyModifyEnd();
            rotationCurve[2]->KeyModifyEnd();
        }
    }

    CorrectAnimTrackInterpolation(skelNodes, animLayer);
}

bool MetroModel::SaveAsFBX(const fs::path& filePath, VFXReader* vfxReader, MetroTexturesDatabase* database) {
    FbxManager* mgr = FbxManager::Create();
    if (!mgr) {
        return false;
    }

    fs::path modelFolder = filePath.parent_path();

    FbxIOSettings* ios = FbxIOSettings::Create(mgr, IOSROOT);
    mgr->SetIOSettings(ios);

    FbxScene* scene = FbxScene::Create(mgr, "Metro model");

    MyDict<CharString, FbxSurfacePhong*> fbxMaterials;
    for (size_t i = 0; i < mMeshes.size(); ++i) {
        const MetroMesh* mesh = mMeshes[i];
        if (!mesh->vertices.empty() && !mesh->faces.empty()) {
            const CharString& textureName = mesh->materials.front();

            auto it = fbxMaterials.find(textureName);
            if (it == fbxMaterials.end()) {
                const CharString& sourceName = database->GetSourceName(textureName);
                const CharString& bumpName = database->GetSourceName(textureName);

                CharString textureTgaName = fs::path(sourceName).filename().u8string() + ".tga";
                CharString texturePath = (modelFolder / textureTgaName).u8string();

                FbxFileTexture* texture = FbxFileTexture::Create(mgr, textureName.c_str());
                texture->SetFileName(texturePath.c_str());
                texture->SetTextureUse(FbxTexture::eStandard);
                texture->SetMappingType(FbxTexture::eUV);
                texture->SetMaterialUse(FbxFileTexture::eModelMaterial);
                texture->UVSwap = false;
                texture->SetTranslation(0.0, 0.0);
                texture->SetScale(1.0, 1.0);
                texture->SetRotation(0.0, 0.0);
                texture->SetAlphaSource(FbxTexture::eBlack);

                FbxFileTexture* bump = nullptr;
                if (!bumpName.empty()) {
                    CharString bumpTgaName = fs::path(bumpName).filename().u8string() + "_nm.tga";
                    CharString bumpPath = (modelFolder / bumpTgaName).u8string();

                    bump = FbxFileTexture::Create(mgr, bumpName.c_str());
                    bump->SetFileName(bumpPath.c_str());
                    bump->SetTextureUse(FbxTexture::eBumpNormalMap);
                    bump->SetMappingType(FbxTexture::eUV);
                    bump->SetMaterialUse(FbxFileTexture::eModelMaterial);
                    bump->UVSwap = false;
                    bump->SetTranslation(0.0, 0.0);
                    bump->SetScale(1.0, 1.0);
                    bump->SetRotation(0.0, 0.0);
                }

                FbxSurfacePhong* material = FbxSurfacePhong::Create(mgr, textureName.c_str());
                material->Emissive = FbxDouble3(0.0, 0.0, 0.0);
                material->Diffuse.ConnectSrcObject(texture);
                material->Specular = FbxDouble3(1.0, 1.0, 1.0);
                material->SpecularFactor = 0.0;
                material->Shininess = 0.0; // simple diffuse

                if (bump) {
                    material->Bump.ConnectSrcObject(bump);
                    material->BumpFactor = 1.0;
                }

                fbxMaterials[textureName] = material;
            }
        }
    }

    MyArray<FbxNode*> meshNodes;
    MyArray<FbxMesh*> fbxMeshes;
    for (size_t i = 0; i < mMeshes.size(); ++i) {
        const MetroMesh* mesh = this->GetMesh(i);
        CharString meshName = CharString("mesh_") + std::to_string(i);

        FbxMesh* fbxMesh = FbxMesh::Create(scene, meshName.c_str());

        // assign vertices
        fbxMesh->InitControlPoints(scast<int>(mesh->vertices.size()));
        FbxVector4* ptrCtrlPoints = fbxMesh->GetControlPoints();

        FbxGeometryElementNormal* normalElement = fbxMesh->CreateElementNormal();
        normalElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
        normalElement->SetReferenceMode(FbxGeometryElement::eDirect);

        FbxGeometryElementUV* uvElement = fbxMesh->CreateElementUV("uv0");
        uvElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
        uvElement->SetReferenceMode(FbxGeometryElement::eDirect);

        for (const MetroVertex& v : mesh->vertices) {
            *ptrCtrlPoints = MetroVecToFbxVec(v.pos);
            normalElement->GetDirectArray().Add(MetroVecToFbxVec(v.normal));
            uvElement->GetDirectArray().Add(FbxVector2(v.uv0.x, 1.0f - v.uv0.y));

            ++ptrCtrlPoints;
        }

        FbxGeometryElementMaterial* materialElement = fbxMesh->CreateElementMaterial();
        materialElement->SetMappingMode(FbxGeometryElement::eAllSame);
        materialElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);
        materialElement->GetIndexArray().Add(0);

        // build polygons
        for (const MetroFace& face : mesh->faces) {
            fbxMesh->BeginPolygon();
            fbxMesh->AddPolygon(scast<int>(face.c));
            fbxMesh->AddPolygon(scast<int>(face.b));
            fbxMesh->AddPolygon(scast<int>(face.a));
            fbxMesh->EndPolygon();
        }

        FbxNode* meshNode = FbxNode::Create(scene, meshName.c_str());
        meshNode->SetNodeAttribute(fbxMesh);
        scene->GetRootNode()->AddChild(meshNode);

        const CharString& textureName = mesh->materials.front();
        auto it = fbxMaterials.find(textureName);
        if (it != fbxMaterials.end()) {
            meshNode->SetShadingMode(FbxNode::eTextureShading);
            meshNode->AddMaterial(it->second);
        }

        fbxMeshes.push_back(fbxMesh);
        meshNodes.push_back(meshNode);
    }

    if (mSkeleton) {
        MyArray<FbxNode*> boneNodes;
        FbxNode* rootBoneNode = CreateFBXSkeleton(scene, mSkeleton, boneNodes);
        scene->GetRootNode()->AddChild(rootBoneNode);

        FbxPose* bindPose = FbxPose::Create(scene, "BindPose");
        bindPose->SetIsBindPose(true);

        for (FbxNode* node : boneNodes) {
            bindPose->Add(node, node->EvaluateGlobalTransform());
        }

        for (size_t i = 0; i < fbxMeshes.size(); ++i) {
            FbxMesh* fbxMesh = fbxMeshes[i];
            FbxNode* meshNode = meshNodes[i];

            const MetroMesh* mesh = mMeshes[i];

            FbxAMatrix meshXMatrix = meshNode->EvaluateGlobalTransform();

            MyArray<ClusterInfo> clusters;
            CollectClusters(mesh, mSkeleton, clusters);

            FbxSkin* skin = FbxSkin::Create(scene, "");
            for (size_t i = 0; i < clusters.size(); ++i) {
                const ClusterInfo& cluster = clusters[i];
                if (!cluster.vertexIdxs.empty()) {
                    FbxCluster* fbxCluster = FbxCluster::Create(scene, "");
                    FbxNode* linkNode = boneNodes[i];
                    fbxCluster->SetLink(linkNode);
                    fbxCluster->SetLinkMode(FbxCluster::eTotalOne);
                    for (size_t j = 0; j < cluster.vertexIdxs.size(); ++j) {
                        fbxCluster->AddControlPointIndex(cluster.vertexIdxs[j], cluster.weigths[j]);
                    }
                    fbxCluster->SetTransformMatrix(meshXMatrix);
                    fbxCluster->SetTransformLinkMatrix(linkNode->EvaluateGlobalTransform());
                    skin->AddCluster(fbxCluster);
                }
            }

            fbxMesh->AddDeformer(skin);
            bindPose->Add(meshNode, meshXMatrix);
        }

        scene->AddPose(bindPose);


        for (size_t i = 0; i < mMotions.size(); ++i) {
            const MetroMotion* motion = mMotions[i];
            AddAnimTrackToScene(scene, motion, motion->GetName(), boneNodes);
        }
    }

    // now export all this
    FbxDocumentInfo* info = scene->GetSceneInfo();
    if (info) {
        info->Original_ApplicationVendor = FbxString("iOrange");
        info->Original_ApplicationName = FbxString("MetroEX");
        info->mTitle = FbxString("Metro Exodus model");
        info->mComment = FbxString("Exported using MetroEX created by iOrange");
    }

    FbxGlobalSettings& settings = scene->GetGlobalSettings();
    settings.SetAxisSystem(FbxAxisSystem(FbxAxisSystem::eOpenGL));
    settings.SetOriginalUpAxis(FbxAxisSystem(FbxAxisSystem::eOpenGL));
    settings.SetSystemUnit(FbxSystemUnit(1.0f));

    // export
    FbxExporter* exp = FbxExporter::Create(mgr, "");
    const int format = mgr->GetIOPluginRegistry()->GetNativeWriterFormat();

    exp->SetFileExportVersion(FBX_2011_00_COMPATIBLE);

    ios->SetBoolProp(EXP_FBX_MATERIAL, true);
    ios->SetBoolProp(EXP_FBX_TEXTURE, true);
    ios->SetBoolProp(EXP_FBX_EMBEDDED, false);
    ios->SetBoolProp(EXP_FBX_SHAPE, true);
    ios->SetBoolProp(EXP_FBX_GOBO, true);
    ios->SetBoolProp(EXP_FBX_ANIMATION, true);
    ios->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

    if (exp->Initialize(filePath.u8string().c_str(), format, ios)) {
        if (!exp->Export(scene)) {
            //FbxStatus& status = exp->GetStatus();
            //std::cerr << status.GetErrorString() << std::endl;
            return false;
        }
    }

    exp->Destroy();
    mgr->Destroy();

    return true;
}

bool MetroModel::IsAnimated() const {
    return mSkeleton != nullptr;
}

const AABBox& MetroModel::GetBBox() const {
    return mBBox;
}

const vec4& MetroModel::GetBSphere() const {
    return mBSphere;
}

size_t MetroModel::GetNumMeshes() const {
    return mMeshes.size();
}

const MetroMesh* MetroModel::GetMesh(const size_t idx) const {
    return mMeshes[idx];
}

const MetroSkeleton* MetroModel::GetSkeleton() const {
    return mSkeleton;
}

size_t MetroModel::GetNumMotions() const {
    return mMotions.size();
}

const MetroMotion* MetroModel::GetMotion(const size_t idx) const {
    return mMotions[idx];
}


// for string delimiter
StringArray SplitString(const CharString& s, const char delimiter) {
    StringArray result;

    size_t pos_start = 0, pos_end;
    CharString token;

    while ((pos_end = s.find(delimiter, pos_start)) != CharString::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + 1;
        result.emplace_back(token);
    }

    result.push_back(s.substr(pos_start));
    return result;
}

void MetroModel::ReadSubChunks(MemStream& stream) {
    while (!stream.Ended()) {
        const size_t chunkId = stream.ReadTyped<uint32_t>();
        const size_t chunkSize = stream.ReadTyped<uint32_t>();
        const size_t chunkEnd = stream.GetCursor() + chunkSize;

        switch (chunkId) {
            case MC_HeaderChunk: {
                MdlHeader hdr;
                stream.ReadStruct(hdr);

                if (hdr.scales.y <= MM_Epsilon) {
                    hdr.scales.y = 1.0f;
                }

                if (mCurrentMesh) {
                    mCurrentMesh->scales = hdr.scales;
                    mCurrentMesh->bbox = hdr.bbox;
                    mCurrentMesh->type = hdr.type;
                    mCurrentMesh->idx = hdr.index;
                } else {
                    mBBox = hdr.bbox;
                    mBSphere = hdr.bsphere;
                }
            } break;

            case MC_MaterialsChunk: {
                if (!mCurrentMesh) {
                    return;
                }

                mCurrentMesh->materials.resize(kMetroModelMaxMaterials);
                for (auto& s : mCurrentMesh->materials) {
                    s = stream.ReadStringZ();
                }
            } break;

            case MC_VerticesChunk: {
                if (mCurrentMesh) {
                    mCurrentMesh->skinned = false;

                    const size_t vertexType = stream.ReadTyped<uint32_t>();
                    const size_t numVertices = stream.ReadTyped<uint32_t>();
                    const size_t numShadowVertices = stream.ReadTyped<uint16_t>();

                    mCurrentMesh->vertices.resize(numVertices);

                    const VertexStatic* srcVerts = rcast<const VertexStatic*>(stream.GetDataAtCursor());
                    MetroVertex* dstVerts = mCurrentMesh->vertices.data();

                    for (size_t i = 0; i < numVertices; ++i) {
                        *dstVerts = ConvertVertex(*srcVerts);
                        dstVerts->pos *= mCurrentMesh->scales.y;
                        ++srcVerts;
                        ++dstVerts;
                    }
                }
            } break;

            case MC_SkinnedVerticesChunk: {
                if (mCurrentMesh) {
                    mCurrentMesh->skinned = true;

                    const size_t numBones = stream.ReadTyped<uint8_t>();

                    mCurrentMesh->bonesRemap.resize(numBones);
                    stream.ReadToBuffer(mCurrentMesh->bonesRemap.data(), numBones);

                    //std::vector<MetroOBB> obbs(numBones);
                    //stream.ReadToBuffer(obbs.data(), obbs.size() * sizeof(MetroOBB));
                    stream.SkipBytes(numBones * sizeof(MetroOBB));

                    const size_t numVertices = stream.ReadTyped<uint32_t>();
                    const size_t numShadowVertices = stream.ReadTyped<uint16_t>();

                    mCurrentMesh->vertices.resize(numVertices);

                    const VertexSkinned* srcVerts = rcast<const VertexSkinned*>(stream.GetDataAtCursor());
                    MetroVertex* dstVerts = mCurrentMesh->vertices.data();

                    for (size_t i = 0; i < numVertices; ++i) {
                        *dstVerts = ConvertVertex(*srcVerts);
                        dstVerts->pos *= mCurrentMesh->scales.y;
                        ++srcVerts;
                        ++dstVerts;
                    }
                }
            } break;

            case MC_FacesChunk: {
                if (mCurrentMesh) {
                    const size_t numFaces = mCurrentMesh->skinned ? stream.ReadTyped<uint16_t>() : stream.ReadTyped<uint32_t>();
                    const size_t numShadowFaces = stream.ReadTyped<uint16_t>();

                    mCurrentMesh->faces.resize(numFaces);
                    stream.ReadToBuffer(mCurrentMesh->faces.data(), numFaces * sizeof(MetroFace));
                }
            } break;

            case MC_SubMeshesChunk: {
                MemStream meshesStream = stream.Substream(chunkSize);
                size_t nextMeshId = 0;
                while (!meshesStream.Ended()) {
                    const size_t subMeshId = meshesStream.ReadTyped<uint32_t>();
                    const size_t subMeshSize = meshesStream.ReadTyped<uint32_t>();

                    if (subMeshId == nextMeshId) {
                        mCurrentMesh = new MetroMesh();
                        mMeshes.push_back(mCurrentMesh);

                        MemStream subStream = meshesStream.Substream(subMeshSize);
                        this->ReadSubChunks(subStream);
                        ++nextMeshId;

                        meshesStream.SkipBytes(subMeshSize);
                    } else {
                        break;
                    }
                }
                mCurrentMesh = nullptr;
            } break;

            case MC_MeshesInline: {
                stream.SkipBytes(16); // wtf ???
                MemStream subStream = stream.Substream(chunkSize - 16);
                this->ReadSubChunks(subStream);
            } break;

            case MC_MeshesLinks: {
                const size_t numStrings = stream.ReadTyped<uint32_t>();
                StringArray links;
                for (size_t i = 0; i < numStrings; ++i) {
                    CharString linksString = stream.ReadStringZ();
                    if (!linksString.empty()) {
                        StringArray splittedLinks = SplitString(linksString, ',');
                        links.insert(links.end(), splittedLinks.begin(), splittedLinks.end());
                    }
                }

                if (mVFXReader && !links.empty()) {
                    this->LoadLinkedMeshes(links);
                }
            } break;

            case MC_SkeletonLink: {
                CharString skeletonRef = stream.ReadStringZ();
                CharString skelFilePath = "content\\meshes\\" + skeletonRef + ".skeleton.bin";
                const size_t fileIdx = mVFXReader->FindFile(skelFilePath);
                if (MetroFile::InvalidFileIdx != fileIdx) {
                    BytesArray content;
                    if (mVFXReader->ExtractFile(fileIdx, content)) {
                        mSkeleton = new MetroSkeleton();
                        if (!mSkeleton->LoadFromData(content.data(), content.size())) {
                            MySafeDelete(mSkeleton);
                        }
                    }
                }
            } break;

            case MC_SkeletonInline: {
                mSkeleton = new MetroSkeleton();
                if (!mSkeleton->LoadFromData(stream.GetDataAtCursor(), chunkSize)) {
                    MySafeDelete(mSkeleton);
                }
            } break;
        }

        stream.SetCursor(chunkEnd);
    }
}

void MetroModel::LoadLinkedMeshes(const StringArray& links) {
    mCurrentMesh = nullptr;

    for (const CharString& lnk : links) {
        size_t fileIdx = MetroFile::InvalidFileIdx;

        if (lnk[0] == '.' && lnk[1] == '\\') { // relative path
            const MetroFile* folder = mVFXReader->GetParentFolder(mThisFileIdx);
            fileIdx = mVFXReader->FindFile(lnk.substr(2) + ".mesh", folder);
        } else {
            CharString meshFilePath = "content\\meshes\\" + lnk + ".mesh";
            fileIdx = mVFXReader->FindFile(meshFilePath);
        }
        if (MetroFile::InvalidFileIdx != fileIdx) {
            BytesArray meshContent;
            if (mVFXReader->ExtractFile(fileIdx, meshContent)) {
                MemStream stream(meshContent.data(), meshContent.size());
                this->ReadSubChunks(stream);
            }

            mCurrentMesh = nullptr;
        }
    }
}

void MetroModel::LoadMotions() {
    CharString motionsStr;
    if (mSkeleton) {
        motionsStr = mSkeleton->GetMotionsStr();
    }

    if (motionsStr.empty()) {
        return;
    }

    MyArray<size_t> motionFiles;

    StringArray motionFolders = SplitString(motionsStr, ',');
    for (const CharString& f : motionFolders) {
        CharString fullFolderPath = "content\\motions\\" + f + "\\";

        const auto& v = mVFXReader->FindFilesInFolder(fullFolderPath, ".m2");
        motionFiles.insert(motionFiles.end(), v.begin(), v.end());
    }

    mMotions.reserve(motionFiles.size());
    for (const size_t idx : motionFiles) {
        BytesArray content;
        if (mVFXReader->ExtractFile(idx, content)) {
            const MetroFile& mf = mVFXReader->GetFile(idx);
            CharString motionName = fs::path(mf.name).stem().u8string();

            MetroMotion* motion = new MetroMotion(motionName);
            if (motion->LoadFromData(content.data(), content.size())) {
                mMotions.push_back(motion);
            } else {
                MySafeDelete(motion);
            }
        }
    }
}
