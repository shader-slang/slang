// slang-ast-serialize.cpp
#include "slang-ast-serialize.h"

#include "slang-ast-generated.h"
#include "slang-ast-generated-macro.h"

namespace Slang {

// Perhaps we have hold the Rtti type and the the serialize information
struct ASTSerializeType
{
    enum class Kind
    {
        BasicType,          ///< Int, Float, Char etc
        Reference,          ///< References some other AST Node (or array)
        FixedArray,         ///< Needs to hold the underlying type (Hold the size)
        Struct,             ///< Need to know the fields etc
        Class,              ///< For now can only be AST nodes, and also has the fields
    };
    Kind kind;
    uint8_t alignment;
};

struct Entry
{
    enum class EntryKind
    {
        ASTNode,
        Array,
        String,
    };
};

enum class ASTSerialIndex : uint32_t;
typedef uint32_t ASTSerialSourceLoc;

class ASTSerialReader : public RefObject
{
public:
    NodeBase* getNode(ASTSerialIndex index)
    {
        SLANG_UNUSED(index);
        return nullptr;
    }

    SourceLoc getSourceLoc(ASTSerialSourceLoc loc)
    {
        SLANG_UNUSED(loc);
        return SourceLoc();
    }
};

class ASTSerialWriter : public RefObject
{
public:
    ASTSerialIndex getSerialIndex(NodeBase* node)
    {
        if (node == nullptr)
        {
            return ASTSerialIndex(0);
        }

        return ASTSerialIndex(0);
    }
    ASTSerialSourceLoc addSourceLoc(SourceLoc sourceLoc)
    {
        SLANG_UNUSED(sourceLoc);
        return 0;
    }
};



struct ASTSerialDictionary
{
    // Both arrays must be the same size
    ASTSerialIndex keys;                ///< Indexes into an array of keys
    ASTSerialIndex values;              ///< Index into an array of values
};

// We need to know the serial size 
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

// We need to have a way to map between the two.
// If no mapping is needed, (just a copy), then we don't bother with the functions
template <typename T>
struct ASTSerialBasicType
{
    typedef T NativeType;
    typedef T SerialType;

    // We want the alignment to be the same as the size of the type for basic types
    // NOTE! Might be different from SLANG_ALIGN_OF(SerialType) 
    enum
    {
        SerialAlignment = sizeof(SerialType)
    };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial) { SLANG_UNUSED(writer); *(T*)serial = *(const T*)native; }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native) { SLANG_UNUSED(reader); *(T*)native = *(const T*)serial; }

    static const ASTSerialType* getType()
    {
        static const ASTSerialType type = { sizeof(SerialType), uint8_t(SerialAlignment), &toSerial, &toNative };
        return &type;
    }
};

template <typename NATIVE_T, typename SERIAL_T>
struct ASTSerialConvertType
{
    typedef NATIVE_T NativeType;
    typedef SERIAL_T SerialType;

    // We want the alignment to be the same as the size of the type for basic types
    // NOTE! Might be different from SLANG_ALIGN_OF(SerialType) 
    enum
    {
        SerialAlignment = ASTSerialBasicType<SERIAL_T>::SerialAlignment
    };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial) { SLANG_UNUSED(writer); *(SERIAL_T*)serial = SERIAL_T(*(const NATIVE_T*)native); }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native) { SLANG_UNUSED(reader); *(NATIVE_T*)native = NATIVE_T(*(const T*)serial); }

    static const ASTSerialType* getType()
    {
        static const ASTSerialType type = { sizeof(SERIAL_T), uint8_t(SerialAlignment), &toSerial, &toNative };
        return &type;
    }
};

template <typename T>
struct ASTSerialAsIsType
{
    typedef T NativeType;
    typedef T SerialType;

    // We want the alignment to be the same as the size of the type for basic types
    // NOTE! Might be different from SLANG_ALIGN_OF(SerialType) 
    enum
    {
        SerialAlignment = SLANG_ALIGN_OF(T)
    };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial) { SLANG_UNUSED(writer); *(T*)serial = *(const T*)native; }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native) { SLANG_UNUSED(reader); *(T*)native = *(const T*)serial; }

    static const ASTSerialType* getType()
    {
        static const ASTSerialType type = { sizeof(SerialType), uint8_t(SerialAlignment), &toSerial, &toNative };
        return &type;
    }
};


template <typename T>
struct ASTGetSerialType;

// Implement for Basic Types

template <>
struct ASTGetSerialType<uint8_t> : public ASTSerialBasicType<uint8_t> {};
template <>
struct ASTGetSerialType<uint16_t> : public ASTSerialBasicType<uint16_t> {};
template <>
struct ASTGetSerialType<uint32_t> : public ASTSerialBasicType<uint32_t> {};
template <>
struct ASTGetSerialType<uint64_t> : public ASTSerialBasicType<uint64_t> {};

