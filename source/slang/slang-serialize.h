// slang-serialize.h
#ifndef SLANG_SERIALIZE_H
#define SLANG_SERIALIZE_H

//#include <type_traits>

#include "../core/slang-riff.h"
#include "../core/slang-byte-encode-util.h"

#include "../core/slang-stream.h"

#include "slang-serialize-types.h"

#include "../compiler-core/slang-name.h"

namespace Slang
{

class Linkage;

/*
A discussion of the serialization system design can be found in

docs/design/serialization.md
*/

// Predeclare
typedef uint32_t SerialSourceLoc;
class NodeBase;
class Val;
struct ValNodeDesc;

// Pre-declare
class SerialClasses;
class SerialWriter;
class SerialReader;

struct SerialClass;
struct SerialField;

// Type used to implement mechanisms to convert to and from serial types.
template <typename T, typename /*enumTypeSFINAE*/ = void>
struct SerialTypeInfo;

enum class SerialTypeKind : uint8_t
{
    Unknown,

    String,             ///< String                         
    Array,              ///< Array
    ImportSymbol,       ///< Holds the name of the import symbol. Represented in exactly the same way as a string

    NodeBase,           ///< NodeBase derived
    RefObject,          ///< RefObject derived types

    CountOf,
};
typedef uint16_t SerialSubType;

struct SerialInfo
{
    enum
    {
        // Data held in serialized format, the maximally allowed alignment 
        MAX_ALIGNMENT = 8,
    };

    // We only allow up to MAX_ALIGNMENT bytes of alignment. We store alignments as shifts, so 2 bits needed for 1 - 8
    enum class EntryInfo : uint8_t
    {
        Alignment1 = 0,
    };

    static EntryInfo makeEntryInfo(int alignment, int nextAlignment)
    {
        // Make sure they are power of 2
        SLANG_ASSERT((alignment & (alignment - 1)) == 0);
        SLANG_ASSERT((nextAlignment & (nextAlignment - 1)) == 0);

        const int alignmentShift = ByteEncodeUtil::calcMsb8(alignment);
        const int nextAlignmentShift = ByteEncodeUtil::calcMsb8(nextAlignment);
        return EntryInfo((nextAlignmentShift << 2) | alignmentShift);
    }
    static EntryInfo makeEntryInfo(int alignment)
    {
        // Make sure they are power of 2
        SLANG_ASSERT((alignment & (alignment - 1)) == 0);
        return EntryInfo(ByteEncodeUtil::calcMsb8(alignment));
    }
        /// Apply with the next alignment
    static EntryInfo combineWithNext(EntryInfo cur, EntryInfo next)
    {
        return EntryInfo((int(cur) & ~0xc0) | ((int(next) & 3) << 2));
    }

    static int getAlignment(EntryInfo info) { return 1 << (int(info) & 3); }
    static int getNextAlignment(EntryInfo info) { return 1 << ((int(info) >> 2) & 3); }

    /* Alignment is a little tricky. We have a 'Entry' header before the payload. The payload alignment may change.
    If we only align on the Entry header, then it's size *must* be some modulo of the maximum alignment allowed.

    We could hold Entry separate from payload. We could make the header not require the alignment of the payload - but then
    we'd need payload alignment separate from entry alignment.
    */
    struct Entry
    {
        SerialTypeKind typeKind;
        EntryInfo info;

        size_t calcSize(SerialClasses* serialClasses) const;
    };

    struct StringEntry : Entry
    {
        char sizeAndChars[1];
    };

    struct ObjectEntry : Entry
    {
        SerialSubType subType;      ///< Can be ASTType or other subtypes (as used for RefObjects for example)
        uint32_t _pad0;             ///< Necessary, because a node *can* have MAX_ALIGNEMENT
    };

    struct ValEntry : Entry
    {
        SerialSubType subType;
        uint32_t operandCount;
    };

    struct ArrayEntry : Entry
    {
        uint16_t elementSize;
        uint32_t elementCount;
    };

