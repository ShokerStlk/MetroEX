#pragma once

#include <mycommon.h>
#include <msclr/marshal_cppstd.h>

#include "metro/MetroReflection.h"
#include "MetroProps.h"

using namespace System;
using namespace msclr::interop;

/*
 HOW TO ADD NEW TEMPLATE CLASS
 1) Create new MetroProp_XXX class (SIMPLE_PROP_CLASS)
 2) Create new MetroProp_XXX_array class, if needed (SIMPLE_PROP_ARRAY_CLASS)
 3) Create new convertion rule in ConvertToMetroProp()
*/

#pragma region (1) MetroProp_XXX

#define SIMPLE_PROP_CLASS(def_TypeGame, def_TypeReal, def_ConvertEq)                                            \
class MetroProp_##def_TypeGame : public IMetroProp {                                                            \
public:                                                                                                         \
    def_TypeReal mValue;                                                                                        \
                                                                                                                \
    MetroProp_##def_TypeGame(const CharString& propName) :                                                      \
            IMetroProp(propName, STRINGIFY(def_TypeGame)) { ; }                                                 \
                                                                                                                \
    virtual void ReadProp(MetroReflectionReader& reader, const StringArray& strings, bool verify) override {    \
        if (verify) {                                                                                           \
            METRO_READ_MEMBER_MANUAL_VERIFY(reader, mValue, GetPropName());                                     \
        } else {                                                                                                \
            METRO_READ_MEMBER_SKIP_VERIFY(reader, mValue);                                                      \
        }                                                                                                       \
    };                                                                                                          \
                                                                                                                \
    virtual void ReadProp(String^ strValue) override {                                                          \
        mValue = def_ConvertEq##(strValue);                                                                     \
    };                                                                                                          \
                                                                                                                \
    virtual String^ ToString() override {                                                                       \
        return String::Format("{0}", mValue);                                                                   \
    };                                                                                                          \
                                                                                                                \
    virtual bool propEqual(String^ refVal) override {                                                           \
        return mValue == def_ConvertEq##(refVal);                                                               \
    };                                                                                                          \
                                                                                                                \
    virtual bool propSmaller(String^ refVal) override {                                                         \
        return mValue < def_ConvertEq##(refVal);                                                                \
    };                                                                                                          \
                                                                                                                \
    virtual bool propBigger(String^ refVal) override {                                                          \
        return mValue > def_ConvertEq##(refVal);                                                                \
    };                                                                                                          \
};

SIMPLE_PROP_CLASS(bool, bool,       Convert::ToBoolean)
SIMPLE_PROP_CLASS(fp32, float,      Convert::ToSingle)
SIMPLE_PROP_CLASS(u8,   uint8_t,    Convert::ToByte)
SIMPLE_PROP_CLASS(u16,  uint16_t,   Convert::ToUInt16)
SIMPLE_PROP_CLASS(u32,  uint32_t,   Convert::ToUInt32)
SIMPLE_PROP_CLASS(s8,   int8_t,     Convert::ToSByte)
SIMPLE_PROP_CLASS(s16,  int16_t,    Convert::ToInt16)
SIMPLE_PROP_CLASS(s32,  int32_t,    Convert::ToInt32)

#undef SIMPLE_PROP_CLASS

class MetroProp_stringZ : public IMetroProp {
public:
    RefString mValue;
    bool      mIsChoose;

    MetroProp_stringZ(const CharString& propName, bool isChoose = false) : IMetroProp(propName, "stringZ") {
        mIsChoose = isChoose;
    }

    inline CharString Text() {
        return mValue.str;
    }

    inline String^ TextManaged() {
        return marshal_as<String^>(Text());
    }

    static CharString AddValueFromStringTable(RefString& str, const StringArray& strings) {
        if (str.ref != RefString::InvalidRef) {
            size_t stSize = strings.size();
            assert(stSize > str.ref);
            str.str = strings[str.ref];
        }
        return str.str;
    }

    static void AddValuesFromStringTable(MyArray<RefString> strArray, const StringArray& strings) {
        for (RefString& str : strArray)
        {
            AddValueFromStringTable(str, strings);
        }
    }

    static RefString ToStringZ(String^ input) {
        RefString result;
        result.ref = RefString::InvalidRef;
        result.str = marshal_as<CharString>(input);
        return result;
    }

