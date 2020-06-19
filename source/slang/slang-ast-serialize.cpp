// slang-ast-serialize.cpp
#include "slang-ast-serialize.h"

#include "slang-ast-generated.h"
#include "slang-ast-generated-macro.h"

#include "slang-compiler.h"
#include "slang-type-layout.h"



namespace Slang {

// Things stored as references
// NodeBase derived types
// String
// Array 
// Scope 
// LookupResult

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

struct Pointer
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
     
    Pointer():
        m_kind(Kind::Unknown),
        m_ptr(nullptr)
    {
    }

    Pointer(RefObject* in):
        m_kind(Kind::RefObject),
        m_ptr((void*)in)
    {
    }
    Pointer(NodeBase* in):
        m_kind(Kind::NodeBase),
        m_ptr((void*)in)
    {
    }

    static Kind getKind(const RefObject*) { return Kind::RefObject; }
    static Kind getKind(const NodeBase*) { return Kind::NodeBase; }

    Kind m_kind;
    void* m_ptr;
};

enum class ASTSerialIndex : uint32_t;
typedef uint32_t ASTSerialSourceLoc;

class ASTSerialReader : public RefObject
{
public:
    
    Pointer getPointer(ASTSerialIndex index)
    {
        SLANG_UNUSED(index);
        return Pointer();
    }

#if 0
    void* getArray(ASTSerialIndex index, Index& outCount)
    {
        SLANG_UNUSED(index);
        outCount = 0;
        return nullptr;
    }
#endif

    template <typename T>
    void getArray(ASTSerialIndex index, List<T>& outArray)
    {
        SLANG_UNUSED(index);
        outArray.clear();
    }

    String getString(ASTSerialIndex index)
    {
        SLANG_UNUSED(index);
        return String();
    }

    Name* getName(ASTSerialIndex index)
    {
        if (index == ASTSerialIndex(0))
        {
            return nullptr;
        }
        return nullptr;
    }

    UnownedStringSlice getStringSlice(ASTSerialIndex index)
    {
        SLANG_UNUSED(index);
        return UnownedStringSlice();
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
    ASTSerialIndex addPointer(const Pointer& ptr)
    {
        SLANG_UNUSED(ptr);
        return ASTSerialIndex(0);
    }
    template <typename T>
    ASTSerialIndex addArray(const T* in, Index count)
    {
        SLANG_UNUSED(in);
        SLANG_UNUSED(count);
        return ASTSerialIndex(0);
    }

    ASTSerialIndex addString(const String& in)
    {
        SLANG_UNUSED(in);
        return ASTSerialIndex(0);
    }
    ASTSerialIndex addName(const Name* name)
    {
        if (name == nullptr)
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
struct ASTSerialBasicTypeInfo
{
    typedef T NativeType;
    typedef T SerialType;

    // We want the alignment to be the same as the size of the type for basic types
    // NOTE! Might be different from SLANG_ALIGN_OF(SerialType) 
    enum { SerialAlignment = sizeof(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial) { SLANG_UNUSED(writer); *(T*)serial = *(const T*)native; }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native) { SLANG_UNUSED(reader); *(T*)native = *(const T*)serial; }

    static const ASTSerialType* getType()
    {
        static const ASTSerialType type = { sizeof(SerialType), uint8_t(SerialAlignment), &toSerial, &toNative };
        return &type;
    }
};

template <typename NATIVE_T, typename SERIAL_T>
struct ASTSerialConvertTypeInfo
{
    typedef NATIVE_T NativeType;
    typedef SERIAL_T SerialType;

    enum { SerialAlignment = ASTSerialBasicTypeInfo<SERIAL_T>::SerialAlignment };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial) { SLANG_UNUSED(writer); *(SERIAL_T*)serial = SERIAL_T(*(const NATIVE_T*)native); }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native) { SLANG_UNUSED(reader); *(NATIVE_T*)native = NATIVE_T(*(const SERIAL_T*)serial); }
};

template <typename T>
struct ASTSerialIdentityTypeInfo
{
    typedef T NativeType;
    typedef T SerialType;

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial) { SLANG_UNUSED(writer); *(T*)serial = *(const T*)native; }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native) { SLANG_UNUSED(reader); *(T*)native = *(const T*)serial; }
};

template <typename T>
struct ASTSerialTypeInfo;

// Implement for Basic Types

