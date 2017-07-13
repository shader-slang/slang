// reflection.cpp
#include "reflection.h"

#include "compiler.h"
#include "type-layout.h"

#include <assert.h>

// Implementation to back public-facing reflection API

using namespace Slang;


// Conversion routines to help with strongly-typed reflection API

static inline ExpressionType* convert(SlangReflectionType* type)
{
    return (ExpressionType*) type;
}

static inline SlangReflectionType* convert(ExpressionType* type)
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

// Type Reflection


SLANG_API SlangTypeKind spReflectionType_GetKind(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return SLANG_TYPE_KIND_NONE;

    // TODO(tfoley: Don't emit the same type more than once...

    if (auto basicType = type->As<BasicExpressionType>())
    {
        return SLANG_TYPE_KIND_SCALAR;
    }
    else if (auto vectorType = type->As<VectorExpressionType>())
    {
        return SLANG_TYPE_KIND_VECTOR;
    }
    else if (auto matrixType = type->As<MatrixExpressionType>())
    {
        return SLANG_TYPE_KIND_MATRIX;
    }
    else if (auto constantBufferType = type->As<ConstantBufferType>())
    {
        return SLANG_TYPE_KIND_CONSTANT_BUFFER;
    }
    else if (type->As<TextureBufferType>())
    {
        return SLANG_TYPE_KIND_TEXTURE_BUFFER;
    }
    else if (type->As<GLSLShaderStorageBufferType>())
    {
        return SLANG_TYPE_KIND_SHADER_STORAGE_BUFFER;
    }
    else if (auto samplerStateType = type->As<SamplerStateType>())
    {
        return SLANG_TYPE_KIND_SAMPLER_STATE;
    }
    else if (auto textureType = type->As<TextureTypeBase>())
    {
        return SLANG_TYPE_KIND_RESOURCE;
    }

    // TODO: need a better way to handle this stuff...
#define CASE(TYPE)                          \
    else if(type->As<TYPE>()) do {          \
        return SLANG_TYPE_KIND_RESOURCE;    \
    } while(0)

    CASE(HLSLBufferType);
    CASE(HLSLRWBufferType);
    CASE(HLSLBufferType);
    CASE(HLSLRWBufferType);
    CASE(HLSLStructuredBufferType);
    CASE(HLSLRWStructuredBufferType);
    CASE(HLSLAppendStructuredBufferType);
    CASE(HLSLConsumeStructuredBufferType);
    CASE(HLSLByteAddressBufferType);
    CASE(HLSLRWByteAddressBufferType);
    CASE(UntypedBufferResourceType);
#undef CASE

    else if (auto arrayType = type->As<ArrayExpressionType>())
    {
        return SLANG_TYPE_KIND_ARRAY;
    }
    else if( auto declRefType = type->As<DeclRefType>() )
    {
        auto declRef = declRefType->declRef;
        if( auto structDeclRef = declRef.As<StructSyntaxNode>() )
        {
            return SLANG_TYPE_KIND_STRUCT;
        }
    }
    else if (auto errorType = type->As<ErrorType>())
    {
        // This means we saw a type we didn't understand in the user's code
        return SLANG_TYPE_KIND_NONE;
    }

    assert(!"unexpected");
    return SLANG_TYPE_KIND_NONE;
}

