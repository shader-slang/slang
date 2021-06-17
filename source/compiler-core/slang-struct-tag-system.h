#ifndef SLANG_COMPILER_CORE_STRUCT_TAG_SYSTEM_H
#define SLANG_COMPILER_CORE_STRUCT_TAG_SYSTEM_H

#include "../../slang.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-smart-pointer.h"

#include "../core/slang-dictionary.h"
#include "../core/slang-semantic-version.h"
#include "../core/slang-memory-arena.h"

namespace Slang {

struct StructTagUtil
{
    struct TypeInfo
    {
        slang::StructTagKind kind;              ///< The kind
        slang::StructTagCategory category;      ///< The category 
        uint8_t typeIndex;                      ///< Type index for the category type
        uint8_t majorVersion;                   ///< The major semantic version
        uint8_t minorVersion;                   ///< The minor semantic version
    };

        /// True if it's a primary struct 
    static bool isPrimary(slang::StructTag tag) { return (slang::StructTagInt(tag) & slang::kStructTagPrimaryMask) != 0; }
        /// True if it's an extension
    static bool isExtension(slang::StructTag tag) { return !isPrimary(tag); }

    inline static TypeInfo getTypeInfo(slang::StructTag tag);
    
        /// Get the category and type from the value
    static slang::StructTagInt getCategoryTypeIndex(slang::StructTag tag) { return (slang::StructTagInt(tag) & slang::kStructTagCategoryTypeIndexMask) >> slang::kStructTagCategoryTypeIndexShift; }

        /// Get the type index
    static Index getTypeIndex(slang::StructTag tag) { return Index((slang::StructTagInt(tag) & slang::kStructTagTypeIndexMask)  >> slang::kStructTagTypeIndexShift); }

        /// Get the category
    static slang::StructTagCategory getCategory(slang::StructTag tag) { return slang::StructTagCategory((slang::StructTagInt(tag) & slang::kStructTagCategoryMask) >> slang::kStructTagCategoryShift); }

        /// They are the same type and have same major version
    static bool areSameMajorType(slang::StructTag a, slang::StructTag b)
    {
        const auto typeMask = slang::StructTagInt(slang::kStructTagCategoryTypeMajorMask);
        return ((slang::StructTagInt(a) ^ slang::StructTagInt(b)) & typeMask) == 0;
    }

        /// This will *only* determine if *just* this type is compatible for read and not if it contains other types (say in the form of extensions)
    static bool isReadCompatible(slang::StructTag inTag, slang::StructTag inCurrentTag)
    {
        // Uniquely identifies the 'type'.
        const auto typeMask = slang::StructTagInt(slang::kStructTagCategoryTypeMajorMask);
        const auto minorMask = slang::StructTagInt(slang::kStructTagMinorMask);

        const auto tag = slang::StructTagInt(inTag);
        const auto currentTag = slang::StructTagInt(inCurrentTag);

        // If they are the same type, and the input types minor is greater than equal to current minor we can accept for read (singly)
        return ((tag ^ currentTag) & typeMask) == 0 && (tag & minorMask) >= (currentTag & minorMask);
    }
};

/* static */ inline StructTagUtil::TypeInfo StructTagUtil::getTypeInfo(slang::StructTag tag)
{
    const auto intTag = slang::StructTagInt(tag);

    TypeInfo info;
    info.kind = (intTag & slang::kStructTagPrimaryMask) ? slang::StructTagKind::Primary : slang::StructTagKind::Extension;
    info.category = getCategory(tag);
    info.typeIndex = uint8_t(getTypeIndex(tag));
    info.majorVersion = uint8_t((intTag & slang::kStructTagMajorMask) >> slang::kStructTagMajorShift);
    info.minorVersion = uint8_t((intTag & slang::kStructTagMinorMask) >> slang::kStructTagMinorShift);
    return info;
}

/// We can have a 'field' that is made up of 2 elements, so we have two entries.
/// If m_countType is Unknown, then the entry can be ignored
struct StructTagField
{
    enum class Type : uint8_t
    {
        Unknown,
        TaggedStruct,
        PtrTaggedStruct,
        PtrPtrTaggedStruct,
        I32,
        I64,
    };

    SLANG_FORCE_INLINE static bool isInRange(Type type, Type start, Type end) { return Index(type) >= Index(start) && Index(type) <= Index(end); }

        /// True if it's an integral 
    static bool isIntegral(Type type) { return isInRange(type, Type::I32, Type::I64); }
        /// True if it's a pointer or pointer to a pointer
    static bool isPtrLike(Type type) { return isInRange(type, Type::PtrTaggedStruct, Type::PtrPtrTaggedStruct); }

    Type m_type;
    Type m_countType;
    uint16_t m_offset;
    uint16_t m_countOffset;
};

struct StructTagType 
{
public:
    typedef StructTagField Field;

