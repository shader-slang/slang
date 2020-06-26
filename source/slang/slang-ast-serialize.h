// slang-ast-serialize.h
#ifndef SLANG_AST_SERIALIZE_H
#define SLANG_AST_SERIALIZE_H

#include <type_traits>

#include "slang-ast-support-types.h"
#include "slang-ast-all.h"

#include "slang-ast-builder.h"

#include "../core/slang-byte-encode-util.h"

#include "../core/slang-stream.h"

namespace Slang
{

/*
AST Serialization Overview
==========================

The AST node types are generally types derived from the NodeBase. The C++ extractor is used to associate an ASTNodeType with
every NodeBase type, such that casting is fast and simple and we have a simple integer to uniquely identify those types. The
extractor also performs another task of associating with the type name all of the fields held in just that type. The definition
of the fields is stored in an 'x macro' which is in the slang-ast-generated-macro.h file, for example

```
#define SLANG_FIELDS_ASTNode_DeclRefExpr(_x_, _param_)\
    _x_(scope, (RefPtr<Scope>), _param_)\
    _x_(declRef, (DeclRef<Decl>), _param_)\
    _x_(name, (Name*), _param_)
``

For the type DeclRefExpr, this holds all of the fields held in just DeclRefExpr in this case `scope`, `declRef` and `name`.
DeclRefExpr derives from Expr and this might hold other fields and so forth.

The implementation makes a distinction between the 'native' types, the regular C++ in memory types and 'serial' types.
Each serializable C++ type has an associated 'serial' type - with the distinction that it can be written out and (with perhaps some other data)
read back in to recreate the C++ type. The serial type can be a C++ type, but is such it can be written and read from disk and still
represent the same data. 

We need a mechanism to be able to do do a conversion between native and serial types. To make the association we use the template

```
template <typename T>
struct ASTSerialTypeInfo;
```

and specialize it for each native type. The specialization holds

SerialType - The type that will be used to represent the native type
NativeType - The native typs
SerialAlignment - A value that holds what kind of alignment the SerialType needs to be serializable (it may be different from SLANG_ALIGN_OF(SerialType)!)
toSerial - A function that with the help of ASTSerialWriter convert the NativeType into the SerialType
toNative - A function that with the help of ASTSerialReader convert the SerialType into the NativeType

It is useful to have a structure that holds the type information, so it can be stored. That is achieved with

```
template <typename T>
struct ASTSerialGetType;
```

This template can be specialized for a specific native types - but all it holds is just a function getType, which returns a ASTSerialType*,
which just holds the information held in the ASTSerialTypeInfo template, but additionally including the size of the SerialType.

So we need to define a specialized ASTSerialTypeInfo for each type that can be a field in a NodeBase derived type. We don't need to define
anything explicitly for the NodeBase derived types, as we will just generate the layout from the fields. How do we know the fields? We just
used the macros generated from the C++ extactor.

So first a few things to observe...

1) Some types don't need any conversion to be serializable - int8_t, or float the bits can just be written out and read in (1)
2) Some types need a conversion but it's very simple - for example an enum without explicit size, being written as an explicit size
3) Some types can be written out but would not be directly readable or usable with different targets/processors, so need converting
4) Some types require complex conversions that require programmer code - like Dictionary/List

For types that need no conversion (1), we can just use the template ASTSerialIdentityTypeInfo

```
template <>
struct ASTSerialTypeInfo<SomeType> : public ASTSerialIdentityTypeInfo<SomeType> {};
```

This specialization means that SomeType can be written out and read in across targets/compilers without problems.

For (2) we have another template that will do the conversion for us

```
template <typename NATIVE_T, typename SERIAL_T>
struct ASTSerialConvertTypeInfo;
```

That we can use as above, and specify the native and serial types.

For (3) there are a few scenarios. For any field in a serial type we must store in the serialized type such that the representation
will work across all processors/compilers. So one problematic type is `bool`. It's not specified how it's laid out in memory - and
some compiles have stored it as a word. Most recently it's been stored as a byte. To make sure bool is ok for serialization therefore
we store as a uint8_t.

Another example would be double. It's 64 bits, but on some arches/compilers it's SLANG_ALIGN_OF is 4 and on others it's 8. On some
arches a non aligned read will lead to a fault. To work around this problem therefore we have to ensure double has the alignment that
will work across all targets - and that alignment is 8. In that specific case that issue is handled via ASTSerialBasicTypeInfo, which
makes the SerialAlignment the sizeof the type.

For (4) there are a few things to say. First a type can always implement a custom version of how to do a conversion by specializing
`ASTSerialTypeInfo`. But there remains another nagging issue - types which allocate/use other memory that changes at runtime. Clearly
we cannot define 'any size of memory' in a fixed SerialType defined in a specialization of ASTSerialTypeInfo. The mechanism to work around
this is to allow arbitrary arrays to be stored, that can be accessed via an ASTSerialIndex. This will be discussed more once we discuss
a little more about the file system, and ASTSerialIndex. 

Serialization Format
====================

The serialization format used is 'stream-like' with each 'object' stored in order. Each object is given an index starting from 1.
0 is used to be in effect nullptr. The stream looks like

```
ASTSerialInfo::Entry (for index 1)
Payload for type in entry

ASTSerialInfo::Entry (for index 2)
Payload for type in entry

... 
... 

That when writing we have an array that maps each index to a pointer to the associated header. We also have a map that maps native pointers
to their indices. The Payload *is* the SerialType for thing saved. The payload directly follows the Entry data.

Each object in this list can only be a few types of things - those derived from ASTSerialInfo::Type. 

The actual Entry followed by the payloads are allocated and stored when writing in a MemoryArena. When we want to write into a stream, we
can just iterate over each entry in order and write it out.

You may have spotted a problem here - that some Entry types can be stored without alignment (for example a string - which stores the length
VarInt encoded followed by the characters). Others require an alignment - for example an NodeBase derived type that contains a int64_t will
*require* 8 byte alignment. That as a feature of the serialization format we want to be able to just map the data into memory, and be able
to access all the SerialType as is on the CPU. For that to work we *require* that the payload for each entry has the right alignment for
the associated SerialType.

To achieve this we store in the Entry it's alignment requirement *AND* the next entries alignment. With this when we read, as we as stepping
through the entries we can find where the next Entry starts. Because the payload comes directly after the Entry - the Entrys size must be
a modulo of the largest alignment the payload can have.

For the code that does the conversion between native and serial types it uses either the ASTSerialWriter or ASTSerialReader. This provides
the mechanism to turn a pointer into a serializable ASTSerialIndex and vice versa. There are some special functions for turning string like
types to and forth.

The final mechanism is that of 'Arrays'. An array allows reading or writing a chunk of data associated with a ASTSerialIndex. The chunk of
data *must* hold data that is serializable. If the array holds pointers - then the serialized array must hold ASTSerialIndices that
represent those pointers. When reading back in they are converted back.

Arrays are the escape hatch that allows for more complex types to serialize. Dictionaries for example are saved as a serial type that is
two ASTSerialIndices one to a keys array and one to a values array.

Note that writing has two phases, serializing out into an ASTSerialWriter, and then secondly writing out to a stream. 

NodeBase Types
==============

The ASTSerialTypeInfo mechanism is generally for *fields* of NodeBase types. That for NodeBase derived types we use the C++ extractors
field list to work out the native fields offsets and types. With this we can then calculate the layout for NodeBase types such that they
follow the requirements for serialization - such as alignment and so forth.

This information is held in the ASTSerialClasses, which for a given ASTNodeType gives an ASTSerialClassInfo, that specifies fields for
just that type. Super types fields need to be serialized too, and this information can be found by using the ClassReflectInfo to find the
super type.

Reading
=======

Due to the care in writing reading is relatively simple. We can just take the contents of the file and put in memory, as long as in memory
it has an alignment of at least MAX_ALIGNMENT. Then we can build up an entries table by stepping through the data and writing the pointer.

The toNative functions take an ASTSerialReader - this allows the implementation to ask for pointers and arrays from other parts of the serialized
data. It also allows for types to be lazily reconstructed if necessary.

Lazy reconstruction may be useful in the future to partially reconstruct a sub part of the serialized data. In the current implementation, lazy
evaluation is used on Strings. The m_objects array holds all of the recreated native 'objects'. Since the objects can be derived from different
base classes the associated Entry will describe what it really is.

For the String type, we initially store the object pointer as null. If a string is requested from that index, we see if the object pointer is null,
if it is we have to construct the StringRepresentation that will be used.

An extra wrinkle is that we allow accessing of a serialized String as a Name or a string or a UnownedSubString. Fortunately a Name just holds a string,
and a Name remains in scope as long as it's NamePool does which is passed in.
*/


class ASTSerialClasses;

// Type used to implement mechanisms to convert to and from serial types.
template <typename T>
struct ASTSerialTypeInfo;

struct ASTSerialInfo
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

