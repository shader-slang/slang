#ifndef BIND_LOCATION_H
#define BIND_LOCATION_H

#include "core/slang-basic.h"
#include "core/slang-free-list.h"
#include "core/slang-memory-arena.h"
#include "core/slang-writer.h"

#include "slang.h"

namespace renderer_test {

struct BindPoint
{
    typedef BindPoint ThisType;

        /// 
    bool isValid() const { return m_space >= 0; }
    bool isInvalid() const { return m_space < 0; }

    void setInvalid() { m_space = -1; }

    bool operator==(const ThisType& rhs) const { return m_space == rhs.m_space && m_offset == rhs.m_offset; }
    bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

    int GetHashCode() const { return Slang::combineHash(Slang::GetHashCode(m_space), Slang::GetHashCode(m_offset)); }

    BindPoint() = default;
    BindPoint(Slang::Index space, size_t offset):m_space(space), m_offset(offset) {}

    static BindPoint makeInvalid() { return BindPoint(-1, 0); }

    Slang::Index m_space = 0;               ///< The register space 
    size_t m_offset = 0;                    ///< The offset, might be a byte address or a register index
};

struct BindPoints
{
    Slang::Index findSingle() const
    {
        Slang::Index found; 
        if (calcValidCount(&found) == 1)
        {
            return found;
        }
        return -1;
    }
    Slang::Index calcValidCount(Slang::Index* outFoundIndex) const
    {
        using namespace Slang;
        Index found = -1;
        Index validCount = 0;
        for (Index i = 0; i < Index(SLANG_PARAMETER_CATEGORY_COUNT); ++i)
        {
            const auto& point = m_points[i];
            if (point.isValid())
            {
                found = i;
                validCount++;
            }
        }
        if (outFoundIndex)
        {
            *outFoundIndex = found;
        }
        return validCount;
    }
    void setInvalid()
    {
        for (auto& point : m_points) 
        {
            point.setInvalid();
        }
    }
    BindPoint& operator[](SlangParameterCategory category) { return m_points[Slang::Index(category)]; }
    const BindPoint& operator[](SlangParameterCategory category) const { return m_points[Slang::Index(category)]; }

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
    slang::TypeReflection::Kind m_kind;              ///< The kind, used if type is not set. Same as m_type.kind otherwise
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

    BindLocation toField(slang::VariableLayoutReflection* var) const;
    BindLocation toField(const char* name) const;
    BindLocation toIndex(Slang::Index index) const;

    const BindPointSet* getPointSet() const { return m_bindPointSet; }
    void setPoints(const BindPoints& points);

        /// Add an offset
    void addOffset(SlangParameterCategory category, ptrdiff_t offset);

        /// True if holds tracking for this category
    bool hasCategory(SlangParameterCategory category) const { return getBindPointForCategory(category).isValid(); }

    BindPoint getBindPointForCategory(SlangParameterCategory category) const;
    BindPoint* getValidBindPointForCategory(SlangParameterCategory category);
    const BindPoint* getValidBindPointForCategory(SlangParameterCategory category) const;
    slang::TypeLayoutReflection* getTypeLayout() const { return m_typeLayout; }

    void setEmptyBinding() { m_bindPointSet.setNull(); m_point = BindPoint::makeInvalid(); m_category = SLANG_PARAMETER_CATEGORY_NONE; }

    BindSet* getBindSet() const { return m_bindSet; }

    template <typename T>
    T* getUniform() const { return reinterpret_cast<T*>(getUniform(sizeof(T))); }
    void* getUniform(size_t size) const;

    SlangResult setInplace(const void* data, size_t sizeInBytes) const;
    SlangResult setBufferContents(const void* initialData, size_t sizeInBytes) const;

    BindLocation() {}
    BindLocation(BindSet* bindSet, slang::TypeLayoutReflection* typeLayout, const BindPoints& points, BindSet_Resource* resource = nullptr);
    BindLocation(BindSet* bindSet, slang::TypeLayoutReflection* typeLayout, SlangParameterCategory category, const BindPoint& point, BindSet_Resource* resource = nullptr);

    BindLocation(const ThisType& rhs) = default;

    BindSet* m_bindSet = nullptr;                           ///< Bind set we are traversing
    slang::TypeLayoutReflection* m_typeLayout = nullptr;    ///< The type layout

    BindSet_Resource* m_resource = nullptr;                 ///< The resource if we are in 