    struct SerialValOperand
    {
        int type;
        uint64_t payload;
    };
};

typedef uint32_t SerialIndexRaw;
enum class SerialIndex : SerialIndexRaw;

/* A type to convert pointers into types such that they can be passed around to readers/writers without
having to know the specific type. If there was a base class that all the serialized types derived from,
that was dynamically castable this would not be necessary */
struct SerialPointer
{
    // Helpers so we can choose what kind of pointer we have based on the (unused) type of the pointer passed in
    SLANG_FORCE_INLINE RefObject* _get(const RefObject*) { return m_kind == SerialTypeKind::RefObject ? reinterpret_cast<RefObject*>(m_ptr) : nullptr; }
    SLANG_FORCE_INLINE NodeBase* _get(const NodeBase*) { return m_kind == SerialTypeKind::NodeBase ? reinterpret_cast<NodeBase*>(m_ptr) : nullptr; }

    template <typename T>
    T* dynamicCast()
    {
        return Slang::dynamicCast<T>(_get((T*)nullptr));
    }

    SerialPointer() :
        m_kind(SerialTypeKind::Unknown),
        m_ptr(nullptr)
    {
    }

    SerialPointer(RefObject* in) :
        m_kind(SerialTypeKind::RefObject),
        m_ptr((void*)in)
    {
    }
    SerialPointer(NodeBase* in) :
        m_kind(SerialTypeKind::NodeBase),
        m_ptr((void*)in)
    {
    }

        /// True if the ptr is set
    SLANG_FORCE_INLINE operator bool() const { return m_ptr != nullptr; }

        /// Directly set pointer/kind
    void set(SerialTypeKind kind, void* ptr) { m_kind = kind; m_ptr = ptr; }

    static SerialTypeKind getKind(const RefObject*) { return SerialTypeKind::RefObject; }
    static SerialTypeKind getKind(const NodeBase*) { return SerialTypeKind::NodeBase; }

    SerialTypeKind m_kind;
    void* m_ptr;
};

class SerialFilter
{
public:
    virtual SerialIndex writePointer(SerialWriter* writer, const NodeBase* ptr) = 0;
    virtual SerialIndex writePointer(SerialWriter* writer, const RefObject* ptr) = 0;
};

class SerialObjectFactory
{
public:
    virtual void* create(SerialTypeKind typeKind, SerialSubType subType) = 0;
    virtual void* getOrCreateVal(ValNodeDesc&& desc) = 0;
};

class SerialExtraObjects
{
public:
    template <typename T>
    void set(T* obj) { m_objects[Index(T::kExtraType)] = obj; }
    template <typename T>
    void set(const RefPtr<T>& obj)
    {
        m_objects[Index(T::kExtraType)] = obj.Ptr();
    }

        /// Get the extra type
    template <typename T>
    T* get() { return reinterpret_cast<T*>(m_objects[Index(T::kExtraType)]); }

    SerialExtraObjects()
    {
        for (auto& obj : m_objects) obj = nullptr;
    }

protected:
   void* m_objects[Index(SerialExtraType::CountOf)];
};

enum class PostSerializationFixUpKind
{
    ValPtr,
};

/* This class is the interface used by toNative implementations to recreate a type. */
class SerialReader : public RefObject
{
public:

    typedef SerialInfo::Entry Entry;
    
    template <typename T>
    void getArray(SerialIndex index, List<T>& out);

    const void* getArray(SerialIndex index, Index& outCount);

    SerialPointer getPointer(SerialIndex index);
    SerialPointer getValPointer(SerialIndex index);

    String getString(SerialIndex index);
    Name* getName(SerialIndex index);
    UnownedStringSlice getStringSlice(SerialIndex index);
    
    SlangResult loadEntries(const uint8_t* data, size_t dataCount) { return loadEntries(data, dataCount, m_classes, m_entries); }
        /// For each entry construct an object. Does *NOT* deserialize them
    SlangResult constructObjects(NamePool* namePool);
        /// Entries must be loaded (with loadEntries), and objects constructed (with constructObjects) before deserializing
    SlangResult deserializeObjects();

        /// NOTE! data must stay ins scope when reading takes place
    SlangResult load(const uint8_t* data, size_t dataCount, NamePool* namePool);

        /// Get the entries list
    const List<const Entry*>& getEntries() const { return m_entries; }

        /// Access the objects list
        /// NOTE that if a SerialObject holding a RefObject and needs to be kept in scope, add the RefObject* via addScope
    List<SerialPointer>& getObjects() { return m_objects; }
    const List<SerialPointer>& getObjects() const { return m_objects; }