template <>
struct ASTGetSerialType<int8_t> : public ASTSerialBasicType<int8_t> {};
template <>
struct ASTGetSerialType<int16_t> : public ASTSerialBasicType<int16_t> {};
template <>
struct ASTGetSerialType<int32_t> : public ASTSerialBasicType<int32_t> {};
template <>
struct ASTGetSerialType<int64_t> : public ASTSerialBasicType<int64_t> {};

template <>
struct ASTGetSerialType<float> : public ASTSerialBasicType<float> {};
template <>
struct ASTGetSerialType<double> : public ASTSerialBasicType<double> {};

// Fixed arrays

template <typename T, size_t N>
struct ASTGetSerialType<T[N]>
{
    typedef typename ASTGetSerialType<T> ElementASTSerialType;
    typedef typename ElementASTSerialType::SerialType SerialElementType;

    typedef T NativeType[N];
    typedef SerialElementType SerialType[N];
    
    enum
    {
        SerialAlignment = ASTGetSerialType<T>::SerialAlignment
    };

    static void toSerial(ASTSerialWriter* writer, const void* inNative, void* outSerial)
    {
        SerialElementType* serial = (SerialElementType*)outSerial;
        const T* native = (const T*)inNative;
        for (Index i = 0; i < Index(N); ++i)
        {
            ElementASTSerialType::toSerial(writer, native + i, serial + i);
        }
    }
    static void toNative(ASTSerialReader* reader, const void* inSerial, void* outNative)
    {
        const SerialElementType* serial = (const SerialElementType*)inSerial;
        T* native = (T*)outNative;
        for (Index i = 0; i < Index(N); ++i)
        {
            ElementASTSerialType::toNative(reader, serial + i, native + i);
        }
    }

    static const ASTSerialType* getType()
    {
        static const ASTSerialType type = { sizeof(SerialType), uint8_t(SerialAlignment), &toSerial, &toNative };
        return &type;
    }
};

// Special case bool - as we can't rely on size alignment 
template <>
struct ASTGetSerialType<bool>
{
    typedef bool NativeType;
    typedef uint8_t SerialType;

    enum
    {
        SerialAlignment = sizeof(SerialType)
    };

    static void toSerial(ASTSerialWriter* writer, const void* inNative, void* outSerial)
    {
        SLANG_UNUSED(writer);
        *(SerialType*)outSerial = *(const NativeType*)inNative ? 1 : 0;
    }
    static void toNative(ASTSerialReader* reader, const void* inSerial, void* outNative)
    {
        SLANG_UNUSED(reader);
        *(NativeType*)outNative = (*(const SerialType*)inSerial) != 0;
    }

    static const ASTSerialType* getType()
    {
        static const ASTSerialType type = { sizeof(SerialType), uint8_t(SerialAlignment), &toSerial, &toNative };
        return &type;
    }
};

// Pointer
// Perhaps if I have different pointer types, that *aren't* derived from NodeBase, I can use derivation to change the behavior
template <typename T>
struct ASTGetSerialType<T*>
{
    typedef T* NativeType;
    typedef ASTSerialIndex SerialType;

    enum
    {
        SerialAlignment = sizeof(SerialType)
    };

    static void toSerial(ASTSerialWriter* writer, const void* inNative, void* outSerial)
    {
        *(SerialType*)outSerial = writer->getSerialIndex(*(NodeBase**)inNative);
    }
    static void toNative(ASTSerialReader* reader, const void* inSerial, void* outNative)
    {
        *(NodeBase**)outNative = reader->getNode(*(const SerialType*)inSerial);
    }
    static const ASTSerialType* getType()
    {
        static const ASTSerialType type = { sizeof(SerialType), uint8_t(SerialAlignment), &toSerial, &toNative };
        return &type;
    }
};

struct ASTGetSerialDeclRefBaseType
{
    typedef DeclRefBase NativeType;
    struct SerialType
    {
        ASTSerialIndex substitutions;
        ASTSerialIndex decl;
    };
    enum
    {
        SerialAlignment = sizeof(ASTSerialIndex),
    };

    static void toSerial(ASTSerialWriter* writer, const void* inNative, void* outSerial)
    {
        SerialType& serial = *(SerialType*)outSerial;
        const NativeType& native = *(const NativeType*)inNative;

        serial.decl = writer->getSerialIndex(native.decl);
        serial.substitutions = writer->getSerialIndex(native.substitutions.substitutions);
    }
    static void toNative(ASTSerialReader* reader, const void* inSerial, void* outNative)
    {
        DeclRefBase& native = *(DeclRefBase*)(outNative);
        const SerialType& serial = *(const SerialType*)inSerial;

        native.decl = static_cast<Decl*>(reader->getNode(serial.decl));
        native.substitutions.substitutions = static_cast<Substitutions*>(reader->getNode(serial.substitutions));
    }
    static const ASTSerialType* getType()
    {
        static const ASTSerialType type = { sizeof(SerialType), uint8_t(SerialAlignment), &toSerial, &toNative };
        return &type;
    }
};

