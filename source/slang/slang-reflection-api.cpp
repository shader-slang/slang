// slang-reflection-api.cpp

#include "../core/slang-basic.h"
#include "slang-syntax.h"

#include "../../slang.h"

#include "slang-compiler.h"
#include "slang-type-layout.h"
#include "slang-syntax.h"
#include <assert.h>

// Don't signal errors for stuff we don't implement here,
// and instead just try to return things defensively
//
// Slang developers can switch this when debugging.
#define SLANG_REFLECTION_UNEXPECTED() do {} while(0)

namespace Slang
{

// Conversion routines to help with strongly-typed reflection API

static inline UserDefinedAttribute* convert(SlangReflectionUserAttribute* attrib)
{
    return (UserDefinedAttribute*)attrib;
}
static inline SlangReflectionUserAttribute* convert(UserDefinedAttribute* attrib)
{
    return (SlangReflectionUserAttribute*)attrib;
}

static inline Type* convert(SlangReflectionType* type)
{
    return (Type*) type;
}

static inline SlangReflectionType* convert(Type* type)
{
    return (SlangReflectionType*) type;
}

static inline TypeLayout* convert(SlangReflectionTypeLayout* type)
{
    return (TypeLayout*) type;
}

static inline SlangReflectionTypeLayout* convert(TypeLayout* type)
{
    return (SlangReflectionTypeLayout*) type;
}

static inline SpecializationParamLayout* convert(SlangReflectionTypeParameter * typeParam)
{
    return (SpecializationParamLayout*) typeParam;
}

static inline VarDeclBase* convert(SlangReflectionVariable* var)
{
    return (VarDeclBase*) var;
}

static inline SlangReflectionVariable* convert(VarDeclBase* var)
{
    return (SlangReflectionVariable*) var;
}

static inline VarLayout* convert(SlangReflectionVariableLayout* var)
{
    return (VarLayout*) var;
}

static inline SlangReflectionVariableLayout* convert(VarLayout* var)
{
    return (SlangReflectionVariableLayout*) var;
}

static inline EntryPointLayout* convert(SlangReflectionEntryPoint* entryPoint)
{
    return (EntryPointLayout*) entryPoint;
}

static inline SlangReflectionEntryPoint* convert(EntryPointLayout* entryPoint)
{
    return (SlangReflectionEntryPoint*) entryPoint;
}

static inline ProgramLayout* convert(SlangReflection* program)
{
    return (ProgramLayout*) program;
}

static inline SlangReflection* convert(ProgramLayout* program)
{
    return (SlangReflection*) program;
}

// user attribute

static unsigned int getUserAttributeCount(Decl* decl)
{
    unsigned int count = 0;
    for (auto x : decl->getModifiersOfType<UserDefinedAttribute>())
    {
        SLANG_UNUSED(x);
        count++;
    }
    return count;
}

static SlangReflectionUserAttribute* findUserAttributeByName(Session* session, Decl* decl, const char* name)
{
    auto nameObj = session->tryGetNameObj(name);
    for (auto x : decl->getModifiersOfType<UserDefinedAttribute>())
    {
        if (x->keywordName == nameObj)
            return (SlangReflectionUserAttribute*)(x);
    }
    return nullptr;
}

static SlangReflectionUserAttribute* getUserAttributeByIndex(Decl* decl, unsigned int index)
{
    unsigned int id = 0;
    for (auto x : decl->getModifiersOfType<UserDefinedAttribute>())
    {
        if (id == index)
            return convert(x);
        id++;
    }
    return nullptr;
}


// Attempt "do what I mean" remapping from the parameter category the user asked about,
// over to a parameter category that they might have meant.
static SlangParameterCategory maybeRemapParameterCategory(
    TypeLayout*             typeLayout,
    SlangParameterCategory  category)
{
    // Do we have an entry for the category they asked about? Then use that.
    if (typeLayout->FindResourceInfo(LayoutResourceKind(category)))
        return category;

    // Do we have an entry for the `DescriptorTableSlot` category?
    if (typeLayout->FindResourceInfo(LayoutResourceKind::DescriptorTableSlot))
    {
        // Is the category they were asking about one that makes sense for the type
        // of this variable?
        Type* type = typeLayout->getType();
        while (auto arrayType = as<ArrayExpressionType>(type))
            type = arrayType->baseType;
        switch (spReflectionType_GetKind(convert(type)))
        {
            case SLANG_TYPE_KIND_CONSTANT_BUFFER:
                if (category == SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER)
                    return SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT;
                break;

            case SLANG_TYPE_KIND_RESOURCE:
                if (category == SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE)
                    return SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT;
                break;

            case SLANG_TYPE_KIND_SAMPLER_STATE:
                if (category == SLANG_PARAMETER_CATEGORY_SAMPLER_STATE)
                    return SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT;
                break;

                // TODO: implement more helpers here

            default:
                break;
        }
    }

    return category;
}

// Helpers for getting parameter count

static unsigned getParameterCount(RefPtr<TypeLayout> typeLayout)
{
    if (auto parameterGroupLayout = as<ParameterGroupTypeLayout>(typeLayout))
    {
        typeLayout = parameterGroupLayout->offsetElementTypeLayout;
    }

    if (auto structLayout = as<StructTypeLayout>(typeLayout))
    {
        return (unsigned)structLayout->fields.getCount();
    }

    return 0;
}

static VarLayout* getParameterByIndex(RefPtr<TypeLayout> typeLayout, unsigned index)
{
    if (auto parameterGroupLayout = as<ParameterGroupTypeLayout>(typeLayout))
    {
        typeLayout = parameterGroupLayout->offsetElementTypeLayout;
    }

    if (auto structLayout = as<StructTypeLayout>(typeLayout))
    {
        return structLayout->fields[index];
    }

    return 0;
}

static SlangParameterCategory getParameterCategory(
    LayoutResourceKind kind)
{
    return SlangParameterCategory(kind);
}

static SlangParameterCategory getParameterCategory(
    TypeLayout*  typeLayout)
{
    auto resourceInfoCount = typeLayout->resourceInfos.getCount();
    if (resourceInfoCount == 1)
    {
        return getParameterCategory(typeLayout->resourceInfos[0].kind);
    }
    else if (resourceInfoCount == 0)
    {
        // TODO: can this ever happen?
        return SLANG_PARAMETER_CATEGORY_NONE;
    }
    return SLANG_PARAMETER_CATEGORY_MIXED;
}

static bool hasDefaultConstantBuffer(ScopeLayout* layout)
{
    auto typeLayout = layout->parametersLayout->getTypeLayout();
    return as<ParameterGroupTypeLayout>(typeLayout) != nullptr;
}


} // namespace Slang

using namespace Slang;

// Implementation to back public-facing reflection API

SLANG_API char const* spReflectionUserAttribute_GetName(SlangReflectionUserAttribute* attrib)
{
    auto userAttr = convert(attrib);
    if (!userAttr) return nullptr;
    return userAttr->getKeywordName()->text.getBuffer();
}
SLANG_API unsigned int spReflectionUserAttribute_GetArgumentCount(SlangReflectionUserAttribute* attrib)
{
    auto userAttr = convert(attrib);
    if (!userAttr) return 0;
    return (unsigned int)userAttr->args.getCount();
}
SlangReflectionType* spReflectionUserAttribute_GetArgumentType(SlangReflectionUserAttribute* attrib, unsigned int index)
{
    auto userAttr = convert(attrib);
    if (!userAttr) return nullptr;
    return convert(userAttr->args[index]->type.type);
}
SLANG_API SlangResult spReflectionUserAttribute_GetArgumentValueInt(SlangReflectionUserAttribute* attrib, unsigned int index, int * rs)
{
    auto userAttr = convert(attrib);
    if (!userAttr) return SLANG_ERROR_INVALID_PARAMETER;
    if (index >= (unsigned int)userAttr->args.getCount()) return SLANG_ERROR_INVALID_PARAMETER;
    NodeBase* val = nullptr;
    if (userAttr->intArgVals.TryGetValue(index, val))
    {
        *rs = (int)as<ConstantIntVal>(val)->value;
        return 0;
    }
    return SLANG_ERROR_INVALID_PARAMETER;
}
SLANG_API SlangResult spReflectionUserAttribute_GetArgumentValueFloat(SlangReflectionUserAttribute* attrib, unsigned int index, float * rs)
{
    auto userAttr = convert(attrib);
    if (!userAttr) return SLANG_ERROR_INVALID_PARAMETER;
    if (index >= (unsigned int)userAttr->args.getCount()) return SLANG_ERROR_INVALID_PARAMETER;
    if (auto cexpr = as<FloatingPointLiteralExpr>(userAttr->args[index]))
    {
        *rs = (float)cexpr->value;
        return 0;
    }
    return SLANG_ERROR_INVALID_PARAMETER;
}
SLANG_API const char* spReflectionUserAttribute_GetArgumentValueString(SlangReflectionUserAttribute* attrib, unsigned int index, size_t* bufLen)
{
    auto userAttr = convert(attrib);
    if (!userAttr) return nullptr;
    if (index >= (unsigned int)userAttr->args.getCount()) return nullptr;
    if (auto cexpr = as<StringLiteralExpr>(userAttr->args[index]))
    {
        if (bufLen)
            *bufLen = cexpr->token.getContentLength();
        return cexpr->token.getContent().begin();
    }
    return nullptr;
}

// type Reflection

