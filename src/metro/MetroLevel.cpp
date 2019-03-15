#include "MetroLevel.h"
#include "VFXReader.h"


enum DescriptionChunk : size_t {
    DC_Header       = 0x00000001,   // seems to be same as MdlHeader
    DC_Materials    = 0x00000002,
    DC_Info         = 0x00000015,
};

enum GeomChunks : size_t {
    GC_Vertices     = 0x00000009,
    GC_Indices      = 0x0000000A,
};




MetroLevel::MetroLevel() {

}
MetroLevel::~MetroLevel() {
    std::for_each(mMeshes.begin(), mMeshes.end(), [](MetroMesh* mesh) { delete mesh; });
}

bool MetroLevel::LoadFromData(const uint8_t* data, const size_t length, VFXReader* vfxReader, const size_t fileIdx) {
    bool result = false;

    const MetroFile* folder = vfxReader->GetParentFolder(fileIdx);
    if (folder) {
        MemStream stream(data, length);

        const size_t version = stream.ReadTyped<uint32_t>();
        if (version == 1) {
            const size_t numMeshes = stream.ReadTyped<uint32_t>();
            mMeshes.reserve(numMeshes);

            for (size_t i = 0; i < numMeshes; ++i) {
                CharString partName = stream.ReadStringZ();
                const size_t descriptionFileIdx = vfxReader->FindFile(partName, folder);
                const size_t geometryFileIdx = vfxReader->FindFile(partName + ".geom_pc", folder);
                if (MetroFile::InvalidFileIdx != descriptionFileIdx && MetroFile::InvalidFileIdx != geometryFileIdx) {
                    MyArray<GeomObjectInfo> infos;
                    BytesArray content;
                    if (vfxReader->ExtractFile(descriptionFileIdx, content)) {
                        this->ReadGeometryDescription(content.data(), content.size(), infos);
                    }

                    content.clear();
                    if (!infos.empty() && vfxReader->ExtractFile(geometryFileIdx, content)) {
                        this->ReadLevelGeometry(content.data(), content.size(), infos);
                    }
                }
            }

            result = !mMeshes.empty();
        }
    }

    return result;
}

size_t MetroLevel::GetNumMeshes() const {
    return mMeshes.size();
}

const MetroMesh* MetroLevel::GetMesh(const size_t idx) const {
    return mMeshes[idx];
}


void MetroLevel::ReadGeometryDescription(const uint8_t* data, const size_t length, MyArray<GeomObjectInfo>& infos) {
    MemStream stream(data, length);

    stream.SkipBytes(20); // unknown

    size_t nextIdx = 0;
    while (!stream.Ended()) {
        const size_t objectIdx = stream.ReadTyped<uint32_t>();
        if (objectIdx != nextIdx) {
            break;
        }

        const size_t objectSize = stream.ReadTyped<uint32_t>();
        const size_t objectEnd = stream.GetCursor() + objectSize;

        MemStream objectStream = stream.Substream(objectSize);
        GeomObjectInfo info = {};
        this->ReadGeomObjectInfo(objectStream, info);

        infos.emplace_back(info);

        stream.SetCursor(objectEnd);
        nextIdx++;
    }
}

void MetroLevel::ReadGeomObjectInfo(MemStream& stream, GeomObjectInfo& info) {
    while (!stream.Ended()) {
        const size_t chunkId = stream.ReadTyped<uint32_t>();
        const size_t chunkSize = stream.ReadTyped<uint32_t>();
        const size_t chunkEnd = stream.GetCursor() + chunkSize;

        switch (chunkId) {
            case DC_Info: {
                stream.ReadStruct(info);
            } break;
        }

        stream.SetCursor(chunkEnd);
    }
}

void MetroLevel::ReadLevelGeometry(const uint8_t* data, const size_t length, const MyArray<GeomObjectInfo>& infos) {
    MemStream stream(data, length);

    stream.SkipBytes(24); // some header ???

    MyArray<MetroMesh*> meshes(infos.size());
    for (MetroMesh*& m : meshes) {
        m = new MetroMesh();
        m->scales = vec3(1.0f);
    }

    const size_t numMeshes = meshes.size();

    while (!stream.Ended()) {
        const size_t chunkId = stream.ReadTyped<uint32_t>();
        const size_t chunkSize = stream.ReadTyped<uint32_t>();
        const size_t chunkEnd = stream.GetCursor() + chunkSize;

        switch (chunkId) {
            case GC_Vertices: {
                const size_t numVertices = stream.ReadTyped<uint32_t>();
                const size_t numSmallVertices = stream.ReadTyped<uint32_t>();

                const VertexLevel* vertsPtr = rcast<const VertexLevel*>(stream.GetDataAtCursor());
                size_t vertsRead = 0, vertsSmallRead = 0;
                for (size_t i = 0; i < numMeshes; ++i) {
                    MetroMesh* mesh = meshes[i];
                    const GeomObjectInfo& info = infos[i];

                    vertsRead += info.numVertices;
                    vertsSmallRead += info.numShadowVertices;

                    mesh->vertices.resize(info.numVertices);
                    MetroVertex* dstVerts = mesh->vertices.data();

                    const VertexLevel* srcVerts = vertsPtr + info.vbOffset;

                    for (size_t i = 0; i < info.numVertices; ++i) {
                        *dstVerts = ConvertVertex(*srcVerts);
                        ++srcVerts;
                        ++dstVerts;
                    }
                }

                stream.SkipBytes(numVertices * sizeof(VertexLevel));
            } break;

            case GC_Indices: {
                const size_t numIndices = stream.ReadTyped<uint32_t>();
                const size_t numSmallIndices = stream.ReadTyped<uint32_t>();

                const uint16_t* indicesPtr = rcast<const uint16_t*>(stream.GetDataAtCursor());
                for (size_t i = 0; i < numMeshes; ++i) {
                    MetroMesh* mesh = meshes[i];
                    const GeomObjectInfo& info = infos[i];

                    mesh->faces.resize(info.numIndices / 3);

                    const uint16_t* srcIndices = indicesPtr + info.ibOffset;
                    memcpy(mesh->faces.data(), srcIndices, mesh->faces.size() * sizeof(MetroFace));
                }

                stream.SkipBytes(numIndices * 2);
            } break;
        }

        stream.SetCursor(chunkEnd);
    }

    mMeshes.insert(mMeshes.end(), meshes.begin(), meshes.end());
}
