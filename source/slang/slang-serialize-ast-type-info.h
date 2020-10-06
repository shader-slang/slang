// slang-serialize-ast-type-info.h
#ifndef SLANG_SERIALIZE_AST_TYPE_INFO_H
#define SLANG_SERIALIZE_AST_TYPE_INFO_H

#include "slang-ast-support-types.h"
#include "slang-ast-all.h"

#include "slang-serialize-type-info.h"
#include "slang-serialize-misc-type-info.h"

namespace Slang {


/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! AST types !!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

// SyntaxClass<T>
template <typename T>
struct SerialTypeInfo<SyntaxClass<T>>
{
    typedef SyntaxClass<T> NativeType;
    typedef uint16_t SerialType;

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        SLANG_UNUSED(writer);
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;
        dst = SerialType(src.classInfo->m_classId);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        SLANG_UNUSED(reader);
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;
        dst.classInfo = ASTClassInfo::getInfo(ASTNodeType(src));
    }
};

struct SerialDeclRefBaseTypeInfo
{
    typedef DeclRefBase NativeType;
    struct SerialType
    {
        SerialIndex substitutions;
        SerialIndex decl;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* inNative, void* outSerial)
    {
        SerialType& serial = *(SerialType*)outSerial;
        const NativeType& native = *(const NativeType*)inNative;

        serial.decl = writer->addPointer(native.decl);
        serial.substitutions = writer->addPointer(native.substitutions.substitutions);
    }
    static void toNative(SerialReader* reader, const void* inSerial, void* outNative)
    {
        DeclRefBase& native = *(DeclRefBase*)(outNative);
        const SerialType& serial = *(const SerialType*)inSerial;

        native.decl = reader->getPointer(serial.decl).dynamicCast<Decl>();
        native.substitutions.substitutions = reader->getPointer(serial.substitutions).dynamicCast<Substitutions>();
    }
    static const SerialFieldType* getFieldType()
    {
        static const SerialFieldType type = { sizeof(SerialType), uint8_t(SerialAlignment), &toSerial, &toNative };
        return &type;
    }
};
// Special case DeclRef, because it always uses the same type
template <typename T>
struct SerialGetFieldType<DeclRef<T>>
{
    static const SerialFieldType* getFieldType() { return SerialDeclRefBaseTypeInfo::getFieldType(); }
};


template <typename T>
struct SerialTypeInfo<DeclRef<T>> : public SerialDeclRefBaseTypeInfo {};

// MatrixCoord can just go as is
template <>
struct SerialTypeInfo<MatrixCoord> : SerialIdentityTypeInfo<MatrixCoord> {};


// QualType

template <>
struct SerialTypeInfo<QualType>
{
    typedef QualType NativeType;
    struct SerialType
    {
        SerialIndex type;
        uint8_t isLeftValue;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialIndex) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto dst = (SerialType*)serial;
        auto src = (const NativeType*)native;
        dst->isLeftValue = src->isLeftValue ? 1 : 0;
        dst->type = writer->addPointer(src->type);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto src = (const SerialType*)serial;
        auto dst = (NativeType*)native;
        dst->type = reader->getPointer(src->type).dynamicCast<Type>();
        dst->isLeftValue = src->isLeftValue != 0;
    }
};


// LookupResult::Breadcrumb
template <>
struct SerialTypeInfo<LookupResultItem::Breadcrumb>
{
    typedef LookupResultItem::Breadcrumb NativeType;
    struct SerialType
    {
        NativeType::Kind kind;
        NativeType::ThisParameterMode thisParameterMode;
        SerialTypeInfo<DeclRef<Decl>>::SerialType declRef;
        SerialTypeInfo<RefPtr<NativeType>> next;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        dst.kind = src.kind;
        dst.thisParameterMode = src.thisParameterMode;
        toSerialValue(writer, src.declRef, dst.declRef);
        toSerialValue(writer, src.next, dst.next);
    }

    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& dst = *(NativeType*)native;
        auto& src = *(const SerialType*)serial;

        dst.kind = src.kind;
        dst.thisParameterMode = src.thisParameterMode;
        toNativeValue(reader, src.declRef, dst.declRef);
        toNativeValue(reader, src.next, dst.next);
    }
};