SLANG_API SlangTypeKind spReflectionType_GetKind(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return SLANG_TYPE_KIND_NONE;

    // TODO(tfoley: Don't emit the same type more than once...

    if (auto basicType = as<BasicExpressionType>(type))
    {
        return SLANG_TYPE_KIND_SCALAR;
    }
    else if (auto vectorType = as<VectorExpressionType>(type))
    {
        return SLANG_TYPE_KIND_VECTOR;
    }
    else if (auto matrixType = as<MatrixExpressionType>(type))
    {
        return SLANG_TYPE_KIND_MATRIX;
    }
    else if (auto parameterBlockType = as<ParameterBlockType>(type))
    {
        return SLANG_TYPE_KIND_PARAMETER_BLOCK;
    }
    else if (auto constantBufferType = as<ConstantBufferType>(type))
    {
        return SLANG_TYPE_KIND_CONSTANT_BUFFER;
    }
    else if( auto streamOutputType = as<HLSLStreamOutputType>(type) )
    {
        return SLANG_TYPE_KIND_OUTPUT_STREAM;
    }
    else if (as<TextureBufferType>(type))
    {
        return SLANG_TYPE_KIND_TEXTURE_BUFFER;
    }
    else if (as<GLSLShaderStorageBufferType>(type))
    {
        return SLANG_TYPE_KIND_SHADER_STORAGE_BUFFER;
    }
    else if (auto samplerStateType = as<SamplerStateType>(type))
    {
        return SLANG_TYPE_KIND_SAMPLER_STATE;
    }
    else if (auto textureType = as<TextureTypeBase>(type))
    {
        return SLANG_TYPE_KIND_RESOURCE;
    }
    else if (auto feedbackType = as<FeedbackType>(type))
    {
        return SLANG_TYPE_KIND_FEEDBACK;
    }
    // TODO: need a better way to handle this stuff...
#define CASE(TYPE)                          \
    else if(as<TYPE>(type)) do {          \
        return SLANG_TYPE_KIND_RESOURCE;    \
    } while(0)

    CASE(HLSLStructuredBufferType);
    CASE(HLSLRWStructuredBufferType);
    CASE(HLSLRasterizerOrderedStructuredBufferType);
    CASE(HLSLAppendStructuredBufferType);
    CASE(HLSLConsumeStructuredBufferType);
    CASE(HLSLByteAddressBufferType);
    CASE(HLSLRWByteAddressBufferType);
    CASE(HLSLRasterizerOrderedByteAddressBufferType);
    CASE(UntypedBufferResourceType);
#undef CASE

    else if (auto arrayType = as<ArrayExpressionType>(type))
    {
        return SLANG_TYPE_KIND_ARRAY;
    }
    else if( auto declRefType = as<DeclRefType>(type) )
    {
        const auto& declRef = declRefType->declRef;
        if(declRef.is<StructDecl>() )
        {
            return SLANG_TYPE_KIND_STRUCT;
        }
        else if (declRef.is<GlobalGenericParamDecl>())
        {
            return SLANG_TYPE_KIND_GENERIC_TYPE_PARAMETER;
        }
        else if (declRef.is<InterfaceDecl>())
        {
            return SLANG_TYPE_KIND_INTERFACE;
        }
        else if (declRef.is<FuncDecl>())
        {
            // This is a reference to an entry point
            return SLANG_TYPE_KIND_STRUCT;
        }
    }
    else if( auto specializedType = as<ExistentialSpecializedType>(type) )
    {
        return SLANG_TYPE_KIND_SPECIALIZED;
    }
    else if (auto errorType = as<ErrorType>(type))
    {
        // This means we saw a type we didn't understand in the user's code
        return SLANG_TYPE_KIND_NONE;
    }

    SLANG_REFLECTION_UNEXPECTED();
    return SLANG_TYPE_KIND_NONE;
}

SLANG_API unsigned int spReflectionType_GetFieldCount(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return 0;

    // TODO: maybe filter based on kind

    if(auto declRefType = as<DeclRefType>(type))
    {
        auto declRef = declRefType->declRef;
        if( auto structDeclRef = declRef.as<StructDecl>())
        {
            return (unsigned int)getFields(structDeclRef, MemberFilterStyle::Instance).getCount();
        }
    }

    return 0;
}

SLANG_API SlangReflectionVariable* spReflectionType_GetFieldByIndex(SlangReflectionType* inType, unsigned index)
{
    auto type = convert(inType);
    if(!type) return nullptr;

    // TODO: maybe filter based on kind

    if(auto declRefType = as<DeclRefType>(type))
    {
        auto declRef = declRefType->declRef;
        if( auto structDeclRef = declRef.as<StructDecl>())
        {
            auto fields = getFields(structDeclRef, MemberFilterStyle::Instance);
            auto fieldDeclRef = fields[index];
            return (SlangReflectionVariable*) fieldDeclRef.getDecl();
        }
    }

    return nullptr;
}

SLANG_API size_t spReflectionType_GetElementCount(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return 0;

    if(auto arrayType = as<ArrayExpressionType>(type))
    {
        return arrayType->arrayLength ? (size_t) getIntVal(arrayType->arrayLength) : 0;
    }
    else if( auto vectorType = as<VectorExpressionType>(type))
    {
        return (size_t) getIntVal(vectorType->elementCount);
    }

    return 0;
}

SLANG_API SlangReflectionType* spReflectionType_GetElementType(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return nullptr;

    if(auto arrayType = as<ArrayExpressionType>(type))
    {
        return (SlangReflectionType*) arrayType->baseType;
    }
    else if( auto parameterGroupType = as<ParameterGroupType>(type))
    {
        return convert(parameterGroupType->elementType);
    }
    else if( auto vectorType = as<VectorExpressionType>(type))
    {
        return convert(vectorType->elementType);
    }
    else if( auto matrixType = as<MatrixExpressionType>(type))
    {
        return convert(matrixType->getElementType());
    }

    return nullptr;
}

SLANG_API unsigned int spReflectionType_GetRowCount(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return 0;

    if(auto matrixType = as<MatrixExpressionType>(type))
    {
        return (unsigned int) getIntVal(matrixType->getRowCount());
    }
    else if(auto vectorType = as<VectorExpressionType>(type))
    {
        return 1;
    }
    else if( auto basicType = as<BasicExpressionType>(type) )
    {
        return 1;
    }

    return 0;
}

SLANG_API unsigned int spReflectionType_GetColumnCount(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return 0;

    if(auto matrixType = as<MatrixExpressionType>(type))
    {
        return (unsigned int) getIntVal(matrixType->getColumnCount());
    }
    else if(auto vectorType = as<VectorExpressionType>(type))
    {
        return (unsigned int) getIntVal(vectorType->elementCount);
    }
    else if( auto basicType = as<BasicExpressionType>(type) )
    {
        return 1;
    }

    return 0;
}

SLANG_API SlangScalarType spReflectionType_GetScalarType(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return 0;

    if(auto matrixType = as<MatrixExpressionType>(type))
    {
        type = matrixType->getElementType();
    }
    else if(auto vectorType = as<VectorExpressionType>(type))
    {
        type = vectorType->elementType;
    }

    if(auto basicType = as<BasicExpressionType>(type))
    {
        switch (basicType->baseType)
        {
#define CASE(BASE, TAG) \
        case BaseType::BASE: return SLANG_SCALAR_TYPE_##TAG

            CASE(Void,      VOID);
            CASE(Bool,      BOOL);
            CASE(Int8,      INT8);
            CASE(Int16,     INT16);
            CASE(Int,       INT32);
            CASE(Int64,     INT64);
            CASE(UInt8,     UINT8);
            CASE(UInt16,    UINT16);
            CASE(UInt,      UINT32);
            CASE(UInt64,    UINT64);
            CASE(Half,      FLOAT16);
            CASE(Float,     FLOAT32);
            CASE(Double,    FLOAT64);

#undef CASE

        default:
            SLANG_REFLECTION_UNEXPECTED();
            return SLANG_SCALAR_TYPE_NONE;
            break;
        }
    }

    return SLANG_SCALAR_TYPE_NONE;
}

SLANG_API unsigned int spReflectionType_GetUserAttributeCount(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if (!type) return 0;
    if (auto declRefType = as<DeclRefType>(type))
    {
        return getUserAttributeCount(declRefType->declRef.getDecl());
    }
    return 0;
}
SLANG_API SlangReflectionUserAttribute* spReflectionType_GetUserAttribute(SlangReflectionType* inType, unsigned int index)
{
    auto type = convert(inType);
    if (!type) return 0;
    if (auto declRefType = as<DeclRefType>(type))
    {
        return getUserAttributeByIndex(declRefType->declRef.getDecl(), index);
    }
    return 0;
}
SLANG_API SlangReflectionUserAttribute* spReflectionType_FindUserAttributeByName(SlangReflectionType* inType, char const* name)
{
    auto type = convert(inType);
    if (!type) return 0;
    if (auto declRefType = as<DeclRefType>(type))
    {
        ASTBuilder* astBuilder = declRefType->getASTBuilder();
        auto globalSession = astBuilder->getGlobalSession();

        return findUserAttributeByName(globalSession, declRefType->declRef.getDecl(), name);
    }
    return 0;
}

