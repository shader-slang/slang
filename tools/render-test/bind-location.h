#ifndef BIND_LOCATION_H
#define BIND_LOCATION_H

#include "core/slang-basic.h"
#include "core/slang-free-list.h"
#include "core/slang-memory-arena.h"
#include "core/slang-writer.h"

#include "slang.h"

namespace renderer_test {

/*
Bind Set/Point/Value
====================

The following classes are designed as a mechanism to simplify binding within the test system. The underlying issues are

* How binding occurs is very dependent on the underlying target (CPU is different from Dx for example)
  + CPU everything is just backed by uniform 'memory'/GPU uses different registers for different types
  + With unbound arrays CPU can just indirect to a buffer, on GPU it might need use of register spaces or some other mechanism (as in VK)
  + CPU groups together global/entry point parameters, GPU typically does not
* Having a mechanism that will the data/binding for the test independent of the actual target, allows that code/implementation to be shared across many targets.
* How a resource/state is configured within binding also varies significantly between targets

One way to handle this disparity, would be to build an abstraction layer, that could create the device specific
resources/state and set them. This is not the approach taken here though. The idea here is to have a mechanism to
be able to build structures in memory, and record where binding takes place without having to create any
device specific resources or state. This data can then be used to construct and then bind as is appropriate.

The process broadly for test system is is

1) Set up any default buffers required for a target (for example the uniform/entry point buffers for CPU)
2) Add any default Value/buffers that are needed by traversing reflection
3) Create/Set the Values for the elements of ShaderInputLayoutEntry
4) Go through the values set on the BindSet, creating Resources/State etc appropriate for the target
5) Go through the bindings setting the Resource bindings as appropriate for the target
6) Execute
7) If the computation takes place outside of Values backing memory, copy back the data for output entries
8) Write the output entries

To do this we need a mechanism to store a binding location. In the general case a BindingLocation might
track the location of many different categories of data.

We also need a way to record what we want to create on the device for execution. To do this we have the
BindSet::Value. 'Value' was used instead of 'Resource' because the types of things the Value might represent
may not be resource like or might be multiple resources. In simple use cases though a 'Value' is typically
synonymous with some kind of Resource on the device.

A Value knows the underlying type it represents as was determined via the slang layout/reflection. That an added
feature of 'Values' is there are able hold a buffer that is typically mapped onto some linear buffer on the
device. Doing so means that we do not need to store BindLocation mappings for say uniform data (like float or
matrix), it can just be stored in the memory buffer. When the resources are constructed for execution, we can
just copy over that data.

This all sounds well and good but there is a final underlying important aspect. That is that some resource
like bindings may have to be stored in a buffer. For example on a CPU we could have a constant buffer that contained
another constant buffer as a field. On CPU this field would be converted into a pointer which needs to be set up. On CUDA this might be some
device specific value. So before we can copy the memory representation to a device specific buffer we must convert
any such bindings into something appropriate in the memory buffer associated with the Value. To do this we can traverse
a record of all of the bindings (which are held on the BindSet), and then set the appropriate date for the device from
data stored in the associated 'Value'.

A final observation is that on CPU targets, the memory buffer held in the Value can just be used directly. 

NOTE: 

That these classes are written so they can be used to track locations across multiple categories such that binding
can work across many different types of targets. For the moment the mechanism/s are only tested on CPU like binding,
and there are quirks in how locations are traversed that have knowledge of how such bindings work. It may be necessary
for this to work more generally to only allow certain kinds of transitions based on some well defined specific
binding styles. 
*/

/* A bind point records a specific binding point (typically for a category). It records a space and an offset.
As with Slangs layout reflection, the offset meaning is dependent on category. It might be an offset to
a 'register'. If category is 'uniform' it might be a memory offset. The space defines the 'space' a register
is in.
Note that m_space is ignored (but must be valid) for uniform offsets. 
*/
struct BindPoint
{
    typedef BindPoint ThisType;

        /// 
    bool isValid() const { return m_space >= 0; }
    bool isInvalid() const { return m_space < 0; }

    void setInvalid() { m_space = -1; m_offset = 0; }

    bool operator==(const ThisType& rhs) const { return m_space == rhs.m_space && m_offset == rhs.m_offset; }
    bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

    Slang::HashCode getHashCode() const { return Slang::combineHash(Slang::getHashCode(m_space), Slang::getHashCode(m_offset)); }

    BindPoint() = default;
    BindPoint(Slang::Index space, size_t offset):m_space(space), m_offset(offset) {}

    static BindPoint makeInvalid() { return BindPoint(-1, 0); }

    Slang::Index m_space = 0;               ///< The register space 
    size_t m_offset = 0;                    ///< The offset, might be a byte address or a register index
};

/* Stores the BindPoints by category. */
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

