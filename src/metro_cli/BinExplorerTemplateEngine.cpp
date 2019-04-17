#include <mycommon.h>
#include <msclr/marshal_cppstd.h>

#include "metro/MetroBinArrayArchive.h"
#include "metro/MetroBinArchive.h"
#include "MetroProps.h"
#include "MetroPropsTypes.h"

#include "BinExplorerTemplateEngine.h"

using namespace System;
using namespace System::Xml;
using namespace System::Collections::Generic;
using namespace msclr::interop;

std::shared_ptr<IMetroBinArchive> BinExplorerTemplate::ExtractBinArchive(String^ templatePath, const MemStream& binStream) {
    // <!> binStream must be window-ed (SetWindow / Substream) to only contain data from single .bin file

    // Load XML template
    XmlDocument^ templateDoc = gcnew XmlDocument();
    templateDoc->Load(templatePath);

    if (templateDoc == nullptr) {
        assert("Can't find template by this path" == "true");
        return nullptr;
    }

    // Open root schema
    XmlElement^ rootSchemaElem = TemplateGetRootSchema(templateDoc);
    if (rootSchemaElem == nullptr) {
        assert("Can't find root schema" == "true");
        return nullptr;
    }

    // Read generic .bin data
    SchemaInfo^ binSchema = gcnew SchemaInfo(rootSchemaElem);

    // Convert to IMetroBinArchive
    std::shared_ptr<IMetroBinArchive> bin = nullptr;
    String^ binSchemaName   = binSchema->name;
    String^ binSchemaHeader = binSchema->header;

    if (binSchema->type->Equals(kTypeBinArchive)) {
        bin = std::static_pointer_cast<IMetroBinArchive>(
                std::make_shared<MetroBinArchive>(
                    marshal_as<CharString>(binSchemaName),
                    binStream.Substream(binStream.Remains()), // <!>
                    MetroBinArchive::kHeaderDoAutoSearch
                )
        );
    }
    else if (binSchema->type->Equals(kTypeBinArrayArchive)) {
        bin = std::static_pointer_cast<IMetroBinArchive>(
                std::make_shared<MetroBinArrayArchive>(
                    marshal_as<CharString>(binSchemaName),
                    binStream.Substream(binStream.Remains()), // <!>
                    marshal_as<CharString>(binSchemaHeader)
                )
        );
    }
    else {
        assert(binSchema->type == "unknown bin_type in XML");
        return nullptr;
    }

    return bin;
}

std::shared_ptr<MetroBinConfigs> BinExplorerTemplate::ExtractBinArchiveData(String^ templatePath, IMetroBinArchive* inputBinArchive) {
    // Load XML template
    XmlDocument^ templateDoc = gcnew XmlDocument();
    templateDoc->Load(templatePath);

    if (templateDoc == nullptr) {
        assert("Can't find template by this path" == "true");
        return nullptr;
    }

    // Open root schema
    XmlElement^ rootSchemaElem = TemplateGetRootSchema(templateDoc);
    if (rootSchemaElem == nullptr) {
        assert("Can't find root schema" == "true");
        return nullptr;
    }

    // Read generic .bin data
    SchemaInfo^ binSchema = gcnew SchemaInfo(rootSchemaElem);

    // Validate binType, prepare bin object
    array<MetroBinArchive*>^ arrayOfBin; // array of all sub-.bin-s inside inputBinArchive

    int resultArraySize = 0;

    if (inputBinArchive->IsBinArchive()) {
        // Get bin's count
        assert(binSchema->type->Equals(kTypeBinArchive));
        resultArraySize = 1;
        // Fill array with bin's
        arrayOfBin = gcnew array<MetroBinArchive*>(resultArraySize);
        arrayOfBin[0] = (MetroBinArchive*)inputBinArchive;
    }
    else if (inputBinArchive->IsBinArrayArchive()) {
        // Get bin's count
        assert(binSchema->type->Equals(kTypeBinArrayArchive));
        MetroBinArrayArchive* binArrayArchive = (MetroBinArrayArchive*)inputBinArchive;
        resultArraySize = (int)binArrayArchive->GetBinCnt();
        // Fill array with bin's
        arrayOfBin = gcnew array<MetroBinArchive*>(resultArraySize);
        for (int i = 0; i < resultArraySize; i++) {
            arrayOfBin[i] = &(binArrayArchive->GetBinByIdx(i));
        }
    }
    else {
        assert(binSchema->type == "unknown or wrong bin_type in XML ");
        return nullptr;
    }

    // Prepare result array
    std::shared_ptr<MetroBinConfigs> result  = std::make_shared<MetroBinConfigs>(inputBinArchive->GetFileName(), resultArraySize);
    MetroConfigsArray& dataArray             = result->mConfigs;

    // Get list of all props
    XmlNodeList^ propsList = rootSchemaElem->GetElementsByTagName("property");
    if (propsList->Count == 0) {
        assert(propsList->Count > 0);
        return nullptr;
    }

    // Read all .bin data by props list
    bool verify = binSchema->verifyDbgInfo != nullptr && Convert::ToBoolean(binSchema->verifyDbgInfo) == true;
    for (int i = 0; i < arrayOfBin->Length; ++i) {
        const MetroBinArchive& bin = *arrayOfBin[i];
        StringArray strings = bin.ReadStringTable();
        MetroReflectionReader reader = bin.ReturnReflectionReader(bin.GetOffsetFirstDataBegin());
        dataArray.push_back(ReadValuesByPropsList(propsList, bin.GetFileName(), strings, reader, verify));
    }

    // Return result
    return result;
}