SLANG_API SlangResourceShape spReflectionType_GetResourceShape(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return 0;

    while(auto arrayType = as<ArrayExpressionType>(type))
    {
        type = arrayType->baseType;
    }

    if(auto textureType = as<TextureTypeBase>(type))
    {
        return textureType->getShape();
    }

    // TODO: need a better way to handle this stuff...
#define CASE(TYPE, SHAPE, ACCESS)   \
    else if(as<TYPE>(type)) do {  \
        return SHAPE;               \
    } while(0)

    CASE(HLSLStructuredBufferType,                      SLANG_STRUCTURED_BUFFER,        SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWStructuredBufferType,                    SLANG_STRUCTURED_BUFFER,        SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(HLSLRasterizerOrderedStructuredBufferType,     SLANG_STRUCTURED_BUFFER,        SLANG_RESOURCE_ACCESS_RASTER_ORDERED);
    CASE(HLSLAppendStructuredBufferType,                SLANG_STRUCTURED_BUFFER,        SLANG_RESOURCE_ACCESS_APPEND);
    CASE(HLSLConsumeStructuredBufferType,               SLANG_STRUCTURED_BUFFER,        SLANG_RESOURCE_ACCESS_CONSUME);
    CASE(HLSLByteAddressBufferType,                     SLANG_BYTE_ADDRESS_BUFFER,      SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWByteAddressBufferType,                   SLANG_BYTE_ADDRESS_BUFFER,      SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(HLSLRasterizerOrderedByteAddressBufferType,    SLANG_BYTE_ADDRESS_BUFFER,      SLANG_RESOURCE_ACCESS_RASTER_ORDERED);
    CASE(RaytracingAccelerationStructureType,           SLANG_ACCELERATION_STRUCTURE,   SLANG_RESOURCE_ACCESS_READ);
    CASE(UntypedBufferResourceType,                     SLANG_BYTE_ADDRESS_BUFFER,      SLANG_RESOURCE_ACCESS_READ);
#undef CASE

    return SLANG_RESOURCE_NONE;
}

SLANG_API SlangResourceAccess spReflectionType_GetResourceAccess(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return 0;

    while(auto arrayType = as<ArrayExpressionType>(type))
    {
        type = arrayType->baseType;
    }

    if(auto textureType = as<TextureTypeBase>(type))
    {
        return textureType->getAccess();
    }

    // TODO: need a better way to handle this stuff...
#define CASE(TYPE, SHAPE, ACCESS)   \
    else if(as<TYPE>(type)) do {  \
        return ACCESS;              \
    } while(0)

    CASE(HLSLStructuredBufferType,                      SLANG_STRUCTURED_BUFFER,    SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWStructuredBufferType,                    SLANG_STRUCTURED_BUFFER,    SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(HLSLRasterizerOrderedStructuredBufferType,     SLANG_STRUCTURED_BUFFER,    SLANG_RESOURCE_ACCESS_RASTER_ORDERED);
    CASE(HLSLAppendStructuredBufferType,                SLANG_STRUCTURED_BUFFER,    SLANG_RESOURCE_ACCESS_APPEND);
    CASE(HLSLConsumeStructuredBufferType,               SLANG_STRUCTURED_BUFFER,    SLANG_RESOURCE_ACCESS_CONSUME);
    CASE(HLSLByteAddressBufferType,                     SLANG_BYTE_ADDRESS_BUFFER,  SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWByteAddressBufferType,                   SLANG_BYTE_ADDRESS_BUFFER,  SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(HLSLRasterizerOrderedByteAddressBufferType,    SLANG_BYTE_ADDRESS_BUFFER,  SLANG_RESOURCE_ACCESS_RASTER_ORDERED);
    CASE(UntypedBufferResourceType,                     SLANG_BYTE_ADDRESS_BUFFER,  SLANG_RESOURCE_ACCESS_READ);

    // This isn't entirely accurate, but I can live with it for now
    CASE(GLSLShaderStorageBufferType, SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_READ_WRITE);
#undef CASE

    return SLANG_RESOURCE_ACCESS_NONE;
}

SLANG_API char const* spReflectionType_GetName(SlangReflectionType* inType)
{
    auto type = convert(inType);

    if( auto declRefType = as<DeclRefType>(type) )
    {
        auto declRef = declRefType->declRef;

        // Don't return a name for auto-generated anonymous types
        // that represent `cbuffer` members, etc.
        auto decl = declRef.getDecl();
        if(decl->hasModifier<ImplicitParameterGroupElementTypeModifier>())
            return nullptr;

        return getText(declRef.getName()).begin();
    }

    return nullptr;
}

SLANG_API SlangReflectionType * spReflection_FindTypeByName(SlangReflection * reflection, char const * name)
{
    auto programLayout = convert(reflection);
    auto program = programLayout->getProgram();

    // TODO: We should extend this API to support getting error messages
    // when type lookup fails.
    //
    Slang::DiagnosticSink sink(
        programLayout->getTargetReq()->getLinkage()->getSourceManager());

    try
    {
        Type* result = program->getTypeFromString(name, &sink);
        return (SlangReflectionType*)result;
    }
    catch( ... )
    {
        return nullptr;
    }
}

SLANG_API SlangReflectionTypeLayout* spReflection_GetTypeLayout(
    SlangReflection* reflection,
    SlangReflectionType* inType,
    SlangLayoutRules /*rules*/)
{
    auto context = convert(reflection);
    auto type = convert(inType);
    auto targetReq = context->getTargetReq();

    auto typeLayout = targetReq->getTypeLayout(type);
    return convert(typeLayout);
}

SLANG_API SlangReflectionType* spReflectionType_GetResourceResultType(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return nullptr;

    while(auto arrayType = as<ArrayExpressionType>(type))
    {
        type = arrayType->baseType;
    }

    if (auto textureType = as<TextureTypeBase>(type))
    {
        return convert(textureType->elementType);
    }

    // TODO: need a better way to handle this stuff...
#define CASE(TYPE, SHAPE, ACCESS)                                                       \
    else if(as<TYPE>(type)) do {                                                      \
        return convert(as<TYPE>(type)->elementType);                            \
    } while(0)

    // TODO: structured buffer needs to expose type layout!

    CASE(HLSLStructuredBufferType,                  SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWStructuredBufferType,                SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(HLSLRasterizerOrderedStructuredBufferType, SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_RASTER_ORDERED);
    CASE(HLSLAppendStructuredBufferType,            SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_APPEND);
    CASE(HLSLConsumeStructuredBufferType,           SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_CONSUME);
#undef CASE

    return nullptr;
}

// type Layout Reflection

SLANG_API SlangReflectionType* spReflectionTypeLayout_GetType(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return nullptr;

    return (SlangReflectionType*) typeLayout->type;
}

SLANG_API SlangTypeKind spReflectionTypeLayout_getKind(SlangReflectionTypeLayout* inTypeLayout)
{
    if(!inTypeLayout) return SLANG_TYPE_KIND_NONE;

    if( auto type = spReflectionTypeLayout_GetType(inTypeLayout) )
    {
        return spReflectionType_GetKind(type);
    }

    auto typeLayout = convert(inTypeLayout);
    if( as<StructTypeLayout>(typeLayout) )
    {
        return SLANG_TYPE_KIND_STRUCT;
    }
    else if( as<ParameterGroupTypeLayout>(typeLayout) )
    {
        return SLANG_TYPE_KIND_CONSTANT_BUFFER;
    }

    return SLANG_TYPE_KIND_NONE;
}

namespace
{
    static size_t getReflectionSize(LayoutSize size)
    {
        if(size.isFinite())
            return size.getFiniteValue();

        return SLANG_UNBOUNDED_SIZE;
    }
}

SLANG_API size_t spReflectionTypeLayout_GetSize(SlangReflectionTypeLayout* inTypeLayout, SlangParameterCategory category)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto info = typeLayout->FindResourceInfo(LayoutResourceKind(category));
    if(!info) return 0;

    return getReflectionSize(info->count);
}

SLANG_API int32_t spReflectionTypeLayout_getAlignment(SlangReflectionTypeLayout* inTypeLayout, SlangParameterCategory category)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    if( category == SLANG_PARAMETER_CATEGORY_UNIFORM )
    {
        return int32_t(typeLayout->uniformAlignment);
    }
    else
    {
        return 1;
    }
}

SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_GetFieldByIndex(SlangReflectionTypeLayout* inTypeLayout, unsigned index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return nullptr;

    if(auto structTypeLayout = as<StructTypeLayout>(typeLayout))
    {
        return (SlangReflectionVariableLayout*) structTypeLayout->fields[index].Ptr();
    }

    return nullptr;
}

SLANG_API SlangInt spReflectionTypeLayout_findFieldIndexByName(SlangReflectionTypeLayout* inTypeLayout, const char* nameBegin, const char* nameEnd)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return -1;

    UnownedStringSlice name = nameEnd != nullptr ? UnownedStringSlice(nameBegin, nameEnd) : UnownedTerminatedStringSlice(nameBegin);

    if(auto structTypeLayout = as<StructTypeLayout>(typeLayout))
    {
        Index fieldCount = structTypeLayout->fields.getCount();
        for(Index f = 0; f < fieldCount; ++f)
        {
            auto field = structTypeLayout->fields[f];
            if(getReflectionName(field->varDecl)->text.getUnownedSlice() == name)
                return f;
        }
    }

    return -1;
}

SLANG_API size_t spReflectionTypeLayout_GetElementStride(SlangReflectionTypeLayout* inTypeLayout, SlangParameterCategory category)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    if( auto arrayTypeLayout = as<ArrayTypeLayout>(typeLayout))
    {
        switch (category)
        {
        // We store the stride explicitly for the uniform case
        case SLANG_PARAMETER_CATEGORY_UNIFORM:
            return arrayTypeLayout->uniformStride;

        // For most other cases (resource registers), the "stride"
        // of an array is simply the number of resources (if any)
        // consumed by its element type.
        default:
            {
                auto elementTypeLayout = arrayTypeLayout->elementTypeLayout;
                auto info = elementTypeLayout->FindResourceInfo(LayoutResourceKind(category));
                if(!info) return 0;
                return getReflectionSize(info->count);
            }

        // An important special case, though, is Vulkan descriptor-table slots,
        // where an entire array will use a single `binding`, so that the
        // effective stride is zero:
        case SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT:
            return 0;
        }
    }

    return 0;
}

SLANG_API SlangReflectionTypeLayout* spReflectionTypeLayout_GetElementTypeLayout(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return nullptr;

    if( auto arrayTypeLayout = as<ArrayTypeLayout>(typeLayout))
    {
        return (SlangReflectionTypeLayout*) arrayTypeLayout->elementTypeLayout.Ptr();
    }
    else if( auto constantBufferTypeLayout = as<ParameterGroupTypeLayout>(typeLayout))
    {
        return convert(constantBufferTypeLayout->offsetElementTypeLayout.Ptr());
    }
    else if( auto structuredBufferTypeLayout = as<StructuredBufferTypeLayout>(typeLayout))
    {
        return convert(structuredBufferTypeLayout->elementTypeLayout.Ptr());
    }
    else if( auto specializedTypeLayout = as<ExistentialSpecializedTypeLayout>(typeLayout) )
    {
        return convert(specializedTypeLayout->baseTypeLayout.Ptr());
    }

    return nullptr;
}

SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_GetElementVarLayout(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return nullptr;

    if( auto parameterGroupTypeLayout = as<ParameterGroupTypeLayout>(typeLayout))
    {
        return convert(parameterGroupTypeLayout->elementVarLayout.Ptr());
    }

    return nullptr;
}

SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_getContainerVarLayout(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return nullptr;

    if( auto parameterGroupTypeLayout = as<ParameterGroupTypeLayout>(typeLayout))
    {
        return convert(parameterGroupTypeLayout->containerVarLayout.Ptr());
    }

    return nullptr;
}

SLANG_API SlangParameterCategory spReflectionTypeLayout_GetParameterCategory(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return SLANG_PARAMETER_CATEGORY_NONE;

    return getParameterCategory(typeLayout);
}

SLANG_API unsigned spReflectionTypeLayout_GetCategoryCount(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    return (unsigned) typeLayout->resourceInfos.getCount();
}

SLANG_API SlangParameterCategory spReflectionTypeLayout_GetCategoryByIndex(SlangReflectionTypeLayout* inTypeLayout, unsigned index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return SLANG_PARAMETER_CATEGORY_NONE;

    return typeLayout->resourceInfos[index].kind;
}

SLANG_API SlangMatrixLayoutMode spReflectionTypeLayout_GetMatrixLayoutMode(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return SLANG_MATRIX_LAYOUT_MODE_UNKNOWN;

    if( auto matrixLayout = as<MatrixTypeLayout>(typeLayout) )
    {
        return matrixLayout->mode;
    }
    else
    {
        return SLANG_MATRIX_LAYOUT_MODE_UNKNOWN;
    }

}

SLANG_API int spReflectionTypeLayout_getGenericParamIndex(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return -1;

    if(auto genericParamTypeLayout = as<GenericParamTypeLayout>(typeLayout))
    {
        return (int) genericParamTypeLayout->paramIndex;
    }
    else
    {
        return -1;
    }
}

SLANG_API SlangReflectionTypeLayout* spReflectionTypeLayout_getPendingDataTypeLayout(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return nullptr;

    auto pendingDataTypeLayout = typeLayout->pendingDataTypeLayout.Ptr();
    return convert(pendingDataTypeLayout);
}

SLANG_API SlangReflectionVariableLayout* spReflectionVariableLayout_getPendingDataLayout(SlangReflectionVariableLayout* inVarLayout)
{
    auto varLayout = convert(inVarLayout);
    if(!varLayout) return nullptr;

    auto pendingDataLayout = varLayout->pendingVarLayout.Ptr();
    return convert(pendingDataLayout);
}

SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_getSpecializedTypePendingDataVarLayout(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return nullptr;

    if( auto specializedTypeLayout = as<ExistentialSpecializedTypeLayout>(typeLayout) )
    {
        auto pendingDataVarLayout = specializedTypeLayout->pendingDataVarLayout.Ptr();
        return convert(pendingDataVarLayout);
    }
    else
    {
        return nullptr;
    }
}

namespace Slang
{
    struct BindingRangePathLink
    {
        BindingRangePathLink(
            BindingRangePathLink*   parent,
            VarLayout*              var)
            : parent(parent)
            , var(var)
        {}

        BindingRangePathLink*   parent;
        VarLayout*              var;
    };


    Int _calcIndexOffset(BindingRangePathLink* path, LayoutResourceKind kind)
    {
        Int result = 0;
        for( auto link = path; link; link = link->parent )
        {
            if( auto resInfo = link->var->FindResourceInfo(kind) )
            {
                result += resInfo->index;
            }
        }
        return result;
    }

    Int _calcSpaceOffset(BindingRangePathLink* path, LayoutResourceKind kind)
    {
        Int result = 0;
        for( auto link = path; link; link = link->parent )
        {
            if( auto resInfo = link->var->FindResourceInfo(kind) )
            {
                result += resInfo->space;
            }
        }
        return result;
    }

    SlangBindingType _calcResourceBindingType(
        Type* type)
    {
        if( auto resourceType = as<ResourceType>(type) )
        {
            auto shape = resourceType->getBaseShape();

            auto access = resourceType->getAccess();
            auto mutableFlag = access != SLANG_RESOURCE_ACCESS_READ ? SLANG_BINDING_TYPE_MUTABLE_FLAG : 0;

            switch( shape )
            {
            default:
                return SLANG_BINDING_TYPE_TEXTURE | mutableFlag;

            case SLANG_TEXTURE_BUFFER:
                return SLANG_BINDING_TYPE_TYPED_BUFFER | mutableFlag;
            }

        }
        else if( auto structuredBufferType = as<HLSLStructuredBufferTypeBase>(type) )
        {
            if( as<HLSLStructuredBufferType>(type) )
            {
                return SLANG_BINDING_TYPE_RAW_BUFFER;
            }
            else
            {
                return SLANG_BINDING_TYPE_MUTABLE_RAW_BUFFER;
            }
        }
        else if( as<RaytracingAccelerationStructureType>(type) )
        {
            return SLANG_BINDING_TYPE_RAY_TRACTING_ACCELERATION_STRUCTURE;
        }
        else if( auto untypedBufferType = as<UntypedBufferResourceType>(type) )
        {
            if( as<HLSLByteAddressBufferType>(type) )
            {
                return SLANG_BINDING_TYPE_RAW_BUFFER;
            }
            else
            {
                return SLANG_BINDING_TYPE_MUTABLE_RAW_BUFFER;
            }
        }
        else if( as<ConstantBufferType>(type) )
        {
            return SLANG_BINDING_TYPE_CONSTANT_BUFFER;
        }
        else
        {
            return SLANG_BINDING_TYPE_UNKNOWN;
        }
    }

    SlangBindingType _calcResourceBindingType(
        TypeLayout* typeLayout)
    {
        if(auto type = typeLayout->getType())
        {
            return _calcResourceBindingType(type);
        }

        if(as<ParameterGroupTypeLayout>(typeLayout))
        {
            return SLANG_BINDING_TYPE_CONSTANT_BUFFER;
        }
        else
        {
            return SLANG_BINDING_TYPE_UNKNOWN;
        }
    }


    SlangBindingType _calcBindingType(
        Slang::TypeLayout*  typeLayout,
        LayoutResourceKind  kind)
    {
        // If the type or type layout implies a specific binding type
        // (e.g., a `Texture2D` implies a texture binding), then we
        // will always favor the binding type implied.
        //
        if( auto bindingType = _calcResourceBindingType(typeLayout) )
        {
            if(bindingType != SLANG_BINDING_TYPE_UNKNOWN)
                return bindingType;
        }

        // As a fallback, we may look at the kind of resources consumed
        // by a type layout, and use that to infer the type of binding
        // used. Note that, for example, a `float4` might represent
        // multiple different kinds of binding, depending on where/how
        // it is used (e.g., as a varying parameter, a root constant, etc.).
        //
        switch( kind )
        {
        default:
            return SLANG_BINDING_TYPE_UNKNOWN;

        // Some cases of `LayoutResourceKind` can be mapped
        // directly to a `BindingType` because there is only
        // one case of types that have that resource kind.

    #define CASE(FROM, TO) \
        case LayoutResourceKind::FROM: return SLANG_BINDING_TYPE_##TO

        CASE(ConstantBuffer, CONSTANT_BUFFER);
        CASE(SamplerState, SAMPLER);
        CASE(VaryingInput, VARYING_INPUT);
        CASE(VaryingOutput, VARYING_OUTPUT);
        CASE(ExistentialObjectParam, EXISTENTIAL_VALUE);
        CASE(PushConstantBuffer, PUSH_CONSTANT);
        CASE(Uniform,               INLINE_UNIFORM_DATA);
        // TODO: register space

    #undef CASE
        }
    }

    static DeclRefType* asInterfaceType(Type* type)
    {
        if(auto declRefType = as<DeclRefType>(type))
        {
            if(declRefType->declRef.as<InterfaceDecl>())
            {
                return declRefType;
            }
        }
        return nullptr;
    }

    struct ExtendedTypeLayoutContext
    {
        TypeLayout* m_typeLayout;
        TypeLayout::ExtendedInfo* m_extendedInfo;

        Dictionary<Int, Int> m_mapSpaceToDescriptorSetIndex;

        Int _findOrAddDescriptorSet(Int space)
        {
            Int index = 0;
            if(m_mapSpaceToDescriptorSetIndex.TryGetValue(space, index))
                return index;

            index = m_extendedInfo->m_descriptorSets.getCount();
            m_mapSpaceToDescriptorSetIndex.Add(space, index);

            RefPtr<TypeLayout::ExtendedInfo::DescriptorSetInfo> descriptorSet = new TypeLayout::ExtendedInfo::DescriptorSetInfo();
            m_extendedInfo->m_descriptorSets.add(descriptorSet);

            return index;
        }

        void addRangesRec(TypeLayout* typeLayout, BindingRangePathLink* path, LayoutSize multiplier)
        {
            if( auto structTypeLayout = as<StructTypeLayout>(typeLayout) )
            {
                // For a structure type, we need to recursively
                // add the ranges for each field.
                //
                // Along the way we will make sure to properly update
                // the offset information on the fields so that
                // they properly show their binding-range offset
                // within the parent type.
                //
                Index structBindingRangeIndex = m_extendedInfo->m_bindingRanges.getCount();
                for( auto fieldVarLayout : structTypeLayout->fields )
                {
                    Index fieldBindingRangeIndex = m_extendedInfo->m_bindingRanges.getCount();
                    fieldVarLayout->bindingRangeOffset = fieldBindingRangeIndex - structBindingRangeIndex;

                    auto fieldTypeLayout = fieldVarLayout->getTypeLayout();


                    BindingRangePathLink fieldLink(path, fieldVarLayout);
                    addRangesRec(fieldTypeLayout, &fieldLink, multiplier);
                }
                return;
            }
            else if( auto arrayTypeLayout = as<ArrayTypeLayout>(typeLayout) )
            {
                // For an array, we need to recursively add the
                // element type of the array, but with an adjusted
                // `multiplier` to account for the element count.
                //
                auto elementTypeLayout = arrayTypeLayout->elementTypeLayout;
                LayoutSize elementCount = LayoutSize::infinite();
                if( auto arrayType = as<ArrayExpressionType>(arrayTypeLayout->type) )
                {
                    if( auto elementCountVal = arrayType->arrayLength )
                    {
                        elementCount = LayoutSize::RawValue(getIntVal(elementCountVal));
                    }
                }
                addRangesRec(elementTypeLayout, path, multiplier * elementCount);
                return;
            }
            else if( auto parameterGroupTypeLayout = as<ParameterGroupTypeLayout>(typeLayout))
            {
                // A parameter group (whether a `ConstantBuffer<>` or `ParameterBlock<>`
                // introduces a separately-allocated "sub-object" in the application's
                // layout for shader objects.
                //
                // We will represent the parameter group with a single sub-object
                // binding range (and an associated sub-object range).
                //
                // We start out by looking at the resources consumed by the parameter group
                // itself, to determine what kind of binding range to report it as.
                //
                Index bindingRangeIndex = m_extendedInfo->m_bindingRanges.getCount();
                SlangBindingType bindingType = SLANG_BINDING_TYPE_CONSTANT_BUFFER;
                Index spaceOffset = -1;
                LayoutResourceKind kind = LayoutResourceKind::None;

                // TODO: It is unclear if this should be looking at the resource
                // usage of the parameter group, or of its "container" layout.
                //
                for(auto& resInfo : parameterGroupTypeLayout->resourceInfos)
                {
                    kind = resInfo.kind;
                    switch(kind)
                    {
                    default:
                        continue;

                        // Note: the only case where a parameter group should
                        // reflect as consuming `Uniform` storage is on CPU/CUDA,
                        // where that will be the only resource it contains.
                        //
                        // TODO: If we ever support targets that don't have
                        // constant buffers at all, this logic would be questionable.
                        //
                    case LayoutResourceKind::ConstantBuffer:
                    case LayoutResourceKind::PushConstantBuffer:
                    case LayoutResourceKind::RegisterSpace:
                    case LayoutResourceKind::DescriptorTableSlot:
                    case LayoutResourceKind::Uniform:
                        break;
                    }

                    bindingType = _calcBindingType(typeLayout, kind);
                    spaceOffset = _calcSpaceOffset(path, kind);
                    break;
                }

                TypeLayout::ExtendedInfo::BindingRangeInfo bindingRange;
                bindingRange.leafTypeLayout = typeLayout;
                bindingRange.bindingType = bindingType;
                bindingRange.count = multiplier;
                bindingRange.descriptorSetIndex = -1;
                bindingRange.firstDescriptorRangeIndex = 0;
                bindingRange.descriptorRangeCount = 0;

                if( kind == LayoutResourceKind::PushConstantBuffer )
                {
                    if(auto resInfo = parameterGroupTypeLayout->elementVarLayout->typeLayout->FindResourceInfo(LayoutResourceKind::Uniform))
                    {
                        bindingRange.count *= resInfo->count;
                    }
                }

                // Every parameter group will introduce a sub-object range,
                // which will include bindings based on the type of data
                // inside the sub-object.
                //
                TypeLayout::ExtendedInfo::SubObjectRangeInfo subObjectRange;
                subObjectRange.bindingRangeIndex = bindingRangeIndex;

                // It is possible that the sub-object has descriptor ranges
                // that will need to be exposed upward, into the parent.
                //
                // Note: it is a subtle point, but we are only going to expose
                // *descriptor ranges* upward and not *binding ranges*. The
                // distinction here comes down to:
                //
                // * Descriptor ranges are used to describe the entries that
                //   must be allocated in one or more API descriptor sets to
                //   physically hold a value of a given type (layout).
                //
                // * Binding ranges are used to describe the entries that must
                //   be allocated in an application shader object to logically
                //   hold a value of a given type (layout).
                //
                // In practice, a binding range might logically belong to a
                // sub-object, but physically belong to a parent. Consider:
                //
                //    cbuffer C { Texture2D a; float b; }
                //
                // Independent of the API we compile for, we expect the global
                // scope to have a sub-object for `C`, and for that sub-object
                // to have a binding range for `a` (that is, we bind the texture
                // into the sub-object).
                //
                // When compiling for D3D12 or Vulkan, we expect that the global
                // scope must have two descriptor ranges for `C`: one for the
                // constant buffer itself, and another for the texture `a`.
                // The reason for this is that `a` needs to be bound as part
                // of a descriptor set, and `C` doesn't create/allocate its own
                // descriptor set(s).
                //
                // When compiling for CPU or CUDA, we expect that the global scope
                // will have a descriptor range for `C` but *not* one for `C.a`,
                // because the physical storage for `C.a` is provided by the
                // memory allocation for `C` itself.

                if( spaceOffset != -1 )
                {
                    // The logic here assumes that when a parameter group consumes
                    // resources that must "leak" into the outer scope (including
                    // reosurces consumed by the group "container"), those resources
                    // will amount to descriptor ranges that are part of the same
                    // descriptor set.
                    //
                    // (If the contents of a group consume whole spaces/sets, then
                    // those resources will be accounted for separately).
                    //
                    Int descriptorSetIndex = _findOrAddDescriptorSet(spaceOffset);
                    auto descriptorSet = m_extendedInfo->m_descriptorSets[descriptorSetIndex];
                    auto firstDescriptorRangeIndex = descriptorSet->descriptorRanges.getCount();

                    // First, we need to deal with any descriptor ranges that are
                    // introduced by the "container" type itself.
                    //
                    switch(kind)
                    {
                        // If the parameter group was allocated to consume one or
                        // more whole register spaces/sets, then nothing should
                        // leak through that is measured in descriptor sets.
                        //
                    case LayoutResourceKind::RegisterSpace:
                    case LayoutResourceKind::None:
                        break;

                    default:
                        {
                            // In a constant-buffer-like case, then all the (non-space/set) resource
                            // usage of the "container" should be reflected as descriptor
                            // ranges in the parent scope.
                            //
                            for(auto resInfo : parameterGroupTypeLayout->containerVarLayout->typeLayout->resourceInfos)
                            {
                                switch( resInfo.kind )
                                {
                                case LayoutResourceKind::RegisterSpace:
                                    continue;

                                default:
                                    break;
                                }

                                TypeLayout::ExtendedInfo::DescriptorRangeInfo descriptorRange;
                                descriptorRange.kind = resInfo.kind;
                                descriptorRange.bindingType = _calcBindingType(typeLayout, resInfo.kind);
                                descriptorRange.count = multiplier;
                                descriptorRange.indexOffset = _calcIndexOffset(path, resInfo.kind);

                                if( resInfo.kind == LayoutResourceKind::PushConstantBuffer )
                                {
                                    if(auto uniformResInfo = parameterGroupTypeLayout->elementVarLayout->typeLayout->FindResourceInfo(LayoutResourceKind::Uniform))
                                    {
                                        descriptorRange.count *= uniformResInfo->count;
                                    }
                                }

                                descriptorSet->descriptorRanges.add(descriptorRange);
                            }
                        }

                    }

                    // Second, we need to consider resource usage from the "element"
                    // type that might leak through to the parent.
                    //
                    switch(kind)
                    {
                        // If the parameter group was allocated as a full register space/set,
                        // *or* if it was allocated as ordinary uniform storage (likely
                        // because it was compiled for CPU/CUDA), then there should
                        // be no "leakage" of descriptor ranges from the element type
                        // to the parent.
                        //
                    case LayoutResourceKind::RegisterSpace:
                    case LayoutResourceKind::Uniform:
                    case LayoutResourceKind::None:
                        break;

                    default:
                        {
                            // If we are in the constant-buffer-like case, on an API
                            // where constant bufers "leak" resource usage to the
                            // outer context, then we need to add the descriptor ranges
                            // implied by the element type.
                            //
                            // HACK: We enumerate these nested ranges by recurisvely
                            // calling `addRangesRec`, which adds all of descriptor ranges,
                            // binding ranges, and sub-object ranges, and then we trim
                            // the lists we don't actually care about as a post-process.
                            //
                            // TODO: We could try to consider a model where we first
                            // query the extended layout information of the element
                            // type (which might already be cached) and then enumerate
                            // the descriptor ranges and copy them over.
                            //
                            // TODO: It is possible that there could be cases where
                            // some, but not all, of the nested descriptor ranges ought
                            // to be enumerated here. In that case we might have to introduce
                            // a kind of "mask" parameter that is passed down into
                            // the recursive call so that only the appropriate ranges
                            // get added.

                            // We need to add a link to the "path" that is used when looking
                            // up binding information, to ensure that the descriptor ranges
                            // that get enumerated here have correct register/binding offsets.
                            //
                            BindingRangePathLink elementPath(path, parameterGroupTypeLayout->elementVarLayout);

                            Index bindingRangeCountBefore = m_extendedInfo->m_bindingRanges.getCount();
                            Index subObjectRangeCountBefore = m_extendedInfo->m_subObjectRanges.getCount();

                            addRangesRec(parameterGroupTypeLayout->elementVarLayout->typeLayout, &elementPath, multiplier);

                            m_extendedInfo->m_bindingRanges.setCount(bindingRangeCountBefore);
                            m_extendedInfo->m_subObjectRanges.setCount(subObjectRangeCountBefore);
                        }
                        break;
                    }

                    auto descriptorRangeCount = descriptorSet->descriptorRanges.getCount() - firstDescriptorRangeIndex;
                    bindingRange.descriptorSetIndex = descriptorSetIndex;
                    bindingRange.firstDescriptorRangeIndex = firstDescriptorRangeIndex;
                    bindingRange.descriptorRangeCount = descriptorRangeCount;
                }

                m_extendedInfo->m_bindingRanges.add(bindingRange);
                m_extendedInfo->m_subObjectRanges.add(subObjectRange);
                return;
            }
            else if(asInterfaceType(typeLayout->type))
            {
                // An `interface` type should introduce a sub-object range,
                // with no concrete descriptor ranges to store its value
                // (since we don't know until runtime what type of
                // value will be plugged in).
                //

                LayoutResourceKind kind = LayoutResourceKind::ExistentialObjectParam;
                auto count = multiplier;
                auto spaceOffset = _calcSpaceOffset(path, kind);

                Int descriptorSetIndex = _findOrAddDescriptorSet(spaceOffset);
                auto descriptorSet = m_extendedInfo->m_descriptorSets[descriptorSetIndex];

                TypeLayout::ExtendedInfo::BindingRangeInfo bindingRange;
                bindingRange.leafTypeLayout = typeLayout;
                bindingRange.bindingType = SLANG_BINDING_TYPE_EXISTENTIAL_VALUE;
                bindingRange.count = multiplier;
                bindingRange.descriptorSetIndex = descriptorSetIndex;
                bindingRange.firstDescriptorRangeIndex = descriptorSet->descriptorRanges.getCount();
                bindingRange.descriptorRangeCount = 1;

                TypeLayout::ExtendedInfo::SubObjectRangeInfo subObjectRange;
                subObjectRange.bindingRangeIndex = m_extendedInfo->m_bindingRanges.getCount();

                m_extendedInfo->m_bindingRanges.add(bindingRange);
                m_extendedInfo->m_subObjectRanges.add(subObjectRange);
            }
            // TODO: We need to handle `interface` types here, because they are
            // another case that introduces a "sub-object" for the purposes of
            // application-side allocation.
            //
            // TODO: There are a few cases of "leaf" fields that might
            // still result in multiple descriptors (or at least multiple
            // `LayoutResourceKind`s) depending on the target.
            //
            // For eample, combined texture-sampler types should be treated
            // as "leaf" fields for this code (since a portable engine would
            // need to abstract over them), but would map to two descriptors
            // on targets that don't actually support combining them.
            else
            {
                Int resourceKindCount = typeLayout->resourceInfos.getCount();
                if(resourceKindCount == 0)
                {
                    // This is a field that consumes no resources, and as
                    // such does not need a binding or descriptor ranges
                    // allocated for it.
                    //
                    return;
                }
                else if(resourceKindCount == 1)
                {
                    auto& resInfo = typeLayout->resourceInfos[0];
                    LayoutResourceKind kind = resInfo.kind;

                    auto bindingType = _calcBindingType(typeLayout, kind);
                    if(bindingType == SLANG_BINDING_TYPE_INLINE_UNIFORM_DATA)
                    {
                        // We do not consider uniform resource usage
                        // in the ranges we compute.
                        //
                        // TODO: We may need to revise that rule for types that
                        // represent resources, even when one or more targets
                        // map those resource types to ordinary/uniform data.
                        //
                        return;
                    }

                    // This leaf field will map to a single binding range and,
                    // if it is appropriate, a single descriptor range.
                    //
                    auto count = resInfo.count * multiplier;
                    auto indexOffset = _calcIndexOffset(path, kind);
                    auto spaceOffset = _calcSpaceOffset(path, kind);

                    Int descriptorSetIndex = -1;
                    Int firstDescriptorIndex = 0;
                    RefPtr<TypeLayout::ExtendedInfo::DescriptorSetInfo> descriptorSet;
                    switch( kind )
                    {
                    case LayoutResourceKind::RegisterSpace:
                    case LayoutResourceKind::VaryingInput:
                    case LayoutResourceKind::VaryingOutput:
                    case LayoutResourceKind::HitAttributes:
                    case LayoutResourceKind::RayPayload:
                        // Resource kinds that represent "varying" input/output
                        // do not manifest as entries in API descriptor tables.
                        //
                        // TODO: Neither do root constants, if we are being
                        // precise. This API really needs to carefully match
                        // the semantics of the target platform/API in terms
                        // of what things are descriptor-bound and which are
                        // not, so that a user can easily allocate the platform-specific
                        // descriptor sets using this info.
                        //
                        // (That said, we are purposefully *not* breaking apart
                        // samplers and SRV/UAV/CBV stuff for our D3D reflection
                        // of descriptor sets. It seems like the policy here
                        // really requires careful thought)
                        //
                        // TODO: Maybe the best answer is to leave decomposition
                        // of stuff into descriptor sets up to the application
                        // layer? This is especially true if a common case would
                        // be an application that doesn't support arbitrary manual
                        // binding of parameters to register/spaces.
                        //
                        break;

                    default:
                        {
                            TypeLayout::ExtendedInfo::DescriptorRangeInfo descriptorRange;
                            descriptorRange.kind = kind;
                            descriptorRange.bindingType = bindingType;
                            descriptorRange.count = count;
                            descriptorRange.indexOffset = indexOffset;

                            descriptorSetIndex = _findOrAddDescriptorSet(spaceOffset);
                            descriptorSet = m_extendedInfo->m_descriptorSets[descriptorSetIndex];

                            firstDescriptorIndex = descriptorSet->descriptorRanges.getCount();
                            descriptorSet->descriptorRanges.add(descriptorRange);
                        }
                        break;
                    }


                    TypeLayout::ExtendedInfo::BindingRangeInfo bindingRange;
                    bindingRange.leafTypeLayout = typeLayout;
                    bindingRange.bindingType = _calcBindingType(typeLayout, kind);
                    bindingRange.count = count;
                    bindingRange.descriptorSetIndex = descriptorSetIndex;
                    bindingRange.firstDescriptorRangeIndex = firstDescriptorIndex;
                    bindingRange.descriptorRangeCount = 1;

                    m_extendedInfo->m_bindingRanges.add(bindingRange);
                }
                else
                {
                    // This type appears to be one that consumes multiple
                    // resource kinds, but was not handled by any of
                    // the special-case logic above. In such a case we
                    // are in trouble.
                    //
                    return;
                }
            }
        }
    };

    TypeLayout::ExtendedInfo* getExtendedTypeLayout(TypeLayout* typeLayout)
    {
        if( !typeLayout->m_extendedInfo )
        {
            RefPtr<TypeLayout::ExtendedInfo> extendedInfo = new TypeLayout::ExtendedInfo;

            ExtendedTypeLayoutContext context;
            context.m_typeLayout = typeLayout;
            context.m_extendedInfo = extendedInfo;

            context.addRangesRec(typeLayout, nullptr, 1);

            typeLayout->m_extendedInfo = extendedInfo;
        }
        return typeLayout->m_extendedInfo;
    }
}

SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeCount(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto extTypeLayout = Slang::getExtendedTypeLayout(typeLayout);
    return extTypeLayout->m_bindingRanges.getCount();
}

SLANG_API SlangBindingType spReflectionTypeLayout_getBindingRangeType(SlangReflectionTypeLayout* inTypeLayout, SlangInt index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return SLANG_BINDING_TYPE_UNKNOWN;

    auto extTypeLayout = Slang::getExtendedTypeLayout(typeLayout);
    if(index < 0) return 0;
    if(index >= extTypeLayout->m_bindingRanges.getCount()) return 0;
    auto& bindingRange = extTypeLayout->m_bindingRanges[index];

    return bindingRange.bindingType;
}

SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeBindingCount(SlangReflectionTypeLayout* inTypeLayout, SlangInt index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto extTypeLayout = Slang::getExtendedTypeLayout(typeLayout);
    if(index < 0) return 0;
    if(index >= extTypeLayout->m_bindingRanges.getCount()) return 0;
    auto& bindingRange = extTypeLayout->m_bindingRanges[index];

    auto count = bindingRange.count;
    return count.isFinite() ? SlangInt(count.getFiniteValue()) : -1;
}

#if 0
SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeIndexOffset(SlangReflectionTypeLayout* inTypeLayout, SlangInt index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    return Slang::_findBindingRange(typeLayout, index).indexOffset;
}

SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeSpaceOffset(SlangReflectionTypeLayout* inTypeLayout, SlangInt index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    return Slang::_findBindingRange(typeLayout, index).spaceOffset;
}
#endif

SLANG_API SlangReflectionTypeLayout* spReflectionTypeLayout_getBindingRangeLeafTypeLayout(SlangReflectionTypeLayout* inTypeLayout, SlangInt index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto extTypeLayout = Slang::getExtendedTypeLayout(typeLayout);
    if(index < 0) return 0;
    if(index >= extTypeLayout->m_bindingRanges.getCount()) return 0;
    auto& bindingRange = extTypeLayout->m_bindingRanges[index];

    return convert(bindingRange.leafTypeLayout);
}

SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeDescriptorSetIndex(SlangReflectionTypeLayout* inTypeLayout, SlangInt index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto extTypeLayout = Slang::getExtendedTypeLayout(typeLayout);
    if(index < 0) return 0;
    if(index >= extTypeLayout->m_bindingRanges.getCount()) return 0;
    auto& bindingRange = extTypeLayout->m_bindingRanges[index];

    return bindingRange.descriptorSetIndex;
}

SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeFirstDescriptorRangeIndex(SlangReflectionTypeLayout* inTypeLayout, SlangInt index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto extTypeLayout = Slang::getExtendedTypeLayout(typeLayout);
    if(index < 0) return 0;
    if(index >= extTypeLayout->m_bindingRanges.getCount()) return 0;
    auto& bindingRange = extTypeLayout->m_bindingRanges[index];

    return bindingRange.firstDescriptorRangeIndex;
}

SLANG_API SlangInt spReflectionTypeLayout_getDescriptorSetCount(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto extTypeLayout = Slang::getExtendedTypeLayout(typeLayout);

    return extTypeLayout->m_descriptorSets.getCount();
}

SLANG_API SlangInt spReflectionTypeLayout_getDescriptorSetSpaceOffset(SlangReflectionTypeLayout* inTypeLayout, SlangInt setIndex)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto extTypeLayout = Slang::getExtendedTypeLayout(typeLayout);

    if(setIndex < 0) return 0;
    if(setIndex >= extTypeLayout->m_descriptorSets.getCount()) return 0;
    auto descriptorSet = extTypeLayout->m_descriptorSets[setIndex];

    return descriptorSet->spaceOffset;
}

