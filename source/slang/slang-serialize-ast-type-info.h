// slang-serialize-ast-type-info.h
#ifndef SLANG_SERIALIZE_AST_TYPE_INFO_H
#define SLANG_SERIALIZE_AST_TYPE_INFO_H

#include "slang-ast-support-types.h"
#include "slang-ast-all.h"

#include "slang-serialize-type-info.h"
#include "slang-serialize-misc-type-info.h"

#include "slang-serialize-value-type-info.h"

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

// All the templates for DeclRef<T> can use this implementation.
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

// LookupResultItem
SLANG_VALUE_TYPE_INFO(LookupResultItem)
// QualType
SLANG_VALUE_TYPE_INFO(QualType)


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
SLANG_VALUE_TYPE_INFO(GlobalGenericParamSubstitution_ConstraintArg)

// SpecializationArg
SLANG_VALUE_TYPE_INFO(SpecializationArg)
// ExpandedSpecializationArg
SLANG_VALUE_TYPE_INFO(ExpandedSpecializationArg)
// TypeExp
SLANG_VALUE_TYPE_INFO(TypeExp)
// DeclCheckStateExt
SLANG_VALUE_TYPE_INFO(DeclCheckStateExt)

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

// RequirementWitness::Flavor
template <>
struct SerialTypeInfo<RequirementWitness::Flavor> : public SerialConvertTypeInfo<RequirementWitness::Flavor, uint8_t> {};

// RequirementWitness
SLANG_VALUE_TYPE_INFO(RequirementWitness)

} // namespace Slang

#endif