std::shared_ptr<MetroBinConfigs> BinExplorerTemplate::ExtractBinArchiveData(String^ templatePath, const MemStream& binStream) {
    std::shared_ptr<IMetroBinArchive> binArchive = BinExplorerTemplate::ExtractBinArchive(
        templatePath,
        binStream
    );

    if (binArchive == nullptr) {
        return nullptr;
    }

    std::shared_ptr<MetroBinConfigs> binData = BinExplorerTemplate::ExtractBinArchiveData(
        templatePath,
        binArchive.get()
    );

    return binData;
}

MetroProps BinExplorerTemplate::ReadValuesByPropsList(XmlNodeList^ propsList, const CharString& binName, const StringArray& strings, MetroReflectionReader& reader, bool verify) {
    MetroProps binProps = MetroProps(binName, propsList->Count); // contain actual props

    for each(XmlNode^ propNode in propsList)
    {
        // Read prop name and type
        String^ propName = propNode->Attributes["name"]->Value;
        String^ propType = propNode->Attributes["type"]->Value;

        // Conditions parser
        if (CheckPropertyCondition(binProps, propNode) == false) {
            continue;
        }

        // Add new empty prop
        std::shared_ptr<IMetroProp> prop = ConvertToMetroProp(propName, propType);

        if (prop == nullptr) {
            continue;
        }

        // Read property
        prop->ReadProp(reader, strings, verify);

        // Add prop to list
        binProps.mProps[prop->GetPropName()] = prop;
    }

    // Debug print
    // PrintPropsToLog(binProps);

    // Return result
    return binProps;
}