SLANG_API SlangInt spReflectionTypeLayout_getDescriptorSetDescriptorRangeCount(SlangReflectionTypeLayout* inTypeLayout, SlangInt setIndex)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto extTypeLayout = Slang::getExtendedTypeLayout(typeLayout);

    if(setIndex < 0) return 0;
    if(setIndex >= extTypeLayout->m_descriptorSets.getCount()) return 0;
    auto descriptorSet = extTypeLayout->m_descriptorSets[setIndex];

    return descriptorSet->descriptorRanges.getCount();
}

SLANG_API SlangInt spReflectionTypeLayout_getDescriptorSetDescriptorRangeIndexOffset(SlangReflectionTypeLayout* inTypeLayout, SlangInt setIndex, SlangInt rangeIndex)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto extTypeLayout = Slang::getExtendedTypeLayout(typeLayout);

    if(setIndex < 0) return 0;
    if(setIndex >= extTypeLayout->m_descriptorSets.getCount()) return 0;
    auto descriptorSet = extTypeLayout->m_descriptorSets[setIndex];

    if(rangeIndex < 0) return 0;
    if(rangeIndex >= descriptorSet->descriptorRanges.getCount()) return 0;
    auto& range = descriptorSet->descriptorRanges[rangeIndex];

    return range.indexOffset;
}