    virtual void ReadProp(MetroReflectionReader& reader, const StringArray& strings, bool verify) override {
        // Read raw value
        if (mIsChoose) {
            if (verify) {
                METRO_READ_MEMBER_CHOOSE_MANUAL_VERIFY(reader, mValue, GetPropName());
            } else {
                METRO_READ_MEMBER_CHOOSE_SKIP_VERIFY(reader, mValue);
            }
        } else {
            if (verify) {
                METRO_READ_MEMBER_MANUAL_VERIFY(reader, mValue, GetPropName());
            } else {
                METRO_READ_MEMBER_SKIP_VERIFY(reader, mValue);
            }
        }

        // Add string from StringTable
        AddValueFromStringTable(mValue, strings);
    };

    virtual void ReadProp(String^ strValue) override {
        mValue = ToStringZ(strValue);
    };

    static String^ ToString(const RefString& input) {
        if (input.str.length() > 0) {
            return marshal_as<String^>(input.str);
        }
        if (input.ref == RefString::InvalidRef) {
            return "";
        }
        return String::Format("(ref) {0}", input.ref);
    }

    virtual String^ ToString() override {
        return ToString(mValue);
    }

    virtual bool propEqual(String^ refVal) override {
        assert("propEqual not implemented for stringZ" == "use propStrEqual");
        return false;
    };

    virtual bool propSmaller(String^ refVal) override {
        assert("propSmaller implemented for stringZ" == "false");
        return false;
    };

    virtual bool propBigger(String^ refVal) override {
        assert("propBigger implemented for stringZ" == "false");
        return false;
    };

    virtual bool propStrEqual(String^ refVal) {
        return TextManaged()->Equals(refVal);
    };

    virtual bool propStrContain(String^ refVal) {
        return TextManaged()->Contains(refVal);
    };
};

class MetroProp_vec3f : public IMetroProp {
public:
    vec3 mValue;

    MetroProp_vec3f(const CharString& propName) : IMetroProp(propName, "vec3f") { ; }

    static vec3 ToVec3f(String^ input) {
        vec3 result = vec3(0.0f, 0.0f, 0.0f);
        if (input->Length > 0) {
            array<String^>^ strList = input->Split(',');
            for (int i = 0; i < 3; i++) {
                if (strList[i] != nullptr) {
                    result[i] = Convert::ToSingle(strList[i]->Trim());
                }
            }
        }
        return result;
    }

    virtual void ReadProp(MetroReflectionReader& reader, const StringArray& strings, bool verify) override {
        if (verify) {
            METRO_READ_MEMBER_MANUAL_VERIFY(reader, mValue, GetPropName());
        } else {
            METRO_READ_MEMBER_SKIP_VERIFY(reader, mValue);
        }
    };

    virtual void ReadProp(String^ strValue) override {
        mValue = ToVec3f(strValue);
    };

    static String^ ToString(const vec3& input) {
        return String::Format("{0}, {1}, {2}", input.x, input.y, input.z);
    }

    virtual String^ ToString() override {
        return ToString(mValue);
    }

    virtual bool propEqual(String^ refVal) override {
        return mValue == ToVec3f(refVal);
    };

    virtual bool propSmaller(String^ refVal) override {
        vec3 ref = ToVec3f(refVal);
        return (
            mValue.x < ref.x &&
            mValue.y < ref.y &&
            mValue.z < ref.z
        );
    };

    virtual bool propBigger(String^ refVal) override {
        vec3 ref = ToVec3f(refVal);
        return (
            mValue.x > ref.x &&
            mValue.y > ref.y &&
            mValue.z > ref.z
        );
    };
};

class MetroProp_vec4f : public IMetroProp {
public:
    vec4 mValue;

    MetroProp_vec4f(const CharString& propName) : IMetroProp(propName, "vec4f") { ; }

    static vec4 ToVec4f(String^ input) {
        vec4 result = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        if (input->Length > 0) {
            array<String^>^ strList = input->Split(',');
            for (int i = 0; i < 4; i++) {
                if (strList[i] != nullptr) {
                    result[i] = Convert::ToSingle(strList[i]->Trim());
                }
            }
        }
        return result;
    }

    virtual void ReadProp(MetroReflectionReader& reader, const StringArray& strings, bool verify) override {
        if (verify) {
            METRO_READ_MEMBER_MANUAL_VERIFY(reader, mValue, GetPropName());
        } else {
            METRO_READ_MEMBER_SKIP_VERIFY(reader, mValue);
        }
    };

    virtual void ReadProp(String^ strValue) override {
        mValue = ToVec4f(strValue);
    };

    static String^ ToString(const vec4& input) {
        return String::Format("{0}, {1}, {2}, {3}", input.x, input.y, input.z, input.w);
    }

    virtual String^ ToString() override {
        return ToString(mValue);
    }

