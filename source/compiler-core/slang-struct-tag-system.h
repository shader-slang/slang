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
    static uint32_t getTypeIndex(slang::StructTag tag) { return uint32_t((slang::StructTagInt(tag) & slang::kStructTagTypeIndexMask)  >> slang::kStructTagTypeIndexShift); }

        /// Get the category
    static slang::StructTagCategory getCategory(slang::StructTag tag) { return slang::StructTagCategory((slang::StructTagInt(tag) & slang::kStructTagCategoryMask) >> slang::kStructTagCategoryShift); }

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

inline  /* static */StructTagUtil::TypeInfo StructTagUtil::getTypeInfo(slang::StructTag tag)
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

struct StructTagType 
{
public:

    enum class FieldType : uint8_t
    {
        Unknown,
        Ptr,
        PtrPtr,
        I32,
        I64,
    };

        /// We can have a 'field' that is made up of 2 elements, so we have two entries.
        /// If m_typeB is Unknown, then the entry can be ignored
    struct Field
    {
        FieldType m_typeA;
        FieldType m_typeB;
        uint16_t m_offsetA;
        uint16_t m_offsetB;
    };

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

struct StructTagTypeTraits
{
    typedef StructTagType::FieldType Type;
    typedef StructTagType::Field Field;

    template <typename T>
    struct Impl { static Type getType() { return Type::Unknown; } };

    // Doesn't currently handle fixed arrays, but could be added quite easily, with say a byte for the fixed size. 
    
    // Integer types
    // We won't bother with sign for now
    template <> struct Impl<uint64_t> { static Type getType() { return Type::I64; } };
    template <> struct Impl<int64_t> { static Type getType() { return Type::I64; } };
    template <> struct Impl<uint32_t> { static Type getType() { return Type::I32; } };
    template <> struct Impl<int32_t> { static Type getType() { return Type::I32; } };

    template <typename T> struct Impl<T*>
    {
        static Type getType()
        {
            const Type innerType = Impl<T>::getType();
            if (innerType == Type::PtrPtr)
            {
                return Type::Unknown;
            }
            return innerType == Type::Ptr ? Type::PtrPtr : Type::Ptr;
        }
    };

    template <typename T, typename F>
    static uint16_t getOffset(T* obj, const F* f)
    {
        return uint16_t((const char*)f - (const char*)obj);
    }

        /// f1 should hold the count
    template <typename T, typename F0, typename F1>
    static Field getFieldTypeWithCount(const T* obj, const F0* ptr, const F1* count)
    {
        Field field;
        field.m_typeA = Impl<F0>::getType();
        field.m_typeB = Impl<F1>::getType();
        field.m_offsetA = getOffset(obj, ptr);
        field.m_offsetB = getOffset(obj, count);
        return field;
    }
};

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


    slang::StructTagCategory m_category;      ///< The category type
    String m_name;                              ///< The name
    
    // All the types in this category
    List<StructTagType*> m_types;
};

class StructTagSystem : public RefObject
{
public:

    enum class CompatibilityResult
    {
        Compatible,                 ///< Compatible as is
        ConvertCompatible,          ///< Compatible if converted
        Incompatible,               ///< Cannot be made compatible
    };

        /// Add a category
    StructTagCategoryInfo* addCategoryInfo(slang::StructTagCategory category, const String& name);
    StructTagCategoryInfo* getCategoryInfo(slang::StructTagCategory category);

        /// Determine the compatibility
    CompatibilityResult calcPtrArrayCompatible(const void*const* in, Index count);
    CompatibilityResult calcArrayCompatible(const void* in, Index count);
    CompatibilityResult calcCompatible(const void* in);

    template <typename T>
    const T* getReadCompatible(const void* in, MemoryArena& arena) { return reinterpret_cast<const T*>(getReadCompatible(in, arena)); }

    const void* getReadCompatible(const void* in, MemoryArena& arena);

    template <typename T>
    const T* getReadArray(const void* in, MemoryArena& arena) { return reinterpret_cast<const T*>(getArray(in, arena)); }
    const void* getReadArray(const void* in, Index count, MemoryArena& arena);

        /// Copies type
    SlangResult copy(const StructTagType* type, void* dst, const void* src);

        /// Make a copy of the in structure (in the arena) such that it conforms to current versions, and return the copy
    void* clone(const void* in, MemoryArena& arena);

        /// Get struct type 
    StructTagType* getType(slang::StructTag tag);

        /// Add the struct type
    StructTagType* addType(slang::StructTag tag, const String& name, size_t sizeInBytes);
    
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
