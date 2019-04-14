#include "MetroPatchTool.h"
#include "MetroTypes.h"
#include "MetroCompression.h"

#include <fstream>

struct DirEntry {
    fs::path    path;
    MetroFile   file;
};

MyArray<fs::directory_entry> CollectDirectoryContent(const fs::path dirPath) {
    MyArray<fs::directory_entry> result;

    for (const auto& entry : fs::directory_iterator(dirPath)) {
        result.push_back(entry);
    }

    return std::move(result);
}

MyArray<DirEntry> CollectDirectoryRecursive(DirEntry& entry, size_t& nextIdx) {
    MyArray<DirEntry> result;

    MyArray<fs::directory_entry> content = CollectDirectoryContent(entry.path);

    entry.file.numFiles = content.size();
    entry.file.firstFile = nextIdx;

    // we first collect all files in this directory
    for (const auto& e : content) {
        if (e.is_regular_file()) {
            DirEntry fileEntry;
            fileEntry.path = e.path();
            fileEntry.file = {};
            fileEntry.file.idx = nextIdx++;
            fileEntry.file.name = fileEntry.path.filename().u8string();

            result.emplace_back(fileEntry);
        }
    }

    size_t startFolder = result.size();
    size_t numFolders = 0;

    // then we collect all subfolders
    for (const auto& e : content) {
        if (e.is_directory()) {
            DirEntry dirEntry;
            dirEntry.path = e.path();
            dirEntry.file = {};
            dirEntry.file.flags = MetroFile::Flag_Folder | MetroFile::Flag_Unknown4;
            dirEntry.file.idx = nextIdx++;
            dirEntry.file.name = dirEntry.path.filename().u8string();

            result.emplace_back(dirEntry);
            ++numFolders;
        }
    }

    for (size_t i = 0; i < numFolders; ++i) {
        MyArray<DirEntry> collected = CollectDirectoryRecursive(result[startFolder + i], nextIdx);
        result.insert(result.end(), collected.begin(), collected.end());
    }

    return std::move(result);
}

void WriteStringZ(MemWriteStream& stream, const CharString& str) {
    stream.Write(str.c_str(), str.length() + 1);
}

void WriteStringXored(MemWriteStream& stream, const CharString& str) {
    static const char sEmpty[3] = { 1, 0, 0 };
    if (str.empty()) {
        stream.Write(sEmpty, sizeof(sEmpty));
    } else {
        BytesArray temp(str.length() + 1);
        memcpy(temp.data(), str.c_str(), temp.size());

        const uint8_t xorMask = scast<uint8_t>(rand() % 235) + 15;
        for (size_t i = 0, end = temp.size() - 1; i < end; ++i) {
            temp[i] ^= xorMask;
        }

        const uint16_t header = (scast<uint16_t>(xorMask) << 8) | scast<uint16_t>(temp.size() & 0xFF);
        stream.Write(header);
        stream.Write(temp.data(), temp.size());
    }
}


MetroPatchTool::MetroPatchTool() {

}
MetroPatchTool::~MetroPatchTool() {

}


void WriteFileToVFX(const MetroFile& mf, MemWriteStream& stream) {
    stream.Write(scast<uint16_t>(mf.flags));
    if (mf.IsFile()) {
        stream.Write(scast<uint16_t>(mf.pakIdx));
        stream.Write(scast<uint32_t>(mf.offset));
        stream.Write(scast<uint32_t>(mf.sizeUncompressed));
        stream.Write(scast<uint32_t>(mf.sizeCompressed));
    } else {
        stream.Write(scast<uint16_t>(mf.numFiles));
        stream.Write(scast<uint32_t>(mf.firstFile));
    }
    WriteStringXored(stream, mf.name);
}

bool WriteFileToVFS(DirEntry& entry, std::ofstream& vfsStream) {
    bool result = false;

    std::ifstream file(entry.path, std::ofstream::binary);
    if (file) {
        BytesArray fileData;
        file.seekg(0, std::ios::end);
        fileData.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(rcast<char*>(fileData.data()), fileData.size());
        file.close();

        //BytesArray compressed;
        //MetroCompression::CompressStream()

        vfsStream.write(rcast<const char*>(fileData.data()), fileData.size());

        entry.file.sizeCompressed = fileData.size();
        entry.file.sizeUncompressed = entry.file.sizeCompressed;

        result = true;
    }

    return result;
}

bool MetroPatchTool::CreatePatchFromFolder(const fs::path& contentFolder, const fs::path& vfxPath) {
    bool result = false;

    CharString vfsName = vfxPath.stem().u8string() + ".vfs0";
    fs::path vfsPath = vfxPath.parent_path() / vfsName;

    DirEntry content;
    content.path = contentFolder;
    content.file = {};
    content.file.flags = MetroFile::Flag_Folder;
    content.file.idx = 0;
    content.file.name = "content";

    size_t nextIdx = 1;

    MyArray<DirEntry> allFiles = CollectDirectoryRecursive(content, nextIdx);
    if (!allFiles.empty()) {
        allFiles.insert(allFiles.begin(), content);

        const CharString contentVersion = "492798"; // "491177"
        const MetroGuid guid = {0x9FE25B12, 0xF276, 0x40F4, 0xEAB8, {0x0F, 0xE1, 0xA4, 0xC6, 0x9E, 0x7A}};
        size_t numFiles = allFiles.size();

        MemWriteStream vfxStream;
        std::ofstream vfsFile(vfsPath, std::ofstream::binary);

        // write header
        vfxStream.Write(scast<uint32_t>(3));
        vfxStream.Write(scast<uint32_t>(MetroCompression::Type_LZ4));
        WriteStringZ(vfxStream, contentVersion);
        vfxStream.Write(guid);
        vfxStream.Write(scast<uint32_t>(1));        // num packages
        vfxStream.Write(scast<uint32_t>(numFiles)); // num files
        vfxStream.Write(scast<uint32_t>(0));        // unknown

        // write package
        WriteStringZ(vfxStream, vfsName);
        vfxStream.Write(scast<uint32_t>(0));        // num levels
        vfxStream.Write(scast<uint32_t>(2));        // chunk

        size_t vfsOffset = 0;
        for (auto& e : allFiles) {
            //#NOTE_SK: if file - first add it to vfs and init offsets and size
            if (e.file.IsFile()) {
                e.file.offset = vfsOffset;
                WriteFileToVFS(e, vfsFile);
                vfsOffset += e.file.sizeCompressed;
            }

            WriteFileToVFX(e.file, vfxStream);
        }
        vfxStream.Write<uint64_t>(0);

        vfsFile.flush();
        vfsFile.close();

        std::ofstream vfxFile(vfxPath, std::ofstream::binary);
        if (vfxFile.good()) {
            vfxFile.write(rcast<const char*>(vfxStream.buffer.data()), vfxStream.buffer.size());
            vfxFile.flush();
            vfxFile.close();
        }
    }

    return result;
}