        /// Add an object to be kept in scope
    void addScopeWithoutAddRef(const RefObject* obj) { m_scope.add(obj); }
        /// Add obj with a reference
    void addScope(const RefObject* obj) { const_cast<RefObject*>(obj)->addReference(); m_scope.add(obj); }

        /// Used for attaching extra objects necessary for serializing
    SerialExtraObjects& getExtraObjects() { return m_extraObjects; }

        /// Ctor
    SerialReader(SerialClasses* classes, SerialObjectFactory* objectFactory):
        m_classes(classes),
        m_objectFactory(objectFactory)
    {
    }
    ~SerialReader();

        /// Load the entries table (without deserializing anything)
        /// NOTE! data must stay ins scope for outEntries to be valid
    static SlangResult loadEntries(const uint8_t* data, size_t dataCount, SerialClasses* serialClasses, List<const Entry*>& outEntries);

protected:
    List<const Entry*> m_entries;       ///< The entries

    List<SerialPointer> m_objects;      ///< The constructed objects
    NamePool* m_namePool;               ///< Pool names are added to

    List<const RefObject*> m_scope;     ///< Keeping objects in scope

    SerialExtraObjects m_extraObjects;

    SerialObjectFactory* m_objectFactory;
    SerialClasses* m_classes;           ///< Information used to deserialize
};

// ---------------------------------------------------------------------------
template <typename T>
void SerialReader::getArray(SerialIndex index, List<T>& out)
{
    typedef SerialTypeInfo<T> ElementTypeInfo;
    typedef typename ElementTypeInfo::SerialType ElementSerialType;

    Index count;
    auto serialElements = (const ElementSerialType*)getArray(index, count);

    if (count == 0)
    {
        out.clear();
        return;
    }

    if (std::is_same<T, ElementSerialType>::value)
    {
        // If they are the same we can just write out
        out.clear();
        out.insertRange(0, (const T*)serialElements, count);
    }
    else
    {
        // Else we need to convert
        out.setCount(count);
        for (Index i = 0; i < count; ++i)
        {
            ElementTypeInfo::toNative(this, (const void*)&serialElements[i], (void*)&out[i]);
        }
    }
}

/* This is a class used tby toSerial implementations to turn native type into the serial type */
class SerialWriter : public RefObject
{
public:
    typedef uint32_t Flags;
    struct Flag
    {
        enum Enum : Flags
        {
                /// If set will zero initialize backing memory. This is slower but 
                /// is desirable to make two serializations of the same thing produce the 
                /// identical serialized result.
            ZeroInitialize = 0x1,

                /// If set will not serialize function body.
            SkipFunctionBody = 0x2,
        };
    };

    SerialIndex addPointer(const NodeBase* ptr);
    SerialIndex addPointer(const RefObject* ptr);

        /// Write the object at ptr of type serialCls
    SerialIndex writeObject(const SerialClass* serialCls, const void* ptr);

        /// Write the object at the pointer
    SerialIndex writeObject(const NodeBase* ptr);
    SerialIndex writeObject(const RefObject* ptr);
    SerialIndex writeValObject(const Val* ptr);

        /// Add an array - may need to convert to serialized format
    template <typename T>
    SerialIndex addArray(const T* in, Index count);

    template <typename NATIVE_TYPE>
    /// Add an array where all the elements are already in serialized format (ie there is no need to do a conversion)
    SerialIndex addSerialArray(const void* elements, Index elementCount)
    {
        typedef SerialTypeInfo<NATIVE_TYPE> TypeInfo;
        return addSerialArray(sizeof(typename TypeInfo::SerialType), SerialTypeInfo<NATIVE_TYPE>::SerialAlignment, elements, elementCount);
    }

        /// Add an array where all the elements are already in serialized format (ie there is no need to do a conversion)
    SerialIndex addSerialArray(size_t elementSize, size_t alignment, const void* elements, Index elementCount);

        /// Add the string
    SerialIndex addString(const UnownedStringSlice& slice) { return _addStringSlice(SerialTypeKind::String, m_sliceMap, slice); }
    SerialIndex addString(const String& in);
    SerialIndex addName(const Name* name);