SLANG_API SlangInt spReflectionTypeLayout_getDescriptorSetDescriptorRangeDescriptorCount(SlangReflectionTypeLayout* inTypeLayout, SlangInt setIndex, SlangInt rangeIndex)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto extTypeLayout = Slang::getExtendedTypeLayout(typeLayout);

    if(setIndex < 0) return 0;
    if(setIndex >= extTypeLayout->m_descriptorSets.getCount()) return 0;
    auto descriptorSet = extTypeLayout->m_descriptorSets[setIndex];

    if(rangeIndex < 0) return 0;
    if(rangeIndex >= descriptorSet->descriptorRanges.getCount()) return 0;
    auto& range = descriptorSet->descriptorRanges[rangeIndex];

    auto count = range.count;
    return count.isFinite() ? count.getFiniteValue() : -1;
}

SLANG_API SlangBindingType spReflectionTypeLayout_getDescriptorSetDescriptorRangeType(SlangReflectionTypeLayout* inTypeLayout, SlangInt setIndex, SlangInt rangeIndex)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto extTypeLayout = Slang::getExtendedTypeLayout(typeLayout);

    if(setIndex < 0) return 0;
    if(setIndex >= extTypeLayout->m_descriptorSets.getCount()) return 0;
    auto descriptorSet = extTypeLayout->m_descriptorSets[setIndex];

    if(rangeIndex < 0) return 0;
    if(rangeIndex >= descriptorSet->descriptorRanges.getCount()) return 0;
    auto& range = descriptorSet->descriptorRanges[rangeIndex];

    return range.bindingType;
}

