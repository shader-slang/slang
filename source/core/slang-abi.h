#ifndef SLANG_CORE_ABI_H
#define SLANG_CORE_ABI_H

#include "../../slang.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "slang-smart-pointer.h"

#include "slang-dictionary.h"
#include "slang-semantic-version.h"
#include "slang-memory-arena.h"

namespace Slang {

struct AbiUtil
{
    typedef slang::AbiStructTypeValue TypeValue;

    struct TypeInfo
    {
        slang::AbiKind kind;                ///< The kind
        slang::AbiCategory category;        ///< The category
        uint8_t type;                       ///< Type for the category
        uint8_t majorVersion;               ///< The major semantic version
        uint8_t minorVersion;               ///< The minor semantic version
    };

    static TypeInfo getTypeInfo(TypeValue value) {  return _getTypeInfo((value & slang::kAbiPrimaryMask) ? slang::AbiKind::PrimaryStruct : slang::AbiKind::ExtensionStruct, value); }
    static TypeInfo getTypeInfo(slang::AbiExtensionType type) { return _getTypeInfo(slang::AbiKind::ExtensionStruct, TypeValue(type)); } 
    static TypeInfo getTypeInfo(slang::AbiPrimaryType type) { return _getTypeInfo(slang::AbiKind::PrimaryStruct, TypeValue(type)); }

        /// Get the category and type from the value
    static slang::AbiCategoryAndType getCategoryAndType(TypeValue value) { return slang::AbiCategoryAndType((value & slang::kAbiCategoryTypeMask) >> slang::kAbiCategoryTypeShift); }

        /// This will *only* determine if *just* this type is compatible for read and not if it contains other types (say in the form of extensions)
    static bool isReadCompatible(TypeValue value, TypeValue currentValue)
    {
        // Uniquely identifies the 'type'.
        const TypeValue typeMask = slang::kAbiCategoryTypeMajorMask;
        const TypeValue minorMask = slang::kAbiMinorMask;

        // If they are the same type, and the input types minor is greater than equal to current minor we can accept for read (singly)
        return ((value ^ currentValue) & typeMask) == 0 && (value & minorMask) >= (currentValue & minorMask);
    }

private:
    static TypeInfo _getTypeInfo(slang::AbiKind kind, TypeValue value)
    {
        TypeInfo info;
        info.kind = kind;
        info.category = slang::AbiCategory((value & slang::kAbiCategoryMask) >> slang::kAbiCategoryShift);
        info.type = uint8_t((value & slang::kAbiCategoryTypeMask)  >> slang::kAbiCategoryShift);
        info.majorVersion = uint8_t((value & slang::kAbiMajorMask) >> slang::kAbiMajorShift);
        info.minorVersion = uint8_t((value & slang::kAbiMinorMask) >> slang::kAbiMinorShift);
        return info;
    }
};

class AbiSystem : public RefObject
{
public:
    typedef slang::AbiStructTypeValue CategoryAndType;

    struct TypeInfo
    {
        slang::AbiStructTypeValue m_type;       ///< The type/current version
        String m_name;                          ///< The name of the type
        size_t m_sizeInBytes;                   ///< The size of this version in bytes
    };

        /// Add a category
    void addCategory(uint32_t category, const String& name);
    void addCategory(slang::AbiCategory category, const String& name) { addCategory(uint32_t(category), name); }

    void addType(slang::AbiPrimaryType primaryType, const String qualifiedName, size_t sizeInBytes) { addType(slang::AbiStructTypeValue(primaryType), qualifiedName, sizeInBytes); }
    void addType(slang::AbiExtensionType extensionType, const String qualifiedName, size_t sizeInBytes) { addType(slang::AbiStructTypeValue(extensionType), qualifiedName, sizeInBytes); }

    void addType(slang::AbiStructTypeValue typeValue, const String qualifiedName, size_t sizeInBytes);

    template <typename T>
    const T* getReadCompatible(const void* in, MemoryArena& arena) { return reinterpret_cast<const T*>(getReadCompatible(in, arena)); }

    const void* getReadCompatible(const void* in, MemoryArena& arena);

        /// Make a copy of the in structure (in the arena) such that it conforms to current versions, and return the copy
    void* clone(const void* in, MemoryArena& arena);

        /// Get type info
    const TypeInfo* getTypeInfo(slang::AbiStructTypeValue value);

protected:

    struct CategoryInfo
    {
        String m_name;
    };

    // If we want to find all the types in a category we can search through the m_typeInfos list.
    List<CategoryInfo> m_categories;                                    ///< Info about a category
    Dictionary<slang::AbiCategoryAndType, Index> m_typeToInfoIndex;     ///< Maps category and type to info
    List<TypeInfo> m_typeInfos;                                         ///< All of the type infos
};

} // namespace Slang

#endif // SLANG_CORE_BLOB_H
