// slang-ast-serialize.h
#ifndef SLANG_AST_SERIALIZE_H
#define SLANG_AST_SERIALIZE_H

#include <type_traits>

#include "slang-ast-support-types.h"
#include "slang-ast-all.h"

namespace Slang
{

enum class ASTSerialIndex : uint32_t;
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

    ASTSerialPointer getPointer(ASTSerialIndex index);
    template <typename T>
    void getArray(ASTSerialIndex index, List<T>& outArray);
    String getString(ASTSerialIndex index);
    Name* getName(ASTSerialIndex index);
    UnownedStringSlice getStringSlice(ASTSerialIndex index);
    SourceLoc getSourceLoc(ASTSerialSourceLoc loc);
};

// ---------------------------------------------------------------------------
template <typename T>
void ASTSerialReader::getArray(ASTSerialIndex index, List<T>& outArray)
{
    SLANG_UNUSED(index);
    outArray.clear();
}

/* This is a class used tby toSerial implementations to turn native type into the serial type */
class ASTSerialWriter : public RefObject
{
public:
    ASTSerialIndex addPointer(const ASTSerialPointer& ptr);

    template <typename T>
    ASTSerialIndex addArray(const T* in, Index count);

    ASTSerialIndex addString(const String& in);
    ASTSerialIndex addName(const Name* name);
    ASTSerialSourceLoc addSourceLoc(SourceLoc sourceLoc);
};

// ---------------------------------------------------------------------------
template <typename T>
ASTSerialIndex ASTSerialWriter::addArray(const T* in, Index count)
{
    typedef typename ASTSerialTypeInfo<T> ElementTypeInfo;
    typedef typename ElementTypeInfo::SerialType ElementSerialType;

    if (std::is_same<T, ElementSerialType>::value)
    {
        // If they are the same we can just write out
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
    }

    return ASTSerialIndex(0);
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

struct ASTSerializeUtil
{
    static SlangResult selfTest();
};

} // namespace Slang

#endif