SLANG_API SlangParameterCategory spReflectionTypeLayout_getDescriptorSetDescriptorRangeCategory(SlangReflectionTypeLayout* inTypeLayout, SlangInt setIndex, SlangInt rangeIndex)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto extTypeLayout = Slang::getExtendedTypeLayout(typeLayout);

    if(setIndex < 0) return 0;
    if(setIndex >= extTypeLayout->m_descriptorSets.getCount()) return 0;
    auto descriptorSet = extTypeLayout->m_descriptorSets[setIndex];

    if(rangeIndex < 0) return 0;
    if(rangeIndex >= descriptorSet->descriptorRanges.getCount()) return 0;
    auto& range = descriptorSet->descriptorRanges[rangeIndex];

    return SlangParameterCategory(range.kind);
}

SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeCount(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto extTypeLayout = Slang::getExtendedTypeLayout(typeLayout);

    return extTypeLayout->m_subObjectRanges.getCount();
}

SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeBindingRangeIndex(SlangReflectionTypeLayout* inTypeLayout, SlangInt subObjectRangeIndex)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto extTypeLayout = Slang::getExtendedTypeLayout(typeLayout);

    if(subObjectRangeIndex < 0) return 0;
    if(subObjectRangeIndex >= extTypeLayout->m_subObjectRanges.getCount()) return 0;

    return extTypeLayout->m_subObjectRanges[subObjectRangeIndex].bindingRangeIndex;
}


#if 0
SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeSubObjectRangeIndex(SlangReflectionTypeLayout* inTypeLayout, SlangInt index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    return Slang::_findBindingRange(typeLayout, index).subObjectRangeIndex;
}
#endif


SLANG_API SlangInt spReflectionTypeLayout_getFieldBindingRangeOffset(SlangReflectionTypeLayout* inTypeLayout, SlangInt fieldIndex)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    if( auto structTypeLayout = as<StructTypeLayout>(typeLayout) )
    {
        getExtendedTypeLayout(structTypeLayout);

        return structTypeLayout->fields[fieldIndex]->bindingRangeOffset;
    }

    return 0;
}

#if 0
SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeCount(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    return Slang::_calcSubObjectRangeCount(typeLayout);
}

SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeObjectCount(SlangReflectionTypeLayout* inTypeLayout, SlangInt index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto count = Slang::_findSubObjectRange(typeLayout, index).count;
    return count.isFinite() ? SlangInt(count.getFiniteValue()) : -1;
}

SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeBindingRangeIndex(SlangReflectionTypeLayout* inTypeLayout, SlangInt index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    return Slang::_findSubObjectRange(typeLayout, index).bindingRangeIndex;
}


SLANG_API SlangReflectionTypeLayout* spReflectionTypeLayout_getSubObjectRangeTypeLayout(SlangReflectionTypeLayout* inTypeLayout, SlangInt index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    return convert(Slang::_findSubObjectRange(typeLayout, index).leafTypeLayout);
}

SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeDescriptorRangeCount(SlangReflectionTypeLayout* inTypeLayout, SlangInt subObjectRangeIndex)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto subObjectRange = Slang::_findSubObjectRange(typeLayout, subObjectRangeIndex);
    return Slang::_getSubObjectDescriptorRangeCount(subObjectRange);
}

SLANG_API SlangBindingType spReflectionTypeLayout_getSubObjectRangeDescriptorRangeBindingType(SlangReflectionTypeLayout* inTypeLayout, SlangInt subObjectRangeIndex, SlangInt bindingRangeIndexInSubObject)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto subObjectRange = Slang::_findSubObjectRange(typeLayout, subObjectRangeIndex);
    return Slang::_getSubObjectDescriptorRange(subObjectRange, bindingRangeIndexInSubObject).bindingType;
}

SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeDescriptorRangeBindingCount(SlangReflectionTypeLayout* inTypeLayout, SlangInt subObjectRangeIndex, SlangInt bindingRangeIndexInSubObject)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto subObjectRange = Slang::_findSubObjectRange(typeLayout, subObjectRangeIndex);
    auto count = Slang::_getSubObjectDescriptorRange(subObjectRange, bindingRangeIndexInSubObject).count;
    return count.isFinite() ? count.getFiniteValue() : -1;
}

SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeDescriptorRangeIndexOffset(SlangReflectionTypeLayout* inTypeLayout, SlangInt subObjectRangeIndex, SlangInt bindingRangeIndexInSubObject)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto subObjectRange = Slang::_findSubObjectRange(typeLayout, subObjectRangeIndex);
    return Slang::_getSubObjectDescriptorRange(subObjectRange, bindingRangeIndexInSubObject).indexOffset;
}

SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeDescriptorRangeSpaceOffset(SlangReflectionTypeLayout* inTypeLayout, SlangInt subObjectRangeIndex, SlangInt bindingRangeIndexInSubObject)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto subObjectRange = Slang::_findSubObjectRange(typeLayout, subObjectRangeIndex);
    return Slang::_getSubObjectDescriptorRange(subObjectRange, bindingRangeIndexInSubObject).spaceOffset;
}
#endif

// Variable Reflection

SLANG_API char const* spReflectionVariable_GetName(SlangReflectionVariable* inVar)
{
    auto var = convert(inVar);
    if(!var) return nullptr;

    // If the variable is one that has an "external" name that is supposed
    // to be exposed for reflection, then report it here
    if(auto reflectionNameMod = var->findModifier<ParameterGroupReflectionName>())
        return getText(reflectionNameMod->nameAndLoc.name).getBuffer();

    return getText(var->getName()).getBuffer();
}

SLANG_API SlangReflectionType* spReflectionVariable_GetType(SlangReflectionVariable* inVar)
{
    auto var = convert(inVar);
    if(!var) return nullptr;

    return  convert(var->getType());
}

SLANG_API SlangReflectionModifier* spReflectionVariable_FindModifier(SlangReflectionVariable* inVar, SlangModifierID modifierID)
{
    auto var = convert(inVar);
    if(!var) return nullptr;

    Modifier* modifier = nullptr;
    switch( modifierID )
    {
    case SLANG_MODIFIER_SHARED:
        modifier = var->findModifier<HLSLEffectSharedModifier>();
        break;

    default:
        return nullptr;
    }

    return (SlangReflectionModifier*) modifier;
}

SLANG_API unsigned int spReflectionVariable_GetUserAttributeCount(SlangReflectionVariable* inVar)
{
    auto varDecl = convert(inVar);
    if (!varDecl) return 0;
    return getUserAttributeCount(varDecl);
}
SLANG_API SlangReflectionUserAttribute* spReflectionVariable_GetUserAttribute(SlangReflectionVariable* inVar, unsigned int index)
{
    auto varDecl = convert(inVar);
    if (!varDecl) return 0;
    return getUserAttributeByIndex(varDecl, index);
}
SLANG_API SlangReflectionUserAttribute* spReflectionVariable_FindUserAttributeByName(SlangReflectionVariable* inVar, SlangSession* session, char const* name)
{
    auto varDecl = convert(inVar);
    if (!varDecl) return 0;
    return findUserAttributeByName(asInternal(session), varDecl, name);
}

// Variable Layout Reflection

SLANG_API SlangReflectionVariable* spReflectionVariableLayout_GetVariable(SlangReflectionVariableLayout* inVarLayout)
{
    auto varLayout = convert(inVarLayout);
    if(!varLayout) return nullptr;

    return convert(varLayout->varDecl.getDecl());
}

SLANG_API SlangReflectionTypeLayout* spReflectionVariableLayout_GetTypeLayout(SlangReflectionVariableLayout* inVarLayout)
{
    auto varLayout = convert(inVarLayout);
    if(!varLayout) return nullptr;

    return convert(varLayout->getTypeLayout());
}

SLANG_API size_t spReflectionVariableLayout_GetOffset(SlangReflectionVariableLayout* inVarLayout, SlangParameterCategory category)
{
    auto varLayout = convert(inVarLayout);
    if(!varLayout) return 0;

    auto info = varLayout->FindResourceInfo(LayoutResourceKind(category));

    if (!info)
    {
        // No match with requested category? Try again with one they might have meant...
        category = maybeRemapParameterCategory(varLayout->getTypeLayout(), category);
        info = varLayout->FindResourceInfo(LayoutResourceKind(category));
    }

    if(!info) return 0;

    return info->index;
}

SLANG_API size_t spReflectionVariableLayout_GetSpace(SlangReflectionVariableLayout* inVarLayout, SlangParameterCategory category)
{
    auto varLayout = convert(inVarLayout);
    if(!varLayout) return 0;


    auto info = varLayout->FindResourceInfo(LayoutResourceKind(category));
    if (!info)
    {
        // No match with requested category? Try again with one they might have meant...
        category = maybeRemapParameterCategory(varLayout->getTypeLayout(), category);
        info = varLayout->FindResourceInfo(LayoutResourceKind(category));
    }

    UInt space = 0;

    // First, deal with any offset applied to the specific resource kind specified
    if (info)
    {
        space += info->space;
    }

    // Note: this code used to try and take a variable with
    // an offset for `LayoutResourceKind::RegisterSpace` and
    // add it to the space returned, but that isn't going
    // to be right in some cases.
    //
    // Imageine if we have:
    //
    //  struct X { Texture2D y; }
    //  struct S { Texture2D t; ParmaeterBlock<X> x; }
    //
    //  Texture2D gA;
    //  S gS;
    //
    // We expect `gS` to have an offset for `LayoutResourceKind::ShaderResourceView`
    // of one (since its texture must come after `gA`), and an offset for
    // `LayoutResourceKind::RegisterSpace` of one (since the default space will be
    // space zero). It would be incorrect for us to imply that `gS.t` should
    // be `t1, space1`, though, because the space offset of `gS` doesn't actually
    // apply to `t`.
    //
    // For now we are punting on this issue and leaving it in the hands of the
    // application to determine when a space offset from an "outer" variable should
    // apply to the locations of things in an "inner" variable.
    //
    // There is no policy we can apply locally in this function that
    // will Just Work, so the best we can do is try to not lie.

    return space;
}

SLANG_API char const* spReflectionVariableLayout_GetSemanticName(SlangReflectionVariableLayout* inVarLayout)
{
    auto varLayout = convert(inVarLayout);
    if(!varLayout) return 0;

    if (!(varLayout->flags & Slang::VarLayoutFlag::HasSemantic))
        return 0;

    return varLayout->semanticName.getBuffer();
}

SLANG_API size_t spReflectionVariableLayout_GetSemanticIndex(SlangReflectionVariableLayout* inVarLayout)
{
    auto varLayout = convert(inVarLayout);
    if(!varLayout) return 0;

    if (!(varLayout->flags & Slang::VarLayoutFlag::HasSemantic))
        return 0;

    return varLayout->semanticIndex;
}

SLANG_API SlangStage spReflectionVariableLayout_getStage(
    SlangReflectionVariableLayout* inVarLayout)
{
    auto varLayout = convert(inVarLayout);
    if(!varLayout) return SLANG_STAGE_NONE;

    // A parameter that is not a varying input or output is
    // not considered to belong to a single stage.
    //
    // TODO: We might need to reconsider this for, e.g., entry
    // point parameters, where they might be stage-specific even
    // if they are uniform.
    if (!varLayout->FindResourceInfo(Slang::LayoutResourceKind::VaryingInput)
        && !varLayout->FindResourceInfo(Slang::LayoutResourceKind::VaryingOutput))
    {
        return SLANG_STAGE_NONE;
    }

    // TODO: We should find the stage for a variable layout by
    // walking up the tree of layout information, until we find
    // something that has a definitive stage attached to it (e.g.,
    // either an entry point or a GLSL translation unit).
    //
    // We don't currently have parent links in the reflection layout
    // information, so doing that walk would be tricky right now, so
    // it is easier to just bloat the representation and store yet another
    // field on every variable layout.
    return (SlangStage) varLayout->stage;
}


// Shader Parameter Reflection

SLANG_API unsigned spReflectionParameter_GetBindingIndex(SlangReflectionParameter* inVarLayout)
{
    SlangReflectionVariableLayout* varLayout = (SlangReflectionVariableLayout*)inVarLayout;
    return (unsigned) spReflectionVariableLayout_GetOffset(
        varLayout,
        spReflectionTypeLayout_GetParameterCategory(
            spReflectionVariableLayout_GetTypeLayout(varLayout)));
}

SLANG_API unsigned spReflectionParameter_GetBindingSpace(SlangReflectionParameter* inVarLayout)
{
    SlangReflectionVariableLayout* varLayout = (SlangReflectionVariableLayout*)inVarLayout;
    return (unsigned) spReflectionVariableLayout_GetSpace(
        varLayout,
        spReflectionTypeLayout_GetParameterCategory(
            spReflectionVariableLayout_GetTypeLayout(varLayout)));
}



// Entry Point Reflection

SLANG_API char const* spReflectionEntryPoint_getName(
    SlangReflectionEntryPoint* inEntryPoint)
{
    auto entryPointLayout = convert(inEntryPoint);
    return entryPointLayout ? getCstr(entryPointLayout->name) : nullptr;
}

SLANG_API unsigned spReflectionEntryPoint_getParameterCount(
    SlangReflectionEntryPoint* inEntryPoint)
{
    auto entryPointLayout = convert(inEntryPoint);
    if(!entryPointLayout) return 0;

    return getParameterCount(entryPointLayout->parametersLayout->typeLayout);
}

SLANG_API SlangReflectionVariableLayout* spReflectionEntryPoint_getParameterByIndex(
    SlangReflectionEntryPoint*  inEntryPoint,
    unsigned                    index)
{
    auto entryPointLayout = convert(inEntryPoint);
    if(!entryPointLayout) return 0;

    return convert(getParameterByIndex(entryPointLayout->parametersLayout->typeLayout, index));
}