template <>
struct ASTSerialTypeInfo<uint8_t> : public ASTSerialBasicTypeInfo<uint8_t> {};
template <>
struct ASTSerialTypeInfo<uint16_t> : public ASTSerialBasicTypeInfo<uint16_t> {};
template <>
struct ASTSerialTypeInfo<uint32_t> : public ASTSerialBasicTypeInfo<uint32_t> {};
template <>
struct ASTSerialTypeInfo<uint64_t> : public ASTSerialBasicTypeInfo<uint64_t> {};

template <>
struct ASTSerialTypeInfo<int8_t> : public ASTSerialBasicTypeInfo<int8_t> {};
template <>
struct ASTSerialTypeInfo<int16_t> : public ASTSerialBasicTypeInfo<int16_t> {};
template <>
struct ASTSerialTypeInfo<int32_t> : public ASTSerialBasicTypeInfo<int32_t> {};
template <>
struct ASTSerialTypeInfo<int64_t> : public ASTSerialBasicTypeInfo<int64_t> {};

template <>
struct ASTSerialTypeInfo<float> : public ASTSerialBasicTypeInfo<float> {};
template <>
struct ASTSerialTypeInfo<double> : public ASTSerialBasicTypeInfo<double> {};

// SamplerStateFlavor

template <>
struct ASTSerialTypeInfo<SamplerStateFlavor> : public ASTSerialConvertTypeInfo<SamplerStateFlavor, uint8_t> {};

// TextureFlavor

template <>
struct ASTSerialTypeInfo<TextureFlavor>
{
    typedef TextureFlavor NativeType;
    typedef uint16_t SerialType;
    enum { SerialAlignment = sizeof(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial) { SLANG_UNUSED(writer); *(SerialType*)serial = ((const NativeType*)native)->flavor; }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native) { SLANG_UNUSED(reader); ((NativeType*)native)->flavor = *(const SerialType*)serial; }
};

// Fixed arrays

template <typename T, size_t N>
struct ASTSerialTypeInfo<T[N]>
{
    typedef typename ASTSerialTypeInfo<T> ElementASTSerialType;
    typedef typename ElementASTSerialType::SerialType SerialElementType;

    typedef T NativeType[N];
    typedef SerialElementType SerialType[N];
    
    enum { SerialAlignment = ASTSerialTypeInfo<T>::SerialAlignment };

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
};

// Special case bool - as we can't rely on size alignment 
template <>
struct ASTSerialTypeInfo<bool>
{
    typedef bool NativeType;
    typedef uint8_t SerialType;

    enum { SerialAlignment = sizeof(SerialType) };

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
};


// Pointer
// Could handle different pointer base types with some more template magic here, but instead went with Pointer type to keep
// things simpler.
template <typename T>
struct ASTSerialTypeInfo<T*>
{
    typedef T* NativeType;
    typedef ASTSerialIndex SerialType;

    enum
    {
        SerialAlignment = sizeof(SerialType)
    };

    static void toSerial(ASTSerialWriter* writer, const void* inNative, void* outSerial)
    {
        *(SerialType*)outSerial = writer->addPointer(Pointer(*(T**)inNative));
    }
    static void toNative(ASTSerialReader* reader, const void* inSerial, void* outNative)
    {
        *(T**)outNative = reader->getPointer(*(const SerialType*)inSerial).dynamicCast<T>();
    }
};

struct ASTSerialDeclRefBaseTypeInfo
{
    typedef DeclRefBase NativeType;
    struct SerialType
    {
        ASTSerialIndex substitutions;
        ASTSerialIndex decl;
    };
    enum { SerialAlignment = sizeof(ASTSerialIndex) };

    static void toSerial(ASTSerialWriter* writer, const void* inNative, void* outSerial)
    {
        SerialType& serial = *(SerialType*)outSerial;
        const NativeType& native = *(const NativeType*)inNative;

        serial.decl = writer->addPointer(native.decl);
        serial.substitutions = writer->addPointer(native.substitutions.substitutions);
    }
    static void toNative(ASTSerialReader* reader, const void* inSerial, void* outNative)
    {
        DeclRefBase& native = *(DeclRefBase*)(outNative);
        const SerialType& serial = *(const SerialType*)inSerial;

        native.decl = reader->getPointer(serial.decl).dynamicCast<Decl>();
        native.substitutions.substitutions = reader->getPointer(serial.substitutions).dynamicCast<Substitutions>();
    }
    static const ASTSerialType* getType()
    {
        static const ASTSerialType type = { sizeof(SerialType), uint8_t(SerialAlignment), &toSerial, &toNative };
        return &type;
    }
};