    enum class Type : uint8_t
    {
        String,             ///< String                         
        Node,               ///< NodeBase derived
        RefObject,          ///< RefObject derived types          
        Array,              ///< Array
    };

    
    /* Alignment is a little tricky. We have a 'Entry' header before the payload. The payload alignment may change.
    If we only align on the Entry header, then it's size *must* be some modulo of the maximum alignment allowed.

    We could hold Entry separate from payload. We could make the header not require the alignment of the payload - but then
    we'd need payload alignment separate from entry alignment.
    */
    struct Entry
    {
        Type type;
        EntryInfo info;

        size_t calcSize(ASTSerialClasses* serialClasses) const;
    };

    struct StringEntry : Entry
    {
        char sizeAndChars[1];
    };

    struct NodeEntry : Entry
    {
        uint16_t astNodeType;
        uint32_t _pad0;             ///< Necessary, because a node *can* have MAX_ALIGNEMENT
    };

    struct RefObjectEntry : Entry
    {
        enum class SubType : uint8_t
        {
            Breadcrumb,
        };
        SubType subType;
        uint8_t _pad0;
        uint32_t _pad1;             ///< Necessary because RefObjectEntry *can* have MAX_ALIGNEMENT
    };

    struct ArrayEntry : Entry
    {
        uint16_t elementSize;
        uint32_t elementCount;
    };
};

