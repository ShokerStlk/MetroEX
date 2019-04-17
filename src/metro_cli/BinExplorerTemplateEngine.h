#pragma once

using namespace System;
using namespace System::Xml;

class IMetroBinArchive;
class MetroBinConfigs;
class MetroProps;
class MemStream;

ref class BinExplorerTemplate {
public:
    static String^ kTypeBinArchive        = "BinArchive";
    static String^ kTypeBinArrayArchive   = "BinArrayArchive";

    ref struct SchemaInfo {
        String^ name;
        String^ header;
        String^ type;
        String^ verifyDbgInfo;

        SchemaInfo(XmlElement^ schemaElem) {
            name = schemaElem->GetAttribute("bin_name");
            assert(name != nullptr && name->Length > 0);
            if (name == nullptr || name->Length == 0) {
                name = "__unknown__.bin";
            }

            header = schemaElem->GetAttribute("bin_header");
            assert(header != nullptr && (header->Length == 4 || header->Length == 0));
            if (header == nullptr || (header->Length != 0 || header->Length != 4)) {
                header = "";
            }

            type = schemaElem->GetAttribute("bin_type");
            assert(type != nullptr);
            if (type == nullptr) {
                type = kTypeBinArchive;
            }

            verifyDbgInfo = schemaElem->GetAttribute("bin_verify_dbg_info");
            if (verifyDbgInfo == nullptr || verifyDbgInfo->Length == 0) {
                verifyDbgInfo = "False";
            }
        }
    };

    inline static String^ BinExplorerTemplate::TemplateReadVersion(XmlDocument^ templateDoc) {
        return templateDoc->DocumentElement->GetAttribute("template_version");
    }

    inline static XmlElement^ BinExplorerTemplate::TemplateGetRootSchema(XmlDocument^ templateDoc) {
        return templateDoc->GetElementById("bin_schema");
    }

    // Convert raw MemStream to IMetroBinArchive by template
    static std::shared_ptr<IMetroBinArchive> ExtractBinArchive(String^ templatePath, const MemStream& binStream);

    // Extract data from raw MemStream \ IMetroBinArchive by template
    static std::shared_ptr<MetroBinConfigs> ExtractBinArchiveData(String^ templatePath, IMetroBinArchive* inputBinArchive);
    static std::shared_ptr<MetroBinConfigs> ExtractBinArchiveData(String^ templatePath, const MemStream& binStream);

private:
    static MetroProps ReadValuesByPropsList(XmlNodeList^ propsList, const CharString& binName, const StringArray& strings, MetroReflectionReader& reader, bool verify);
    static bool CheckPropertyCondition(MetroProps& binProps, XmlNode^ propNode);

public:
    static void                                 PrintPropsToLog         (const MetroProps& binProps);
    static XmlDocument^                         ExportToXML             (String^ templatePath, MetroBinConfigs& binData);
    static std::shared_ptr<MetroBinConfigs>     ImportFromXML           (String^ inputPath);
};