    virtual bool propEqual(String^ refVal) override {
        return mValue == ToVec4f(refVal);
    };

    virtual bool propSmaller(String^ refVal) override {
        vec4 ref = ToVec4f(refVal);
        return (
            mValue.x < ref.x &&
            mValue.y < ref.y &&
            mValue.z < ref.z &&
            mValue.w < ref.w
        );
    };

    virtual bool propBigger(String^ refVal) override {
        vec4 ref = ToVec4f(refVal);
        return (
            mValue.x > ref.x &&
            mValue.y > ref.y &&
            mValue.z > ref.z &&
            mValue.w > ref.w
        );
    };
};

class MetroProp_color4f : public IMetroProp {
public:
    color4f mValue;

    MetroProp_color4f(const CharString& propName) : IMetroProp(propName, "color4f") { ; }

    static color4f ToColor4f(String^ input) {
        return MetroProp_vec4f::ToVec4f(input);
    }

    virtual void ReadProp(MetroReflectionReader& reader, const StringArray& strings, bool verify) override {
        if (verify) {
            METRO_READ_MEMBER_MANUAL_VERIFY(reader, mValue, GetPropName());
        } else {
            METRO_READ_MEMBER_SKIP_VERIFY(reader, mValue);
        }
    };

    virtual void ReadProp(String^ strValue) override {
        mValue = ToColor4f(strValue);
    };

    static String^ ToString(const color4f& input) {
        return MetroProp_vec4f::ToString(input);
    }

    virtual String^ ToString() override {
        return ToString(mValue);
    }

    virtual bool propEqual(String^ refVal) override {
        return mValue == ToColor4f(refVal);
    };

    virtual bool propSmaller(String^ refVal) override {
        color4f ref = ToColor4f(refVal);
        return (
            mValue.x < ref.x &&
            mValue.y < ref.y &&
            mValue.z < ref.z &&
            mValue.w < ref.w
        );
    };

    virtual bool propBigger(String^ refVal) override {
        color4f ref = ToColor4f(refVal);
        return (
            mValue.x > ref.x &&
            mValue.y > ref.y &&
            mValue.z > ref.z &&
            mValue.w > ref.w
        );
    };
};

#pragma endregion

#pragma region (2) MetroProp_XXX_array

#define SIMPLE_PROP_ARRAY_CLASS(def_TypeGame, def_TypeReal, def_ToString, def_ReadExtra, def_ConvertEq)         \
class MetroProp_##def_TypeGame : public IMetroProp {                                                            \
public:                                                                                                         \
    MyArray<##def_TypeReal##> mValue;                                                                           \
                                                                                                                \
    MetroProp_##def_TypeGame(const CharString& propName) :                                                      \
            IMetroProp(propName, STRINGIFY(def_TypeGame)) { ; }                                                 \
                                                                                                                \
    virtual void ReadProp(MetroReflectionReader& reader, const StringArray& strings, bool verify) override {    \
        if (verify) {                                                                                           \
            METRO_READ_ARRAY_MEMBER_MANUAL_VERIFY(reader, mValue, GetPropName());                               \
        } else {                                                                                                \
            METRO_READ_ARRAY_MEMBER_SKIP_VERIFY(reader, mValue);                                                \
        }                                                                                                       \
        def_ReadExtra;                                                                                          \
    };                                                                                                          \
                                                                                                                \
    virtual void ReadProp(String^ strValue) override {                                                          \
        using namespace System::Text;                                                                           \
        if (strValue->Length > 0) {                                                                             \
            if (strValue[0] == '|') {                                                                           \
                StringBuilder ^sb = gcnew StringBuilder(strValue);                                              \
                sb[0] = ' ';                                                                                    \
                strValue = sb->ToString();                                                                      \
            }                                                                                                   \
            array<String^>^ strList = strValue->Split('|');                                                     \
            mValue.reserve(strList->Length);                                                                    \
            for (int i = 0; i < strList->Length; i++) {                                                         \
                String^ strPart = strList[i];                                                                   \
                if (strPart == nullptr || strPart == "") {                                                      \
                    assert("invalid array format" == "true");                                                   \
                    continue;                                                                                   \
                }                                                                                               \
                mValue.push_back(def_ConvertEq##(strPart->Trim()));                                             \
            }                                                                                                   \
        }                                                                                                       \
    };                                                                                                          \
                                                                                                                \
    virtual String^ ToString() override {                                                                       \
        String^ output = "";                                                                                    \
        for (auto& elem : mValue) {                                                                             \
            output += String::Format("|{0} ", def_ToString);                                                    \
        }                                                                                                       \
        return output;                                                                                          \
    };                                                                                                          \
                                                                                                                \
    virtual bool propEqual(String^ refVal) override {                                                           \
        assert("propEqual implemented for Array" == "false");                                                   \
        return false;                                                                                           \
    };                                                                                                          \
                                                                                                                \
    virtual bool propSmaller(String^ refVal) override {                                                         \
        assert("propSmaller implemented for Array" == "false");                                                 \
        return false;                                                                                           \
    };                                                                                                          \
                                                                                                                \
    virtual bool propBigger(String^ refVal) override {                                                          \
        assert("propBigger implemented for Array" == "false");                                                  \
        return false;                                                                                           \
    };                                                                                                          \
};

