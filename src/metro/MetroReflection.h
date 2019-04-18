#pragma once
#include "mycommon.h"
#include "mymath.h"
#include "hashing.h"

struct MetroReflectionFlags {
    static const uint8_t None           = 0;
    static const uint8_t HasDebugInfo   = 1;
    static const uint8_t Editor         = 2;
    static const uint8_t StringsTable   = 4;
    static const uint8_t Plain          = 8;
    static const uint8_t NoSections     = 16;
    static const uint8_t MultiChunk     = 32;
};


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

METRO_REGISTER_INHERITED_TYPE_ALIAS(color4f, vec4f, color)

METRO_REGISTER_TYPE_ARRAY_ALIAS(bool, bool)
METRO_REGISTER_TYPE_ARRAY_ALIAS(uint8_t, u8)
METRO_REGISTER_TYPE_ARRAY_ALIAS(uint16_t, u16)
METRO_REGISTER_TYPE_ARRAY_ALIAS(uint32_t, u32)
METRO_REGISTER_TYPE_ARRAY_ALIAS(float, fp32)


class MetroReflectionReader {
public:
    MetroReflectionReader()
        : mStream()
        , mSTable(nullptr)
        , mFlags(MetroReflectionFlags::None) {
    }

    MetroReflectionReader(const MemStream& s, const uint8_t flags = MetroReflectionFlags::None)
        : mStream(s)
        , mSTable(nullptr)
        , mFlags(flags) {
    }

    MetroReflectionReader(const MetroReflectionReader& other)
        : mStream(other.mStream)
        , mSTable(other.mSTable)
        , mFlags(other.mFlags)
        , mSectionName(other.mSectionName) {
    }

    MetroReflectionReader(MetroReflectionReader&& other)
        : mStream(std::move(other.mStream))
        , mSTable(other.mSTable)
        , mFlags(other.mFlags)
        , mSectionName(std::move(other.mSectionName)) {
    }

    inline MetroReflectionReader& operator =(const MetroReflectionReader& other) {
        mStream = other.mStream;
        mSTable = other.mSTable;
        mFlags = other.mFlags;
        mSectionName = other.mSectionName;
        return *this;
    }

    inline MetroReflectionReader& operator =(MetroReflectionReader&& other) {
        mStream = std::move(other.mStream);
        mSTable = other.mSTable;
        mFlags = other.mFlags;
        mSectionName = std::move(other.mSectionName);
        return *this;
    }

    inline MemStream& GetStream() {
        return mStream;
    }

    inline bool Good() const {
        return mStream.Good();
    }

    inline bool HasDebugInfo() const {
        return TestBit(mFlags, MetroReflectionFlags::HasDebugInfo);
    }

    inline bool HasStringsTable() const {
        return TestBit(mFlags, MetroReflectionFlags::StringsTable);
    }

    inline bool HasNoSections() const {
        return TestBit(mFlags, MetroReflectionFlags::NoSections);
    }

    void SetSTable(const StringsTable* stable) {
        mSTable = stable;
    }

    void SetSectionName(const CharString& sectionName) {
        mSectionName = sectionName;
    }

    const CharString& GetSectionName() const {
        return mSectionName;
    }

    bool ReadEditorTag(const CharString& propName) {
        //static CharString sChooseStr("choose");

        if (this->HasDebugInfo()) {
            CharString name = mStream.ReadStringZ();
            assert(name == propName);
            if (name != propName) {
                return false;
            }

            CharString chooseStr = mStream.ReadStringZ();
            //#TODO_SK: different choose attributes could be
            //assert(chooseStr == sChooseStr);
            //if (chooseStr != sChooseStr) {
            //    return false;
            //}
        }
        return true;
    }

