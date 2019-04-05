#pragma once
#include "metro/VFXReader.h"
#include "metro/MetroTexturesDatabase.h"
#include "metro/MetroConfigDatabase.h"

#include "ui\MainForm.h"

static void LoadDatabasesFromFile(VFXReader* VFXReader, MetroEX::MainForm^ form) {
	size_t fileIdx = 0;

	// Load textures_handles_storage.bin
	fileIdx = VFXReader->FindFile("content\\textures_handles_storage.bin");
	if (MetroFile::InvalidFileIdx != fileIdx) {
		BytesArray content;
		if (VFXReader->ExtractFile(fileIdx, content)) {
			form->mTexturesDatabase = new MetroTexturesDatabase();
			form->mTexturesDatabase->LoadFromData(content.data(), content.size());

			fileIdx = VFXReader->FindFile("content\\scripts\\texture_aliases.bin");
			if (MetroFile::InvalidFileIdx != fileIdx) {
				if (VFXReader->ExtractFile(fileIdx, content)) {
					form->mTexturesDatabase->LoadAliasesFromData(content.data(), content.size());
				}
			}
		}
	}

	// Load config.bin
	fileIdx = VFXReader->FindFile("content\\config.bin");
	if (MetroFile::InvalidFileIdx != fileIdx) {
		BytesArray content;
		if (VFXReader->ExtractFile(fileIdx, content)) {
			form->mConfigsDatabase = new MetroConfigsDatabase();
			form->mConfigsDatabase->LoadFromData(content.data(), content.size());
		}
	}
}