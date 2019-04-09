#pragma once
#include "mycommon.h"
#include "mymath.h"


#define METRO_MAKE_TYPE_ALIAS_STRING_NAME(type, alias)  sMetroRegisteredType##type##Alias##alias##Str
#define METRO_MAKE_TYPE_ARRAY_ALIAS_STRING_NAME(type, alias)  sMetroRegisteredTypeArray##type##Alias##alias##Str

template <typename T>
const CharString& MetroTypeGetAlias() {
    static const CharString empty;
    assert(false);
    return empty;
}

template <typename T>
const CharString& MetroTypeArrayGetAlias() {
    static const CharString empty;
    assert(false);
    return empty;
}


#define METRO_REGISTER_TYPE_ALIAS(type, alias)                                                  \
template <> inline const CharString& MetroTypeGetAlias<type>() {                                \
    static const CharString METRO_MAKE_TYPE_ALIAS_STRING_NAME(type, alias) = STRINGIFY(alias);  \
    return METRO_MAKE_TYPE_ALIAS_STRING_NAME(type, alias);                                      \
}


#define METRO_REGISTER_TYPE_ARRAY_ALIAS(type, alias)                                                            \
template <> inline const CharString& MetroTypeArrayGetAlias<type>() {                                           \
    static const CharString METRO_MAKE_TYPE_ARRAY_ALIAS_STRING_NAME(type, alias) = STRINGIFY(alias) "_array";   \
    return METRO_MAKE_TYPE_ARRAY_ALIAS_STRING_NAME(type, alias);                                                \
}


#define METRO_REGISTER_INHERITED_TYPE_ALIAS(type, baseType, alias)                                                      \
template <> inline const CharString& MetroTypeGetAlias<type>() {                                                        \
    static const CharString METRO_MAKE_TYPE_ALIAS_STRING_NAME(type, alias) = STRINGIFY(alias) ", " STRINGIFY(baseType); \
    return METRO_MAKE_TYPE_ALIAS_STRING_NAME(type, alias);                                                              \
}



METRO_REGISTER_TYPE_ALIAS(bool, bool)
METRO_REGISTER_TYPE_ALIAS(uint8_t, u8)
METRO_REGISTER_TYPE_ALIAS(uint16_t, u16)
METRO_REGISTER_TYPE_ALIAS(uint32_t, u32)
METRO_REGISTER_TYPE_ALIAS(int8_t, s8)
METRO_REGISTER_TYPE_ALIAS(int16_t, s16)
METRO_REGISTER_TYPE_ALIAS(int32_t, s32)
METRO_REGISTER_TYPE_ALIAS(float, fp32)
METRO_REGISTER_TYPE_ALIAS(vec2, vec2f)
METRO_REGISTER_TYPE_ALIAS(vec3, vec3f)
METRO_REGISTER_TYPE_ALIAS(vec4, vec4f)
METRO_REGISTER_TYPE_ALIAS(quat, vec4f)
METRO_REGISTER_TYPE_ALIAS(CharString, stringz)
METRO_REGISTER_TYPE_ALIAS(RefString, stringz)

METRO_REGISTER_INHERITED_TYPE_ALIAS(color4f, vec4f, color)

METRO_REGISTER_TYPE_ARRAY_ALIAS(bool, bool)
METRO_REGISTER_TYPE_ARRAY_ALIAS(uint8_t, u8)
METRO_REGISTER_TYPE_ARRAY_ALIAS(uint16_t, u16)
METRO_REGISTER_TYPE_ARRAY_ALIAS(uint32_t, u32)
METRO_REGISTER_TYPE_ARRAY_ALIAS(float, fp32)


class MetroReflectionReader {
public:
    MetroReflectionReader(const MemStream& s, const bool verifyTypeInfo = false, const bool refStrings = false)
        : mStream(s)
        , mVerifyTypesInfo(verifyTypeInfo)
        , mReadRefStrings(refStrings)
    {}

    inline MemStream& GetStream() {
        return mStream;
    }

    void SetOptions(const bool verifyTypeInfo = false, const bool refStrings = false) {
        mVerifyTypesInfo = verifyTypeInfo;
        mReadRefStrings = refStrings;
    }

    bool ReadEditorTag(const CharString& propName) {
        CharString name = mStream.ReadStringZ();
        assert(name == propName);
        if (name != propName) {
            return false;
        }

        CharString type = mStream.ReadStringZ();
        return true;
    }

    bool VerifyTypeInfo(const CharString& propName, const CharString& typeAlias) {
        if (mVerifyTypesInfo) {
            CharString name = mStream.ReadStringZ();
            assert(name == propName);
            if (name != propName) {
                return false;
            }

            CharString type = mStream.ReadStringZ();
            assert(type == typeAlias);
            if (type != typeAlias) {
                return false;
            }
        }
        return true;
    }

    CharString BeginSection() {
        uint32_t dataCrc, dataSize;
        uint8_t flags;
        CharString sectionName;

        (*this) >> dataCrc;
        (*this) >> dataSize;
        (*this) >> flags;       //#TODO_SK: check ???
        (*this) >> sectionName;

        return sectionName;
    }

