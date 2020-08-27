// slang-ast-serialize.cpp
#include "slang-ast-serialize.h"

#include "slang-ast-generated.h"
#include "slang-ast-generated-macro.h"

#include "slang-compiler.h"
#include "slang-type-layout.h"

#include "slang-ast-dump.h"

#include "slang-ast-support-types.h"

#include "../core/slang-byte-encode-util.h"

namespace Slang {


// Things stored as references:
// 
// NodeBase derived types
// Array 
// 
// RefObject derived types:
// 
// Breadcrumb
// StringRepresentation
// Scope



// Helpers to convert fields treated as values

class ASTSerialReader;
class ASTSerialWriter;

template <typename NATIVE_TYPE, typename SERIAL_TYPE>
static void _toSerialValue(ASTSerialWriter* writer, const NATIVE_TYPE& src, SERIAL_TYPE& dst)
{
    ASTSerialTypeInfo<NATIVE_TYPE>::toSerial(writer, &src, &dst);
}

template <typename SERIAL_TYPE, typename NATIVE_TYPE>
static void _toNativeValue(ASTSerialReader* reader, const SERIAL_TYPE& src, NATIVE_TYPE& dst)
{
    ASTSerialTypeInfo<NATIVE_TYPE>::toNative(reader, &src, &dst);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Serial <-> Native conversion  !!!!!!!!!!!!!!!!!!!!!!!!


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

// Don't need to convert the index type

template <>
struct ASTSerialTypeInfo<ASTSerialIndex> : public ASTSerialIdentityTypeInfo<ASTSerialIndex> {};


// Because is sized, we don't need to convert
template <>
struct ASTSerialTypeInfo<FeedbackType::Kind> : public ASTSerialIdentityTypeInfo<FeedbackType::Kind> {};

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
    typedef ASTSerialTypeInfo<T> ElementASTSerialType;
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
        SerialAlignment = SLANG_ALIGN_OF(SerialType)
    };

    static void toSerial(ASTSerialWriter* writer, const void* inNative, void* outSerial)
    {
        *(SerialType*)outSerial = writer->addPointer(*(T**)inNative);
    }
    static void toNative(ASTSerialReader* reader, const void* inSerial, void* outNative)
    {
        *(T**)outNative = reader->getPointer(*(const SerialType*)inSerial).dynamicCast<T>();
    }
};

// Special case Name
template <>
struct ASTSerialTypeInfo<Name*> : public ASTSerialTypeInfo<RefObject*>
{
    // Special case 
    typedef Name* NativeType;
    static void toNative(ASTSerialReader* reader, const void* inSerial, void* outNative)
    {
        *(Name**)outNative = reader->getName(*(const SerialType*)inSerial);
    }
};

template <>
struct ASTSerialTypeInfo<const Name*> : public ASTSerialTypeInfo<Name*>
{
};


struct ASTSerialDeclRefBaseTypeInfo
{
    typedef DeclRefBase NativeType;
    struct SerialType
    {
        ASTSerialIndex substitutions;
        ASTSerialIndex decl;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

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
    enum { SerialAlignment = SLANG_ALIGN_OF(ASTSerialSourceLoc) };

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

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

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

// SyntaxClass<T>
template <typename T>
struct ASTSerialTypeInfo<SyntaxClass<T>>
{
    typedef SyntaxClass<T> NativeType;
    typedef uint16_t SerialType;

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

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
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;
        dst.classInfo = ReflectClassInfo::getInfo(ASTNodeType(src));
    }
};

// Handle RefPtr - just convert into * to do the conversion
template <typename T>
struct ASTSerialTypeInfo<RefPtr<T>>
{
    typedef RefPtr<T> NativeType;
    typedef typename ASTSerialTypeInfo<T*>::SerialType SerialType;
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

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

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        auto dst = (SerialType*)serial;
        auto src = (const NativeType*)native;
        dst->isLeftValue = src->isLeftValue ? 1 : 0;
        dst->type = writer->addPointer(src->type);
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        auto src = (const SerialType*)serial;
        auto dst = (NativeType*)native;
        dst->type = reader->getPointer(src->type).dynamicCast<Type>();
        dst->isLeftValue = src->isLeftValue != 0;
    }
};


// LookupResult::Breadcrumb
template <>
struct ASTSerialTypeInfo<LookupResultItem::Breadcrumb>
{
    typedef LookupResultItem::Breadcrumb NativeType;
    struct SerialType
    {
        NativeType::Kind kind;
        NativeType::ThisParameterMode thisParameterMode;
        ASTSerialTypeInfo<DeclRef<Decl>>::SerialType declRef;
        ASTSerialTypeInfo<RefPtr<NativeType>> next; 
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        dst.kind = src.kind;
        dst.thisParameterMode = src.thisParameterMode;
        _toSerialValue(writer, src.declRef, dst.declRef);
        _toSerialValue(writer, src.next, dst.next);
    }
     
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        auto& dst = *(NativeType*)native;
        auto& src = *(const SerialType*)serial;