    SlangParameterCategory m_category = SLANG_PARAMETER_CATEGORY_NONE;  ///< If there isn't a set this defines the category
    BindPoint m_point;                                     ///< If there isn't a bind point set, this defines the point

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
            return Slang::combineHash(m_point.GetHashCode(), Slang::GetHashCode(m_category));
        }
        bool operator==(const ThisType& rhs) const
        {
            return m_category == rhs.m_category && m_point == rhs.m_point;
        }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        SlangParameterCategory m_category;      ///< The category of parameter
        BindPoint m_point;
    };

    Resource* getAt(Resource* resource, size_t offset) const;
    void setAt(Resource* resource, size_t offset, Resource* value);

    Resource* getAt(SlangParameterCategory category, const BindPoint& point);
    void setAt(SlangParameterCategory category, const BindPoint& point, Resource* value);

    Resource* getAt(const BindLocation& loc);
    void setAt(const BindLocation& loc, Resource* resource);
    void setAt(const BindLocation& loc, SlangParameterCategory category, Resource* resource);

    Resource* createBufferResource(slang::TypeLayoutReflection* type, size_t sizeInBytes, const void* initialData = nullptr);
    Resource* createBufferResource(slang::TypeReflection::Kind kind, size_t sizeInBytes, const void* initialData = nullptr);

    Resource* createTextureResource(slang::TypeLayoutReflection* type);

        /// Calculate from the current location everything that is referenced
    void calcResourceLocations(const BindLocation& location, Slang::List<BindLocation>& outLocations);
    void calcChildResourceLocations(const BindLocation& location, Slang::List<BindLocation>& outLocations);

    void destroyResource(Resource* resource);

        /// TODO(JS): Arghh! This doesn't work, quite as needed. I want to be able to get not only the binding locations
        /// and resources but I need to also know the type of where the binding is set.
        /// The map that stores whats where, does not store the type of the origin of the reference, only the resource it points to
        ///
        /// Now we have a few options. We could
        /// 1) Store the location type. It couldn't be part of the key, it would have to be stored with the resource(!)
        /// 2) Traverse from roots, looking for references, and then work out which ones have a binding
        /// 3) A hybrid? Get the roots we know their types. Go through all resources which have types, and add them.
        ///
        /// Note we can't just look at what resources exist and traverse their types, because in the case of CPU binding, too
        /// non typed constant buffers are created. 
        ///
        /// In some respects 2 is the cleanest, but for it to work we'd also need to keep track of resources that have already been
        /// traversed in a map. 
        ///
        /// I suppose this stuff only counts for uniform binding. 

        /// Get all of the set bindings. This can be used to set the final actual desired output.
    void getBindings(Slang::List<BindLocation>& outLocations, Slang::List<Resource*>& outResources);

    BindSet();

        /// True if is a texture type
    static bool isTextureType(slang::TypeLayoutReflection* typeLayout);

protected:
    Resource* _createBufferResource(slang::TypeReflection::Kind kind, slang::TypeLayoutReflection* typeLayout, size_t bufferSizeInBytes, size_t sizeInBytes, const void* initalData);
    
    Slang::List<Resource*> m_resources;

    Slang::Dictionary<RegisterLocation, Resource*> m_registerBindings;
    Slang::Dictionary<UniformLocation, Resource*> m_uniformBindings;

    Slang::MemoryArena m_arena;
};

class BindRoot : public Slang::RefObject
{
public:
    typedef RefObject Super;

    virtual BindLocation find(const char* name) = 0;
        /// The setting of an array count is dependent on the underlying implementation.
        /// On the CPU this means making sure there is a buffer that is large enough
        /// And using that for storage.
        /// But this does NOT set the actual location in the appropriate manner - that is
        /// something that has to be done by the process that sets all the 'resource' handles etc elsewhere
    virtual SlangResult setArrayCount(const BindLocation& location, int count) = 0;

        /// Find all of the roots 
    virtual void getRoots(Slang::List<BindLocation>& outLocations) = 0;

        /// Parse (specifying some location in HLSL style expression) slice to get to a location.
    SlangResult parse(const Slang::String& text, const Slang::String& sourcePath, Slang::WriterHelper streamOut, BindLocation& outLocation);
};

class CPULikeBindRoot : public BindRoot
{
public:
    typedef BindRoot Super;

    typedef BindSet::Resource Resource;

    // BindRoot
    virtual BindLocation find(const char* name) SLANG_OVERRIDE;
    virtual SlangResult setArrayCount(const BindLocation& location, int count) SLANG_OVERRIDE;
    virtual void getRoots(Slang::List<BindLocation>& outLocations) SLANG_OVERRIDE;

    slang::VariableLayoutReflection* getParameterByName(const char* name);
    slang::VariableLayoutReflection* getEntryPointParameterByName(const char* name);

    void addDefaultBuffers();

    SlangResult init(BindSet* bindSet, slang::ShaderReflection* reflection, int entryPointIndex); 

    // Used when we have uniform buffers (as used on CPU/CUDA)
    slang::ShaderReflection* m_reflection = nullptr;
    Resource* m_rootBuffer = nullptr;
    Resource* m_entryPointBuffer = nullptr;
    slang::EntryPointReflection* m_entryPoint;
    BindSet* m_bindSet = nullptr;
};

} // renderer_test

#endif //BIND_LOCATION_H
