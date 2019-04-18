#pragma once
#include "metro/VFXReader.h"
#include "metro/MetroTexturesDatabase.h"
#include "metro/MetroConfigDatabase.h"
#include "metro/MetroMaterialsDatabase.h"

static void LoadDatabasesFromFile(VFXReader* vfxReader, MetroTexturesDatabase*& texDb, MetroConfigsDatabase*& cfgDb) {
    size_t fileIdx = 0;

    texDb = nullptr;
    cfgDb = nullptr;

    // Load textures_handles_storage.bin
    fileIdx = vfxReader->FindFile("content\\textures_handles_storage.bin");
    if (MetroFile::InvalidFileIdx != fileIdx) {
        MemStream stream = vfxReader->ExtractFile(fileIdx);
        if (stream) {
            texDb = new MetroTexturesDatabase();
            texDb->LoadFromData(stream);

            fileIdx = vfxReader->FindFile("content\\scripts\\texture_aliases.bin");
            if (MetroFile::InvalidFileIdx != fileIdx) {
                stream = vfxReader->ExtractFile(fileIdx);
                if (stream) {
                    texDb->LoadAliasesFromData(stream);
                }
            }
        }
    }

    // Load config.bin
    fileIdx = vfxReader->FindFile("content\\config.bin");
    if (MetroFile::InvalidFileIdx != fileIdx) {
        MemStream stream = vfxReader->ExtractFile(fileIdx);
        if (stream) {
            cfgDb = new MetroConfigsDatabase();
            cfgDb->LoadFromData(stream);
        }
    }

    // load materials database
    //if (cfgDb) {
    //    MemStream materialsStream = cfgDb->GetFileStream(R"(content\scripts\materials.bin)");
    //    if (materialsStream.Good()) {
    //        MetroMaterialsDatabase materialsDb;
    //        materialsDb.LoadFromData(materialsStream);
    //    }
    //}
}
