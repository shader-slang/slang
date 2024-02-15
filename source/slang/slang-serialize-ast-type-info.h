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

// MatrixCoord can just go as is
template <>
struct SerialTypeInfo<MatrixCoord> : SerialIdentityTypeInfo<MatrixCoord> {};

inline void serializeValPointerValue(SerialWriter* writer, Val* ptrValue, SerialIndex* outSerial)
{
    if (ptrValue)
        ptrValue = ptrValue->resolve();
    *(SerialIndex*)outSerial = writer->addPointer(ptrValue);
}

inline void deserializeValPointerValue(SerialReader* reader, const SerialIndex* inSerial, void* outPtr)
{
    auto val = reader->getValPointer(*(const SerialIndex*)inSerial);
    *(void**)outPtr = val.m_ptr;
}

template<typename T>
struct PtrSerialTypeInfo<T, std::enable_if_t<std::is_base_of_v<Val, T>>>
{
    typedef T* NativeType;
    typedef SerialIndex SerialType;
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* inNative, void* outSerial)
    {
        auto ptrValue = *(T**)inNative;
        serializeValPointerValue(writer, ptrValue, (SerialIndex*)outSerial);
    }

    static void toNative(SerialReader* reader, const void* inSerial, void* outNative)
    {
        deserializeValPointerValue(reader, (SerialIndex*)inSerial, outNative);
    }
};

template <typename T>
struct SerialTypeInfo<DeclRef<T>> : public SerialTypeInfo<DeclRefBase*> {};

template<>
struct SerialTypeInfo<CapabilitySet>
{
    typedef CapabilitySet NativeType;
    typedef SerialIndex SerialType;
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };
    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        dst = writer->addArray(src.getExpandedAtoms().getBuffer(), src.getExpandedAtoms().getCount());
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& dst = *(NativeType*)native;
        auto& src = *(const SerialType*)serial;

        reader->getArray(src, dst.getExpandedAtoms());
    }
};

template<>
struct SerialTypeInfo<CapabilityConjunctionSet>
{
    typedef CapabilityConjunctionSet NativeType;
    typedef SerialIndex SerialType;
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };
    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        dst = writer->addArray(src.getExpandedAtoms().getBuffer(), src.getExpandedAtoms().getCount());
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& dst = *(NativeType*)native;
        auto& src = *(const SerialType*)serial;

        reader->getArray(src, dst.getExpandedAtoms());
    }
};

// ValNodeOperand
template <>
struct SerialTypeInfo<ValNodeOperand>
{
    typedef ValNodeOperand NativeType;
    struct SerialType
    {
        int8_t kind;
        int64_t val;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;
        dst.kind = int8_t(src.kind);
        if (src.kind == ValNodeOperandKind::ConstantValue)
            dst.val = src.values.intOperand;
        else if (src.kind == ValNodeOperandKind::ValNode)
            serializeValPointerValue(writer, (Val*)src.values.nodeOperand, (SerialIndex*)&dst.val);
        else
            SerialTypeInfo<NodeBase*>::toSerial(writer, &src.values.nodeOperand, (SerialIndex*)&dst.val);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& dst = *(NativeType*)native;
        auto& src = *(const SerialType*)serial;

        // Initialize
        dst = NativeType();
        dst.kind = ValNodeOperandKind(src.kind);
        if (dst.kind == ValNodeOperandKind::ConstantValue)
            dst.values.intOperand = int64_t(src.val);
        else if (dst.kind == ValNodeOperandKind::ValNode)
            deserializeValPointerValue(reader, (SerialIndex*)&src.val, (Val**)&dst.values.nodeOperand);
        else
            SerialTypeInfo<NodeBase*>::toNative(reader, (SerialIndex*)&src.val, (NodeBase**)&dst.values.nodeOperand);
    }
};

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

// SPIRVAsm
SLANG_VALUE_TYPE_INFO(SPIRVAsmOperand)
SLANG_VALUE_TYPE_INFO(SPIRVAsmInst)

} // namespace Slang

#endif