// LookupResultItem
template <>
struct SerialTypeInfo<LookupResultItem>
{
    typedef LookupResultItem NativeType;
    struct SerialType
    {
        SerialTypeInfo<DeclRef<Decl>>::SerialType declRef;
        SerialTypeInfo<RefPtr<NativeType::Breadcrumb>> breadcrumbs;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        toSerialValue(writer, src.declRef, dst.declRef);
        toSerialValue(writer, src.breadcrumbs, dst.breadcrumbs);
    }

    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& dst = *(NativeType*)native;
        auto& src = *(const SerialType*)serial;

        toNativeValue(reader, src.declRef, dst.declRef);
        toNativeValue(reader, src.breadcrumbs, dst.breadcrumbs);
    }
};

// LookupResult
template <>
struct SerialTypeInfo<LookupResult>
{
    typedef LookupResult NativeType;
    typedef SerialIndex SerialType;
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
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
            dst = SerialIndex(0);
        }
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
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
struct SerialTypeInfo<GlobalGenericParamSubstitution::ConstraintArg>
{
    typedef GlobalGenericParamSubstitution::ConstraintArg NativeType;
    struct SerialType
    {
        SerialIndex decl;
        SerialIndex val;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialIndex) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& dst = *(SerialType*)serial;
        auto& src = *(const NativeType*)native;

        dst.decl = writer->addPointer(src.decl);
        dst.val = writer->addPointer(src.val);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        dst.decl = reader->getPointer(src.decl).dynamicCast<Decl>();
        dst.val = reader->getPointer(src.val).dynamicCast<Val>();
    }
};

// ExpandedSpecializationArg
template <>
struct SerialTypeInfo<ExpandedSpecializationArg>
{
    typedef ExpandedSpecializationArg NativeType;
    struct SerialType
    {
        SerialIndex val;
        SerialIndex witness;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialIndex) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& dst = *(SerialType*)serial;
        auto& src = *(const NativeType*)native;

        dst.witness = writer->addPointer(src.witness);
        dst.val = writer->addPointer(src.val);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        dst.witness = reader->getPointer(src.witness).dynamicCast<Val>();
        dst.val = reader->getPointer(src.val).dynamicCast<Val>();
    }
};

// TypeExp
template <>
struct SerialTypeInfo<TypeExp>
{
    typedef TypeExp NativeType;
    struct SerialType
    {
        SerialIndex type;
        SerialIndex expr;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialIndex) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& dst = *(SerialType*)serial;
        auto& src = *(const NativeType*)native;

        dst.type = writer->addPointer(src.type);
        dst.expr = writer->addPointer(src.exp);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        dst.type = reader->getPointer(src.type).dynamicCast<Type>();
        dst.exp = reader->getPointer(src.expr).dynamicCast<Expr>();
    }
};

// DeclCheckStateExt
template <>
struct SerialTypeInfo<DeclCheckStateExt>
{
    typedef DeclCheckStateExt NativeType;
    typedef DeclCheckStateExt::RawType SerialType;

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        SLANG_UNUSED(writer);
        *(SerialType*)serial = (*(const NativeType*)native).getRaw();
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        SLANG_UNUSED(reader);
        (*(NativeType*)serial).setRaw(*(const SerialType*)native);
    }
};

// Modifiers
template <>
struct SerialTypeInfo<Modifiers>
{
    typedef Modifiers NativeType;
    typedef SerialIndex SerialType;

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        // We need to make into an array
        List<SerialIndex> modifierIndices;
        for (Modifier* modifier : *(NativeType*)native)
        {
            modifierIndices.add(writer->addPointer(modifier));
        }
        *(SerialType*)serial = writer->addArray(modifierIndices.getBuffer(), modifierIndices.getCount());
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
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

// ASTNodeType
template <>
struct SerialTypeInfo<ASTNodeType> : public SerialConvertTypeInfo<ASTNodeType, uint16_t> {};

// LookupResultItem_Breadcrumb::ThisParameterMode
template <>
struct SerialTypeInfo<LookupResultItem_Breadcrumb::ThisParameterMode> : public SerialConvertTypeInfo<LookupResultItem_Breadcrumb::ThisParameterMode, uint8_t> {};

// LookupResultItem_Breadcrumb::Kind
template <>
struct SerialTypeInfo<LookupResultItem_Breadcrumb::Kind> : public SerialConvertTypeInfo<LookupResultItem_Breadcrumb::Kind, uint8_t> {};

} // namespace Slang

#endif