bool BinExplorerTemplate::CheckPropertyCondition(MetroProps& binProps, XmlNode^ propNode) {
    MetroConfigProps& props = binProps.mProps;

    if (propNode->HasChildNodes)
    {
        for (int i = 0; i < propNode->ChildNodes->Count; i++)
        {
            XmlNode^ condNode = propNode->ChildNodes[i];
            if (condNode->Name->Equals("condition")) {
                String^ cid = condNode->Attributes["cid"]->Value;
                do {
                    if (cid->Equals("propEqual")) {
                        String^ _propToCheck = condNode["prop"]->InnerText;
                        String^ _targetValue = condNode["value"]->InnerText;

                        IMetroProp* propToCheck = props[marshal_as<CharString>(_propToCheck)].get();
                        assert(propToCheck != nullptr);

                        if (propToCheck == nullptr) {
                            continue;
                        }

                        if (propToCheck->propEqual(_targetValue) == false) {
                            return false;
                        }
                        break;
                    }
                    if (cid->Equals("propSmaller")) {
                        String^ _propToCheck = condNode["prop"]->InnerText;
                        String^ _targetValue = condNode["value"]->InnerText;

                        IMetroProp* propToCheck = props[marshal_as<CharString>(_propToCheck)].get();
                        assert(propToCheck != nullptr);

                        if (propToCheck == nullptr) {
                            continue;
                        }

                        if (propToCheck->propSmaller(_targetValue) == false) {
                            return false;
                        }
                        break;
                    }
                    if (cid->Equals("propBigger")) {
                        String^ _propToCheck = condNode["prop"]->InnerText;
                        String^ _targetValue = condNode["value"]->InnerText;

                        IMetroProp* propToCheck = props[marshal_as<CharString>(_propToCheck)].get();
                        assert(propToCheck != nullptr);

                        if (propToCheck == nullptr) {
                            continue;
                        }

                        if (propToCheck->propBigger(_targetValue) == false) {
                            return false;
                        }
                        break;
                    }
                    if (cid->Equals("propSmallerEqual")) {
                        String^ _propToCheck = condNode["prop"]->InnerText;
                        String^ _targetValue = condNode["value"]->InnerText;

                        IMetroProp* propToCheck = props[marshal_as<CharString>(_propToCheck)].get();
                        assert(propToCheck != nullptr);

                        if (propToCheck == nullptr) {
                            continue;
                        }

                        if (propToCheck->propEqual(_targetValue) == false &&
                            propToCheck->propSmaller(_targetValue) == false) {
                            return false;
                        }
                        break;
                    }
                    if (cid->Equals("propBiggerEqual")) {
                        String^ _propToCheck = condNode["prop"]->InnerText;
                        String^ _targetValue = condNode["value"]->InnerText;

                        IMetroProp* propToCheck = props[marshal_as<CharString>(_propToCheck)].get();
                        assert(propToCheck != nullptr);

                        if (propToCheck == nullptr) {
                            continue;
                        }

                        if (propToCheck->propEqual(_targetValue) == false &&
                            propToCheck->propBigger(_targetValue) == false) {
                            return false;
                        }
                        break;
                    }
                    if (cid->Equals("propStrEqual")) {
                        String^ _propToCheck = condNode["prop"]->InnerText;
                        String^ _targetValue = condNode["value"]->InnerText;

                        IMetroProp* propToCheck = props[marshal_as<CharString>(_propToCheck)].get();
                        assert(propToCheck != nullptr);

                        if (propToCheck == nullptr) {
                            continue;
                        }

                        if (propToCheck->GetPropType() == "stringZ") {
                            MetroProp_stringZ* propToCheckStr = (MetroProp_stringZ*)propToCheck;
                            if (propToCheckStr->propStrEqual(_targetValue) == false) {
                                return false;
                            }
                        }
                        else {
                            assert("propStrEqual can only work with stringZ property" == "false");
                            return false;
                        }
                        break;
                    }
                    if (cid->Equals("propStrContain")) {
                        String^ _propToCheck = condNode["prop"]->InnerText;
                        String^ _targetValue = condNode["value"]->InnerText;

                        IMetroProp* propToCheck = props[marshal_as<CharString>(_propToCheck)].get();
                        assert(propToCheck != nullptr);

                        if (propToCheck == nullptr) {
                            continue;
                        }

                        if (propToCheck->GetPropType() == "stringZ") {
                            MetroProp_stringZ* propToCheckStr = (MetroProp_stringZ*)propToCheck;
                            if (propToCheckStr->propStrContain(_targetValue) == false) {
                                return false;
                            }
                        }
                        else {
                            assert("propStrContain can only work with stringZ property" == "false");
                            return false;
                        }
                        break;
                    }

                    assert("unknown cid!" == cid);
                } while (false);
            }
        }
    }

    return true; // conditions passed
}

void BinExplorerTemplate::PrintPropsToLog(const MetroProps& binProps) {
    const MetroConfigProps& props = binProps.mProps;

    LogPrint(LogLevel::Info, "BEGIN PRINT >>> " + binProps.mConfigName);
    for (auto const& prop : props) // don't track insertion order
    {
        IMetroProp* propObj = prop.second.get();
        String^     propName = marshal_as<String^>(propObj->GetPropName());
        String^     propType = marshal_as<String^>(propObj->GetPropType());
        String^     propValue = propObj->ToString();

        String^ output = String::Format("{0,-25} = ({1}) {2}", propName, propType, propValue);
        LogPrint(LogLevel::Info, marshal_as<CharString>(output));
    }
    LogPrint(LogLevel::Info, "<<< END PRINT");
}