        /// Adding import symbols
    SerialIndex addImportSymbol(const UnownedStringSlice& slice)
    {
        return _addStringSlice(SerialTypeKind::ImportSymbol, m_importSymbolMap, slice);
    }
    SerialIndex addImportSymbol(const String& string)
    {
        return _addStringSlice(SerialTypeKind::ImportSymbol, m_importSymbolMap, string.getUnownedSlice());
    }

        /// Set a the ptr associated with an index.
        /// NOTE! That there cannot be a pre-existing setting.
    void setPointerIndex(const NodeBase* ptr, SerialIndex index);
    void setPointerIndex(const RefObject* ptr, SerialIndex index);

        /// Get the entries table holding how each index maps to an entry
    const List<SerialInfo::Entry*>& getEntries() const { return m_entries; }

        /// Write to a stream
    SlangResult write(Stream* stream);

        /// Write a data chunk with fourCC
    SlangResult writeIntoContainer(FourCC fourCC, RiffContainer* container);

        /// Used for attaching extra objects necessary for serializing
    SerialExtraObjects& getExtraObjects() { return m_extraObjects; }

        /// Get the flag
    Flags getFlags() const { return m_flags; }

        /// Ctor
    SerialWriter(SerialClasses* classes, SerialFilter* filter, Flags flags = Flag::ZeroInitialize);

protected:

    typedef Dictionary<UnownedStringSlice, Index> SliceMap;

    SerialIndex _addStringSlice(SerialTypeKind typeKind, SliceMap& sliceMap, const UnownedStringSlice& slice);

    SerialIndex _add(const void* nativePtr, SerialInfo::Entry* entry)
    {
        m_entries.add(entry);
        // Okay I need to allocate space for this
        SerialIndex index = SerialIndex(m_entries.getCount() - 1);
        // Add to the map
        m_ptrMap.add(nativePtr, Index(index));
        return index;
    }

    Dictionary<const void*, Index> m_ptrMap;    // Maps a pointer to an entry index

    // NOTE! Assumes the content stays in scope!
    SliceMap m_sliceMap;
    SliceMap m_importSymbolMap;

    SerialExtraObjects m_extraObjects;      ///< Extra objects

    List<SerialInfo::Entry*> m_entries;     ///< The entries
    MemoryArena m_arena;                    ///< Holds the payloads
    SerialClasses* m_classes;
    SerialFilter* m_filter;                 ///< Filter to control what is serialized

    Flags m_flags;                          ///< Flags to control behavior
};

// ---------------------------------------------------------------------------
template <typename T>
SerialIndex SerialWriter::addArray(const T* in, Index count)
{
    typedef SerialTypeInfo<T> ElementTypeInfo;
    typedef typename ElementTypeInfo::SerialType ElementSerialType;

    if (std::is_same<T, ElementSerialType>::value)
    {
        // If they are the same we can just write out
        return addSerialArray(sizeof(T), SLANG_ALIGN_OF(ElementSerialType), in, count);
    }
    else
    {
        // Else we need to convert
        List<ElementSerialType> work;
        work.setCount(count);

        if (getFlags() & Flag::ZeroInitialize)
        {
            ::memset(work.getBuffer(), 0, sizeof(ElementSerialType) * count);
        }

        for (Index i = 0; i < count; ++i)
        {
            ElementTypeInfo::toSerial(this, &in[i], &work[i]);
        }
        return addSerialArray(sizeof(ElementSerialType), SLANG_ALIGN_OF(ElementSerialType), work.getBuffer(), count);
    }
}

/* A SerialFieldType describes the size of field, it's alignment, and contains the
functions that convert between serial and native data */
struct SerialFieldType
{
    typedef void(*ToSerialFunc)(SerialWriter* writer, const void* src, void* dst);
    typedef void(*ToNativeFunc)(SerialReader* reader, const void* src, void* dst);

    size_t serialSizeInBytes;
    uint8_t serialAlignment;
    ToSerialFunc toSerialFunc;
    ToNativeFunc toNativeFunc;
};

/* Describes a field in a SerialClass. */
struct SerialField
{
        /// Returns a suitable ptr for use in make.
        /// NOTE! Sets to 1 so it's constant and not 0 (and so nullptr)
    template <typename T>
    static T* getPtr() { return (T*)1; } 

    template <typename T>
    static SerialField make(const char* name, T* in);

