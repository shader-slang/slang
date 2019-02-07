// reflection.cpp
#include "reflection.h"

#include "compiler.h"
#include "type-layout.h"
#include "syntax.h"
#include <assert.h>

// Don't signal errors for stuff we don't implement here,
// and instead just try to return things defensively
//
// Slang developers can switch this when debugging.
#define SLANG_REFLECTION_UNEXPECTED() do {} while(0)

// Implementation to back public-facing reflection API

using namespace Slang;


// Conversion routines to help with strongly-typed reflection API
static inline Session* convert(SlangSession* session)
{
    return (Session*)session;
}

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

static inline GenericParamLayout* convert(SlangReflectionTypeParameter * typeParam)
{
    return (GenericParamLayout*)typeParam;
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

// user attaribute

unsigned int getUserAttributeCount(Decl* decl)
{
    unsigned int count = 0;
    for (auto x : decl->GetModifiersOfType<UserDefinedAttribute>())
    {
        SLANG_UNUSED(x);
        count++;
    }
    return count;
}

SlangReflectionUserAttribute* findUserAttributeByName(Session* session, Decl* decl, const char* name)
{
    auto nameObj = session->tryGetNameObj(name);
    for (auto x : decl->GetModifiersOfType<UserDefinedAttribute>())
    {
        if (x->name == nameObj)
            return (SlangReflectionUserAttribute*)(x);
    }
    return nullptr;
}

SlangReflectionUserAttribute* getUserAttributeByIndex(Decl* decl, unsigned int index)
{
    unsigned int id = 0;
    for (auto x : decl->GetModifiersOfType<UserDefinedAttribute>())
    {
        if (id == index)
            return convert(x);
        id++;
    }
    return nullptr;
}

SLANG_API char const* spReflectionUserAttribute_GetName(SlangReflectionUserAttribute* attrib)
{
    auto userAttr = convert(attrib);
    if (!userAttr) return nullptr;
    return userAttr->getName()->text.Buffer();
}
SLANG_API unsigned int spReflectionUserAttribute_GetArgumentCount(SlangReflectionUserAttribute* attrib)
{
    auto userAttr = convert(attrib);
    if (!userAttr) return 0;
    return (unsigned int)userAttr->args.Count();
}
SlangReflectionType* spReflectionUserAttribute_GetArgumentType(SlangReflectionUserAttribute* attrib, unsigned int index)
{
    auto userAttr = convert(attrib);
    if (!userAttr) return nullptr;
    return convert(userAttr->args[index]->type.type.Ptr());
}
SLANG_API SlangResult spReflectionUserAttribute_GetArgumentValueInt(SlangReflectionUserAttribute* attrib, unsigned int index, int * rs)
{
    auto userAttr = convert(attrib);
    if (!userAttr) return SLANG_ERROR_INVALID_PARAMETER;
    if (index >= userAttr->args.Count()) return SLANG_ERROR_INVALID_PARAMETER;
    RefPtr<RefObject> val;
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
    if (index >= userAttr->args.Count()) return SLANG_ERROR_INVALID_PARAMETER;
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
    if (index >= userAttr->args.Count()) return nullptr;
    if (auto cexpr = as<StringLiteralExpr>(userAttr->args[index]))
    {
        if (bufLen)
            *bufLen = cexpr->token.Content.size();
        return cexpr->token.Content.begin();
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
            return GetFields(structDeclRef).Count();
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
            auto fieldDeclRef = GetFields(structDeclRef).ToArray()[index];
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
        return arrayType->ArrayLength ? (size_t) GetIntVal(arrayType->ArrayLength) : 0;
    }
    else if( auto vectorType = as<VectorExpressionType>(type))
    {
        return (size_t) GetIntVal(vectorType->elementCount);
    }

    return 0;
}

SLANG_API SlangReflectionType* spReflectionType_GetElementType(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return nullptr;

    if(auto arrayType = as<ArrayExpressionType>(type))
    {
        return (SlangReflectionType*) arrayType->baseType.Ptr();
    }
    else if( auto constantBufferType = as<ConstantBufferType>(type))
    {
        return convert(constantBufferType->elementType.Ptr());
    }
    else if( auto vectorType = as<VectorExpressionType>(type))
    {
        return convert(vectorType->elementType.Ptr());
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
        return (unsigned int) GetIntVal(matrixType->getRowCount());
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
        return (unsigned int) GetIntVal(matrixType->getColumnCount());
    }
    else if(auto vectorType = as<VectorExpressionType>(type))
    {
        return (unsigned int) GetIntVal(vectorType->elementCount);
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
        type = vectorType->elementType.Ptr();
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
        return findUserAttributeByName(declRefType->getSession(), declRefType->declRef.getDecl(), name);
    }
    return 0;
}

SLANG_API SlangResourceShape spReflectionType_GetResourceShape(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return 0;

    while(auto arrayType = as<ArrayExpressionType>(type))
    {
        type = arrayType->baseType.Ptr();
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

    CASE(HLSLStructuredBufferType,                      SLANG_STRUCTURED_BUFFER,    SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWStructuredBufferType,                    SLANG_STRUCTURED_BUFFER,    SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(HLSLRasterizerOrderedStructuredBufferType,     SLANG_STRUCTURED_BUFFER,    SLANG_RESOURCE_ACCESS_RASTER_ORDERED);
    CASE(HLSLAppendStructuredBufferType,                SLANG_STRUCTURED_BUFFER,    SLANG_RESOURCE_ACCESS_APPEND);
    CASE(HLSLConsumeStructuredBufferType,               SLANG_STRUCTURED_BUFFER,    SLANG_RESOURCE_ACCESS_CONSUME);
    CASE(HLSLByteAddressBufferType,                     SLANG_BYTE_ADDRESS_BUFFER,  SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWByteAddressBufferType,                   SLANG_BYTE_ADDRESS_BUFFER,  SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(HLSLRasterizerOrderedByteAddressBufferType,    SLANG_BYTE_ADDRESS_BUFFER,  SLANG_RESOURCE_ACCESS_RASTER_ORDERED);
    CASE(UntypedBufferResourceType,                     SLANG_BYTE_ADDRESS_BUFFER,  SLANG_RESOURCE_ACCESS_READ);
#undef CASE

    return SLANG_RESOURCE_NONE;
}

SLANG_API SlangResourceAccess spReflectionType_GetResourceAccess(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return 0;

    while(auto arrayType = as<ArrayExpressionType>(type))
    {
        type = arrayType->baseType.Ptr();
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
        if(decl->HasModifier<ImplicitParameterGroupElementTypeModifier>())
            return nullptr;

        return getText(declRef.GetName()).begin();
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
    Slang::DiagnosticSink sink;

    RefPtr<Type> result = program->getTypeFromString(name, &sink);
    return (SlangReflectionType*)result.Ptr();
}

SLANG_API SlangReflectionTypeLayout* spReflection_GetTypeLayout(
    SlangReflection* reflection,
    SlangReflectionType* inType,
    SlangLayoutRules /*rules*/)
{
    auto context = convert(reflection);
    auto type = convert(inType);
    auto targetReq = context->getTargetReq();
    auto layoutContext = getInitialLayoutContextForTarget(targetReq, context);
    RefPtr<TypeLayout> result;
    if (targetReq->getTypeLayouts().TryGetValue(type, result))
        return (SlangReflectionTypeLayout*)result.Ptr();
    result = CreateTypeLayout(layoutContext, type);
    targetReq->getTypeLayouts()[type] = result;
    return (SlangReflectionTypeLayout*)result.Ptr();
}

SLANG_API SlangReflectionType* spReflectionType_GetResourceResultType(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return nullptr;

    while(auto arrayType = as<ArrayExpressionType>(type))
    {
        type = arrayType->baseType.Ptr();
    }

    if (auto textureType = as<TextureTypeBase>(type))
    {
        return convert(textureType->elementType.Ptr());
    }

    // TODO: need a better way to handle this stuff...
#define CASE(TYPE, SHAPE, ACCESS)                                                       \
    else if(as<TYPE>(type)) do {                                                      \
        return convert(as<TYPE>(type)->elementType.Ptr());                            \
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

    return (SlangReflectionType*) typeLayout->type.Ptr();
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

    return nullptr;
}

SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_GetElementVarLayout(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return nullptr;

    if( auto constantBufferTypeLayout = as<ParameterGroupTypeLayout>(typeLayout))
    {
        return convert(constantBufferTypeLayout->elementVarLayout.Ptr());
    }

    return nullptr;
}

static SlangParameterCategory getParameterCategory(
    LayoutResourceKind kind)
{
    return SlangParameterCategory(kind);
}

static SlangParameterCategory getParameterCategory(
    TypeLayout*  typeLayout)
{
    auto resourceInfoCount = typeLayout->resourceInfos.Count();
    if(resourceInfoCount == 1)
    {
        return getParameterCategory(typeLayout->resourceInfos[0].kind);
    }
    else if(resourceInfoCount == 0)
    {
        // TODO: can this ever happen?
        return SLANG_PARAMETER_CATEGORY_NONE;
    }
    return SLANG_PARAMETER_CATEGORY_MIXED;
}

static TypeLayout* maybeGetContainerLayout(TypeLayout* typeLayout)
{
    if (auto parameterGroupTypeLayout = as<ParameterGroupTypeLayout>(typeLayout))
    {
        auto containerTypeLayout = parameterGroupTypeLayout->containerVarLayout->typeLayout;
        if (containerTypeLayout->resourceInfos.Count() != 0)
        {
            return containerTypeLayout;
        }
    }

    return typeLayout;
}

SLANG_API SlangParameterCategory spReflectionTypeLayout_GetParameterCategory(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return SLANG_PARAMETER_CATEGORY_NONE;

    typeLayout = maybeGetContainerLayout(typeLayout);

    return getParameterCategory(typeLayout);
}

SLANG_API unsigned spReflectionTypeLayout_GetCategoryCount(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    typeLayout = maybeGetContainerLayout(typeLayout);

    return (unsigned) typeLayout->resourceInfos.Count();
}

SLANG_API SlangParameterCategory spReflectionTypeLayout_GetCategoryByIndex(SlangReflectionTypeLayout* inTypeLayout, unsigned index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return SLANG_PARAMETER_CATEGORY_NONE;

    typeLayout = maybeGetContainerLayout(typeLayout);

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
        return genericParamTypeLayout->paramIndex;
    }
    else
    {
        return -1;
    }
}


// Variable Reflection

SLANG_API char const* spReflectionVariable_GetName(SlangReflectionVariable* inVar)
{
    auto var = convert(inVar);
    if(!var) return nullptr;

    // If the variable is one that has an "external" name that is supposed
    // to be exposed for reflection, then report it here
    if(auto reflectionNameMod = var->FindModifier<ParameterGroupReflectionName>())
        return getText(reflectionNameMod->nameAndLoc.name).Buffer();

    return getText(var->getName()).Buffer();
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
        modifier = var->FindModifier<HLSLEffectSharedModifier>();
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
    return findUserAttributeByName(convert(session), varDecl, name);
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

namespace Slang
{
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
                if(category == SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER)
                    return SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT;
                break;

            case SLANG_TYPE_KIND_RESOURCE:
                if(category == SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE)
                    return SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT;
                break;

            case SLANG_TYPE_KIND_SAMPLER_STATE:
                if(category == SLANG_PARAMETER_CATEGORY_SAMPLER_STATE)
                    return SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT;
                break;

            // TODO: implement more helpers here

            default:
                break;
            }
        }

        return category;
    }
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

    // Next, deal with any dedicated register-space offset applied to, e.g., a parameter block
    if (auto spaceInfo = varLayout->FindResourceInfo(LayoutResourceKind::RegisterSpace))
    {
        space += spaceInfo->index;
    }

    return space;
}

SLANG_API char const* spReflectionVariableLayout_GetSemanticName(SlangReflectionVariableLayout* inVarLayout)
{
    auto varLayout = convert(inVarLayout);
    if(!varLayout) return 0;

    if (!(varLayout->flags & Slang::VarLayoutFlag::HasSemantic))
        return 0;

    return varLayout->semanticName.Buffer();
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

// Helpers for getting parameter count

namespace Slang
{
    static unsigned getParameterCount(RefPtr<TypeLayout> typeLayout)
    {
        if(auto parameterGroupLayout = as<ParameterGroupTypeLayout>(typeLayout))
        {
            typeLayout = parameterGroupLayout->offsetElementTypeLayout;
        }

        if(auto structLayout = as<StructTypeLayout>(typeLayout))
        {
            return (unsigned) structLayout->fields.Count();
        }

        return 0;
    }

    static VarLayout* getParameterByIndex(RefPtr<TypeLayout> typeLayout, unsigned index)
    {
        if(auto parameterGroupLayout = as<ParameterGroupTypeLayout>(typeLayout))
        {
            typeLayout = parameterGroupLayout->offsetElementTypeLayout;
        }

        if(auto structLayout = as<StructTypeLayout>(typeLayout))
        {
            return structLayout->fields[index];
        }

        return 0;
    }
}

// Entry Point Reflection

SLANG_API char const* spReflectionEntryPoint_getName(
    SlangReflectionEntryPoint* inEntryPoint)
{
    auto entryPointLayout = convert(inEntryPoint);
    if(!entryPointLayout) return 0;

    return getText(entryPointLayout->entryPoint->getName()).begin();
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

    return SlangStage(entryPointLayout->profile.GetStage());
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
    auto numThreadsAttribute = entryPointFunc->FindModifier<NumThreadsAttribute>();
    if (numThreadsAttribute)
    {
        sizeAlongAxis[0] = numThreadsAttribute->x;
        sizeAlongAxis[1] = numThreadsAttribute->y;
        sizeAlongAxis[2] = numThreadsAttribute->z;
    }
    else
    {
        // Fall back to the GLSL case, which requires a search over global-scope declarations
        // to look for as with the `local_size_*` qualifier
        auto module = as<ModuleDecl>(entryPointFunc->ParentDecl);
        if (module)
        {
            for (auto dd : module->Members)
            {
                for (auto mod : dd->GetModifiersOfType<GLSLLocalSizeLayoutModifier>())
                {
                    if (auto xMod = as<GLSLLocalSizeXLayoutModifier>(mod))
                        sizeAlongAxis[0] = (SlangUInt) getIntegerLiteralValue(xMod->valToken);
                    else if (auto yMod = as<GLSLLocalSizeYLayoutModifier>(mod))
                        sizeAlongAxis[1] = (SlangUInt) getIntegerLiteralValue(yMod->valToken);
                    else if (auto zMod = as<GLSLLocalSizeZLayoutModifier>(mod))
                        sizeAlongAxis[2] = (SlangUInt) getIntegerLiteralValue(zMod->valToken);
                }
            }
        }
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

    if (entryPointLayout->profile.GetStage() != Stage::Fragment)
        return 0;

    return (entryPointLayout->flags & EntryPointLayout::Flag::usesAnySampleRateInput) != 0;
}

// SlangReflectionTypeParameter
SLANG_API char const* spReflectionTypeParameter_GetName(SlangReflectionTypeParameter * inTypeParam)
{
    auto typeParam = convert(inTypeParam);
    return typeParam->decl->getName()->text.Buffer();
}

SLANG_API unsigned spReflectionTypeParameter_GetIndex(SlangReflectionTypeParameter * inTypeParam)
{
    auto typeParam = convert(inTypeParam);
    return (unsigned)(typeParam->index);
}

SLANG_API unsigned int spReflectionTypeParameter_GetConstraintCount(SlangReflectionTypeParameter* inTypeParam)
{
    auto typeParam = convert(inTypeParam);
    auto constraints = typeParam->decl->getMembersOfType<GenericTypeConstraintDecl>();
    return (unsigned int)constraints.Count();
}

SLANG_API SlangReflectionType* spReflectionTypeParameter_GetConstraintByIndex(SlangReflectionTypeParameter * inTypeParam, unsigned index)
{
    auto typeParam = convert(inTypeParam);
    auto constraints = typeParam->decl->getMembersOfType<GenericTypeConstraintDecl>();
    return (SlangReflectionType*)constraints.ToArray()[index]->sup.Ptr();
}

// Shader Reflection

SLANG_API unsigned spReflection_GetParameterCount(SlangReflection* inProgram)
{
    auto program = convert(inProgram);
    if(!program) return 0;

    auto globalStructLayout = getGlobalStructLayout(program);
    if (!globalStructLayout)
        return 0;

    return (unsigned) globalStructLayout->fields.Count();
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

SLANG_API unsigned int spReflection_GetTypeParameterCount(SlangReflection * reflection)
{
    auto program = convert(reflection);
    return (unsigned int)program->globalGenericParams.Count();
}

SLANG_API SlangReflectionTypeParameter* spReflection_GetTypeParameterByIndex(SlangReflection * reflection, unsigned int index)
{
    auto program = convert(reflection);
    return (SlangReflectionTypeParameter*)program->globalGenericParams[index].Ptr();
}

SLANG_API SlangReflectionTypeParameter * spReflection_FindTypeParameter(SlangReflection * inProgram, char const * name)
{
    auto program = convert(inProgram);
    if (!program) return nullptr;
    GenericParamLayout * result = nullptr;
    program->globalGenericParamsMap.TryGetValue(name, result);
    return (SlangReflectionTypeParameter*)result;
}

SLANG_API SlangUInt spReflection_getEntryPointCount(SlangReflection* inProgram)
{
    auto program = convert(inProgram);
    if(!program) return 0;

    return SlangUInt(program->entryPoints.Count());
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

    // TODO: improve on dumb linear search
    for(auto ep : program->entryPoints)
    {
        if(ep->entryPoint->getName()->text == name)
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