        dst.kind = src.kind;
        dst.thisParameterMode = src.thisParameterMode;
        _toNativeValue(reader, src.declRef, dst.declRef);
        _toNativeValue(reader, src.next, dst.next);
    }
};

// LookupResultItem
template <>
struct ASTSerialTypeInfo<LookupResultItem>
{
    typedef LookupResultItem NativeType;
    struct SerialType
    {
        ASTSerialTypeInfo<DeclRef<Decl>>::SerialType declRef;
        ASTSerialTypeInfo<RefPtr<NativeType::Breadcrumb>> breadcrumbs;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        _toSerialValue(writer, src.declRef, dst.declRef);
        _toSerialValue(writer, src.breadcrumbs, dst.breadcrumbs);
    }

    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        auto& dst = *(NativeType*)native;
        auto& src = *(const SerialType*)serial;

        _toNativeValue(reader, src.declRef, dst.declRef);
        _toNativeValue(reader, src.breadcrumbs, dst.breadcrumbs);
    }
};

// LookupResult
template <>
struct ASTSerialTypeInfo<LookupResult>
{
    typedef LookupResult NativeType;
    typedef ASTSerialIndex SerialType;
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        if (src.isOverloaded())
        {
            // Save off as an array
            dst = writer->addArray(src.items.getBuffer(), src.items.getCount());
        }
        else if (src.item.declRef.getDecl())
        {
            dst = writer->addArray(&src.item, 1);
        }
        else
        {
            dst = ASTSerialIndex(0);
        }
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        auto& dst = *(NativeType*)native;
        auto& src = *(const SerialType*)serial;

        // Initialize
        dst = NativeType();

        List<LookupResultItem> items;
        reader->getArray(src, items);

        if (items.getCount() == 1)
        {
            dst.item = items[0];
        }
        else
        {
            dst.items.swapWith(items);
            // We have to set item such that it is valid/member of items, if items is non empty
            dst.item = dst.items[0];
        }
    }
};


// GlobalGenericParamSubstitution::ConstraintArg
template <>
struct ASTSerialTypeInfo<GlobalGenericParamSubstitution::ConstraintArg>
{
    typedef GlobalGenericParamSubstitution::ConstraintArg NativeType;
    struct SerialType
    {
        ASTSerialIndex decl;
        ASTSerialIndex val;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(ASTSerialIndex) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        auto& dst = *(SerialType*)serial;
        auto& src = *(const NativeType*)native;

        dst.decl = writer->addPointer(src.decl);
        dst.val = writer->addPointer(src.val);
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        dst.decl = reader->getPointer(src.decl).dynamicCast<Decl>();
        dst.val = reader->getPointer(src.val).dynamicCast<Val>();
    }
};

// ExpandedSpecializationArg
template <>
struct ASTSerialTypeInfo<ExpandedSpecializationArg>
{
    typedef ExpandedSpecializationArg NativeType;
    struct SerialType
    {
        ASTSerialIndex val;
        ASTSerialIndex witness;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(ASTSerialIndex) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        auto& dst = *(SerialType*)serial;
        auto& src = *(const NativeType*)native;

        dst.witness = writer->addPointer(src.witness);
        dst.val = writer->addPointer(src.val);
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        dst.witness = reader->getPointer(src.witness).dynamicCast<Val>();
        dst.val = reader->getPointer(src.val).dynamicCast<Val>();
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
    enum { SerialAlignment = SLANG_ALIGN_OF(ASTSerialIndex) };

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
        dst.exp = reader->getPointer(src.expr).dynamicCast<Expr>();
    }
};