    const char* name;                   ///< The name of the field
    const SerialFieldType* type;        ///< The type of the field
    uint32_t nativeOffset;              ///< Offset to field from base of type
    uint32_t serialOffset;              ///< Offset in serial type
};

typedef uint8_t SerialClassFlags;

struct SerialClassFlag
{
    enum Enum : SerialClassFlags
    {
        DontSerialize = 0x01,               ///< If set the type is not serialized, so can turn into SerialIndex(0)
    };
};

/* SerialClass defines the type (typeKind/subType) and the fields in just this class definition (ie not it's super class).
Also contains a pointer to the super type if there is one */
struct SerialClass
{    
    SerialTypeKind typeKind;            ///< The type kind
    SerialSubType subType;              ///< Subtype - meaning depends on typeKind

    uint8_t alignment;                  ///< Alignment of this type
    SerialClassFlags flags;             ///< Flags 

    uint32_t size;                      ///< Size of the field in bytes

    Index fieldsCount;
    const SerialField* fields;

    const SerialClass* super;           ///< The super class
};

// An instance could be shared across Sessions, but for simplicity of life time
// here we don't deal with that 
class SerialClasses : public RefObject
{
public:
        /// Will add it's own copy into m_classesByType
        /// In process will calculate alignment, offset etc for fields
        /// NOTE! the super set, *must* be an already added to this SerialClasses
    const SerialClass* add(const SerialClass* cls);

    const SerialClass* add(SerialTypeKind kind, SerialSubType subType, const SerialField* fields, Index fieldsCount, const SerialClass* superCls);

        /// Add a type which will not serialize
    const SerialClass* addUnserialized(SerialTypeKind kind, SerialSubType subType);

        /// Returns true if this cls is *owned* by this SerialClasses
    bool isOwned(const SerialClass* cls) const;

        /// Returns true if the SerialClasses structure appears ok
    bool isOk() const;

        /// Get a serial class based on its type/subType
    const SerialClass* getSerialClass(SerialTypeKind typeKind, SerialSubType subType) const
    {
        const auto& classes = m_classesByTypeKind[Index(typeKind)];
        return (subType < classes.getCount()) ? classes[subType] : nullptr;
    }
    
        /// Ctor
    SerialClasses();

protected:
    SerialClass* _createSerialClass(const SerialClass* cls);

    MemoryArena m_arena;

    List<const SerialClass*> m_classesByTypeKind[Index(SerialTypeKind::CountOf)];
};

// !!!!!!!!!!!!!!!!!!!!! SerialGetFieldType<T> !!!!!!!!!!!!!!!!!!!!!!!!!!!
// Getting the type info, let's use a static variable to hold the state to keep simple

template <typename T>
struct SerialGetFieldType
{
    static const SerialFieldType* getFieldType()
    {
        typedef SerialTypeInfo<T> Info;
        static const SerialFieldType type = { sizeof(typename Info::SerialType), uint8_t(Info::SerialAlignment), &Info::toSerial, &Info::toNative };
        return &type;
    }
};

// !!!!!!!!!!!!!!!!!!!!! SerialGetFieldType<T> !!!!!!!!!!!!!!!!!!!!!!!!!!!

template <typename T>
/* static */SerialField SerialField::make(const char* name, T* in)
{
    uint8_t* ptr = reinterpret_cast<uint8_t*>(in);

    SerialField field;
    field.name = name;
    field.type = SerialGetFieldType<T>::getFieldType();
    // This only works because we in is an offset from 1
    field.nativeOffset = uint32_t(size_t(ptr) - 1);
    field.serialOffset = 0;
    return field;
}

// !!!!!!!!!!!!!!!!!!!!! Convenience functions !!!!!!!!!!!!!!!!!!!!!!!!!!!

template <typename NATIVE_TYPE, typename SERIAL_TYPE>
SLANG_FORCE_INLINE void toSerialValue(SerialWriter* writer, const NATIVE_TYPE& src, SERIAL_TYPE& dst)
{
    SerialTypeInfo<NATIVE_TYPE>::toSerial(writer, &src, &dst);
}

template <typename SERIAL_TYPE, typename NATIVE_TYPE>
SLANG_FORCE_INLINE void toNativeValue(SerialReader* reader, const SERIAL_TYPE& src, NATIVE_TYPE& dst)
{
    SerialTypeInfo<NATIVE_TYPE>::toNative(reader, &src, &dst);
}

} // namespace Slang

#endif