typedef uint32_t ASTSerialIndexRaw;
enum class ASTSerialIndex : ASTSerialIndexRaw;
typedef uint32_t ASTSerialSourceLoc;

/* A type to convert pointers into types such that they can be passed around to readers/writers without
having to know the specific type. If there was a base class that all the serialized types derived from,
that was dynamically castable this would not be necessary */
struct ASTSerialPointer
{
    enum class Kind
    {
        Unknown,
        RefObject,
        NodeBase
    };

    // Helpers so we can choose what kind of pointer we have based on the (unused) type of the pointer passed in
    SLANG_FORCE_INLINE RefObject* _get(const RefObject*) { return m_kind == Kind::RefObject ? reinterpret_cast<RefObject*>(m_ptr) : nullptr; }
    SLANG_FORCE_INLINE NodeBase* _get(const NodeBase*) { return m_kind == Kind::NodeBase ? reinterpret_cast<NodeBase*>(m_ptr) : nullptr; }

    template <typename T>
    T* dynamicCast()
    {
        return Slang::dynamicCast<T>(_get((T*)nullptr));
    }

    ASTSerialPointer() :
        m_kind(Kind::Unknown),
        m_ptr(nullptr)
    {
    }

    ASTSerialPointer(RefObject* in) :
        m_kind(Kind::RefObject),
        m_ptr((void*)in)
    {
    }
    ASTSerialPointer(NodeBase* in) :
        m_kind(Kind::NodeBase),
        m_ptr((void*)in)
    {
    }

    static Kind getKind(const RefObject*) { return Kind::RefObject; }
    static Kind getKind(const NodeBase*) { return Kind::NodeBase; }

    Kind m_kind;
    void* m_ptr;
};


/* This class is the interface used by toNative implementations to recreate a type */
class ASTSerialReader : public RefObject
{
public:

    typedef ASTSerialInfo::Entry Entry;
    typedef ASTSerialInfo::Type Type;

    template <typename T>
    void getArray(ASTSerialIndex index, List<T>& out);

    const void* getArray(ASTSerialIndex index, Index& outCount);

    ASTSerialPointer getPointer(ASTSerialIndex index);
    String getString(ASTSerialIndex index);
    Name* getName(ASTSerialIndex index);
    UnownedStringSlice getStringSlice(ASTSerialIndex index);
    SourceLoc getSourceLoc(ASTSerialSourceLoc loc);


        /// Load the entries table (without deserializing anything)
        /// NOTE! data must stay ins scope for outEntries to be valid
    SlangResult loadEntries(const uint8_t* data, size_t dataCount, List<const ASTSerialInfo::Entry*>& outEntries);

        /// NOTE! data must stay ins scope when reading takes place
    SlangResult load(const uint8_t* data, size_t dataCount, ASTBuilder* builder, NamePool* namePool);

    ASTSerialReader(ASTSerialClasses* classes):
        m_classes(classes)
    {
    }

protected:
    List<const Entry*> m_entries;       ///< The entries
    List<void*> m_objects;              ///< The constructed objects

    List<RefPtr<RefObject>> m_scope;    ///< Objects to keep in scope during construction

    NamePool* m_namePool;

    ASTSerialClasses* m_classes;        ///< Used to deserialize 
};