    Slang::HashCode getHashCode() const
    {
        using namespace Slang;
        HashCode hash = 0x5435abbc;
        int bits = 0;
        int bit = 1;
        for (Index i = 0; i < SLANG_PARAMETER_CATEGORY_COUNT; ++i, bit += bit)
        {
            const auto& point = m_points[i];
            if (point.isValid())
            {
                hash = combineHash(hash, point.getHashCode());
                bits |= bit;
            }
        }
        // The categories set is important too, so merge that in
        return combineHash(bits, hash);
    }

    BindPoint& operator[](SlangParameterCategory category) { return m_points[Slang::Index(category)]; }
    const BindPoint& operator[](SlangParameterCategory category) const { return m_points[Slang::Index(category)]; }

    BindPoint m_points[SLANG_PARAMETER_CATEGORY_COUNT];
};

/* A BindPointSet is really just a reference counted 'BindPoints'. This allows for BindPoints to be shared between
multiple BindLocations if they hold the same value. */
class BindPointSet : public Slang::RefObject
{
public:
    typedef Slang::RefObject Super;

    Slang::HashCode getHashCode() const { return m_points.getHashCode(); }

    BindPointSet(const BindPoints& points) :
        m_points(points)
    {
    }
    BindPointSet() {}

    BindPoints m_points;
};

/* A BindSet::Value represents a 'value' associated with a binding. Typically it will be a Resource type
like a Buffer/Texture on a target device. As well as recording type information, it can also store a chunk
of memory that can hold uniform data, and may hold bindings for some kinds of targets (for example CPU pointers).
Additionally if the Value holds some kind of array, the amount of elements in the array can be stored in m_elementCount.

All Value are constructed stored and tracked on a BindSet. When a BindSet is destroyed any associated Value will become
destroyed.
*/
struct BindSet_Value
{
    slang::TypeReflection::Kind m_kind;              ///< The kind, used if type is not set. Same as m_type.kind otherwise
    slang::TypeLayoutReflection* m_type;            ///< The type
    uint8_t* m_data;
    size_t m_sizeInBytes;                           ///< Total size in bytes
    size_t m_elementCount;                          ///< Only applicable on an array like type, else 0

        /// Can be set by user code to indicate the origin of contents/definition of a value, such that actual resource can be later constructed.
        /// -1 is used to indicate it is not set.
    Slang::Index m_userIndex = -1;                  

    Slang::RefPtr<Slang::RefObject> m_target;       ///< Can be used to store data related to an actual target resource. 
};

class BindSet;

/* Specifies a binding location (including the associated slang reflection type information)

It really can be in 3 type of state.
1) Invalid - not a valid binding (m_typeLayout is null, m_pointSet is not used. 
2) Holds a single bind point defined by category and BindPoint m_point (m_category and m_point are used)
3) Hold multiple bind points by category (in this case m_bindPointSet is used)

NOTE! it is an invariant - that the BindLocation must always be in the 'simplest' form that can represent it.
That is if there is only a single binding it *cannot* be stored as a m_bindPointSet with a single category

That construction through BindPoints, will do this determination automatically.

A BindLocation can be stored in a Hash.
*/
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
    Slang::HashCode getHashCode() const;

        /// Default Ctor - constructs as invalid
    BindLocation() {}
    BindLocation(slang::TypeLayoutReflection* typeLayout, const BindPoints& points, BindSet_Value* value = nullptr);
    BindLocation(slang::TypeLayoutReflection* typeLayout, SlangParameterCategory category, const BindPoint& point, BindSet_Value* value = nullptr);
    BindLocation(slang::VariableLayoutReflection* varLayout, BindSet_Value* value = nullptr);

    BindLocation(const ThisType& rhs) = default;

        /// An invalid location.
        /// Better to return this than use default Ctor as indicates validity in code directly.
    static const BindLocation Invalid;

    slang::TypeLayoutReflection* m_typeLayout = nullptr;    ///< The type layout

    BindSet_Value* m_value = nullptr;                       ///< The value if we are in one. 

    SlangParameterCategory m_category = SLANG_PARAMETER_CATEGORY_NONE;  ///< If there isn't a set this defines the category
    BindPoint m_point;                                     ///< If there isn't a bind point set, this defines the point

        /// Holds multiple BindPoints.
        /// To keep invariants (such that getHashCode and == work), it can only be set if
        /// there is more than one category. If there is just one, m_category and m_point *MUST* be used. 
        /// NOTE! Can only be written to if there is a single reference.
    Slang::RefPtr<BindPointSet> m_bindPointSet;             
};