XmlDocument^ BinExplorerTemplate::ExportToXML(String^ templatePath, MetroBinConfigs& binData) {
    // Load XML template
    XmlDocument^ templateDoc = gcnew XmlDocument();
    templateDoc->Load(templatePath);

    if (templateDoc == nullptr) {
        assert("Can't find template by this path" == "true");
        return nullptr;
    }

    // Open root schema
    XmlElement^ templateRootSchemaElem = TemplateGetRootSchema(templateDoc);
    if (templateRootSchemaElem == nullptr) {
        assert("Can't find root schema" == "true");
        return nullptr;
    }

    // Read generic .bin data
    String^ templateVersion = TemplateReadVersion(templateDoc);
    SchemaInfo^ binSchema   = gcnew SchemaInfo(templateRootSchemaElem);

    // Get list of all props
    XmlNodeList^ propsList = templateRootSchemaElem->GetElementsByTagName("property");
    if (propsList->Count == 0) {
        assert(propsList->Count > 0);
        return nullptr;
    }

    // Create output xml
    XmlDocument^ outputDoc = gcnew XmlDocument();

    XmlComment^ headerComment = outputDoc->CreateComment(
        String::Format(" Generated at: {0} with template \"{1}\" ver. {2} ",
            DateTime::Now,
            System::IO::Path::GetFileName(templatePath),
            templateVersion
        )
    );
    outputDoc->AppendChild(headerComment);

    XmlElement^ rootNode = outputDoc->CreateElement("root");
    rootNode->SetAttribute("name", marshal_as<String^>(binData.mBinName));
    outputDoc->AppendChild(rootNode);

    // Save props to xml
    for (MetroProps& binProps : binData.mConfigs)
    {
        XmlElement^ configNode = outputDoc->CreateElement("config");
        configNode->SetAttribute("name", marshal_as<String^>(binProps.mConfigName));

        for each(XmlNode^ templatePropNode in propsList)
        {
            // Read prop name and type
            String^ propName = templatePropNode->Attributes["name"]->Value;
            String^ propType = templatePropNode->Attributes["type"]->Value;

            // Get prop
            IMetroProp* propObj = binProps.get(marshal_as<CharString>(propName));

            // Check prop condition
            if (CheckPropertyCondition(binProps, templatePropNode) == false) {
                continue;
            }

            // Write to XML
            if (propObj != nullptr) {
                String^ propName = marshal_as<String^>(propObj->GetPropName());
                String^ propType = marshal_as<String^>(propObj->GetPropType());
                String^ propValue = propObj->ToString();

                // Add prop
                XmlElement^ propNode = outputDoc->CreateElement(propName);
                propNode->SetAttribute("type", propType);
                propNode->SetAttribute("value", propValue);
                configNode->AppendChild(propNode);
            }
            else {
                assert("propExist" == "false");
            }
        }
        rootNode->AppendChild(configNode);
    }

    return outputDoc;
}

std::shared_ptr<MetroBinConfigs> BinExplorerTemplate::ImportFromXML(String^ inputPath) {
    // Create empty result
    std::shared_ptr<MetroBinConfigs> result = nullptr;

    // Load XML input
    XmlDocument^ inputDoc = gcnew XmlDocument();
    inputDoc->Load(inputPath);

    if (inputDoc == nullptr) {
        assert("Can't find input by this path" == "true");
        return nullptr;
    }

    String^ binName = inputDoc->DocumentElement->GetAttribute("name");

    // Get list of all configs
    XmlNodeList^ configList = inputDoc->GetElementsByTagName("config");
    if (configList->Count == 0) {
        assert(configList->Count > 0);
        return nullptr;
    }

    // Prepare result
    result = std::make_shared<MetroBinConfigs>(marshal_as<CharString>(binName), configList->Count);

    // Import every config
    for each(XmlElement^ configElem in configList)
    {
        String^ configName = configElem->GetAttribute("name");

        // Import every props (no validation)
        XmlNodeList^ propsList = configElem->ChildNodes;
        if (propsList->Count == 0) {
            assert(propsList->Count > 0);
            return nullptr;
        }

        MetroProps binProps = MetroProps(marshal_as<CharString>(configName), propsList->Count);

        for each(XmlElement^ propsElem in propsList)
        {
            String^ propName = propsElem->Name;
            String^ propType = propsElem->Attributes["type"]->Value;
            String^ propValue = propsElem->Attributes["value"]->Value;

            // Add new empty prop
            std::shared_ptr<IMetroProp> prop = ConvertToMetroProp(propName, propType);

            if (prop == nullptr) {
                continue;
            }

            // Read property
            prop->ReadProp(propValue);

            // Add prop to list
            binProps.mProps[prop->GetPropName()] = prop;
        }

        // Add prop-list to list
        result->mConfigs.push_back(binProps);
    }

    return result;
}
