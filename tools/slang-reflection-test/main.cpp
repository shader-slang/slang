// main.cpp

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <slang.h>
#include <slang-com-helper.h>

#include "../../source/core/slang-test-tool-util.h"

struct PrettyWriter
{
    bool startOfLine = true;
    int indent = 0;
};

static void writeRaw(PrettyWriter& writer, char const* begin, char const* end)
{
    SLANG_ASSERT(end >= begin);
    Slang::StdWriters::getOut().write(begin, size_t(end - begin));
}

static void writeRaw(PrettyWriter& writer, char const* begin)
{
    writeRaw(writer, begin, begin + strlen(begin));
}

static void writeRawChar(PrettyWriter& writer, int c)
{
    char buffer[] = { (char) c, 0 };
    writeRaw(writer, buffer, buffer + 1);
}

static void adjust(PrettyWriter& writer)
{
    if (!writer.startOfLine)
        return;

    int indent = writer.indent;
    for (int ii = 0; ii < indent; ++ii)
        writeRaw(writer, "    ");

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

        writeRawChar(writer, c);
    }
}

static void write(PrettyWriter& writer, SlangUInt val)
{
    adjust(writer);
    Slang::StdWriters::getOut().print("%llu", (unsigned long long)val);
}

static void emitReflectionVarInfoJSON(PrettyWriter& writer, slang::VariableReflection* var);
static void emitReflectionTypeLayoutJSON(PrettyWriter& writer, slang::TypeLayoutReflection* type);
static void emitReflectionTypeJSON(PrettyWriter& writer, slang::TypeReflection* type);

static void emitReflectionVarBindingInfoJSON(
    PrettyWriter&           writer,
    SlangParameterCategory  category,
    SlangUInt               index,
    SlangUInt               count,
    SlangUInt               space = 0)
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
    CASE(VARYING_INPUT, varyingInput);
    CASE(VARYING_OUTPUT, varyingOutput);
    CASE(SAMPLER_STATE, samplerState);
    CASE(UNIFORM, uniform);
    CASE(PUSH_CONSTANT_BUFFER, pushConstantBuffer);
    CASE(DESCRIPTOR_TABLE_SLOT, descriptorTableSlot);
    CASE(SPECIALIZATION_CONSTANT, specializationConstant);
    CASE(MIXED, mixed);
    CASE(REGISTER_SPACE, registerSpace);
    CASE(GENERIC, generic);
    #undef CASE

        default:
            write(writer, "unknown");
            assert(!"unhandled case");
            break;
        }
        write(writer, "\"");
        if( space && category != SLANG_PARAMETER_CATEGORY_REGISTER_SPACE)
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
            if( count == SLANG_UNBOUNDED_SIZE )
            {
                write(writer, "\"unbounded\"");
            }
            else
            {
                write(writer, count);
            }
        }
    }
}