template <typename T>
struct ASTSerialTypeInfo<DeclRef<T>> : public ASTSerialDeclRefBaseTypeInfo {};

// MatrixCoord can just go as is
template <>
struct ASTSerialTypeInfo<MatrixCoord> : ASTSerialIdentityTypeInfo<MatrixCoord> {};

// SourceLoc

// Make the type exposed, so we can look for it if we want to remap.
template <>
struct ASTSerialTypeInfo<SourceLoc>
{
    typedef SourceLoc NativeType;
    typedef ASTSerialSourceLoc SerialType;
    enum { SerialAlignment = sizeof(ASTSerialSourceLoc) };

    static void toSerial(ASTSerialWriter* writer, const void* inNative, void* outSerial)
    {
        *(SerialType*)outSerial = writer->addSourceLoc(*(const NativeType*)inNative);
    }
    static void toNative(ASTSerialReader* reader, const void* inSerial, void* outNative)
    {
        *(NativeType*)outNative = reader->getSourceLoc(*(const SerialType*)inSerial);
    }
};

// List
template <typename T, typename ALLOCATOR>
struct ASTSerialTypeInfo<List<T, ALLOCATOR>>
{
    typedef List<T, ALLOCATOR> NativeType;
    typedef ASTSerialIndex SerialType;

    enum { SerialAlignment = sizeof(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        dst = writer->addArray(src.getBuffer(), src.getCount());
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        auto& dst = *(NativeType*)native;
        auto& src = *(const SerialType*)serial;

        reader->getArray(src, dst);
    }
};

// Dictionary
template <typename KEY, typename VALUE>
struct ASTSerialTypeInfo<Dictionary<KEY, VALUE>>
{
    typedef Dictionary<KEY, VALUE> NativeType;
    struct SerialType
    {
        ASTSerialIndex keys;            ///< Index an array
        ASTSerialIndex values;          ///< Index an array
    };

    typedef typename ASTSerialTypeInfo<KEY>::SerialType KeySerialType;
    typedef typename ASTSerialTypeInfo<VALUE>::SerialType ValueSerialType;

    enum { SerialAlignment = SLANG_ALIGN_OF(ASTSerialIndex) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        List<KeySerialType> keys;
        List<ValueSerialType> values;

        Index count = Index(src.Count());
        keys.setCount(count);
        values.setCount(count);

        Index i = 0;
        for (const auto& pair : src)
        {
            ASTSerialTypeInfo<KEY>::toSerial(writer, &pair.Key, &keys[i]);
            ASTSerialTypeInfo<VALUE>::toSerial(writer, &pair.Value, &values[i]);
            i++;
        }

        dst.keys = writer->addArray(keys.getBuffer(), count);
        dst.values = writer->addArray(values.getBuffer(), count);
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        // Clear it
        dst = NativeType();

        List<KEY> keys;
        List<VALUE> values;

        reader->getArray(src.keys, keys);
        reader->getArray(src.values, values);

        SLANG_ASSERT(keys.getCount() == values.getCount());

        const Index count = keys.getCount();
        for (Index i = 0; i < count; ++i)
        {
            dst.Add(keys[i], values[i]);
        }
    }
};

// QualType

template <>
struct ASTSerialTypeInfo<QualType>
{
    typedef QualType NativeType;
    struct SerialType
    {
        ASTSerialIndex type;
        uint8_t isLeftValue;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(ASTSerialIndex) };

