#include "DlgModelInfo.h"

#include "metro/MetroModel.h"
#include "metro/MetroSkeleton.h"
#include "metro/MetroMotion.h"

// String to std::string wrapper
#include <msclr/marshal_cppstd.h>
using namespace msclr::interop;

namespace MetroEX {

    void DlgModelInfo::SetModel(MetroModel* model) {
        if (!mModelInfo) {
            mModelInfo = gcnew DlgModelInfo::ModelInfo();
        }

        mModelInfo->meshes.Clear();
        mModelInfo->motions.Clear();
        mModelInfo->skeletonPath = String::Empty;
        mModelInfo->numBones = 0;

        if (model) {
            for (size_t i = 0; i < model->GetNumMeshes(); ++i) {
                const MetroMesh* mesh = model->GetMesh(i);

                MeshInfo^ mi = gcnew MeshInfo();
                mi->name = String::Format(L"Mesh_{0}", i);
                mi->texturePath = L"content\\textures\\" + marshal_as<String^>(mesh->materials[0]);
                mi->material1 = marshal_as<String^>(mesh->materials[1]);
                mi->material2 = marshal_as<String^>(mesh->materials[2]);
                mi->material3 = marshal_as<String^>(mesh->materials[3]);
                mi->numVertices = scast<int>(mesh->vertices.size());
                mi->numFaces = scast<int>(mesh->faces.size());

                mModelInfo->meshes.Add(mi);
            }

            for (size_t i = 0; i < model->GetNumMotions(); ++i) {
                const MetroMotion* motion = model->GetMotion(i);

                MotionInfo^ mi = gcnew MotionInfo();
                mi->name = marshal_as<String^>(motion->GetName());
                mi->path = marshal_as<String^>(motion->GetPath());
                mi->duration = motion->GetMotionTimeInSeconds();

                mModelInfo->motions.Add(mi);
            }

            if (model->GetSkeleton()) {
                mModelInfo->skeletonPath = marshal_as<String^>(model->GetSkeletonPath());
                if (!mModelInfo->skeletonPath->Length) {
                    mModelInfo->skeletonPath = L"Inline";
                }
                mModelInfo->numBones = scast<int>(model->GetSkeleton()->GetNumBones());
            }
        }

        this->UpdateUI();
    }

    void DlgModelInfo::UpdateUI() {
        this->txtMeshTexture->Text = String::Empty;
        this->lblMeshVertices->Text = L"Vertices: 0";
        this->lblMeshFaces->Text = L"Triangles: 0";
        this->lstMeshes->Items->Clear();

        this->txtMotionPath->Text = String::Empty;
        this->lblMotionLength->Text = L"Length: 0 seconds";
        this->lstMotions->Items->Clear();

        for each (MeshInfo^ mi in mModelInfo->meshes) {
            this->lstMeshes->Items->Add(mi->name);
        }

        for each (MotionInfo^ mi in mModelInfo->motions) {
            this->lstMotions->Items->Add(mi->name);
        }

        this->txtSkeletonPath->Text = mModelInfo->skeletonPath;
        this->lblNumBones->Text = String::Format(L"Bones: {0}", mModelInfo->numBones);

        if (mModelInfo->meshes.Count > 0) {
            this->lstMeshes->SelectedIndex = 0;
        }
        if (mModelInfo->motions.Count > 0) {
            this->lstMotions->SelectedIndex = 0;
        }
    }

    void DlgModelInfo::lstMeshes_SelectedIndexChanged(System::Object^, System::EventArgs^) {
        const int idx = this->lstMeshes->SelectedIndex;
        if (idx >= 0 && idx < mModelInfo->meshes.Count) {
            MeshInfo^ mi = mModelInfo->meshes[idx];

            this->txtMeshTexture->Text = mi->texturePath;
            this->txtMeshMaterials->Text = mi->material1 + L"," + mi->material2 + L"," + mi->material3;

            this->lblMeshVertices->Text = String::Format(L"Vertices: {0}", mi->numVertices);
            this->lblMeshFaces->Text = String::Format(L"Triangles: {0}", mi->numFaces);
        }
    }

    void DlgModelInfo::lstMotions_SelectedIndexChanged(System::Object^, System::EventArgs^) {
        const int idx = this->lstMotions->SelectedIndex;
        if (idx >= 0 && idx < mModelInfo->motions.Count) {
            MotionInfo^ mi = mModelInfo->motions[idx];

            this->txtMotionPath->Text = mi->path;
            this->lblMotionLength->Text = String::Format(L"Length: {0,12:F2} seconds", mi->duration);
        }
    }
}