/* A BindSet holds all of the Value and bindings. It is designed to be used such that it can hold
all of the bind state needed for setting up a specific binding.

Unfortunately it is not enough to lookup via a path for a Binding, because different targets represents the
'root' variables and values in different ways. The BindRoot interface is designed to handle this aspect. 
*/ 
class BindSet
{
public:
    typedef BindSet_Value Value;

    Value* getAt(const BindLocation& loc) const;
    void setAt(const BindLocation& loc, Value* value);
    void setAt(const BindLocation& loc, SlangParameterCategory category, Value* value);

    Value* createBufferValue(slang::TypeLayoutReflection* type, size_t sizeInBytes, const void* initialData = nullptr);
    Value* createBufferValue(slang::TypeReflection::Kind kind, size_t sizeInBytes, const void* initialData = nullptr);

    Value* createTextureValue(slang::TypeLayoutReflection* type);

        /// Calculate from the current location everything that is referenced
    void calcValueLocations(const BindLocation& location, Slang::List<BindLocation>& outLocations);
    void calcChildResourceLocations(const BindLocation& location, Slang::List<BindLocation>& outLocations);

    void destroyValue(Value* value);

    BindLocation toField(const BindLocation& loc, slang::VariableLayoutReflection* field) const;
    BindLocation toField(const BindLocation& loc, const char* name) const;
    BindLocation toIndex(const BindLocation& location, Slang::Index index) const;

    SlangResult setBufferContents(const BindLocation& loc, const void* initialData, size_t sizeInBytes) const;

        /// Get all of the values
    const Slang::List<Value*>& getValues() const { return m_values; }
        /// Get all of the bindings
    void getBindings(Slang::List<BindLocation>& outLocations, Slang::List<Value*>& outValues) const;

        /// 
    void releaseValueTargets();

        /// Ctor
    BindSet();

        /// Dtor
    ~BindSet();

        /// True if is a texture type
    static bool isTextureType(slang::TypeLayoutReflection* typeLayout);

protected:
    Value* _createBufferValue(slang::TypeReflection::Kind kind, slang::TypeLayoutReflection* typeLayout, size_t bufferSizeInBytes, size_t sizeInBytes, const void* initalData);
    
    Slang::List<Value*> m_values;

    Slang::Dictionary<BindLocation, Value*> m_bindings;

    Slang::MemoryArena m_arena;
};

/* BindRoot is an interface for finding the roots bindings by name. It is an interface because different targets have different ways of
representing how root values are located.
More specifically a CPU target holds the uniform and entry point variables in two buffers. 
*/
class BindRoot : public Slang::RefObject
{
public:
    typedef RefObject Super;

    typedef BindSet::Value Value;

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

    slang::VariableLayoutReflection* getParameterByName(const char* name);
    slang::VariableLayoutReflection* getEntryPointParameterByName(const char* name);

    SlangResult init(BindSet* bindSet, slang::ShaderReflection* reflection, int entryPointIndex);


protected:
    
    BindSet* m_bindSet = nullptr;
    slang::EntryPointReflection* m_entryPoint = nullptr;
    slang::ShaderReflection* m_reflection = nullptr;
};

/* A CPULike implementation of the BindRoot. This can be used for any binding that holds
the entry point variables/uniforms in buffers. This type also stores the Value/Buffers for
the 'root', and entry point, so they can be directly accessed. 
*/
class CPULikeBindRoot : public BindRoot
{
public:
    typedef BindRoot Super;

    // BindRoot
    virtual BindLocation find(const char* name) SLANG_OVERRIDE;
    virtual SlangResult setArrayCount(const BindLocation& location, int count) SLANG_OVERRIDE;
    virtual void getRoots(Slang::List<BindLocation>& outLocations) SLANG_OVERRIDE;

    void addDefaultValues();

    Value* getRootValue() const { return m_rootValue; }
    Value* getEntryPointValue() const { return m_entryPointValue; }

    void* getRootData() { return m_rootValue ? m_rootValue->m_data : nullptr; }
    void* getEntryPointData() { return m_entryPointValue ? m_entryPointValue->m_data : nullptr; } 

    SlangResult init(BindSet* bindSet, slang::ShaderReflection* reflection, int entryPointIndex); 

protected:
    // Used when we have uniform buffers (as used on CPU/CUDA)
    
    Value* m_rootValue = nullptr;
    Value* m_entryPointValue = nullptr;
};

class GPULikeBindRoot : public BindRoot
{
public:
    typedef BindRoot Super;

    // BindRoot
    virtual BindLocation find(const char* name) SLANG_OVERRIDE;
    virtual SlangResult setArrayCount(const BindLocation& location, int count) SLANG_OVERRIDE;
    virtual void getRoots(Slang::List<BindLocation>& outLocations) SLANG_OVERRIDE;

protected:
};



} // renderer_test

#endif //BIND_LOCATION_H