    static void toSerial(ASTSerialWriter* writer, const void* inNative, void* outSerial)
    {
        auto dst = (SerialType*)outSerial;
        auto src = (const NativeType*)inNative;
        dst->isLeftValue = src->isLeftValue ? 1 : 0;
        dst->type = writer->addPointer(src->type);
    }
    static void toNative(ASTSerialReader* reader, const void* inSerial, void* outNative)
    {
        auto src = (const SerialType*)inSerial;
        auto dst = (NativeType*)outNative;
        dst->type = reader->getPointer(src->type).dynamicCast<Type>();
        dst->isLeftValue = src->isLeftValue != 0;
    }
};
// LookupResult

// TODO(JS): Not implemented yet
template <>
struct ASTSerialTypeInfo<LookupResult>
{
    typedef LookupResult NativeType;
    typedef ASTSerialIndex SerialType;
    enum { SerialAlignment = sizeof(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        SLANG_UNUSED(writer);

        auto& src = *(const NativeType*)native;
        SLANG_UNUSED(src);
        auto& dst = *(SerialType*)serial;
        dst = ASTSerialIndex(0);
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        SLANG_UNUSED(reader);

        auto& src = *(const NativeType*)native;
        SLANG_UNUSED(src);
        auto& dst = *(SerialType*)serial;

        SLANG_UNUSED(dst);
    }
};

// TypeExp
template <>
struct ASTSerialTypeInfo<TypeExp>
{
    typedef TypeExp NativeType;
    struct SerialType
    {
        ASTSerialIndex type;
        ASTSerialIndex expr;
    };
    enum { SerialAlignment = sizeof(ASTSerialIndex) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        auto& dst = *(SerialType*)serial;
        auto& src = *(const NativeType*)native;

        dst.type = writer->addPointer(src.type);
        dst.expr = writer->addPointer(src.exp);
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        dst.type = reader->getPointer(src.type).dynamicCast<Type>();
        dst.exp = reader->getPointer(src.type).dynamicCast<Expr>();
    }
};

// DeclCheckStateExt
template <>
struct ASTSerialTypeInfo<DeclCheckStateExt>
{
    typedef DeclCheckStateExt NativeType;
    typedef DeclCheckStateExt::RawType SerialType;

    enum { SerialAlignment = sizeof(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        SLANG_UNUSED(writer);
        *(SerialType*)serial = (*(const NativeType*)native).getRaw();
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        SLANG_UNUSED(reader);
        (*(NativeType*)serial).setRaw(*(const SerialType*)native); 
    }
};

// Modifiers
template <>
struct ASTSerialTypeInfo<Modifiers>
{
    typedef Modifiers NativeType;
    typedef ASTSerialIndex SerialType;

    enum { SerialAlignment = sizeof(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        // We need to make into an array
        List<ASTSerialIndex> modifierIndices;
        for (Modifier* modifier : *(NativeType*)native)
        {
            modifierIndices.add(writer->addPointer(modifier));
        }
        *(SerialType*)serial = writer->addArray(modifierIndices.getBuffer(), modifierIndices.getCount());
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        List<Modifier*> modifiers;
        reader->getArray(*(const SerialType*)serial, modifiers);

        Modifier* prev = nullptr;
        for (Modifier* modifier : modifiers)
        {
            if (prev)
            {
                prev->next = modifier;
            }
        }

        NativeType& dst = *(NativeType*)native;
        dst.first = modifiers.getCount() > 0 ? modifiers[0] : nullptr;
    }
};

// ImageFormat
template <>
struct ASTSerialTypeInfo<ImageFormat> : public ASTSerialConvertTypeInfo<ImageFormat, uint8_t> {};

// Stage
template <>
struct ASTSerialTypeInfo<Stage> : public ASTSerialConvertTypeInfo<Stage, uint8_t> {};

// TokenType
template <>
struct ASTSerialTypeInfo<TokenType> : public ASTSerialConvertTypeInfo<TokenType, uint8_t> {};

// BaseType
template <>
struct ASTSerialTypeInfo<BaseType> : public ASTSerialConvertTypeInfo<BaseType, uint8_t> {};

// SemanticVersion
template <>
struct ASTSerialTypeInfo<SemanticVersion> : public ASTSerialIdentityTypeInfo<SemanticVersion> {};

// ASTNodeType
template <>
struct ASTSerialTypeInfo<ASTNodeType> : public ASTSerialConvertTypeInfo<ASTNodeType, uint16_t> {};

// SyntaxClass<T>
template <typename T>
struct ASTSerialTypeInfo<SyntaxClass<T>>
{
    typedef SyntaxClass<T> NativeType;
    typedef uint16_t SerialType;

