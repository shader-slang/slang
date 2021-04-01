// slang-serialize-reflection.h
#ifndef SLANG_SERIALIZE_REFLECTION_H
#define SLANG_SERIALIZE_REFLECTION_H

#include "../compiler-core/slang-name.h"

namespace Slang
{

struct ReflectClassInfo
{
    typedef ReflectClassInfo ThisType;

    typedef void* (*CreateFunc)(void* context);
    typedef void(*DestructorFunc)(void* ptr);

    /// A constant time implementation of isSubClassOf
    SLANG_FORCE_INLINE bool isSubClassOf(const ThisType& super) const
    {
        // We include super.m_classId, because it's a subclass of itself.
        return m_classId >= super.m_classId && m_classId <= super.m_lastClassId;
    }

    SLANG_FORCE_INLINE static bool isValidTypeId(uint32_t typeId) { return int32_t(typeId) >= 0; }

    // True if typeId derives from this type
    SLANG_FORCE_INLINE bool isDerivedFrom(uint32_t typeId) const
    {
        SLANG_ASSERT(isValidTypeId(typeId) && isValidTypeId(m_classId));
        return typeId >= m_classId && typeId <= m_lastClassId;
    }

    SLANG_FORCE_INLINE static bool isSubClassOf(uint32_t type, const ThisType& super)
    {
        SLANG_ASSERT(isValidTypeId(type) && isValidTypeId(super.m_classId));
        // We include super.m_classId, because it's a subclass of itself.
        return type >= super.m_classId && type <= super.m_lastClassId;
    }

        /// Will produce the same result as isSubClassOf (if enumerated), but more slowly by traversing the m_superClass
        /// Works without initRange being called. 
    bool isSubClassOfSlow(const ThisType& super) const;

        /// Calculate infos m_classId for all the infos specified such that they are honor the inheritance relationship
        /// such that a m_classId of a child is > m_classId && <= m_lastClassId
    static void calcClassIdHierachy(uint32_t baseIndex, ReflectClassInfo*const* infos, Index infosCount);

    uint32_t m_classId;                         ///< Not necessarily set.
    uint32_t m_lastClassId;

    const ReflectClassInfo* m_superClass;       ///< The super class of this class, or nullptr if has no super class. 
    const char* m_name;                         ///< Textual class name, for debugging 
    CreateFunc m_createFunc;                    ///< Callback to use when creating instances (using an ASTBuilder for backing memory)
    DestructorFunc m_destructorFunc;            ///< The destructor for this type. Being just destructor, does not free backing memory for type.

    uint32_t m_sizeInBytes;                     ///< Total size of the type
    uint8_t m_alignment;                        ///< The required alignment of the type
};

// Does nothing - just a mark to the C++ extractor
#define SLANG_REFLECTED
#define SLANG_UNREFLECTED

#define SLANG_PRE_DECLARE(SUFFIX, DEF)

#define SLANG_TYPE_SET(SUFFIX, ...)

// Use these macros to help define Super, and making the base definition NOT have a Super definition.
// For example something like...

#define SLANG_CLASS_REFLECT_SUPER_BASE(SUPER)
#define SLANG_CLASS_REFLECT_SUPER_INNER(SUPER) typedef SUPER Super;
#define SLANG_CLASS_REFLECT_SUPER_LEAF(SUPER) typedef SUPER Super;

// Mark a value class
#define SLANG_VALUE_CLASS(x)

} // namespace Slang

#endif
