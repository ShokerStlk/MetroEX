#pragma once

#include <mycommon.h>
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace msclr::interop;

// Abstract class for all metro properties classes
class IMetroProp {
public:
    const CharString mPropName;
    const CharString mPropType;

    IMetroProp(const CharString& propName, const CharString& propType) :
        mPropName(propName), mPropType(propType)
    {;}
    
    virtual const CharString&   GetPropName()                   { return mPropName; }
    virtual const CharString&   GetPropType()                   { return mPropType; }
    virtual       bool          IsSameType(IMetroProp* refProp) { return mPropType == refProp->GetPropType(); }

    virtual void                ReadProp(MetroReflectionReader& reader, const StringArray& strings, bool verify) = 0;
    virtual void                ReadProp(String^ strValue) = 0;

    virtual String^ ToString    ()                  = 0;
    virtual bool    propEqual   (String^ refVal)    = 0;
    virtual bool    propSmaller (String^ refVal)    = 0;
    virtual bool    propBigger  (String^ refVal)    = 0;
};

// Contain props data from single section \ config (MetroBinArchive)
typedef MyDict<CharString, std::shared_ptr<IMetroProp>> MetroConfigProps; // [property_name] = IMetroProp
class MetroProps {
public:
    const CharString    mConfigName;
    MetroConfigProps    mProps;

    MetroProps(const CharString& _binName, int sizeReserve = 0) : mConfigName(_binName) {
        if (sizeReserve > 0) {
            mProps.reserve(sizeReserve);
        }
    }

    inline bool propExist(const CharString& _propName) const {
        return mProps.find(_propName) != mProps.end();
    }

    template<typename T>
    T* get(const CharString& _propName)
    {
        if (propExist(_propName) == false) {
            return nullptr;
        }
        return static_cast<T*>(mProps[_propName].get());
    }

    IMetroProp* get(const CharString& _propName)
    {
        return get<IMetroProp>(_propName);
    }
};

// Might contain multiple data from MetroBinArrayArchive, or single data from MetroBinArchive
typedef MyArray<MetroProps> MetroConfigsArray; // [idx of config\bin] = MetroProps
class MetroBinConfigs {
public:
    const CharString    mBinName;
    MetroConfigsArray   mConfigs;

    MetroBinConfigs(const CharString& _binName, int sizeReserve = 0) : mBinName(_binName) {
        if (sizeReserve > 0) {
            mConfigs.reserve(sizeReserve);
        }
    }

    MetroProps* findConfig(const CharString& _binName)
    {
        for (MetroProps& binProps : mConfigs)
        {
            if (binProps.mConfigName == _binName) {
                return &binProps;
            }
        }

        return nullptr;
    }
};