    bool VerifyTypeInfo(const CharString& propName, const CharString& typeAlias) {
        if (this->HasDebugInfo()) {
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

    MetroReflectionReader OpenSection(const CharString& sectionName, const bool nameUnknown = false) {
        if (this->HasNoSections()) {
            return *this;
        } else {
            const uint32_t crc = sectionName.empty() ? 0u : Hash_CalculateCRC32(sectionName);

            const uint32_t sectionNameCrc = *rcast<const uint32_t*>(mStream.GetDataAtCursor());
            if (nameUnknown || sectionNameCrc == crc) {
                mStream.SkipBytes(sizeof(uint32_t));

                uint32_t sectionSize;
                uint8_t flags;
                (*this) >> sectionSize;
                (*this) >> flags;

                const size_t dataSize = sectionSize - 1;
                //#TODO_SK: replace with constants
                MetroReflectionReader result(mStream.Substream(dataSize), flags);
                result.SetSTable(mSTable);

                if (this->HasDebugInfo()) {
                    CharString name;
                    result >> name;
                    if (!nameUnknown) {
                        assert(sectionName == name);
                    }
                }

                return std::move(result);
            } else {
                return MetroReflectionReader();
            }
        }
    }

    void CloseSection(const MetroReflectionReader& section) {
        if (section.Good()) {
            if (this->HasNoSections()) {
                //#NOTE_SK: since it's our shadow copy, we just sync our cursors
                mStream.SetCursor(section.mStream.GetCursor());
            } else {
                mStream.SkipBytes(section.mStream.Length());
            }
        }
    }

    template <typename T>
    void ReadStruct(const CharString& memberName, T& v) {
        MetroReflectionReader s = this->OpenSection(memberName);
        if (s.Good()) {
            s >> v;
        }
        this->CloseSection(s);
    }

    template <typename T>
    void ReadStructArray(const CharString& memberName, MyArray<T>& v) {
        this->VerifyTypeInfo(memberName, "array");

        MetroReflectionReader s = this->OpenSection(memberName);
        if (s.Good()) {
            s.VerifyTypeInfo("count", MetroTypeGetAlias<uint32_t>());

            uint32_t arraySize;
            s >> arraySize;
            if (arraySize > 0) {
                v.resize(arraySize);
                for (T& e : v) {
                    MetroReflectionReader subS = s.OpenSection(kEmptyString, true);
                    if (subS.Good()) {
                        subS >> e;
                    }
                    s.CloseSection(subS);
                }
            }
        }
        this->CloseSection(s);
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
        if (this->HasStringsTable()) {
            uint32_t ref;
            (*this) >> ref;
            if (ref != kInvalidValue32 && mSTable) {
                v = mSTable->GetString(ref);
            }
        } else {
            v = mStream.ReadStringZ();
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
    void operator >>(MyArray<type>& v) {    \
        uint32_t numElements = 0;           \
        (*this) >> numElements;             \
        v.resize(numElements);              \
        for (type& e : v) {                 \
            (*this) >> e;                   \
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
    MemStream           mStream;
    const StringsTable* mSTable;
    uint8_t             mFlags;
    CharString          mSectionName;
};


template <typename T>
struct ArrayElementTypeGetter {
    typedef typename T::value_type elem_type;
};


#define METRO_READ_MEMBER_NO_VERIFY(s, memberName)  s >> memberName;

#define METRO_READ_MEMBER(s, memberName)                                                \
    s.VerifyTypeInfo(STRINGIFY(memberName), MetroTypeGetAlias<decltype(memberName)>()); \
    s >> memberName;

#define METRO_READ_STRUCT_MEMBER(s, memberName) s.ReadStruct(STRINGIFY(memberName), memberName)

#define METRO_READ_ARRAY_MEMBER(s, memberName)                                                                                  \
    s.VerifyTypeInfo(STRINGIFY(memberName), MetroTypeArrayGetAlias<ArrayElementTypeGetter<decltype(memberName)>::elem_type>()); \
    s >> memberName;

#define METRO_READ_STRUCT_ARRAY_MEMBER(s, memberName) s.ReadStructArray(STRINGIFY(memberName), memberName)

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

