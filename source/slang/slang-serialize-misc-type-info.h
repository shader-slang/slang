// slang-serialize-misc-type-info.h
#ifndef SLANG_SERIALIZE_MISC_TYPE_INFO_H
#define SLANG_SERIALIZE_MISC_TYPE_INFO_H

#include "slang-serialize-type-info.h"

#include "../compiler-core/slang-source-loc.h"
#include "slang-compiler.h"

namespace Slang {

/* Conversion for serialization for some more misc Slang types
*/


// Because is sized, we don't need to convert
template <>
struct SerialTypeInfo<FeedbackType::Kind> : public SerialIdentityTypeInfo<FeedbackType::Kind> {};

// SamplerStateFlavor

template <>
struct SerialTypeInfo<SamplerStateFlavor> : public SerialConvertTypeInfo<SamplerStateFlavor, uint8_t> {};

// ImageFormat
template <>
struct SerialTypeInfo<ImageFormat> : public SerialConvertTypeInfo<ImageFormat, uint8_t> {};

// Stage
template <>
struct SerialTypeInfo<Stage> : public SerialConvertTypeInfo<Stage, uint8_t> {};

// TokenType
template <>
struct SerialTypeInfo<TokenType> : public SerialConvertTypeInfo<TokenType, uint8_t> {};

// BaseType
template <>
struct SerialTypeInfo<BaseType> : public SerialConvertTypeInfo<BaseType, uint8_t> {};

// SemanticVersion
template <>
struct SerialTypeInfo<SemanticVersion> : public SerialIdentityTypeInfo<SemanticVersion> {};

// SourceLoc

// Make the type exposed, so we can look for it if we want to remap.
template <>
struct SerialTypeInfo<SourceLoc>
{
    typedef SourceLoc NativeType;
    typedef SerialSourceLoc SerialType;
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialSourceLoc) };

    static void toSerial(SerialWriter* writer, const void* inNative, void* outSerial)
    {
        SerialSourceLocWriter* sourceLocWriter = writer->getExtraObjects().get<SerialSourceLocWriter>();
        *(SerialType*)outSerial = sourceLocWriter ? sourceLocWriter->addSourceLoc(*(const NativeType*)inNative) : SerialType(0);
    }
    static void toNative(SerialReader* reader, const void* inSerial, void* outNative)
    {
        SerialSourceLocReader* sourceLocReader = reader->getExtraObjects().get<SerialSourceLocReader>();
        *(NativeType*)outNative = sourceLocReader ? sourceLocReader->getSourceLoc(*(const SerialType*)inSerial) : NativeType::fromRaw(0);
    }
};

// Token
template <>
struct SerialTypeInfo<Token>
{
    typedef Token NativeType;
    struct SerialType
    {
        SerialTypeInfo<BaseType>::SerialType type;
        SerialTypeInfo<SourceLoc>::SerialType loc;
        SerialIndex name;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        SerialTypeInfo<TokenType>::toSerial(writer, &src.type, &dst.type);
        SerialTypeInfo<SourceLoc>::toSerial(writer, &src.loc, &dst.loc);

        if (src.flags & TokenFlag::Name)
        {
            dst.name = writer->addName(src.getName());
        }
        else
        {
            dst.name = writer->addString(src.getContent());
        }
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        dst.flags = 0;
        dst.charsNameUnion.chars = nullptr;

        SerialTypeInfo<TokenType>::toNative(reader, &src.type, &dst.type);
        SerialTypeInfo<SourceLoc>::toNative(reader, &src.loc, &dst.loc);

        // At the other end all token content will appear as Names.
        if (src.name != SerialIndex(0))
        {
            dst.charsNameUnion.name = reader->getName(src.name);
            dst.flags |= TokenFlag::Name;
        }
    }
};

// NameLoc
template <>
struct SerialTypeInfo<NameLoc>
{
    typedef NameLoc NativeType;
    struct SerialType
    {
        SerialTypeInfo<SourceLoc>::SerialType loc;
        SerialIndex name;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        dst.name = writer->addName(src.name);
        SerialTypeInfo<SourceLoc>::toSerial(writer, &src.loc, &dst.loc);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        dst.name = reader->getName(src.name);
        SerialTypeInfo<SourceLoc>::toNative(reader, &src.loc, &dst.loc);
    }
};

// DiagnosticInfo
template <>
struct SerialTypeInfo<const DiagnosticInfo*>
{
    typedef const DiagnosticInfo* NativeType;
    typedef SerialIndex SerialType;

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;
        dst = src ? writer->addString(UnownedStringSlice(src->name)) : SerialIndex(0);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        if (src == SerialIndex(0))
        {
            dst = nullptr;
        }
        else
        {
            dst = findDiagnosticByName(reader->getStringSlice(src));
        }
    }
};

// DeclAssociation
template <>
struct SerialTypeInfo<DeclAssociation> : SerialIdentityTypeInfo<DeclAssociation> {};
template <>
struct SerialTypeInfo<DeclAssociationKind> : public SerialConvertTypeInfo<DeclAssociationKind, uint8_t> {};

} // namespace Slang

#endif