SLANG_API SlangStage spReflectionEntryPoint_getStage(SlangReflectionEntryPoint* inEntryPoint)
{
    auto entryPointLayout = convert(inEntryPoint);

    if(!entryPointLayout) return SLANG_STAGE_NONE;

    return SlangStage(entryPointLayout->profile.getStage());
}

SLANG_API void spReflectionEntryPoint_getComputeThreadGroupSize(
    SlangReflectionEntryPoint*  inEntryPoint,
    SlangUInt                   axisCount,
    SlangUInt*                  outSizeAlongAxis)
{
    auto entryPointLayout = convert(inEntryPoint);

    if(!entryPointLayout)   return;
    if(!axisCount)          return;
    if(!outSizeAlongAxis)   return;

    auto entryPointFunc = entryPointLayout->entryPoint;
    if(!entryPointFunc) return;

    SlangUInt sizeAlongAxis[3] = { 1, 1, 1 };

    // First look for the HLSL case, where we have an attribute attached to the entry point function
    auto numThreadsAttribute = entryPointFunc.getDecl()->findModifier<NumThreadsAttribute>();
    if (numThreadsAttribute)
    {
        sizeAlongAxis[0] = numThreadsAttribute->x;
        sizeAlongAxis[1] = numThreadsAttribute->y;
        sizeAlongAxis[2] = numThreadsAttribute->z;
    }

    //

    if(axisCount > 0) outSizeAlongAxis[0] = sizeAlongAxis[0];
    if(axisCount > 1) outSizeAlongAxis[1] = sizeAlongAxis[1];
    if(axisCount > 2) outSizeAlongAxis[2] = sizeAlongAxis[2];
    for( SlangUInt aa = 3; aa < axisCount; ++aa )
    {
        outSizeAlongAxis[aa] = 1;
    }
}

SLANG_API int spReflectionEntryPoint_usesAnySampleRateInput(
    SlangReflectionEntryPoint* inEntryPoint)
{
    auto entryPointLayout = convert(inEntryPoint);
    if(!entryPointLayout)
        return 0;

    if (entryPointLayout->profile.getStage() != Stage::Fragment)
        return 0;

    return (entryPointLayout->flags & EntryPointLayout::Flag::usesAnySampleRateInput) != 0;
}

SLANG_API SlangReflectionVariableLayout* spReflectionEntryPoint_getVarLayout(
    SlangReflectionEntryPoint* inEntryPoint)
{
    auto entryPointLayout = convert(inEntryPoint);
    if(!entryPointLayout)
        return nullptr;

    return convert(entryPointLayout->parametersLayout);
}

SLANG_API SlangReflectionVariableLayout* spReflectionEntryPoint_getResultVarLayout(
    SlangReflectionEntryPoint* inEntryPoint)
{
    auto entryPointLayout = convert(inEntryPoint);
    if(!entryPointLayout)
        return nullptr;

    return convert(entryPointLayout->resultLayout);
}

SLANG_API int spReflectionEntryPoint_hasDefaultConstantBuffer(
    SlangReflectionEntryPoint* inEntryPoint)
{
    auto entryPointLayout = convert(inEntryPoint);
    if(!entryPointLayout)
        return 0;

    return hasDefaultConstantBuffer(entryPointLayout);
}


// SlangReflectionTypeParameter
SLANG_API char const* spReflectionTypeParameter_GetName(SlangReflectionTypeParameter * inTypeParam)
{
    auto specializationParam = convert(inTypeParam);
    if( auto genericParamLayout = as<GenericSpecializationParamLayout>(specializationParam) )
    {
        return genericParamLayout->decl->getName()->text.getBuffer();
    }
    // TODO: Add case for existential type parameter? They don't have as simple of a notion of "name" as the generic case...
    return nullptr;
}

SLANG_API unsigned spReflectionTypeParameter_GetIndex(SlangReflectionTypeParameter * inTypeParam)
{
    auto typeParam = convert(inTypeParam);
    return (unsigned)(typeParam->index);
}

SLANG_API unsigned int spReflectionTypeParameter_GetConstraintCount(SlangReflectionTypeParameter* inTypeParam)
{
    auto specializationParam = convert(inTypeParam);
    if(auto genericParamLayout = as<GenericSpecializationParamLayout>(specializationParam))
    {
        if( auto globalGenericParamDecl = as<GlobalGenericParamDecl>(genericParamLayout->decl) )
        {
            auto constraints = globalGenericParamDecl->getMembersOfType<GenericTypeConstraintDecl>();
            return (unsigned int)constraints.getCount();
        }
        // TODO: Add case for entry-point generic parameters.
    }
    // TODO: Add case for existential type parameters.
    return 0;
}

SLANG_API SlangReflectionType* spReflectionTypeParameter_GetConstraintByIndex(SlangReflectionTypeParameter * inTypeParam, unsigned index)
{
    auto specializationParam = convert(inTypeParam);
    if(auto genericParamLayout = as<GenericSpecializationParamLayout>(specializationParam))
    {
        if( auto globalGenericParamDecl = as<GlobalGenericParamDecl>(genericParamLayout->decl) )
        {
            auto constraints = globalGenericParamDecl->getMembersOfType<GenericTypeConstraintDecl>();
            return (SlangReflectionType*)constraints[index]->sup.Ptr();
        }
        // TODO: Add case for entry-point generic parameters.
    }
    // TODO: Add case for existential type parameters.
    return 0;
}

// Shader Reflection

SLANG_API unsigned spReflection_GetParameterCount(SlangReflection* inProgram)
{
    auto program = convert(inProgram);
    if(!program) return 0;

    auto globalStructLayout = getGlobalStructLayout(program);
    if (!globalStructLayout)
        return 0;

    return (unsigned) globalStructLayout->fields.getCount();
}

SLANG_API SlangReflectionParameter* spReflection_GetParameterByIndex(SlangReflection* inProgram, unsigned index)
{
    auto program = convert(inProgram);
    if(!program) return nullptr;

    auto globalStructLayout = getGlobalStructLayout(program);
    if (!globalStructLayout)
        return 0;

    return convert(globalStructLayout->fields[index].Ptr());
}

SLANG_API SlangReflectionVariableLayout* spReflection_getGlobalParamsVarLayout(SlangReflection* inProgram)
{
    auto program = convert(inProgram);
    if(!program) return nullptr;

    return convert(program->parametersLayout);
}

SLANG_API unsigned int spReflection_GetTypeParameterCount(SlangReflection * reflection)
{
    auto program = convert(reflection);
    return (unsigned int) program->specializationParams.getCount();
}

SLANG_API SlangReflectionTypeParameter* spReflection_GetTypeParameterByIndex(SlangReflection * reflection, unsigned int index)
{
    auto program = convert(reflection);
    return (SlangReflectionTypeParameter*) program->specializationParams[index].Ptr();
}

SLANG_API SlangReflectionTypeParameter * spReflection_FindTypeParameter(SlangReflection * inProgram, char const * name)
{
    auto program = convert(inProgram);
    if (!program) return nullptr;
    for( auto& param : program->specializationParams )
    {
        auto genericParamLayout = as<GenericSpecializationParamLayout>(param);
        if(!genericParamLayout)
            continue;

        if(getText(genericParamLayout->decl->getName()) != UnownedTerminatedStringSlice(name))
            continue;

        return (SlangReflectionTypeParameter*) genericParamLayout;
    }

    return 0;
}

SLANG_API SlangUInt spReflection_getEntryPointCount(SlangReflection* inProgram)
{
    auto program = convert(inProgram);
    if(!program) return 0;

    return SlangUInt(program->entryPoints.getCount());
}

SLANG_API SlangReflectionEntryPoint* spReflection_getEntryPointByIndex(SlangReflection* inProgram, SlangUInt index)
{
    auto program = convert(inProgram);
    if(!program) return 0;

    return convert(program->entryPoints[(int) index].Ptr());
}

SLANG_API SlangReflectionEntryPoint* spReflection_findEntryPointByName(SlangReflection* inProgram, char const* name)
{
    auto program = convert(inProgram);
    if(!program) return 0;

    // TODO: improve on naive linear search
    for(auto ep : program->entryPoints)
    {
        if(ep->entryPoint.getName()->text == name)
        {
            return convert(ep);
        }
    }

    return nullptr;
}

SLANG_API SlangUInt spReflection_getGlobalConstantBufferBinding(SlangReflection* inProgram)
{
    auto program = convert(inProgram);
    if (!program) return 0;
    auto cb = program->parametersLayout->FindResourceInfo(LayoutResourceKind::ConstantBuffer);
    if (!cb) return 0;
    return cb->index;
}

SLANG_API size_t spReflection_getGlobalConstantBufferSize(SlangReflection* inProgram)
{
    auto program = convert(inProgram);
    if (!program) return 0;
    auto structLayout = getGlobalStructLayout(program);
    auto uniform = structLayout->FindResourceInfo(LayoutResourceKind::Uniform);
    if (!uniform) return 0;
    return getReflectionSize(uniform->count);
}

SLANG_API  SlangReflectionType* spReflection_specializeType(
    SlangReflection*            inProgramLayout,
    SlangReflectionType*        inType,
    SlangInt                    specializationArgCount,
    SlangReflectionType* const* specializationArgs,
    ISlangBlob**                outDiagnostics)
{
    auto programLayout = convert(inProgramLayout);
    if(!programLayout) return nullptr;

    auto unspecializedType = convert(inType);
    if(!unspecializedType) return nullptr;

    auto linkage = programLayout->getProgram()->getLinkage();

    DiagnosticSink sink(linkage->getSourceManager());

    auto specializedType = linkage->specializeType(unspecializedType, specializationArgCount, (Type* const*) specializationArgs, &sink);

    sink.getBlobIfNeeded(outDiagnostics);

    return convert(specializedType);
}

SLANG_API SlangUInt spReflection_getHashedStringCount(
    SlangReflection*  reflection)
{
    auto programLayout = convert(reflection);
    auto slices = programLayout->hashedStringLiteralPool.getAdded();
    return slices.getCount();
}

SLANG_API const char* spReflection_getHashedString(
    SlangReflection*  reflection,
    SlangUInt index,
    size_t* outCount)
{
    auto programLayout = convert(reflection);

    auto slices = programLayout->hashedStringLiteralPool.getAdded();
    auto slice = slices[Index(index)];

    *outCount = slice.getLength();
    return slice.begin();
}

SLANG_API int spComputeStringHash(const char* chars, size_t count)
{
    return (int)getStableHashCode32(chars, count);
}

SLANG_API SlangReflectionTypeLayout* spReflection_getGlobalParamsTypeLayout(
    SlangReflection* reflection)
{
    auto programLayout = convert(reflection);
    if(!programLayout) return nullptr;

    return convert(programLayout->parametersLayout->typeLayout);
}
