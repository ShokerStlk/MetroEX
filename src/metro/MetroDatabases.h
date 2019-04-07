#pragma once
#include "metro/VFXReader.h"
#include "metro/MetroTexturesDatabase.h"
#include "metro/MetroConfigDatabase.h"

#include "ui\MainForm.h"

static void LoadDatabasesFromFile(VFXReader* VFXReader, MetroTexturesDatabase*& texDb, MetroConfigsDatabase*& cfgDb) {
    size_t fileIdx = 0;

    texDb = nullptr;
    cfgDb = nullptr;

    // Load textures_handles_storage.bin
    fileIdx = VFXReader->FindFile("content\\textures_handles_storage.bin");
    if (MetroFile::InvalidFileIdx != fileIdx) {
        BytesArray content;
        if (VFXReader->ExtractFile(fileIdx, content)) {
            texDb = new MetroTexturesDatabase();
            texDb->LoadFromData(content.data(), content.size());

            fileIdx = VFXReader->FindFile("content\\scripts\\texture_aliases.bin");
            if (MetroFile::InvalidFileIdx != fileIdx) {
                if (VFXReader->ExtractFile(fileIdx, content)) {
                    texDb->LoadAliasesFromData(content.data(), content.size());
                }
            }
        }
    }

    // Load config.bin
    fileIdx = VFXReader->FindFile("content\\config.bin");
    if (MetroFile::InvalidFileIdx != fileIdx) {
        BytesArray content;
        if (VFXReader->ExtractFile(fileIdx, content)) {
            cfgDb = new MetroConfigsDatabase();
            cfgDb->LoadFromData(content.data(), content.size());
        }
    }
}