// ---------------------------------------------------------------------------
template <typename T>
void ASTSerialReader::getArray(ASTSerialIndex index, List<T>& out)
{
    typedef ASTSerialTypeInfo<T> ElementTypeInfo;
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


class ASTSerialClasses;

/* This is a class used tby toSerial implementations to turn native type into the serial type */
class ASTSerialWriter : public RefObject
{
public:
    ASTSerialIndex addPointer(const NodeBase* ptr);
    ASTSerialIndex addPointer(const RefObject* ptr);

    template <typename T>
    ASTSerialIndex addArray(const T* in, Index count);

    ASTSerialIndex addString(const UnownedStringSlice& slice);
    ASTSerialIndex addString(const String& in);
    ASTSerialIndex addName(const Name* name);
    ASTSerialSourceLoc addSourceLoc(SourceLoc sourceLoc);

        /// Get the entries table holding how each index maps to an entry
    const List<ASTSerialInfo::Entry*>& getEntries() const { return m_entries; }

        /// Write to a stream
    SlangResult write(Stream* stream);

    ASTSerialWriter(ASTSerialClasses* classes);

protected:

    ASTSerialIndex _addArray(size_t elementSize, size_t alignment, const void* elements, Index elementCount);

    ASTSerialIndex _add(const void* nativePtr, ASTSerialInfo::Entry* entry)
    {
        m_entries.add(entry);
        // Okay I need to allocate space for this
        ASTSerialIndex index = ASTSerialIndex(m_entries.getCount() - 1);
        // Add to the map
        m_ptrMap.Add(nativePtr, Index(index));
        return index;
    }

    Dictionary<const void*, Index> m_ptrMap;    // Maps a pointer to an entry index

    // NOTE! Assumes the content stays in scope!
    Dictionary<UnownedStringSlice, Index> m_sliceMap;

    List<ASTSerialInfo::Entry*> m_entries;      ///< The entries
    MemoryArena m_arena;                        ///< Holds the payloads
    ASTSerialClasses* m_classes;
};

// ---------------------------------------------------------------------------
template <typename T>
ASTSerialIndex ASTSerialWriter::addArray(const T* in, Index count)
{
    typedef ASTSerialTypeInfo<T> ElementTypeInfo;
    typedef typename ElementTypeInfo::SerialType ElementSerialType;

    if (std::is_same<T, ElementSerialType>::value)
    {
        // If they are the same we can just write out
        return _addArray(sizeof(T), SLANG_ALIGN_OF(ElementSerialType), in, count);
    }
    else
    {
        // Else we need to convert
        List<ElementSerialType> work;
        work.setCount(count);

        for (Index i = 0; i < count; ++i)
        {
            ElementTypeInfo::toSerial(this, &in[i], &work[i]);
        }
        return _addArray(sizeof(ElementSerialType), SLANG_ALIGN_OF(ElementSerialType), work.getBuffer(), count);
    }
}

struct ASTSerialType
{
    typedef void(*ToSerialFunc)(ASTSerialWriter* writer, const void* src, void* dst);
    typedef void(*ToNativeFunc)(ASTSerialReader* reader, const void* src, void* dst);

    size_t serialSizeInBytes;
    uint8_t serialAlignment;
    ToSerialFunc toSerialFunc;
    ToNativeFunc toNativeFunc;
};

struct ASTSerialField
{
    const char* name;           ///< The name of the field
    const ASTSerialType* type;        ///< The type of the field
    uint32_t nativeOffset;      ///< Offset to field from base of type
    uint32_t serialOffset;      ///< Offset in serial type    
};


struct ASTSerialClass
{
    ASTNodeType type;
    uint8_t alignment;
    ASTSerialField* fields;
    Index fieldsCount;
    uint32_t size;
};

// An instance could be shared across Sessions, but for simplicity of life time
// here we don't deal with that 
class ASTSerialClasses : public RefObject
{
public:

    const ASTSerialClass* getSerialClass(ASTNodeType type) const { return &m_classes[Index(type)]; }

        /// Ctor
    ASTSerialClasses();

protected:
    MemoryArena m_arena;

    ASTSerialClass m_classes[Index(ASTNodeType::CountOf)];
};


/* None of the functions in this util should *not* be called from production code,
they exist to test features of AST Serialization */
struct ASTSerialTestUtil
{
    static SlangResult selfTest();

        /// Tries to serialize out, read back in and test the results are the same.
        /// Will write dumped out node to files 
    static SlangResult testSerialize(NodeBase* node, RootNamePool* rootNamePool, SharedASTBuilder* sharedASTBuilder, SourceManager* sourceManager);
};

} // namespace Slang

#endif