// DeclCheckStateExt
template <>
struct ASTSerialTypeInfo<DeclCheckStateExt>
{
    typedef DeclCheckStateExt NativeType;
    typedef DeclCheckStateExt::RawType SerialType;

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

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

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

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

// String
template <>
struct ASTSerialTypeInfo<String>
{
    typedef String NativeType;
    typedef ASTSerialIndex SerialType;
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

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
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(ASTSerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        ASTSerialTypeInfo<TokenType>::toSerial(writer, &src.type, &dst.type);
        ASTSerialTypeInfo<SourceLoc>::toSerial(writer, &src.loc, &dst.loc);

        if (src.flags & TokenFlag::Name)
        {
            dst.name = writer->addName(src.getName());
        }
        else
        {
            dst.name = writer->addString(src.getContent());
        }
    }
    static void toNative(ASTSerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        dst.flags = 0;
        dst.charsNameUnion.chars = nullptr;

        ASTSerialTypeInfo<TokenType>::toNative(reader, &src.type, &dst.type);
        ASTSerialTypeInfo<SourceLoc>::toNative(reader, &src.loc, &dst.loc);

        // At the other end all token content will appear as Names.
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
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

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

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

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

// !!!!!!!!!!!!!!!!!!!!! ASTSerialGetType<T> !!!!!!!!!!!!!!!!!!!!!!!!!!!
// Getting the type info, let's use a static variable to hold the state to keep simple

template <typename T>
struct ASTSerialGetType
{
    static const ASTSerialType* getType()
    {
        typedef ASTSerialTypeInfo<T> Info;
        static const ASTSerialType type = { sizeof(typename Info::SerialType), uint8_t(Info::SerialAlignment), &Info::toSerial, &Info::toNative };
        return &type;
    }
};

// Special case DeclRef, because it always uses the same type
template <typename T>
struct ASTSerialGetType<DeclRef<T>>
{
    static const ASTSerialType* getType() { return ASTSerialDeclRefBaseTypeInfo::getType(); }
};

// !!!!!!!!!!!!!!!!!!!!!! Generate fields for a type !!!!!!!!!!!!!!!!!!!!!!!!!!!


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

static ASTSerialClass _makeClass(MemoryArena* arena, ASTNodeType type, const List<ASTSerialField>& fields)
{
    ASTSerialClass cls = { type, 0, 0, 0, 0 };
    cls.fieldsCount = fields.getCount();
    cls.fields = arena->allocateAndCopyArray(fields.getBuffer(), fields.getCount());
    return cls;
}

#define SLANG_AST_SERIAL_FIELD(FIELD_NAME, TYPE, param) fields.add(_calcField(#FIELD_NAME, obj->FIELD_NAME));

// Note that the obj point is not nullptr, because some compilers notice this is 'indexing from null'
// and warn/error. So we offset from 1.
#define SLANG_AST_SERIAL_MAKE_CLASS(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
{ \
    NAME* obj = (NAME*)1; \
    SLANG_UNUSED(obj); \
    fields.clear(); \
    SLANG_FIELDS_ASTNode_##NAME(SLANG_AST_SERIAL_FIELD, param) \
    outClasses[Index(ASTNodeType::NAME)] = _makeClass(arena, ASTNodeType::NAME, fields); \
}

struct ASTFieldAccess
{
    static void calcClasses(MemoryArena* arena, ASTSerialClass outClasses[Index(ASTNodeType::CountOf)])
    {
        List<ASTSerialField> fields;
        SLANG_ALL_ASTNode_NodeBase(SLANG_AST_SERIAL_MAKE_CLASS, _)
    }
};

ASTSerialClasses::ASTSerialClasses():
    m_arena(2048)
{
    memset(m_classes, 0, sizeof(m_classes));
    ASTFieldAccess::calcClasses(&m_arena, m_classes);

    // Now work out the layout
    for (Index i = 0; i < SLANG_COUNT_OF(m_classes); ++i)
    {
        // Set up each class in order, from lowest to highest index
        // Doing so means super class is always setup
        ASTSerialClass& serialClass = m_classes[i];

        const ReflectClassInfo* info = ReflectClassInfo::getInfo(serialClass.type);

        size_t maxAlignment = 1;
        size_t offset = 0;

        const ReflectClassInfo* superInfo = info->m_superClass;
        if (superInfo)
        {
            ASTSerialClass& superSerialInfo = m_classes[superInfo->m_classId];

            // If it's been setup, then alignment must be non zero.
            // The ordering of ASTNodeType, should mean  type have larger ASTNodeType greater than supers ASTNodeType.
            SLANG_ASSERT(superSerialInfo.alignment != 0);

            // Must be a power of 2
            SLANG_ASSERT((superSerialInfo.alignment & (superSerialInfo.alignment - 1)) == 0);

            maxAlignment = superSerialInfo.alignment;
            offset = superSerialInfo.size;

            // Check it is correctly aligned
            SLANG_ASSERT((offset & (maxAlignment - 1)) == 0);
        }

        // Okay, go through fields setting their offset
        ASTSerialField* fields = serialClass.fields;
        for (Index j = 0; j < serialClass.fieldsCount; j++)
        {
            ASTSerialField& field = fields[j];

            size_t alignment = field.type->serialAlignment;
            // Make sure the offset is aligned for the field requirement
            offset = (offset + alignment - 1) & ~(alignment - 1);

            // Save the field offset
            field.serialOffset = uint32_t(offset);

            // Move past the field
            offset += field.type->serialSizeInBytes;

            // Calc the maximum alignment
            maxAlignment = (alignment > maxAlignment) ? alignment : maxAlignment;
        }

        // Align with maximum alignment
        offset = (offset + maxAlignment - 1) & ~(maxAlignment - 1);

        serialClass.alignment = uint8_t(maxAlignment);
        serialClass.size = uint32_t(offset);
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ASTSerialWriter  !!!!!!!!!!!!!!!!!!!!!!!!!!!!

ASTSerialWriter::ASTSerialWriter(ASTSerialClasses* classes) :
    m_arena(2048),
    m_classes(classes)
{
    // 0 is always the null pointer
    m_entries.add(nullptr);
    m_ptrMap.Add(nullptr, 0);
}

ASTSerialIndex ASTSerialWriter::addPointer(const NodeBase* node)
{
    // Null is always 0
    if (node == nullptr)
    {
        return ASTSerialIndex(0);
    }
    // Look up in the map
    Index* indexPtr = m_ptrMap.TryGetValue(node);
    if (indexPtr)
    {
        return ASTSerialIndex(*indexPtr);
    }

    const ASTSerialClass* serialClass = m_classes->getSerialClass(node->astNodeType);

    typedef ASTSerialInfo::NodeEntry NodeEntry;

    NodeEntry* nodeEntry = (NodeEntry*)m_arena.allocateAligned(sizeof(NodeEntry) + serialClass->size, ASTSerialInfo::MAX_ALIGNMENT);

    nodeEntry->type = ASTSerialInfo::Type::Node;
    nodeEntry->astNodeType = uint16_t(node->astNodeType);
    nodeEntry->info = ASTSerialInfo::makeEntryInfo(serialClass->alignment);

    auto index = _add(node, nodeEntry);

    uint8_t* serialPayload = (uint8_t*)(nodeEntry + 1);

    while (serialClass)
    {
        for (Index i = 0; i < serialClass->fieldsCount; ++i)
        {
            auto field = serialClass->fields[i];
            // Work out the offsets
            auto srcField = ((const uint8_t*)node) + field.nativeOffset;
            auto dstField = serialPayload + field.serialOffset;

            field.type->toSerialFunc(this, srcField, dstField);
        }
        // Get the super class
        const ReflectClassInfo* reflectInfo = ReflectClassInfo::getInfo(serialClass->type);
        const ReflectClassInfo* superReflectInfo = reflectInfo->m_superClass;

        serialClass = superReflectInfo ? m_classes->getSerialClass(ASTNodeType(superReflectInfo->m_classId)) : nullptr;
    }

    return index;
}

ASTSerialIndex ASTSerialWriter::addPointer(const RefObject* obj)
{
    // Null is always 0
    if (obj == nullptr)
    {
        return ASTSerialIndex(0);
    }
    // Look up in the map
    Index* indexPtr = m_ptrMap.TryGetValue(obj);
    if (indexPtr)
    {
        return ASTSerialIndex(*indexPtr);
    }

    if (auto stringRep = dynamicCast<StringRepresentation>(obj))
    {
        ASTSerialIndex index = addString(StringRepresentation::asSlice(stringRep));
        m_ptrMap.Add(obj, Index(index));
        return index;
    }
    else if (auto breadcrumb = dynamicCast<LookupResultItem::Breadcrumb>(obj))
    {
        typedef ASTSerialTypeInfo<LookupResultItem::Breadcrumb> TypeInfo;
        typedef ASTSerialInfo::RefObjectEntry RefObjectEntry;

        size_t alignment = TypeInfo::SerialAlignment;
        alignment = (alignment < SLANG_ALIGN_OF(ASTSerialInfo::RefObjectEntry)) ? SLANG_ALIGN_OF(ASTSerialInfo::RefObjectEntry) : alignment;

        RefObjectEntry* entry = (RefObjectEntry*)m_arena.allocateAligned(sizeof(RefObjectEntry) + sizeof(TypeInfo::SerialType), alignment);

        entry->type = ASTSerialInfo::Type::RefObject;
        entry->info = ASTSerialInfo::makeEntryInfo(int(alignment));
        entry->subType = RefObjectEntry::SubType::Breadcrumb;

        auto index = _add(breadcrumb, entry);

        // Do any conversion
        TypeInfo::toSerial(this, breadcrumb, entry + 1);
        return index;
    }
    else if (auto name = dynamicCast<const Name>(obj))
    {
        return addName(name);
    }
    else if (auto scope = dynamicCast<Scope>(obj))
    {
        // We don't serialize scope
        return ASTSerialIndex(0);
    }
    else if (auto module = dynamicCast<Module>(obj))
    {
        // We don't serialize Module
        return ASTSerialIndex(0);
    }

    SLANG_ASSERT(!"Unhandled type");
    return ASTSerialIndex(0);
}

ASTSerialIndex ASTSerialWriter::addString(const UnownedStringSlice& slice)
{
    typedef ByteEncodeUtil Util;
    typedef ASTSerialInfo::StringEntry StringEntry;

    if (slice.getLength() == 0)
    {
        return ASTSerialIndex(0);
    }

    Index newIndex = m_entries.getCount();

    Index* indexPtr = m_sliceMap.TryGetValueOrAdd(slice, newIndex);
    if (indexPtr)
    {
        return ASTSerialIndex(*indexPtr);
    }

    // Okay we need to add the string

    uint8_t encodeBuf[Util::kMaxLiteEncodeUInt32];
    const int encodeCount = Util::encodeLiteUInt32(uint32_t(slice.getLength()), encodeBuf);

    StringEntry* entry = (StringEntry*)m_arena.allocateUnaligned(SLANG_OFFSET_OF(StringEntry, sizeAndChars) + encodeCount + slice.getLength());
    entry->info = ASTSerialInfo::EntryInfo::Alignment1;
    entry->type = ASTSerialInfo::Type::String;

    uint8_t* dst = (uint8_t*)(entry->sizeAndChars);
    for (int i = 0; i < encodeCount; ++i)
    {
        dst[i] = encodeBuf[i];
    }

    memcpy(dst + encodeCount, slice.begin(), slice.getLength());

    m_entries.add(entry);
    return ASTSerialIndex(newIndex);
}


ASTSerialIndex ASTSerialWriter::addString(const String& in)
{
    return addPointer(in.getStringRepresentation());
}

ASTSerialIndex ASTSerialWriter::addName(const Name* name)
{
    if (name == nullptr)
    {
        return ASTSerialIndex(0);
    }

    // Look it up
    Index* indexPtr = m_ptrMap.TryGetValue(name);
    if (indexPtr)
    {
        return ASTSerialIndex(*indexPtr);
    }

    ASTSerialIndex index = addString(name->text);
    m_ptrMap.Add(name, Index(index));
    return index;
}

ASTSerialSourceLoc ASTSerialWriter::addSourceLoc(SourceLoc sourceLoc)
{
    SLANG_UNUSED(sourceLoc);
    return 0;
}

ASTSerialIndex ASTSerialWriter::_addArray(size_t elementSize, size_t alignment, const void* elements, Index elementCount)
{
    typedef ASTSerialInfo::ArrayEntry Entry;

    if (elementCount == 0)
    {
        return ASTSerialIndex(0);
    }

    SLANG_ASSERT(alignment >= 1 && alignment <= ASTSerialInfo::MAX_ALIGNMENT);

    // We must at a minimum have the alignment for the array prefix info
    alignment = (alignment < SLANG_ALIGN_OF(Entry)) ? SLANG_ALIGN_OF(Entry) : alignment;

    size_t payloadSize = elementCount * elementSize;

    Entry* entry = (Entry*)m_arena.allocateAligned(sizeof(Entry) + payloadSize, alignment);

    entry->type = ASTSerialInfo::Type::Array;
    entry->info = ASTSerialInfo::makeEntryInfo(int(alignment));
    entry->elementSize = uint16_t(elementSize);
    entry->elementCount = uint32_t(elementCount);

    memcpy(entry + 1, elements, payloadSize);

    m_entries.add(entry);
    return ASTSerialIndex(m_entries.getCount() - 1);
}

SlangResult ASTSerialWriter::write(Stream* stream)
{
    const Int entriesCount = m_entries.getCount();

    // Add a sentinal so we don't need special handling for 
    ASTSerialInfo::Entry sentinal;
    sentinal.type = ASTSerialInfo::Type::String;
    sentinal.info = ASTSerialInfo::EntryInfo::Alignment1;

    m_entries.add(&sentinal);
    m_entries.removeLast();

    ASTSerialInfo::Entry** entries = m_entries.getBuffer();
    // Note strictly required in our impl of List. But by writing this and
    // knowing that removeLast cannot release memory, means the sentinal must be at the last position.
    entries[entriesCount] = &sentinal;


    static const uint8_t fixBuffer[ASTSerialInfo::MAX_ALIGNMENT] { 0, };
    
    {
        size_t offset = 0;

        ASTSerialInfo::Entry* entry = entries[1];
        // We start on 1, because 0 is nullptr and not used for anything
        for (Index i = 1; i < entriesCount; ++i)
        {
            ASTSerialInfo::Entry* next = entries[i + 1];
            // Before writing we need to store the next alignment

            const size_t nextAlignment = ASTSerialInfo::getAlignment(next->info);
            const size_t alignment = ASTSerialInfo::getAlignment(entry->info);

            entry->info = ASTSerialInfo::combineWithNext(entry->info, next->info);

            // Check we are aligned correctly
            SLANG_ASSERT((offset & (alignment - 1)) == 0);

            // When we write, we need to make sure it take into account the next alignment
            const size_t entrySize = entry->calcSize(m_classes);

            // Work out the fix for next alignment
            size_t nextOffset = offset + entrySize;
            nextOffset = (nextOffset + nextAlignment - 1) & ~(nextAlignment - 1);

            size_t alignmentFixSize = nextOffset - (offset + entrySize);

            // The fix must be less than max alignment. We require it to be less because we aligned each Entry to 
            // MAX_ALIGNMENT, and so < MAX_ALIGNMENT is the most extra bytes we can write
            SLANG_ASSERT( alignmentFixSize < ASTSerialInfo::MAX_ALIGNMENT);
            
            try
            {
                stream->write(entry, entrySize);
                // If we needed to fix so that subsequent alignment is right, write out extra bytes here
                if (alignmentFixSize)
                {
                    stream->write(fixBuffer, alignmentFixSize);
                }
            }
            catch (const IOException&)
            {
                return SLANG_FAIL;
            }

            // Onto next
            offset = nextOffset;
            entry = next;
        }
    }

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ASTSerialInfo::Entry  !!!!!!!!!!!!!!!!!!!!!!!!

size_t ASTSerialInfo::Entry::calcSize(ASTSerialClasses* serialClasses) const
{
    switch (type)
    {
        case Type::String:
        {
            auto entry = static_cast<const StringEntry*>(this);
            const uint8_t* cur = (const uint8_t*)entry->sizeAndChars;
            uint32_t charsSize;
            int sizeSize = ByteEncodeUtil::decodeLiteUInt32(cur, &charsSize);
            return SLANG_OFFSET_OF(StringEntry, sizeAndChars) + sizeSize + charsSize;
        }
        case Type::Node:
        {
            auto entry = static_cast<const NodeEntry*>(this);
            auto serialClass = serialClasses->getSerialClass(ASTNodeType(entry->astNodeType));

            // Align by the alignment of the entry 
            size_t alignment = getAlignment(entry->info);
            size_t size = sizeof(NodeEntry) + serialClass->size;

            size = size + (alignment - 1) & ~(alignment - 1);
            return size;
        }
        case Type::RefObject:
        {
            auto entry = static_cast<const RefObjectEntry*>(this);

            size_t payloadSize;
            switch (entry->subType)
            {
                case RefObjectEntry::SubType::Breadcrumb:
                {
                    payloadSize = sizeof(ASTSerialTypeInfo<LookupResultItem::Breadcrumb>::SerialType);
                    break;
                }
                default:
                {
                    SLANG_ASSERT(!"Unknown type");
                    return 0;
                }
            }

            return sizeof(RefObjectEntry) + payloadSize;
        }
        case Type::Array:
        {
            auto entry = static_cast<const ArrayEntry*>(this);
            return sizeof(ArrayEntry) + entry->elementSize * entry->elementCount;
        }
        default: break;
    }

    SLANG_ASSERT(!"Unknown type");
    return 0;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ASTSerialReader  !!!!!!!!!!!!!!!!!!!!!!!!!!!!

const void* ASTSerialReader::getArray(ASTSerialIndex index, Index& outCount)
{
    if (index == ASTSerialIndex(0))
    {
        outCount = 0;
        return nullptr;
    }

    SLANG_ASSERT(ASTSerialIndexRaw(index) < ASTSerialIndexRaw(m_entries.getCount()));
    const Entry* entry = m_entries[Index(index)];

    switch (entry->type)
    {
        case Type::Array:
        {
            auto arrayEntry = static_cast<const ASTSerialInfo::ArrayEntry*>(entry);
            outCount = Index(arrayEntry->elementCount);
            return (arrayEntry + 1);
        }
        default: break;
    }

    SLANG_ASSERT(!"Not an array");
    outCount = 0;
    return nullptr;
}

ASTSerialPointer ASTSerialReader::getPointer(ASTSerialIndex index)
{
    if (index == ASTSerialIndex(0))
    {
        return ASTSerialPointer();
    }

    SLANG_ASSERT(ASTSerialIndexRaw(index) < ASTSerialIndexRaw(m_entries.getCount()));
    const Entry* entry = m_entries[Index(index)];

    switch (entry->type)
    {
        case Type::String:
        {
            // Hmm. Tricky -> we don't know if will be cast as Name or String. Lets assume string.
            String string = getString(index);
            return ASTSerialPointer(string.getStringRepresentation());
        }
        case Type::Node:
        {
            return ASTSerialPointer((NodeBase*)m_objects[Index(index)]);
        }
        case Type::RefObject:
        {
            return ASTSerialPointer((RefObject*)m_objects[Index(index)]);
        }
        default: break;
    }

    SLANG_ASSERT(!"Cannot access as a pointer");
    return ASTSerialPointer();
}

String ASTSerialReader::getString(ASTSerialIndex index)
{
    if (index == ASTSerialIndex(0))
    {
        return String();
    }

    SLANG_ASSERT(ASTSerialIndexRaw(index) < ASTSerialIndexRaw(m_entries.getCount()));
    const Entry* entry = m_entries[Index(index)];

    // It has to be a string type
    if (entry->type != Type::String)
    {
        SLANG_ASSERT(!"Not a string");
        return String();
    }

    RefObject* obj = (RefObject*)m_objects[Index(index)];

    if (obj)
    {
        StringRepresentation* stringRep = dynamicCast<StringRepresentation>(obj);
        if (stringRep)
        {
            return String(stringRep);
        }
        // Must be a name then
        Name* name = dynamicCast<Name>(obj);
        SLANG_ASSERT(name);
        return name->text;
    }

    // Okay we need to construct as a string
    UnownedStringSlice slice = getStringSlice(index);
    String string(slice);
    StringRepresentation* stringRep = string.getStringRepresentation();

    m_scope.add(stringRep);
    m_objects[Index(index)] = stringRep;
    return string;
}

Name* ASTSerialReader::getName(ASTSerialIndex index)
{
    if (index == ASTSerialIndex(0))
    {
        return nullptr;
    }

    SLANG_ASSERT(ASTSerialIndexRaw(index) < ASTSerialIndexRaw(m_entries.getCount()));
    const Entry* entry = m_entries[Index(index)];

    // It has to be a string type
    if (entry->type != Type::String)
    {
        SLANG_ASSERT(!"Not a string");
        return nullptr;
    }

    RefObject* obj = (RefObject*)m_objects[Index(index)];

    if (obj)
    {
        Name* name = dynamicCast<Name>(obj);
        if (name)
        {
            return name;
        }
        // Can only be a string then
        StringRepresentation* stringRep = dynamicCast<StringRepresentation>(obj);
        SLANG_ASSERT(stringRep);

        // I don't need to scope, as scoped in NamePool
        name  = m_namePool->getName(String(stringRep));

        // Store as name, as can always access the inner string if needed
        m_objects[Index(index)] = name;
        return name;
    }

    UnownedStringSlice slice = getStringSlice(index);
    String string(slice);
    Name* name = m_namePool->getName(string);
    // Don't need to add to scope, because scoped on the pool
    m_objects[Index(index)] = name;
    return name;
}

UnownedStringSlice ASTSerialReader::getStringSlice(ASTSerialIndex index)
{
    SLANG_ASSERT(ASTSerialIndexRaw(index) < ASTSerialIndexRaw(m_entries.getCount()));
    const Entry* entry = m_entries[Index(index)];

    // It has to be a string type
    if (entry->type != Type::String)
    {
        SLANG_ASSERT(!"Not a string");
        return UnownedStringSlice();
    }

    auto stringEntry = static_cast<const ASTSerialInfo::StringEntry*>(entry);

    const uint8_t* src = (const uint8_t*)stringEntry->sizeAndChars;

    // Decode the string
    uint32_t size;
    int sizeSize = ByteEncodeUtil::decodeLiteUInt32(src, &size);
    return UnownedStringSlice((const char*)src + sizeSize, size);
}

SourceLoc ASTSerialReader::getSourceLoc(ASTSerialSourceLoc loc)
{
    SLANG_UNUSED(loc);
    return SourceLoc();
}

SlangResult ASTSerialReader::loadEntries(const uint8_t* data, size_t dataCount, List<const ASTSerialInfo::Entry*>& outEntries)
{
    // Check the input data is at least aligned to the max alignment (otherwise everything cannot be aligned correctly)
    SLANG_ASSERT((size_t(data) & (ASTSerialInfo::MAX_ALIGNMENT - 1)) == 0);

    outEntries.setCount(1);
    outEntries[0] = nullptr;

    const uint8_t*const end = data + dataCount;

    const uint8_t* cur = data;
    while (cur < end)
    {
        const Entry* entry = (const Entry*)cur;
        outEntries.add(entry);

        const size_t entrySize = entry->calcSize(m_classes);
        cur += entrySize;

        // Need to get the next alignment
        const size_t nextAlignment = ASTSerialInfo::getNextAlignment(entry->info);

        // Need to fix cur with the alignment
        cur = (const uint8_t*)((size_t(cur) + nextAlignment - 1) & ~(nextAlignment - 1));
    }

    return SLANG_OK;
}

SlangResult ASTSerialReader::load(const uint8_t* data, size_t dataCount, ASTBuilder* builder, NamePool* namePool)
{
    SLANG_RETURN_ON_FAIL(loadEntries(data, dataCount, m_entries));

    m_namePool = namePool;

    m_objects.clearAndDeallocate();
    m_objects.setCount(m_entries.getCount());
    memset(m_objects.getBuffer(), 0, m_objects.getCount() * sizeof(void*));

    // Go through entries, constructing objects.
    for (Index i = 1; i < m_entries.getCount(); ++i)
    {
        const Entry* entry = m_entries[i];

        switch (entry->type)
        {
            case Type::String:
            {
                // Don't need to construct an object. This is probably a StringRepresentation, or a Name
                // Will evaluate lazily.
                break;
            }
            case Type::Node:
            {
                auto nodeEntry = static_cast<const ASTSerialInfo::NodeEntry*>(entry);
                m_objects[i] = builder->createByNodeType(ASTNodeType(nodeEntry->astNodeType));
                break;
            }
            case Type::RefObject:
            {
                auto objEntry = static_cast<const ASTSerialInfo::RefObjectEntry*>(entry);
                switch (objEntry->subType)
                {
                    case ASTSerialInfo::RefObjectEntry::SubType::Breadcrumb:
                    {
                        typedef LookupResultItem::Breadcrumb Breadcrumb;

                        auto breadcrumb = new LookupResultItem::Breadcrumb(Breadcrumb::Kind::Member, DeclRef<Decl>(), nullptr, nullptr);
                        m_scope.add(breadcrumb);
                        m_objects[i] = breadcrumb;
                        break;
                    }
                    default:
                    {
                        SLANG_ASSERT(!"Unknown type");
                        return SLANG_FAIL;
                    }
                }
                break;
            }
            case Type::Array:
            {
                // Don't need to construct an object, as will be accessed an interpreted by the object that holds it
                break;
            }
        }
    }

    // Deserialize
    for (Index i = 1; i < m_entries.getCount(); ++i)
    {
        const Entry* entry = m_entries[i];
        void* native = m_objects[i];
        if (!native)
        {
            continue;
        }
        switch (entry->type)
        {
            case Type::Node:
            {
                auto nodeEntry = static_cast<const ASTSerialInfo::NodeEntry*>(entry);
                auto serialClass = m_classes->getSerialClass(ASTNodeType(nodeEntry->astNodeType));

                const uint8_t* src = (const uint8_t*)(nodeEntry + 1);
                uint8_t* dst = (uint8_t*)m_objects[i];

                // It must be constructed
                SLANG_ASSERT(dst);

                while (serialClass)
                {
                    for (Index j = 0; j < serialClass->fieldsCount; ++j)
                    {
                        auto field = serialClass->fields[j];
                        auto fieldType = field.type;
                        fieldType->toNativeFunc(this, src + field.serialOffset, dst + field.nativeOffset);
                    }

                    auto cls = ReflectClassInfo::getInfo(serialClass->type);
                    auto superCls = cls->m_superClass;

                    // Get the super class
                    serialClass = superCls ? m_classes->getSerialClass(ASTNodeType(superCls->m_classId)) : nullptr;
                }

                break;
            }
            case Type::RefObject:
            {
                auto objEntry = static_cast<const ASTSerialInfo::RefObjectEntry*>(entry);
                switch (objEntry->subType)
                {
                    case ASTSerialInfo::RefObjectEntry::SubType::Breadcrumb:
                    {
                        typedef LookupResultItem::Breadcrumb Breadcrumb;
                        auto serialType = ASTSerialGetType<Breadcrumb>::getType();
                        serialType->toNativeFunc(this, (entry + 1), m_objects[i]);
                        break;
                    }
                    default:
                    {
                        SLANG_ASSERT(!"Unknown type");
                        return SLANG_FAIL;
                    }
                }
                break;
            }
            default: break;
        }
    }

    return SLANG_OK;
}


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ASTSerializeUtil  !!!!!!!!!!!!!!!!!!!!!!!!!!!!

/* static */SlangResult ASTSerialTestUtil::selfTest()
{
    RefPtr<ASTSerialClasses> classes = new ASTSerialClasses;

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

/* static */SlangResult ASTSerialTestUtil::testSerialize(NodeBase* node, RootNamePool* rootNamePool, SharedASTBuilder* sharedASTBuilder, SourceManager* sourceManager)
{
    RefPtr<ASTSerialClasses> classes = new ASTSerialClasses;

    List<uint8_t> contents;

    {
        OwnedMemoryStream stream(FileAccess::ReadWrite);

        ASTSerialWriter writer(classes);

        // Lets serialize it all
        writer.addPointer(node);
        // Let's stick it all in a stream
        writer.write(&stream);

        stream.swapContents(contents);

        NamePool namePool;
        namePool.setRootNamePool(rootNamePool);

        ASTSerialReader reader(classes);

        ASTBuilder builder(sharedASTBuilder, "Serialize Check");

        // We could now check that the loaded data matches

        {
            const List<ASTSerialInfo::Entry*>& writtenEntries = writer.getEntries();
            List<const ASTSerialInfo::Entry*> readEntries;

            SlangResult res = reader.loadEntries(contents.getBuffer(), contents.getCount(), readEntries);
            SLANG_UNUSED(res);

            SLANG_ASSERT(writtenEntries.getCount() == readEntries.getCount());

            // They should be identical up to the
            for (Index i = 1; i < readEntries.getCount(); ++i)
            {
                auto writtenEntry = writtenEntries[i];
                auto readEntry = readEntries[i];

                const size_t writtenSize = writtenEntry->calcSize(classes);
                const size_t readSize = readEntry->calcSize(classes);

                SLANG_ASSERT(readSize == writtenSize);
                // Check the payload is the same
                SLANG_ASSERT(memcmp(readEntry, writtenEntry, readSize) == 0);
            }

        }

        {
            SlangResult res = reader.load(contents.getBuffer(), contents.getCount(), &builder, &namePool);
            SLANG_UNUSED(res);
        }

        // Lets see what we have
        const ASTDumpUtil::Flags dumpFlags = ASTDumpUtil::Flag::HideSourceLoc | ASTDumpUtil::Flag::HideScope;

        String readDump;
        {
            SourceWriter sourceWriter(sourceManager, LineDirectiveMode::None);
            ASTDumpUtil::dump(reader.getPointer(ASTSerialIndex(1)).dynamicCast<NodeBase>(), ASTDumpUtil::Style::Hierachical, dumpFlags, &sourceWriter);
            readDump = sourceWriter.getContentAndClear();

        }
        String origDump;
        {
            SourceWriter sourceWriter(sourceManager, LineDirectiveMode::None);
            ASTDumpUtil::dump(node, ASTDumpUtil::Style::Hierachical, dumpFlags, &sourceWriter);
            origDump = sourceWriter.getContentAndClear();
        }

        // Write out
        File::writeAllText("ast-read.ast-dump", readDump);
        File::writeAllText("ast-orig.ast-dump", origDump);

        if (readDump != origDump)
        {
            return SLANG_FAIL;
        }
    }

    return SLANG_OK;
}


} // namespace Slang
