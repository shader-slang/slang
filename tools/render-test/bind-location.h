#ifndef BIND_LOCATION_H
#define BIND_LOCATION_H

#include "core/slang-basic.h"
#include "core/slang-free-list.h"
#include "core/slang-memory-arena.h"

#include "slang.h"

namespace renderer_test {

struct BindPoint
{
    Slang::Index m_space = 0;               ///< The register space 
    size_t m_offset = 0;                    ///< The offset, might be a byte address or a register index
};

struct BindPoints
{
    BindPoint m_points[SLANG_PARAMETER_CATEGORY_COUNT];
};

class BindPointSet : public Slang::RefObject
{
public:
    typedef Slang::RefObject Super;

    BindPointSet(const BindPoints& points) :
        m_points(points)
    {
    }
    BindPointSet() {}

    BindPoints m_points;
};

struct BindSet_Resource
{
    slang::TypeReflection::Kind m_kind;              ///< The kind, used if type is not set. Same as m_type.kind othewise
    slang::TypeLayoutReflection* m_type;            ///< The type
    uint8_t* m_data;
    size_t m_sizeInBytes;                           ///< Total size in bytes
    size_t m_elementCount;                          ///< Only applicable on an array like type, else 0
    void* m_userData;
};

class BindSet;

// A problem is how to handle the root case.
// We know the roots are constant buffers on CPU and CUDA
// On other bindings, that isn't the case. In that case it's target specific how to handle that.
// We can add some assumptions to make this work though by assuming first X resources are something specific for a specific lookup
struct BindLocation
{
    typedef BindLocation ThisType;
    
    bool isValid() const { return m_typeLayout != nullptr; }
    bool isInvalid() const { return m_typeLayout == nullptr; }

    BindLocation toField(const char* name) const;
    BindLocation toIndex(int index) const;

    const BindPointSet* getPointSet() const { return m_bindPointSet; }
    void setRanges(const BindPoints& points)
    {
        SLANG_ASSERT(m_bindPointSet);
        if (m_bindPointSet->isUniquelyReferenced())
        {
            m_bindPointSet->m_points = points;
        }
        else
        {
            m_bindPointSet = new BindPointSet(points);
        }
    }

    BindLocation() {}

    BindLocation(BindSet* bindSet, slang::TypeLayoutReflection* typeLayout, const BindPoints& points) :
        m_bindSet(bindSet),
        m_typeLayout(typeLayout)
    {
        SLANG_ASSERT(bindSet);
        m_bindPointSet = new BindPointSet(points);
    }
    BindLocation(BindSet* bindSet, slang::TypeLayoutReflection* typeLayout, BindSet_Resource* resource, size_t offset) :
        m_bindSet(bindSet),
        m_resource(resource),
        m_offset(offset),
        m_typeLayout(typeLayout)
    {
        SLANG_ASSERT(bindSet);
    }

    BindLocation(BindSet* bindSet, slang::TypeLayoutReflection* typeLayout, SlangParameterCategory category, size_t categoryOffset, Slang::Index space) :
        m_bindSet(bindSet),
        m_category(category),
        m_offset(categoryOffset),
        m_typeLayout(typeLayout),
        m_space(space)
    {
        SLANG_ASSERT(bindSet);
    }
    size_t getOffset(SlangParameterCategory category) const;

    BindLocation(const ThisType& rhs) = default;

    BindSet* m_bindSet = nullptr;                           ///< Bind set we are traversing
    slang::TypeLayoutReflection* m_typeLayout = nullptr;    ///< The type layout

    BindSet_Resource* m_resource = nullptr;                ///< The resource if we are in (or nullptr if not in a resource)

    SlangParameterCategory m_category = SLANG_PARAMETER_CATEGORY_NONE;          
    size_t m_offset = 0;                                    ///< The offset within the resource or category. 
    Slang::Index m_space = 0;                               ///< Only applicable when m_category is used

    Slang::RefPtr<BindPointSet> m_bindPointSet;             ///< NOTE! Can only be written to if there is a single reference
};

class BindSet
{
public:
    typedef BindSet_Resource Resource;

    struct UniformLocation
    {
        typedef UniformLocation ThisType;

        int GetHashCode() const
        {
            return Slang::combineHash(Slang::GetHashCode(m_resource), Slang::GetHashCode(m_offset));
        }
        bool operator==(const ThisType& rhs) const { return m_resource == rhs.m_resource && m_offset == rhs.m_offset; }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        Resource* m_resource;               ///< Resource it's in
        size_t m_offset;                    ///< Offset in the resource
    };

    struct RegisterLocation
    {
        typedef RegisterLocation ThisType;

        int GetHashCode() const
        {
            return Slang::combineHash(Slang::combineHash(Slang::GetHashCode(m_space), Slang::GetHashCode(m_offset)), Slang::GetHashCode(m_category));
        }
        bool operator==(const ThisType& rhs) const
        {
            return m_category == rhs.m_category &&
                m_space == rhs.m_space &&
                m_offset == rhs.m_offset;
        }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        SlangParameterCategory m_category;      ///< The category of parameter
        Slang::Index m_space;                   ///< Space (not applicable when it's in a resource)
        size_t m_offset;                        ///< Can be a byte offset (say in a resource), or a register offset
    };

    Resource* getAt(Resource* resource, size_t offset) const;
    void setAt(Resource* resource, size_t offset, Resource* value);

    Resource* getAt(SlangParameterCategory category, Slang::Index space, size_t offset);
    void setAt(SlangParameterCategory category, Slang::Index space, size_t offset, Resource* value);

    Resource* getAt(const BindLocation& loc);

    Resource* newBufferResource(slang::TypeLayoutReflection* type, size_t sizeInBytes);
    Resource* newBufferResource(slang::TypeReflection::Kind kind, slang::TypeLayoutReflection* typeLayout, size_t sizeInBytes);

    Resource* newBufferResource(slang::TypeLayoutReflection* type, size_t sizeInBytes, const void* initialData);
    Resource* newBufferResource(slang::TypeReflection::Kind kind, slang::TypeLayoutReflection* typeLayout, size_t sizeInBytes, const void* initialData);

    BindSet();
protected:

    Slang::List<Resource*> m_resources;

    Slang::Dictionary<RegisterLocation, Resource*> m_registerBindings;
    Slang::Dictionary<UniformLocation, Resource*> m_uniformBindings;

    Slang::MemoryArena m_arena;
};




} // renderer_test

#endif //BIND_LOCATION_H