    template <typename T>
    void ReadStructArray(const CharString& memberName, MyArray<T>& v) {
        if (mVerifyTypesInfo) {
            this->VerifyTypeInfo(memberName, "array");
            CharString& sectionName = this->BeginSection();
            assert(sectionName == memberName);
            this->VerifyTypeInfo("count", MetroTypeGetAlias<uint32_t>());
        }
        uint32_t arraySize;
        (*this) >> arraySize;
        if (arraySize > 0) {
            v.resize(arraySize);
            for (T& e : v) {
                if (mVerifyTypesInfo) {
                    this->BeginSection();
                }

                (*this) >> e;
            }
        }
    }

    template <typename T>
    inline void operator >>(T& v) {
        v.Serialize(*this);
    }

#define IMPLEMENT_SIMPLE_TYPE_READ(type)    \
    void operator >>(type& v) {             \
        v = mStream.ReadTyped<type>();      \
    }

    IMPLEMENT_SIMPLE_TYPE_READ(bool)
    IMPLEMENT_SIMPLE_TYPE_READ(int8_t)
    IMPLEMENT_SIMPLE_TYPE_READ(uint8_t)
    IMPLEMENT_SIMPLE_TYPE_READ(int16_t)
    IMPLEMENT_SIMPLE_TYPE_READ(uint16_t)
    IMPLEMENT_SIMPLE_TYPE_READ(int32_t)
    IMPLEMENT_SIMPLE_TYPE_READ(uint32_t)
    IMPLEMENT_SIMPLE_TYPE_READ(float)

#undef IMPLEMENT_SIMPLE_TYPE_READ

    inline void operator >>(CharString& v) {
        v = mStream.ReadStringZ();
    }

    inline void operator >>(RefString& v) {
        if (mReadRefStrings) {
            (*this) >> v.ref;
        } else {
            (*this) >> v.str;
        }
    }

    inline void operator >>(vec2& v) {
        (*this) >> v.x;
        (*this) >> v.y;
    }

    inline void operator >>(vec3& v) {
        (*this) >> v.x;
        (*this) >> v.y;
        (*this) >> v.z;
    }

    inline void operator >>(vec4& v) {
        (*this) >> v.x;
        (*this) >> v.y;
        (*this) >> v.z;
        (*this) >> v.w;
    }

    inline void operator >>(quat& v) {
        (*this) >> v.x;
        (*this) >> v.y;
        (*this) >> v.z;
        (*this) >> v.w;
    }

    inline void operator >>(color4f& v) {
        (*this) >> v.r;
        (*this) >> v.g;
        (*this) >> v.b;
        (*this) >> v.a;
    }


#define IMPLEMENT_TYPE_ARRAY_READ(type)     \
    void operator >>(MyArray<type>& v) {      \
        uint32_t numElements = 0;           \
        (*this) >> numElements;               \
        v.resize(numElements);              \
        for (type& e : v) {                 \
            (*this) >> e;                     \
        }                                   \
    }

    IMPLEMENT_TYPE_ARRAY_READ(int8_t)
    IMPLEMENT_TYPE_ARRAY_READ(uint8_t)
    IMPLEMENT_TYPE_ARRAY_READ(int16_t)
    IMPLEMENT_TYPE_ARRAY_READ(uint16_t)
    IMPLEMENT_TYPE_ARRAY_READ(int32_t)
    IMPLEMENT_TYPE_ARRAY_READ(uint32_t)
    IMPLEMENT_TYPE_ARRAY_READ(float)

#undef IMPLEMENT_TYPE_ARRAY_READ

private:
    MemStream   mStream;
    bool        mVerifyTypesInfo;
    bool        mReadRefStrings;
};


template <typename T>
struct ArrayElementTypeGetter {
    typedef typename T::value_type elem_type;
};


#define METRO_READ_MEMBER_NO_VERIFY(s, memberName)  s >> memberName;

#define METRO_READ_MEMBER(s, memberName)                                                \
    s.VerifyTypeInfo(STRINGIFY(memberName), MetroTypeGetAlias<decltype(memberName)>()); \
    s >> memberName;

#define METRO_READ_ARRAY_MEMBER(s, memberName)                                                                                  \
    s.VerifyTypeInfo(STRINGIFY(memberName), MetroTypeArrayGetAlias<ArrayElementTypeGetter<decltype(memberName)>::elem_type>()); \
    s >> memberName;

#define METRO_READ_STRUCT_ARRAY_MEMBER(s, memberName)   s.ReadStructArray(STRINGIFY(memberName), memberName)

#define METRO_READ_MEMBER_CHOOSE(s, memberName)                                         \
    s.ReadEditorTag(STRINGIFY(memberName));                                             \
    s.VerifyTypeInfo(STRINGIFY(memberName), MetroTypeGetAlias<decltype(memberName)>()); \
    s >> memberName;

#define METRO_READ_MEMBER_STRARRAY_CHOOSE(s, memberName)                        \
    s.ReadEditorTag(STRINGIFY(memberName));                                     \
    s.VerifyTypeInfo(STRINGIFY(memberName), MetroTypeGetAlias<CharString>());   \
    { CharString tmpStr; do {                                                   \
        s >> tmpStr;                                                            \
        if (!tmpStr.empty()) {                                                  \
            memberName.push_back(tmpStr);                                       \
        }                                                                       \
    } while (!tmpStr.empty()); }