static void emitReflectionVarBindingInfoJSON(
    PrettyWriter&                       writer,
    slang::VariableLayoutReflection*    var)
{
    auto stage = var->getStage();
    if (stage != SLANG_STAGE_NONE)
    {
        write(writer, ",\n");
        char const* stageName = "UNKNOWN";
        switch (stage)
        {
        case SLANG_STAGE_VERTEX:    stageName = "vertex";   break;
        case SLANG_STAGE_HULL:      stageName = "hull";     break;
        case SLANG_STAGE_DOMAIN:    stageName = "domain";   break;
        case SLANG_STAGE_GEOMETRY:  stageName = "geometry"; break;
        case SLANG_STAGE_FRAGMENT:  stageName = "fragment"; break;
        case SLANG_STAGE_COMPUTE:   stageName = "compute";  break;

        default:
            break;
        }

        write(writer, "\"stage\": \"");
        write(writer, stageName);
        write(writer, "\"");
    }

    auto typeLayout = var->getTypeLayout();
    auto categoryCount = var->getCategoryCount();

    if (categoryCount)
    {
        write(writer, ",\n");
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

    if (auto semanticName = var->getSemanticName())
    {
        write(writer, ",\n");
        write(writer,"\"semanticName\": \"");
        write(writer, semanticName);
        write(writer, "\"");

        if (auto semanticIndex = var->getSemanticIndex())
        {
            write(writer, ",\n");
            write(writer,"\"semanticIndex\": ");
            write(writer, semanticIndex);
        }
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

static void emitReflectionModifierInfoJSON(
    PrettyWriter&               writer,
    slang::VariableReflection*  var)
{
    if( var->findModifier(slang::Modifier::Shared) )
    {
        write(writer, ",\n\"shared\": true");
    }
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

    emitReflectionModifierInfoJSON(writer, var->getVariable());

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
        assert(!"unhandled case");
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
    auto kind = type->getKind();
    switch(kind)
    {
    case slang::TypeReflection::Kind::SamplerState:
        write(writer, "\"kind\": \"samplerState\"");
        break;

    case slang::TypeReflection::Kind::Resource:
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
                assert(!"unhandled case");
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
                    assert(!"unhandled case");
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

            // TODO: We should really print the result type for all resource
            // types, but current test output depends on the old behavior, so
            // we only add result type output for structured buffers at first.
            //
            switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
            {
            default:
                break;

            case SLANG_STRUCTURED_BUFFER:
                if( auto resultType = type->getResourceResultType() )
                {
                    write(writer, ",\n");
                    write(writer, "\"resultType\": ");
                    emitReflectionTypeJSON(
                        writer,
                        resultType);
                }
                break;
            }
        }
        break;

    case slang::TypeReflection::Kind::ConstantBuffer:
        write(writer, "\"kind\": \"constantBuffer\"");
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::ParameterBlock:
        write(writer, "\"kind\": \"parameterBlock\"");
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::TextureBuffer:
        write(writer, "\"kind\": \"textureBuffer\"");
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::ShaderStorageBuffer:
        write(writer, "\"kind\": \"shaderStorageBuffer\"");
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::Scalar:
        write(writer, "\"kind\": \"scalar\"");
        write(writer, ",\n");
        emitReflectionScalarTypeInfoJSON(
            writer,
            type->getScalarType());
        break;

    case slang::TypeReflection::Kind::Vector:
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

    case slang::TypeReflection::Kind::Matrix:
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

    case slang::TypeReflection::Kind::Array:
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

    case slang::TypeReflection::Kind::Struct:
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

    case slang::TypeReflection::Kind::GenericTypeParameter:
        write(writer, "\"kind\": \"GenericTypeParameter\",\n");
        emitReflectionNameInfoJSON(writer, type->getName());
        break;
    case slang::TypeReflection::Kind::Interface:
        write(writer, "\"kind\": \"Interface\",\n");
        emitReflectionNameInfoJSON(writer, type->getName());
        break;
    default:
        assert(!"unhandled case");
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

    case slang::TypeReflection::Kind::Array:
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

    case slang::TypeReflection::Kind::Struct:
        {
            auto structTypeLayout = typeLayout;

            write(writer, "\"kind\": \"struct\",\n");
            if( auto name = structTypeLayout->getName() )
            {
                emitReflectionNameInfoJSON(writer, structTypeLayout->getName());
                write(writer, ",\n");
            }
            write(writer, "\"fields\": [\n");
            indent(writer);

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

    case slang::TypeReflection::Kind::ConstantBuffer:
        write(writer, "\"kind\": \"constantBuffer\"");
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeLayoutJSON(
            writer,
            typeLayout->getElementTypeLayout());
        break;

    case slang::TypeReflection::Kind::ParameterBlock:
        write(writer, "\"kind\": \"parameterBlock\"");
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeLayoutJSON(
            writer,
            typeLayout->getElementTypeLayout());
        break;

    case slang::TypeReflection::Kind::TextureBuffer:
        write(writer, "\"kind\": \"textureBuffer\"");
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeLayoutJSON(
            writer,
            typeLayout->getElementTypeLayout());
        break;

    case slang::TypeReflection::Kind::ShaderStorageBuffer:
        write(writer, "\"kind\": \"shaderStorageBuffer\"");
        write(writer, ",\n");
        write(writer, "\"elementType\": ");
        emitReflectionTypeLayoutJSON(
            writer,
            typeLayout->getElementTypeLayout());
        break;
    case slang::TypeReflection::Kind::GenericTypeParameter:
        write(writer, "\"kind\": \"GenericTypeParameter\"");
        write(writer, ",\n");
        emitReflectionNameInfoJSON(writer, typeLayout->getName());
        break;
    case slang::TypeReflection::Kind::Interface:
        write(writer, "\"kind\": \"Interface\",\n");
        write(writer, ",\n");
        emitReflectionNameInfoJSON(writer, typeLayout->getName());
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

    emitReflectionModifierInfoJSON(writer, var);

    write(writer, ",\n");
    write(writer, "\"type\": ");
    emitReflectionTypeJSON(writer, var->getType());
}

static void emitReflectionParamJSON(
    PrettyWriter&                       writer,
    slang::VariableLayoutReflection*    param)
{
    write(writer, "{\n");
    indent(writer);

    emitReflectionNameInfoJSON(writer, param->getName());

    emitReflectionModifierInfoJSON(writer, param->getVariable());

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

static void emitReflectionTypeParamJSON(
    PrettyWriter&                   writer,
    slang::TypeParameterReflection* typeParam)
{
    write(writer, "{\n");
    indent(writer);
    emitReflectionNameInfoJSON(writer, typeParam->getName());
    write(writer, ",\n");
    write(writer, "constraints: \n");
    write(writer, "[\n");
    indent(writer);
    auto constraintCount = typeParam->getConstraintCount();
    for (auto ee : range(constraintCount))
    {
        if (ee != 0) write(writer, ",\n");
        write(writer, "{\n");
        indent(writer);
        emitReflectionTypeInfoJSON(writer, typeParam->getConstraintByIndex(ee));
        dedent(writer);
        write(writer, "\n}");
    }
    dedent(writer);
    write(writer, "\n]");
    dedent(writer);
    write(writer, "\n}");
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

    if (entryPoint->getStage() == SLANG_STAGE_COMPUTE)
    {
        SlangUInt threadGroupSize[3];
        entryPoint->getComputeThreadGroupSize(3, threadGroupSize);

        write(writer, ",\n\"threadGroupSize\": [");
        for (int ii = 0; ii < 3; ++ii)
        {
            if (ii != 0) write(writer, ", ");
            write(writer, threadGroupSize[ii]);
        }
        write(writer, "]");
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

    auto genParamCount = programReflection->getTypeParameterCount();
    if (genParamCount)
    {
        write(writer, ",\n\"typeParams\":\n");
        write(writer, "[\n");
        indent(writer);
        for (auto ee : range(genParamCount))
        {
            if (ee != 0) write(writer, ",\n");

            auto typeParam = programReflection->getTypeParameterByIndex(ee);
            emitReflectionTypeParamJSON(writer, typeParam);
        }
        dedent(writer);
        write(writer, "\n]");
    }
    dedent(writer);
    write(writer, "\n}\n");
}

void emitReflectionJSON(
    SlangReflection*    reflection)
{
    auto programReflection = (slang::ShaderReflection*) reflection;

    PrettyWriter writer;
    
    emitReflectionJSON(writer, programReflection);
}

static SlangResult maybeDumpDiagnostic(SlangResult res, SlangCompileRequest* request)
{
    const char* diagnostic;
    if (SLANG_FAILED(res) && (diagnostic = spGetDiagnosticOutput(request)))
    {
        Slang::StdWriters::getError().put(diagnostic);
    }
    return res;
}

SLANG_TEST_TOOL_API SlangResult innerMain(Slang::StdWriters* stdWriters, SlangSession* session, int argc, const char*const* argv)
{
    Slang::StdWriters::setSingleton(stdWriters);
    
    SlangCompileRequest* request = spCreateCompileRequest(session);

    stdWriters->setRequestWriters(request);

    char const* appName = "slang-reflection-test";
    if (argc > 0) appName = argv[0];

    SLANG_RETURN_ON_FAIL(maybeDumpDiagnostic(spProcessCommandLineArguments(request, &argv[1], argc - 1), request));
    SLANG_RETURN_ON_FAIL(maybeDumpDiagnostic(spCompile(request), request));

    // Okay, let's go through and emit reflection info on whatever
    // we have.

    SlangReflection* reflection = spGetReflection(request);
    emitReflectionJSON(reflection);

    spDestroyCompileRequest(request);
    
    return SLANG_OK;
}

int main(
    int argc,
    char** argv)
{
    SlangSession* session = spCreateSession(nullptr);
    SlangResult res = innerMain(Slang::StdWriters::initDefault(), session, argc, argv);
    spDestroySession(session);

    return SLANG_FAILED(res) ? 1 : 0;
}