    StructTagType(slang::StructTag tag, const String& name, size_t sizeInBytes):
        m_tag(tag),
        m_name(name),
        m_sizeInBytes(slang::StructSize(sizeInBytes))
    {
    }

    slang::StructTag m_tag;                 ///< The type/current version
    String m_name;                          ///< The name of the type
    slang::StructSize m_sizeInBytes;        ///< The size of this version in bytes

    List<Field> m_fields;                   ///< Fields that need to be followed
};

namespace StructTagTypeTraits
{
    typedef StructTagField Field;
    typedef Field::Type Type;
    
    // Helper that works out what a pointer to the inner type is.
    SLANG_FORCE_INLINE Type getPtrType(Type innerType)
    {
        switch (innerType)
        {
            case Type::TaggedStruct:    return Type::PtrTaggedStruct;
            case Type::PtrTaggedStruct: return Type::PtrPtrTaggedStruct;
            default:                    return Type::Unknown;
        }
    }

    template <typename T, typename F>
    SLANG_FORCE_INLINE uint16_t getOffset(T* obj, const F* f)
    {
        return uint16_t((const char*)f - (const char*)obj);
    }

    // Use `substitution failure is not an error` (SFINAE) to detect tagged struct types
    template <typename T>
    struct IsTaggedStruct
    {
        typedef int32_t True;
        typedef int8_t False;

        template <typename C>
        static True check(typename C::Tag*);
        template <typename>
        static False check(...);

        // Is != 0 if it is a TaggedStruct type
        enum { kValue = int(sizeof(check<T>(nullptr)) == sizeof(True)) };
    };

    template <typename T>
    struct Impl { static Type getType() { return IsTaggedStruct<T>::kValue ? Type::TaggedStruct : Type::Unknown; } };

    // Doesn't currently handle fixed arrays, but could be added quite easily, with say a byte for the fixed size. 
    
    // Integer types
    // We won't bother with sign for now
    template <> struct Impl<uint64_t> { static Type getType() { return Type::I64; } };
    template <> struct Impl<int64_t> { static Type getType() { return Type::I64; } };
    template <> struct Impl<uint32_t> { static Type getType() { return Type::I32; } };
    template <> struct Impl<int32_t> { static Type getType() { return Type::I32; } };

    // StructTag is used to indicate it can be any 'tagged struct type'
    template <> struct Impl<slang::StructTag> { static Type getType() { return Type::TaggedStruct; } };

    // Pointer
    template <typename T> struct Impl<T*> { static Type getType() { return getPtrType(Impl<T>::getType()); } };

        /// f1 should hold the count
    template <typename T, typename F0, typename F1>
    Field getFieldWithCount(const T* obj, const F0* ptr, const F1* count)
    {
        Field field;
        field.m_type = Impl<F0>::getType();
        field.m_countType = Impl<F1>::getType();
        field.m_offset = getOffset(obj, ptr);
        field.m_countOffset = getOffset(obj, count);

        SLANG_ASSERT(StructTagField::isPtrLike(field.m_type));
        SLANG_ASSERT(StructTagField::isIntegral(field.m_countType));

        return field;
    }
}

class StructTagCategoryInfo
{
public:
    
        /// Add a type. Will replace a type if there is already one setup for the m_Type
    void addType(StructTagType* type);

        /// Get a type
    StructTagType* getType(Index typeIndex) const { return typeIndex < m_types.getCount() ? m_types[typeIndex] : nullptr; } 

    StructTagCategoryInfo(slang::StructTagCategory category, const String& name) :
        m_category(category),
        m_name(name)
    {
    }
    ~StructTagCategoryInfo();

    slang::StructTagCategory m_category;    ///< The category type
    String m_name;                          ///< The name
    
    // All the types in this category
    List<StructTagType*> m_types;
};

/* Holds the information about TaggedStruct types. Use the StructTagConverter to actually convert to conforming
types. */
class StructTagSystem : public RefObject
{
public:

        /// Add a category
    StructTagCategoryInfo* addCategoryInfo(slang::StructTagCategory category, const String& name);
    StructTagCategoryInfo* getCategoryInfo(slang::StructTagCategory category);

        /// Get struct type 
    StructTagType* getType(slang::StructTag tag);

        /// Add the struct type
    StructTagType* addType(slang::StructTag tag, const String& name, size_t sizeInBytes);

    void appendName(slang::StructTag tag, StringBuilder& out);
    String getName(slang::StructTag tag) { StringBuilder buf; appendName(tag, buf); return buf.ProduceString(); }

    StructTagSystem():
        m_arena(1024)
    {
    }

    ~StructTagSystem();

protected:

        /// Arena stores all of the types
    MemoryArena m_arena;

        /// All of the categories
    List<StructTagCategoryInfo*> m_categories;
};

} // namespace Slang

#endif // SLANG_COMPILER_CORE_STRUCT_TAG_SYSTEM_H