template <typename T>
struct ASTGetSerialType<DeclRef<T>> : public ASTGetSerialDeclRefBaseType {};

// MatrixCoord can just go as is
template <>
struct ASTGetSerialType<MatrixCoord> : ASTSerialAsIsType<MatrixCoord> {};

// SourceLoc

// Make the type exposed, so we can look for it if we want to remap.
template <>
struct ASTGetSerialType<SourceLoc>
{
    typedef SourceLoc NativeType;
    typedef ASTSerialSourceLoc SerialType;
    enum
    {
        SerialAlignment = sizeof(ASTSerialSourceLoc),
    };

    static void toSerial(ASTSerialWriter* writer, const void* inNative, void* outSerial)
    {
        *(SerialType*)outSerial = writer->addSourceLoc(*(const NativeType*)inNative);
    }
    static void toNative(ASTSerialReader* reader, const void* inSerial, void* outNative)
    {
        *(NativeType*)outNative = reader->getSourceLoc(*(const SerialType*)inSerial);
    }
    static const ASTSerialType* getType()
    {
        static const ASTSerialType type = { sizeof(SerialType), uint8_t(SerialAlignment), &toSerial, &toNative };
        return &type;
    }
};


template <typename T, typename ALLOCATOR>
struct ASTGetSerialType<List<T, ALLOCATOR>>
{
    typedef List<T, ALLOCATOR> NativeType;
    typedef ASTSerialIndex SerialType;
    enum
    {
        SerialAlignment = sizeof(SerialType),
    };

    static void toSerial(ASTSerialWriter* writer, const void* inNative, void* outSerial)
    {
        SLANG_UNUSED(writer);
        SLANG_UNUSED(inNative);
        SLANG_UNUSED(outSerial);
        //*(SerialType*)outSerial = writer->addSourceLoc(*(const NativeType*)inNative);
    }
    static void toNative(ASTSerialReader* reader, const void* inSerial, void* outNative)
    {
        SLANG_UNUSED(reader);
        SLANG_UNUSED(inNative);
        SLANG_UNUSED(outSerial);
        //*(NativeType*)outNative = reader->getSourceLoc(*(const SerialType*)inSerial);
    }
    static const ASTSerialType* getType()
    {
        static const ASTSerialType type = { sizeof(SerialType), uint8_t(SerialAlignment), &toSerial, &toNative };
        return &type;
    }
};

// For array, I can't save the 'type', but I can save the alignment (?) and the element size.
// I could just require all indexed things, must be on 64 bit boundaries (and so alignment)
#if 0
struct Entry
{
    enum class Type
    {
        String,    
        Array,
        Node,
    };

    // We could store a skip with each type, that would say the amount of bytes to get to the next entry.
    // We could keep the types separate from the pointers to the contents, and then different allocations have different alignemnt
    Type type;
};
#endif

template <typename T>
ASTSerialField _calcField(const char* name, T& in)
{
    uint8_t* ptr = &reinterpret_cast<uint8_t&>(in);

    ASTSerialField field;
    field.name = name;
    field.type = ASTGetSerialType<T>::getType();
    // This only works because we in is an offset from 1
    field.nativeOffset = uint32_t(size_t(ptr) - 1);
    field.serialOffset = 0;
    return field;
}

#define SLANG_AST_SERIAL_FIELD(FIELD_NAME, TYPE, param) fields.add(_calcField(#FIELD_NAME, obj->FIELD_NAME));

struct ASTFieldAccess
{

// Note that the obj point is not nullptr, because some compilers notice this is 'indexing from null'
// and warn/error. So we offset from 1.
#define SLANG_AST_SERIAL_FIELDS_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    static void serialFields_##NAME(List<ASTSerialField>& fields) \
    { \
        NAME* obj = (NAME*)1; \
        SLANG_UNUSED(fields); \
        SLANG_UNUSED(obj); \
        SLANG_FIELDS_ASTNode_##NAME(SLANG_AST_SERIAL_FIELD, param) \
    }

    SLANG_ALL_ASTNode_NodeBase(SLANG_AST_SERIAL_FIELDS_IMPL, _)

};

/* static */SlangResult ASTSerializeUtil::selfTest()
{
    {
        const ASTSerialType* type = ASTGetSerialType<int[10]>::getType();
        SLANG_UNUSED(type);
    }

    {
        const ASTSerialType* type = ASTGetSerialType<bool[3]>::getType();
        SLANG_UNUSED(type);
    }

    {
        const ASTSerialType* type = ASTGetSerialType<Type*[3]>::getType();
        SLANG_UNUSED(type);
    }

    return SLANG_OK;
}





// We can generate a function that does the conversion forward and backward.
// We need to know the offsets in the original type



} // namespace Slang
