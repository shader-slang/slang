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

    void setInvalid() { m_space = -1; m_offset = 0; }

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
    typedef BindPoints ThisType;

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

    bool operator==(const ThisType& rhs) const
    {
        for (Slang::Index i = 0; i < SLANG_PARAMETER_CATEGORY_COUNT; ++i)
        {
            if (m_points[i] != rhs.m_points[i])
            {
                return false;
            }
        }
        return true;
    }
    bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

    int GetHashCode() const
    {
        int hash = 0x5435abbc;
        int bits = 0;
        int bit = 1;
        for (Slang::Index i = 0; i < SLANG_PARAMETER_CATEGORY_COUNT; ++i, bit += bit)
        {
            const auto& point = m_points[i];
            if (point.isValid())
            {
                hash = Slang::combineHash(hash, point.GetHashCode());
                bits |= bit;
            }
        }
        // The categories set is important too, so merge that in
        return Slang::combineHash(bits, hash);
    }

    BindPoint& operator[](SlangParameterCategory category) { return m_points[Slang::Index(category)]; }
    const BindPoint& operator[](SlangParameterCategory category) const { return m_points[Slang::Index(category)]; }

    BindPoint m_points[SLANG_PARAMETER_CATEGORY_COUNT];
};

class BindPointSet : public Slang::RefObject
{
public:
    typedef Slang::RefObject Super;

    int GetHashCode() const { return m_points.GetHashCode(); }

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

        /// Can be set by user code to indicate the origin of contents/definition of a resource, such that actual resource can be later constructed.
        /// -1 is used to indicate it is not set.
    Slang::Index m_userIndex = -1;                  

    Slang::RefPtr<Slang::RefObject> m_target;       ///< Can be used to store data related to an actual target resource. 
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

    template <typename T>
    T* getUniform() const { return reinterpret_cast<T*>(getUniform(sizeof(T))); }
    void* getUniform(size_t size) const;

        /// Set uniform data
    SlangResult setUniform(const void* data, size_t sizeInBytes) const;

    bool operator==(const ThisType& rhs) const;
    bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        /// Get the hash code
    int GetHashCode() const;

        /// Default Ctor - constructs as invalid
    BindLocation() {}
    BindLocation(slang::TypeLayoutReflection* typeLayout, const BindPoints& points, BindSet_Resource* resource = nullptr);
    BindLocation(slang::TypeLayoutReflection* typeLayout, SlangParameterCategory category, const BindPoint& point, BindSet_Resource* resource = nullptr);

    BindLocation(const ThisType& rhs) = default;

        /// An invalid location.
        /// Better to return this than use default Ctor as indicates validity in code directly.
    static const BindLocation Invalid;

    slang::TypeLayoutReflection* m_typeLayout = nullptr;    ///< The type layout

    BindSet_Resource* m_resource = nullptr;                 ///< The resource if we are in 

    SlangParameterCategory m_category = SLANG_PARAMETER_CATEGORY_NONE;  ///< If there isn't a set this defines the category
    BindPoint m_point;                                     ///< If there isn't a bind point set, this defines the point

        /// Holds multiple BindPoints.
        /// To keep invariants (such that GetHashCode and == work), it can only be set if
        /// there is more than one category. If there is just one, m_category and m_point *MUST* be used. 
        /// NOTE! Can only be written to if there is a single reference.
    Slang::RefPtr<BindPointSet> m_bindPointSet;             
};

class BindSet
{
public:
    typedef BindSet_Resource Resource;

    Resource* getAt(const BindLocation& loc) const;
    void setAt(const BindLocation& loc, Resource* resource);
    void setAt(const BindLocation& loc, SlangParameterCategory category, Resource* resource);

    Resource* createBufferResource(slang::TypeLayoutReflection* type, size_t sizeInBytes, const void* initialData = nullptr);
    Resource* createBufferResource(slang::TypeReflection::Kind kind, size_t sizeInBytes, const void* initialData = nullptr);

    Resource* createTextureResource(slang::TypeLayoutReflection* type);

        /// Calculate from the current location everything that is referenced
    void calcResourceLocations(const BindLocation& location, Slang::List<BindLocation>& outLocations);
    void calcChildResourceLocations(const BindLocation& location, Slang::List<BindLocation>& outLocations);

    void destroyResource(Resource* resource);

    BindLocation toField(const BindLocation& loc, slang::VariableLayoutReflection* field) const;
    BindLocation toField(const BindLocation& loc, const char* name) const;
    BindLocation toIndex(const BindLocation& location, Slang::Index index) const;

    SlangResult setBufferContents(const BindLocation& loc, const void* initialData, size_t sizeInBytes) const;

        /// Get all of the resources
    const Slang::List<Resource*>& getResources() const { return m_resources; }
        /// Get all of the bindings
    void getBindings(Slang::List<BindLocation>& outLocations, Slang::List<Resource*>& outResources) const;

        /// Ctor
    BindSet();

        /// Dtor
    ~BindSet();

        /// True if is a texture type
    static bool isTextureType(slang::TypeLayoutReflection* typeLayout);

protected:
    Resource* _createBufferResource(slang::TypeReflection::Kind kind, slang::TypeLayoutReflection* typeLayout, size_t bufferSizeInBytes, size_t sizeInBytes, const void* initalData);
    
    Slang::List<Resource*> m_resources;

    Slang::Dictionary<BindLocation, Resource*> m_bindings;

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

        /// Get the bindset
    BindSet* getBindSet() const { return m_bindSet; }

protected:
    
    BindSet* m_bindSet = nullptr;
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

    Resource* getRootBuffer() const { return m_rootBuffer; }
    Resource* getEntryPointBuffer() const { return m_entryPointBuffer; }

    void* getRootData() { return m_rootBuffer ? m_rootBuffer->m_data : nullptr; }
    void* getEntryPointData() { return m_entryPointBuffer ? m_entryPointBuffer->m_data : nullptr; } 

    SlangResult init(BindSet* bindSet, slang::ShaderReflection* reflection, int entryPointIndex); 

protected:
    // Used when we have uniform buffers (as used on CPU/CUDA)
    slang::ShaderReflection* m_reflection = nullptr;
    Resource* m_rootBuffer = nullptr;
    Resource* m_entryPointBuffer = nullptr;
    slang::EntryPointReflection* m_entryPoint;
};

} // renderer_test

#endif //BIND_LOCATION_H