    enum { SerialAlignment = sizeof(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        SLANG_UNUSED(writer);
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;
        dst = SerialType(src.classInfo->m_classId);
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        SLANG_UNUSED(reader);
        auto& src = *(const SerialType*)native;
        auto& dst = *(NativeType*)serial;
        dst.classInfo = ReflectClassInfo::getInfo(ASTNodeType(src));
    }
};

// Handle RefPtr - just convert into * to do the conversion
template <typename T>
struct ASTSerialTypeInfo<RefPtr<T>>
{
    typedef RefPtr<T> NativeType;
    typedef ASTSerialIndex SerialType;
    enum { SerialAlignment = sizeof(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        T* obj = src;
        ASTSerialTypeInfo<T*>::toSerial(writer, &obj, serial);
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        T* obj = nullptr;
        ASTSerialTypeInfo<T*>::toNative(reader, serial, &obj);
        *(NativeType*)native = obj;
    }
};

// String
template <>
struct ASTSerialTypeInfo<String>
{
    typedef String NativeType;
    typedef ASTSerialIndex SerialType;
    enum { SerialAlignment = sizeof(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        *(SerialType*)serial = writer->addString(src);
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;
        dst = reader->getString(src);
    }
};


// Token
template <>
struct ASTSerialTypeInfo<Token>
{
    typedef Token NativeType;
    struct SerialType
    {
        ASTSerialTypeInfo<BaseType>::SerialType type;
        ASTSerialTypeInfo<SourceLoc>::SerialType loc;
        ASTSerialIndex name;
    };
    enum { SerialAlignment = sizeof(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        ASTSerialTypeInfo<TokenType>::toSerial(writer, &src.type, &dst.type);
        ASTSerialTypeInfo<SourceLoc>::toSerial(writer, &src.loc, &dst.loc);
        dst.name = writer->addName(src.getName());
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        dst.flags = 0;
        dst.charsNameUnion.chars = nullptr;

        ASTSerialTypeInfo<TokenType>::toNative(reader, &src.type, &dst.type);
        ASTSerialTypeInfo<SourceLoc>::toNative(reader, &src.loc, &dst.loc);

        if (src.name != ASTSerialIndex(0))
        {
            dst.charsNameUnion.name = reader->getName(src.name);
            dst.flags |= TokenFlag::Name;
        }
    }
};

// NameLoc
template <>
struct ASTSerialTypeInfo<NameLoc>
{
    typedef NameLoc NativeType;
    struct SerialType
    {
        ASTSerialTypeInfo<SourceLoc>::SerialType loc;
        ASTSerialIndex name;
    };
    enum { SerialAlignment = sizeof(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        dst.name = writer->addName(src.name);
        ASTSerialTypeInfo<SourceLoc>::toSerial(writer, &src.loc, &dst.loc);
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        dst.name = reader->getName(src.name);
        ASTSerialTypeInfo<SourceLoc>::toNative(reader, &src.loc, &dst.loc);
    }
};

// DiagnosticInfo
template <>
struct ASTSerialTypeInfo<const DiagnosticInfo*>
{
    typedef const DiagnosticInfo* NativeType;
    typedef ASTSerialIndex SerialType;

    enum { SerialAlignment = sizeof(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;
        dst = src ? writer->addString(UnownedStringSlice(src->name)) : ASTSerialIndex(0);
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        if (src == ASTSerialIndex(0))
        {
            dst = nullptr;
        }
        else
        {
            dst = findDiagnosticByName(reader->getStringSlice(src));
        }
    }
};


// Getting the type info, let's use a static variable to hold the state to keep simple
template <typename T>
struct ASTSerialGetType
{
    typedef typename ASTSerialTypeInfo<T> Info;
    static const ASTSerialType* getType()
    {
        static const ASTSerialType type = { sizeof(Info::SerialType), uint8_t(Info::SerialAlignment), &Info::toSerial, &Info::toNative };
        return &type;
    }
};

// Special case DeclRef, because it always uses the same type
template <typename T>
struct ASTSerialGetType<DeclRef<T>>
{
    static const ASTSerialType* getType() { return ASTSerialDeclRefBaseTypeInfo::getType(); }
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
    field.type = ASTSerialGetType<T>::getType();
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
        struct Thing
        {
            Module* node;
        };
        Thing thing;

        //Pointer pointer(thing.node);

        auto field = _calcField("node", thing.node);


        const ASTSerialType* type = ASTSerialGetType<Type*>::getType();
        SLANG_UNUSED(type);
    }

    {
        const ASTSerialType* type = ASTSerialGetType<int[10]>::getType();
        SLANG_UNUSED(type);
    }

    {
        const ASTSerialType* type = ASTSerialGetType<bool[3]>::getType();
        SLANG_UNUSED(type);
    }

    {
        const ASTSerialType* type = ASTSerialGetType<Type*[3]>::getType();
        SLANG_UNUSED(type);
    }

    return SLANG_OK;
}


} // namespace Slang