SLANG_API unsigned int spReflectionType_GetFieldCount(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return 0;

    // TODO: maybe filter based on kind

    if(auto declRefType = dynamic_cast<DeclRefType*>(type))
    {
        auto declRef = declRefType->declRef;
        if( auto structDeclRef = declRef.As<StructSyntaxNode>())
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

    if(auto declRefType = dynamic_cast<DeclRefType*>(type))
    {
        auto declRef = declRefType->declRef;
        if( auto structDeclRef = declRef.As<StructSyntaxNode>())
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

    if(auto arrayType = dynamic_cast<ArrayExpressionType*>(type))
    {
        return (size_t) GetIntVal(arrayType->ArrayLength);
    }
    else if( auto vectorType = dynamic_cast<VectorExpressionType*>(type))
    {
        return (size_t) GetIntVal(vectorType->elementCount);
    }

    return 0;
}

SLANG_API SlangReflectionType* spReflectionType_GetElementType(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return nullptr;

    if(auto arrayType = dynamic_cast<ArrayExpressionType*>(type))
    {
        return (SlangReflectionType*) arrayType->BaseType.Ptr();
    }
    else if( auto constantBufferType = dynamic_cast<ConstantBufferType*>(type))
    {
        return convert(constantBufferType->elementType.Ptr());
    }
    else if( auto vectorType = dynamic_cast<VectorExpressionType*>(type))
    {
        return convert(vectorType->elementType.Ptr());
    }
    else if( auto matrixType = dynamic_cast<MatrixExpressionType*>(type))
    {
        return convert(matrixType->getElementType());
    }

    return nullptr;
}

SLANG_API unsigned int spReflectionType_GetRowCount(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return 0;

    if(auto matrixType = dynamic_cast<MatrixExpressionType*>(type))
    {
        return (unsigned int) GetIntVal(matrixType->getRowCount());
    }
    else if(auto vectorType = dynamic_cast<VectorExpressionType*>(type))
    {
        return 1;
    }
    else if( auto basicType = dynamic_cast<BasicExpressionType*>(type) )
    {
        return 1;
    }

    return 0;
}

SLANG_API unsigned int spReflectionType_GetColumnCount(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return 0;

    if(auto matrixType = dynamic_cast<MatrixExpressionType*>(type))
    {
        return (unsigned int) GetIntVal(matrixType->getColumnCount());
    }
    else if(auto vectorType = dynamic_cast<VectorExpressionType*>(type))
    {
        return (unsigned int) GetIntVal(vectorType->elementCount);
    }
    else if( auto basicType = dynamic_cast<BasicExpressionType*>(type) )
    {
        return 1;
    }

    return 0;
}

SLANG_API SlangScalarType spReflectionType_GetScalarType(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return 0;

    if(auto matrixType = dynamic_cast<MatrixExpressionType*>(type))
    {
        type = matrixType->getElementType();
    }
    else if(auto vectorType = dynamic_cast<VectorExpressionType*>(type))
    {
        type = vectorType->elementType.Ptr();
    }

    if(auto basicType = dynamic_cast<BasicExpressionType*>(type))
    {
        switch (basicType->BaseType)
        {
#define CASE(BASE, TAG) \
        case BaseType::BASE: return SLANG_SCALAR_TYPE_##TAG

            CASE(Void, VOID);
            CASE(Int, INT32);
            CASE(Float, FLOAT32);
            CASE(UInt, UINT32);
            CASE(Bool, BOOL);
            CASE(UInt64, UINT64);

#undef CASE

        default:
            assert(!"unexpected");
            return SLANG_SCALAR_TYPE_NONE;
            break;
        }
    }

    return SLANG_SCALAR_TYPE_NONE;
}

SLANG_API SlangResourceShape spReflectionType_GetResourceShape(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return 0;

    while(auto arrayType = type->As<ArrayExpressionType>())
    {
        type = arrayType->BaseType.Ptr();
    }

    if(auto textureType = type->As<TextureTypeBase>())
    {
        return textureType->getShape();
    }

    // TODO: need a better way to handle this stuff...
#define CASE(TYPE, SHAPE, ACCESS)   \
    else if(type->As<TYPE>()) do {  \
        return SHAPE;               \
    } while(0)

    CASE(HLSLBufferType,                    SLANG_TEXTURE_BUFFER, SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWBufferType,                  SLANG_TEXTURE_BUFFER, SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(HLSLBufferType,                    SLANG_TEXTURE_BUFFER, SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWBufferType,                  SLANG_TEXTURE_BUFFER, SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(HLSLStructuredBufferType,          SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWStructuredBufferType,        SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(HLSLAppendStructuredBufferType,    SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_APPEND);
    CASE(HLSLConsumeStructuredBufferType,   SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_CONSUME);
    CASE(HLSLByteAddressBufferType,         SLANG_BYTE_ADDRESS_BUFFER,  SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWByteAddressBufferType,       SLANG_BYTE_ADDRESS_BUFFER,  SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(UntypedBufferResourceType,         SLANG_BYTE_ADDRESS_BUFFER,  SLANG_RESOURCE_ACCESS_READ);
#undef CASE

    return SLANG_RESOURCE_NONE;
}

SLANG_API SlangResourceAccess spReflectionType_GetResourceAccess(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return 0;

    while(auto arrayType = type->As<ArrayExpressionType>())
    {
        type = arrayType->BaseType.Ptr();
    }

    if(auto textureType = type->As<TextureTypeBase>())
    {
        return textureType->getAccess();
    }

    // TODO: need a better way to handle this stuff...
#define CASE(TYPE, SHAPE, ACCESS)   \
    else if(type->As<TYPE>()) do {  \
        return ACCESS;              \
    } while(0)

    CASE(HLSLBufferType,                    SLANG_TEXTURE_BUFFER, SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWBufferType,                  SLANG_TEXTURE_BUFFER, SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(HLSLBufferType,                    SLANG_TEXTURE_BUFFER, SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWBufferType,                  SLANG_TEXTURE_BUFFER, SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(HLSLStructuredBufferType,          SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWStructuredBufferType,        SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(HLSLAppendStructuredBufferType,    SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_APPEND);
    CASE(HLSLConsumeStructuredBufferType,   SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_CONSUME);
    CASE(HLSLByteAddressBufferType,         SLANG_BYTE_ADDRESS_BUFFER,  SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWByteAddressBufferType,       SLANG_BYTE_ADDRESS_BUFFER,  SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(UntypedBufferResourceType,         SLANG_BYTE_ADDRESS_BUFFER,  SLANG_RESOURCE_ACCESS_READ);

    // This isn't entirely accurate, but I can live with it for now
    CASE(GLSLShaderStorageBufferType,       SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_READ_WRITE);
#undef CASE

    return SLANG_RESOURCE_ACCESS_NONE;
}

SLANG_API SlangReflectionType* spReflectionType_GetResourceResultType(SlangReflectionType* inType)
{
    auto type = convert(inType);
    if(!type) return nullptr;

    while(auto arrayType = type->As<ArrayExpressionType>())
    {
        type = arrayType->BaseType.Ptr();
    }

    if (auto textureType = type->As<TextureTypeBase>())
    {
        return convert(textureType->elementType.Ptr());
    }

    // TODO: need a better way to handle this stuff...
#define CASE(TYPE, SHAPE, ACCESS)                                                       \
    else if(type->As<TYPE>()) do {                                                      \
        return convert(type->As<TYPE>()->elementType.Ptr());                            \
    } while(0)

    CASE(HLSLBufferType,                    SLANG_TEXTURE_BUFFER, SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWBufferType,                  SLANG_TEXTURE_BUFFER, SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(HLSLBufferType,                    SLANG_TEXTURE_BUFFER, SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWBufferType,                  SLANG_TEXTURE_BUFFER, SLANG_RESOURCE_ACCESS_READ_WRITE);

    // TODO: structured buffer needs to expose type layout!

    CASE(HLSLStructuredBufferType,          SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_READ);
    CASE(HLSLRWStructuredBufferType,        SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_READ_WRITE);
    CASE(HLSLAppendStructuredBufferType,    SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_APPEND);
    CASE(HLSLConsumeStructuredBufferType,   SLANG_STRUCTURED_BUFFER, SLANG_RESOURCE_ACCESS_CONSUME);
#undef CASE

    return nullptr;
}

// Type Layout Reflection

SLANG_API SlangReflectionType* spReflectionTypeLayout_GetType(SlangReflectionTypeLayout* inTypeLayout)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return nullptr;

    return (SlangReflectionType*) typeLayout->type.Ptr();
}

SLANG_API size_t spReflectionTypeLayout_GetSize(SlangReflectionTypeLayout* inTypeLayout, SlangParameterCategory category)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    auto info = typeLayout->FindResourceInfo(LayoutResourceKind(category));
    if(!info) return 0;

    return info->count;
}

SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_GetFieldByIndex(SlangReflectionTypeLayout* inTypeLayout, unsigned index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return nullptr;

    if(auto structTypeLayout = dynamic_cast<StructTypeLayout*>(typeLayout))
    {
        return (SlangReflectionVariableLayout*) structTypeLayout->fields[index].Ptr();
    }

    return nullptr;
}

SLANG_API size_t spReflectionTypeLayout_GetElementStride(SlangReflectionTypeLayout* inTypeLayout, SlangParameterCategory category)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return 0;

    if( auto arrayTypeLayout = dynamic_cast<ArrayTypeLayout*>(typeLayout))
    {
        switch (category)
        {
        // We store the stride explictly for the uniform case
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
                return info->count;
            }

        // An import special case, though, is Vulkan descriptor-table slots,
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

    if( auto arrayTypeLayout = dynamic_cast<ArrayTypeLayout*>(typeLayout))
    {
        return (SlangReflectionTypeLayout*) arrayTypeLayout->elementTypeLayout.Ptr();
    }
    else if( auto constantBufferTypeLayout = dynamic_cast<ParameterBlockTypeLayout*>(typeLayout))
    {
        return convert(constantBufferTypeLayout->elementTypeLayout.Ptr());
    }
    else if( auto structuredBufferTypeLayout = dynamic_cast<StructuredBufferTypeLayout*>(typeLayout))
    {
        return convert(structuredBufferTypeLayout->elementTypeLayout.Ptr());
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

    return (unsigned) typeLayout->resourceInfos.Count();
}

SLANG_API SlangParameterCategory spReflectionTypeLayout_GetCategoryByIndex(SlangReflectionTypeLayout* inTypeLayout, unsigned index)
{
    auto typeLayout = convert(inTypeLayout);
    if(!typeLayout) return SLANG_PARAMETER_CATEGORY_NONE;

    return typeLayout->resourceInfos[index].kind;
}

// Variable Reflection

SLANG_API char const* spReflectionVariable_GetName(SlangReflectionVariable* inVar)
{
    auto var = convert(inVar);
    if(!var) return nullptr;

    // If the variable is one that has an "external" name that is supposed
    // to be exposed for reflection, then report it here
    if(auto reflectionNameMod = var->FindModifier<ParameterBlockReflectionName>())
        return reflectionNameMod->nameToken.Content.Buffer();

    return var->getName().Buffer();
}

SLANG_API SlangReflectionType* spReflectionVariable_GetType(SlangReflectionVariable* inVar)
{
    auto var = convert(inVar);
    if(!var) return nullptr;

    return  convert(var->getType());
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
    if(!info) return 0;

    return info->index;
}

SLANG_API size_t spReflectionVariableLayout_GetSpace(SlangReflectionVariableLayout* inVarLayout, SlangParameterCategory category)
{
    auto varLayout = convert(inVarLayout);
    if(!varLayout) return 0;

    auto info = varLayout->FindResourceInfo(LayoutResourceKind(category));
    if(!info) return 0;

    return info->space;
}


// Shader Parameter Reflection

SLANG_API unsigned spReflectionParameter_GetBindingIndex(SlangReflectionParameter* inVarLayout)
{
    auto varLayout = convert(inVarLayout);
    if(!varLayout) return 0;

    if(varLayout->resourceInfos.Count() > 0)
    {
        return (unsigned) varLayout->resourceInfos[0].index;
    }

    return 0;
}

SLANG_API unsigned spReflectionParameter_GetBindingSpace(SlangReflectionParameter* inVarLayout)
{
    auto varLayout = convert(inVarLayout);
    if(!varLayout) return 0;

    if(varLayout->resourceInfos.Count() > 0)
    {
        return (unsigned) varLayout->resourceInfos[0].space;
    }

    return 0;
}

// Helpers for getting parameter count

namespace Slang
{
    static unsigned getParameterCount(RefPtr<TypeLayout> typeLayout)
    {
        if(auto parameterBlockLayout = typeLayout.As<ParameterBlockTypeLayout>())
        {
            typeLayout = parameterBlockLayout->elementTypeLayout;
        }

        if(auto structLayout = typeLayout.As<StructTypeLayout>())
        {
            return (unsigned) structLayout->fields.Count();
        }

        return 0;
    }

    static VarLayout* getParameterByIndex(RefPtr<TypeLayout> typeLayout, unsigned index)
    {
        if(auto parameterBlockLayout = typeLayout.As<ParameterBlockTypeLayout>())
        {
            typeLayout = parameterBlockLayout->elementTypeLayout;
        }

        if(auto structLayout = typeLayout.As<StructTypeLayout>())
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

    return entryPointLayout->entryPoint->getName().begin();
}

SLANG_API unsigned spReflectionEntryPoint_getParameterCount(
    SlangReflectionEntryPoint* inEntryPoint)
{
    auto entryPointLayout = convert(inEntryPoint);
    if(!entryPointLayout) return 0;

    return getParameterCount(entryPointLayout);
}

SLANG_API SlangReflectionVariableLayout* spReflectionEntryPoint_getParameterByIndex(
    SlangReflectionEntryPoint*  inEntryPoint,
    unsigned                    index)
{
    auto entryPointLayout = convert(inEntryPoint);
    if(!entryPointLayout) return 0;

    return convert(getParameterByIndex(entryPointLayout, index));
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

    auto numThreadsAttribute = entryPointFunc->FindModifier<HLSLNumThreadsAttribute>();
    if(!numThreadsAttribute) return;

    if(axisCount > 0) outSizeAlongAxis[0] = numThreadsAttribute->x;
    if(axisCount > 1) outSizeAlongAxis[1] = numThreadsAttribute->y;
    if(axisCount > 2) outSizeAlongAxis[2] = numThreadsAttribute->z;
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

// Shader Reflection

SLANG_API unsigned spReflection_GetParameterCount(SlangReflection* inProgram)
{
    auto program = convert(inProgram);
    if(!program) return 0;

    auto globalLayout = program->globalScopeLayout;
    if(auto globalConstantBufferLayout = globalLayout.As<ParameterBlockTypeLayout>())
    {
        globalLayout = globalConstantBufferLayout->elementTypeLayout;
    }

    if(auto globalStructLayout = globalLayout.As<StructTypeLayout>())
    {
        return (unsigned) globalStructLayout->fields.Count();
    }

    return 0;
}

SLANG_API SlangReflectionParameter* spReflection_GetParameterByIndex(SlangReflection* inProgram, unsigned index)
{
    auto program = convert(inProgram);
    if(!program) return nullptr;

    auto globalLayout = program->globalScopeLayout;
    if(auto globalConstantBufferLayout = globalLayout.As<ParameterBlockTypeLayout>())
    {
        globalLayout = globalConstantBufferLayout->elementTypeLayout;
    }

    if(auto globalStructLayout = globalLayout.As<StructTypeLayout>())
    {
        return convert(globalStructLayout->fields[index].Ptr());
    }

    return nullptr;
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




















namespace Slang {

// Debug helper code: dump reflection data after generation

struct PrettyWriter
{
    StringBuilder sb;
    bool startOfLine = true;
    int indent = 0;
};

static void adjust(PrettyWriter& writer)
{
    if (!writer.startOfLine)
        return;

    int indent = writer.indent;
    for (int ii = 0; ii < indent; ++ii)
        writer.sb << "    ";

    writer.startOfLine = false;
}

static void indent(PrettyWriter& writer)
{
    writer.indent++;
}

static void dedent(PrettyWriter& writer)
{
    writer.indent--;
}

static void write(PrettyWriter& writer, char const* text)
{
    // TODO: can do this more efficiently...
    char const* cursor = text;
    for(;;)
    {
        char c = *cursor++;
        if (!c) break;

        if (c == '\n')
        {
            writer.startOfLine = true;
        }
        else
        {
            adjust(writer);
        }

        writer.sb << c;
    }
}

static void write(PrettyWriter& writer, UInt val)
{
    adjust(writer);
    writer.sb << ((unsigned int) val);
}

static void emitReflectionVarInfoJSON(PrettyWriter& writer, slang::VariableReflection* var);
static void emitReflectionTypeLayoutJSON(PrettyWriter& writer, slang::TypeLayoutReflection* type);
static void emitReflectionTypeJSON(PrettyWriter& writer, slang::TypeReflection* type);

static void emitReflectionVarBindingInfoJSON(
    PrettyWriter&           writer,
    SlangParameterCategory  category,
    UInt                    index,
    UInt                    count,
    UInt                    space = 0)
{
    if( category == SLANG_PARAMETER_CATEGORY_UNIFORM )
    {
        write(writer,"\"kind\": \"uniform\"");
        write(writer, ", ");
        write(writer,"\"offset\": ");
        write(writer, index);
        write(writer, ", ");
        write(writer, "\"size\": ");
        write(writer, count);
    }
    else
    {
        write(writer, "\"kind\": \"");
        switch( category )
        {
    #define CASE(NAME, KIND) case SLANG_PARAMETER_CATEGORY_##NAME: write(writer, #KIND); break
    CASE(CONSTANT_BUFFER, constantBuffer);
    CASE(SHADER_RESOURCE, shaderResource);
    CASE(UNORDERED_ACCESS, unorderedAccess);
    CASE(VERTEX_INPUT, vertexInput);
    CASE(FRAGMENT_OUTPUT, fragmentOutput);
    CASE(SAMPLER_STATE, samplerState);
    CASE(UNIFORM, uniform);
    CASE(DESCRIPTOR_TABLE_SLOT, descriptorTableSlot);
    CASE(SPECIALIZATION_CONSTANT, specializationConstant);
    CASE(MIXED, mixed);
    #undef CASE

        default:
            write(writer, "unknown");
            assert(!"unexpected");
            break;
        }
        write(writer, "\"");
        if( space )
        {
            write(writer, ", ");
            write(writer, "\"space\": ");
            write(writer, space);
        }
        write(writer, ", ");
        write(writer, "\"index\": ");
        write(writer, index);
        if( count != 1)
        {
            write(writer, ", ");
            write(writer, "\"count\": ");
            write(writer, count);
        }
    }
}

static void emitReflectionVarBindingInfoJSON(
    PrettyWriter&                       writer,
    slang::VariableLayoutReflection*    var)
{
    auto typeLayout = var->getTypeLayout();
    auto categoryCount = var->getCategoryCount();

    if( categoryCount != 1 )
    {
        write(writer,"\"bindings\": [\n");
    }
    else
    {
        write(writer,"\"binding\": ");
    }
    indent(writer);

    for(uint32_t cc = 0; cc < categoryCount; ++cc )
    {
        auto category = var->getCategoryByIndex(cc);
        auto index = var->getOffset(category);
        auto space = var->getBindingSpace(category);
        auto count = typeLayout->getSize(category);

        if (cc != 0) write(writer, ",\n");

        write(writer,"{");
        emitReflectionVarBindingInfoJSON(
            writer,
            category,
            index,
            count,
            space);
        write(writer,"}");
    }

    dedent(writer);
    if( categoryCount != 1 )
    {
        write(writer,"\n]");
    }
}

static void emitReflectionNameInfoJSON(
    PrettyWriter&   writer,
    char const*     name)
{
    // TODO: deal with escaping special characters if/when needed
    write(writer, "\"name\": \"");
    write(writer, name);
    write(writer, "\"");
}

static void emitReflectionVarLayoutJSON(
    PrettyWriter&                       writer,
    slang::VariableLayoutReflection*    var)
{
    write(writer, "{\n");
    indent(writer);

    emitReflectionNameInfoJSON(writer, var->getName());
    write(writer, ",\n");

    write(writer, "\"type\": ");
    emitReflectionTypeLayoutJSON(writer, var->getTypeLayout());
    write(writer, ",\n");

    emitReflectionVarBindingInfoJSON(writer, var);

    dedent(writer);
    write(writer, "\n}");
}

static void emitReflectionScalarTypeInfoJSON(
    PrettyWriter&   writer,
    SlangScalarType scalarType)
{
    write(writer, "\"scalarType\": \"");
    switch (scalarType)
    {
    default:
        write(writer, "unknown");
        assert(!"unexpected");
        break;
#define CASE(TAG, ID) case slang::TypeReflection::ScalarType::TAG: write(writer, #ID); break
        CASE(Void, void);
        CASE(Bool, bool);
        CASE(Int32, int32);
        CASE(UInt32, uint32);
        CASE(Int64, int64);
        CASE(UInt64, uint64);
        CASE(Float16, float16);
        CASE(Float32, float32);
        CASE(Float64, float64);
#undef CASE
    }
    write(writer, "\"");
}

static void emitReflectionTypeInfoJSON(
    PrettyWriter&           writer,
    slang::TypeReflection*  type)
{
    switch( type->getKind() )
    {
    case SLANG_TYPE_KIND_SAMPLER_STATE:
        write(writer, "\"kind\": \"samplerState\"");
        break;

    case SLANG_TYPE_KIND_RESOURCE:
        {
            auto shape  = type->getResourceShape();
            auto access = type->getResourceAccess();
            write(writer, "\"kind\": \"resource\"");
            write(writer, ",\n");
            write(writer, "\"baseShape\": \"");
            switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
            {
            default:
                write(writer, "unknown");
                assert(!"unexpected");
                break;

#define CASE(SHAPE, NAME) case SLANG_##SHAPE: write(writer, #NAME); break
                CASE(TEXTURE_1D, texture1D);
                CASE(TEXTURE_2D, texture2D);
                CASE(TEXTURE_3D, texture3D);
                CASE(TEXTURE_CUBE, textureCube);
                CASE(TEXTURE_BUFFER, textureBuffer);
                CASE(STRUCTURED_BUFFER, structuredBuffer);
                CASE(BYTE_ADDRESS_BUFFER, byteAddressBuffer);
#undef CASE
            }
            write(writer, "\"");
            if (shape & SLANG_TEXTURE_ARRAY_FLAG)
            {
                write(writer, ",\n");
                write(writer, "\"array\": true");
            }
            if (shape & SLANG_TEXTURE_MULTISAMPLE_FLAG)
            {
                write(writer, ",\n");
                write(writer, "\"multisample\": true");
            }

            if( access != SLANG_RESOURCE_ACCESS_READ )
            {
                write(writer, ",\n\"access\": \"");
                switch(access)
                {
                default:
                    write(writer, "unknown");
                    assert(!"unexpected");
                    break;

                case SLANG_RESOURCE_ACCESS_READ:
                    break;

                case SLANG_RESOURCE_ACCESS_READ_WRITE:      write(writer, "readWrite"); break;
                case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:  write(writer, "rasterOrdered"); break;
                case SLANG_RESOURCE_ACCESS_APPEND:          write(writer, "append"); break;
                case SLANG_RESOURCE_ACCESS_CONSUME:         write(writer, "consume"); break;
                }
                write(writer, "\"");
            }
        }
        break;

    case SLANG_TYPE_KIND_CONSTANT_BUFFER:
        write(writer, "\"kind\": \"constantBuffer\"");
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case SLANG_TYPE_KIND_TEXTURE_BUFFER:
        write(writer, "\"kind\": \"textureBuffer\"");
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case SLANG_TYPE_KIND_SHADER_STORAGE_BUFFER:
        write(writer, "\"kind\": \"shaderStorageBuffer\"");
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case SLANG_TYPE_KIND_SCALAR:
        write(writer, "\"kind\": \"scalar\"");
        write(writer, ",\n");
        emitReflectionScalarTypeInfoJSON(
            writer,
            type->getScalarType());
        break;

    case SLANG_TYPE_KIND_VECTOR:
        write(writer, "\"kind\": \"vector\"");
        write(writer, ",\n");
        write(writer, "\"elementCount\": ");
        write(writer, type->getElementCount());
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case SLANG_TYPE_KIND_MATRIX:
        write(writer, "\"kind\": \"matrix\"");
        write(writer, ",\n");
        write(writer, "\"rowCount\": ");
        write(writer, type->getRowCount());
        write(writer, ",\n");
        write(writer, "\"columnCount\": ");
        write(writer, type->getColumnCount());
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case SLANG_TYPE_KIND_ARRAY:
        {
            auto arrayType = type;
            write(writer, "\"kind\": \"array\"");
            write(writer, ",\n");
            write(writer, "\"elementCount\": ");
            write(writer, arrayType->getElementCount());
            write(writer, ",\n");
            write(writer, "\"elementType\": ");
            emitReflectionTypeJSON(writer, arrayType->getElementType());
        }
        break;

    case SLANG_TYPE_KIND_STRUCT:
        {
            write(writer, "\"kind\": \"struct\",\n");
            write(writer, "\"fields\": [\n");
            indent(writer);

            auto structType = type;
            auto fieldCount = structType->getFieldCount();
            for( uint32_t ff = 0; ff < fieldCount; ++ff )
            {
                if (ff != 0) write(writer, ",\n");
                emitReflectionVarInfoJSON(
                    writer,
                    structType->getFieldByIndex(ff));
            }
            dedent(writer);
            write(writer, "\n]");
        }
        break;

    default:
        assert(!"unimplemented");
        break;
    }
}

static void emitReflectionTypeLayoutInfoJSON(
    PrettyWriter&                   writer,
    slang::TypeLayoutReflection*    typeLayout)
{
    switch( typeLayout->getKind() )
    {
    default:
        emitReflectionTypeInfoJSON(writer, typeLayout->getType());
        break;

    case SLANG_TYPE_KIND_ARRAY:
        {
            auto arrayTypeLayout = typeLayout;
            auto elementTypeLayout = arrayTypeLayout->getElementTypeLayout();
            write(writer, "\"kind\": \"array\"");
            write(writer, ",\n");
            write(writer, "\"elementCount\": ");
            write(writer, arrayTypeLayout->getElementCount());
            write(writer, ",\n");
            write(writer, "\"elementType\": ");
            emitReflectionTypeLayoutJSON(
                writer,
                elementTypeLayout);
            if (arrayTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) != 0)
            {
                write(writer, ",\n");
                write(writer, "\"uniformStride\": ");
                write(writer, arrayTypeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM));
            }
        }
        break;

    case SLANG_TYPE_KIND_STRUCT:
        {
            write(writer, "\"kind\": \"struct\",\n");
            write(writer, "\"fields\": [\n");
            indent(writer);

            auto structTypeLayout = typeLayout;
            auto fieldCount = structTypeLayout->getFieldCount();
            for( uint32_t ff = 0; ff < fieldCount; ++ff )
            {
                if (ff != 0) write(writer, ",\n");
                emitReflectionVarLayoutJSON(
                    writer,
                    structTypeLayout->getFieldByIndex(ff));
            }
            dedent(writer);
            write(writer, "\n]");
        }
        break;

    case SLANG_TYPE_KIND_CONSTANT_BUFFER:
        write(writer, "\"kind\": \"constantBuffer\"");
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeLayoutJSON(
            writer,
            typeLayout->getElementTypeLayout());
        break;

    case SLANG_TYPE_KIND_TEXTURE_BUFFER:
        write(writer, "\"kind\": \"textureBuffer\"");
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeLayoutJSON(
            writer,
            typeLayout->getElementTypeLayout());
        break;

    case SLANG_TYPE_KIND_SHADER_STORAGE_BUFFER:
        write(writer, "\"kind\": \"shaderStorageBuffer\"");
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeLayoutJSON(
            writer,
            typeLayout->getElementTypeLayout());
        break;
    }

    // TODO: emit size info for types
}

static void emitReflectionTypeLayoutJSON(
    PrettyWriter&                   writer,
    slang::TypeLayoutReflection*    typeLayout)
{
    write(writer, "{\n");
    indent(writer);
    emitReflectionTypeLayoutInfoJSON(writer, typeLayout);
    dedent(writer);
    write(writer, "\n}");
}

static void emitReflectionTypeJSON(
    PrettyWriter&           writer,
    slang::TypeReflection*  type)
{
    write(writer, "{\n");
    indent(writer);
    emitReflectionTypeInfoJSON(writer, type);
    dedent(writer);
    write(writer, "\n}");
}

static void emitReflectionVarInfoJSON(
    PrettyWriter&               writer,
    slang::VariableReflection*  var)
{
    emitReflectionNameInfoJSON(writer, var->getName());
    write(writer, ",\n");

    write(writer, "\"type\": ");
    emitReflectionTypeJSON(writer, var->getType());
}

#if 0
static void emitReflectionBindingInfoJSON(
    PrettyWriter& writer,
    
    ReflectionParameterNode* param)
{
    auto info = &param->binding;

    if( info->category == SLANG_PARAMETER_CATEGORY_MIXED )
    {
        write(writer,"\"bindings\": [\n");
        indent(writer);

        ReflectionSize bindingCount = info->bindingCount;
        assert(bindingCount);
        ReflectionParameterBindingInfo* bindings = info->bindings;
        for( ReflectionSize bb = 0; bb < bindingCount; ++bb )
        {
            if (bb != 0) write(writer, ",\n");

            write(writer,"{");
            auto& binding = bindings[bb];
            emitReflectionVarBindingInfoJSON(
                writer,
                binding.category,
                binding.index,
                (ReflectionSize) param->GetTypeLayout()->GetSize(binding.category),
                binding.space);

            write(writer,"}");
        }
        dedent(writer);
        write(writer,"\n]");
    }
    else
    {
        write(writer,"\"binding\": {");
        indent(writer);

        emitReflectionVarBindingInfoJSON(
            writer,
            info->category,
            info->index,
            (ReflectionSize) param->GetTypeLayout()->GetSize(info->category),
            info->space);

        dedent(writer);
        write(writer,"}");
    }
}
#endif

static void emitReflectionParamJSON(
    PrettyWriter&                       writer,
    slang::VariableLayoutReflection*    param)
{
    write(writer, "{\n");
    indent(writer);

    emitReflectionNameInfoJSON(writer, param->getName());
    write(writer, ",\n");

    emitReflectionVarBindingInfoJSON(writer, param);
    write(writer, ",\n");

    write(writer, "\"type\": ");
    emitReflectionTypeLayoutJSON(writer, param->getTypeLayout());

    dedent(writer);
    write(writer, "\n}");
}

template<typename T>
struct Range
{
public:
    Range(
        T begin,
        T end)
        : mBegin(begin)
        , mEnd(end)
    {}

    struct Iterator
    {
    public:
        explicit Iterator(T value)
            : mValue(value)
        {}

        T operator*() const { return mValue; }
        void operator++() { mValue++; }

        bool operator!=(Iterator const& other)
        {
            return mValue != other.mValue;
        }

    private:
        T mValue;
    };

    Iterator begin() const { return Iterator(mBegin); }
    Iterator end()   const { return Iterator(mEnd); }

private:
    T mBegin;
    T mEnd;
};

template<typename T>
Range<T> range(T begin, T end)
{
    return Range<T>(begin, end);
}

template<typename T>
Range<T> range(T end)
{
    return Range<T>(T(0), end);
}

static void emitReflectionEntryPointJSON(
    PrettyWriter&                   writer,
    slang::EntryPointReflection*    entryPoint)
{
    write(writer, "{\n");
    indent(writer);

    emitReflectionNameInfoJSON(writer, entryPoint->getName());

    switch (entryPoint->getStage())
    {
    case SLANG_STAGE_VERTEX:    write(writer, ",\n\"stage:\": \"vertex\"");     break;
    case SLANG_STAGE_HULL:      write(writer, ",\n\"stage:\": \"hull\"");       break;
    case SLANG_STAGE_DOMAIN:    write(writer, ",\n\"stage:\": \"domain\"");     break;
    case SLANG_STAGE_GEOMETRY:  write(writer, ",\n\"stage:\": \"geometry\"");   break;
    case SLANG_STAGE_FRAGMENT:  write(writer, ",\n\"stage:\": \"fragment\"");   break;
    case SLANG_STAGE_COMPUTE:   write(writer, ",\n\"stage:\": \"compute\"");    break;
    default:
        break;
    }

    auto parameterCount = entryPoint->getParameterCount();
    if (parameterCount)
    {
        write(writer, ",\n\"parameters\": [\n");
        indent(writer);

        for( auto pp : range(parameterCount) )
        {
            if(pp != 0) write(writer, ",\n");

            auto parameter = entryPoint->getParameterByIndex(pp);
            emitReflectionParamJSON(writer, parameter);
        }

        dedent(writer);
        write(writer, "\n]");
    }

    if (entryPoint->usesAnySampleRateInput())
    {
        write(writer, ",\n\"usesAnySampleRateInput\": true");
    }

    dedent(writer);
    write(writer, "\n}");
}

static void emitReflectionJSON(
    PrettyWriter&               writer,
    slang::ShaderReflection*    programReflection)
{
    write(writer, "{\n");
    indent(writer);
    write(writer, "\"parameters\": [\n");
    indent(writer);

    auto parameterCount = programReflection->getParameterCount();
    for( auto pp : range(parameterCount) )
    {
        if(pp != 0) write(writer, ",\n");

        auto parameter = programReflection->getParameterByIndex(pp);
        emitReflectionParamJSON(writer, parameter);
    }

    dedent(writer);
    write(writer, "\n]");

    auto entryPointCount = programReflection->getEntryPointCount();
    if (entryPointCount)
    {
        write(writer, ",\n\"entryPoints\": [\n");
        indent(writer);

        for (auto ee : range(entryPointCount))
        {
            if (ee != 0) write(writer, ",\n");

            auto entryPoint = programReflection->getEntryPointByIndex(ee);
            emitReflectionEntryPointJSON(writer, entryPoint);
        }

        dedent(writer);
        write(writer, "\n]");
    }

    dedent(writer);
    write(writer, "\n}\n");
}

#if 0
ReflectionBlob* ReflectionBlob::Create(
    CollectionOfTranslationUnits*   program)
{
    ReflectionGenerationContext context;
    ReflectionBlob* blob = GenerateReflectionBlob(&context, program);
#if 0
    String debugDump = blob->emitAsJSON();
    OutputDebugStringA("REFLECTION BLOB\n");
    OutputDebugStringA(debugDump.begin());
#endif
    return blob;
}
#endif

// JSON emit logic



String emitReflectionJSON(
    ProgramLayout* programLayout)
{
    auto programReflection = (slang::ShaderReflection*) programLayout;

    PrettyWriter writer;
    emitReflectionJSON(writer, programReflection);
    return writer.sb.ProduceString();
}

}