SIMPLE_PROP_ARRAY_CLASS(fp32_array,       float,        elem, ;, Convert::ToSingle)
SIMPLE_PROP_ARRAY_CLASS(u8_array,         uint8_t,      elem, ;, Convert::ToByte)
SIMPLE_PROP_ARRAY_CLASS(u16_array,        uint16_t,     elem, ;, Convert::ToUInt16)
SIMPLE_PROP_ARRAY_CLASS(u32_array,        uint32_t,     elem, ;, Convert::ToUInt32)
SIMPLE_PROP_ARRAY_CLASS(s8_array,         int8_t,       elem, ;, Convert::ToSByte)
SIMPLE_PROP_ARRAY_CLASS(s16_array,        int16_t,      elem, ;, Convert::ToInt16)
SIMPLE_PROP_ARRAY_CLASS(s32_array,        int32_t,      elem, ;, Convert::ToInt32)
SIMPLE_PROP_ARRAY_CLASS(stringZ_array,    RefString,    MetroProp_stringZ::ToString(elem), MetroProp_stringZ::AddValuesFromStringTable(mValue, strings), MetroProp_stringZ::ToStringZ)
SIMPLE_PROP_ARRAY_CLASS(vec3f_array,      vec3,         MetroProp_vec3f::ToString(elem), ;, MetroProp_vec3f::ToVec3f)
SIMPLE_PROP_ARRAY_CLASS(vec4f_array,      vec4,         MetroProp_vec4f::ToString(elem), ;, MetroProp_vec4f::ToVec4f)
SIMPLE_PROP_ARRAY_CLASS(color4f_array,    color4f,      MetroProp_color4f::ToString(elem), ;, MetroProp_color4f::ToColor4f)

#undef SIMPLE_PROP_ARRAY_CLASS

#pragma endregion

#pragma region (3) Convertion rules

static std::shared_ptr<IMetroProp> ConvertToMetroProp(String^ propName, String^ propType) {
    std::shared_ptr<IMetroProp> prop = nullptr;

    do {
        #define CONVERT_TO_PROP(def_TypeGame)                                                           \
            if (propType->Equals(STRINGIFY(def_TypeGame))) {                                            \
                prop = std::make_shared<MetroProp_##def_TypeGame##>(marshal_as<CharString>(propName));  \
                break;                                                                                  \
            }

        CONVERT_TO_PROP(bool)
        CONVERT_TO_PROP(fp32)
        CONVERT_TO_PROP(u8)
        CONVERT_TO_PROP(u16)
        CONVERT_TO_PROP(u32)
        CONVERT_TO_PROP(s8)
        CONVERT_TO_PROP(s16)
        CONVERT_TO_PROP(s32)
        CONVERT_TO_PROP(vec3f)
        CONVERT_TO_PROP(vec4f)
        CONVERT_TO_PROP(color4f)

        CONVERT_TO_PROP(fp32_array)
        CONVERT_TO_PROP(u8_array)
        CONVERT_TO_PROP(u16_array)
        CONVERT_TO_PROP(u32_array)
        CONVERT_TO_PROP(s8_array)
        CONVERT_TO_PROP(s16_array)
        CONVERT_TO_PROP(s32_array)
        CONVERT_TO_PROP(stringZ_array)
        CONVERT_TO_PROP(vec3f_array)
        CONVERT_TO_PROP(vec4f_array)
        CONVERT_TO_PROP(color4f_array)

        if (propType->Equals("stringZ")) {
            prop = std::make_shared<MetroProp_stringZ>(marshal_as<CharString>(propName), false);
            break;
        }

        if (propType->Equals("stringZ_choose")) {
            prop = std::make_shared<MetroProp_stringZ>(marshal_as<CharString>(propName), true);
            break;
        }

        assert("unknown type to read!" == propType);
#undef REGISTRE_GAME_TYPE
    } while (false);

    return prop;
}

#pragma endregion
