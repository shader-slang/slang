// slang-reflection-test-main.cpp

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <slang.h>
#include <slang-com-helper.h>

#include "../../source/core/slang-test-tool-util.h"

struct PrettyWriter
{
    struct CommaState
    {
        bool needComma = false;
    };

    bool startOfLine = true;
    int indent = 0;
    CommaState* commaState = nullptr;
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


static void writeHexChar(PrettyWriter& writer, int c)
{
    char v = char(c) + (c < 10 ? '0' : ('a' - 10)); 

    char buffer[] = { v, 0 };
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

static void write(PrettyWriter& writer, char const* text, size_t length = 0)
{
    // TODO: can do this more efficiently...
    char const* cursor = text;
    for(;;)
    {
        char c = *cursor++;
        if (!c) break;
        if (length && cursor - text == length) break;
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

static void writeEscapedString(PrettyWriter& writer, char const* text, size_t length)
{
    adjust(writer);

    writeRawChar(writer, '"');

    for (size_t i = 0; i < length; ++i)
    {
        const char c = text[i];
        switch (c)
        {
            case '\n': write(writer, "\\n"); break;
            case '\t': write(writer, "\\t"); break;
            case '\b': write(writer, "\\b"); break;
            case '\f': write(writer, "\\f"); break;
            case '\0': write(writer, "\\0"); break;
            case '"': write(writer, "\\\""); break;
            default:
            {
                if (c < ' ' || int(c) >= 128)
                {
                    // Not strictly right - as we should decode as a unicode code point and write that.
                    write(writer, "\\u00");
                    writeHexChar(writer, (c >> 4) & 0xf);
                    writeHexChar(writer, c & 0xf);
                }
                else
                {
                    writeRawChar(writer, c);
                }
            }
        }
    }

    writeRawChar(writer, '"');
}

static void write(PrettyWriter& writer, uint64_t val)
{
    adjust(writer);
    Slang::StdWriters::getOut().print("%llu", (unsigned long long)val);
}

static void write(PrettyWriter& writer, int64_t val)
{
    adjust(writer);
    Slang::StdWriters::getOut().print("%ll", (long long)val);
}

static void write(PrettyWriter& writer, int32_t val)
{
    adjust(writer);
    Slang::StdWriters::getOut().print("%d", int(val));
}

static void write(PrettyWriter& writer, uint32_t val)
{
    adjust(writer);
    Slang::StdWriters::getOut().print("%u", (unsigned int)val);
}


static void write(PrettyWriter& writer, float val)
{
    adjust(writer);
    Slang::StdWriters::getOut().print("%f", val);
}

    /// Type for tracking whether a comma is needed in a comma-separated JSON list
struct CommaTrackerRAII
{
    CommaTrackerRAII(PrettyWriter& writer)
        : m_writer(&writer)
        , m_previousState(writer.commaState)
    {
        writer.commaState = &m_state;
    }

    ~CommaTrackerRAII()
    {
        m_writer->commaState = m_previousState;
    }

private:
    PrettyWriter::CommaState    m_state;

    PrettyWriter*               m_writer;
    PrettyWriter::CommaState*   m_previousState;
};

    /// Call before items in a comma-separated JSON list to emit the comma if/when needed
static void comma(PrettyWriter& writer)
{
    if( auto state = writer.commaState )
    {
        if( !state->needComma )
        {
            state->needComma = true;
            return;
        }
    }

    write(writer, ",\n");
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
    slang::VariableLayoutReflection*    var,
    SlangCompileRequest*                request = nullptr,
    int                                 entryPointIndex = -1)
{
    auto stage = var->getStage();
    if (stage != SLANG_STAGE_NONE)
    {
        comma(writer);
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
        comma(writer);
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
            auto category = SlangParameterCategory(var->getCategoryByIndex(cc));
            auto index = var->getOffset(category);
            auto space = var->getBindingSpace(category);
            auto count = typeLayout->getSize(category);

            // Query the paramater usage for the specified entry point.
            // Note: both `request` and `entryPointIndex` may be invalid here, but that should just make the function return a failure.
            bool used = false;
            bool usedAvailable = spIsParameterLocationUsed(request, entryPointIndex, 0, category, space, index, used) == SLANG_OK;

            if (cc != 0) write(writer, ",\n");

            write(writer,"{");
            
            emitReflectionVarBindingInfoJSON(
                writer,
                category,
                index,
                count,
                space);
                
            if (usedAvailable)
            {
                write(writer, ", \"used\": ");
                write(writer, used);
            }

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
        comma(writer);
        write(writer,"\"semanticName\": \"");
        write(writer, semanticName);
        write(writer, "\"");

        if (auto semanticIndex = var->getSemanticIndex())
        {
            comma(writer);
            write(writer,"\"semanticIndex\": ");
            write(writer, int(semanticIndex));
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
        comma(writer);
        write(writer, "\"shared\": true");
    }
}

static void emitUserAttributeJSON(PrettyWriter& writer, slang::UserAttribute* userAttribute)
{
    write(writer, "{\n");
    indent(writer);
    write(writer, "\"name\": \"");
    write(writer, userAttribute->getName());
    write(writer, "\",\n");
    write(writer, "\"arguments\": [\n");
    indent(writer);
    for (unsigned int i = 0; i < userAttribute->getArgumentCount(); i++)
    {
        int intVal;
        float floatVal;
        size_t bufSize = 0;
        if (i > 0)
            write(writer, ",\n");
        if (SLANG_SUCCEEDED(userAttribute->getArgumentValueInt(i, &intVal)))
        {
            write(writer, intVal);
        }
        else if (SLANG_SUCCEEDED(userAttribute->getArgumentValueFloat(i, &floatVal)))
        {
            write(writer, floatVal);
        }
        else if (auto str = userAttribute->getArgumentValueString(i, &bufSize))
        {
            write(writer, str, bufSize);
        }
        else
            write(writer, "\"invalid value\"");
    }
    dedent(writer);
    write(writer, "\n]\n");
    dedent(writer);
    write(writer, "}\n");
}

static void emitUserAttributes(PrettyWriter& writer, slang::TypeReflection* type)
{
    auto attribCount = type->getUserAttributeCount();
    if (attribCount)
    {
        write(writer, ",\n\"userAttribs\": [");
        for (unsigned int i = 0; i < attribCount; i++)
        {
            if (i > 0)
                write(writer, ",\n");
            auto attrib = type->getUserAttributeByIndex(i);
            emitUserAttributeJSON(writer, attrib);
        }
        write(writer, "]");
    }
}
static void emitUserAttributes(PrettyWriter& writer, slang::VariableReflection* var)
{
    auto attribCount = var->getUserAttributeCount();
    if (attribCount)
    {
        write(writer, ",\n\"userAttribs\": [");
        for (unsigned int i = 0; i < attribCount; i++)
        {
            if (i > 0)
                write(writer, ",\n");
            auto attrib = var->getUserAttributeByIndex(i);
            emitUserAttributeJSON(writer, attrib);
        }
        write(writer, "]");
    }
}

static void emitReflectionVarLayoutJSON(
    PrettyWriter&                       writer,
    slang::VariableLayoutReflection*    var)
{
    write(writer, "{\n");
    indent(writer);

    CommaTrackerRAII commaTracker(writer);

    if( auto name = var->getName() )
    {
        comma(writer);
        emitReflectionNameInfoJSON(writer, name);
    }

    comma(writer);
    write(writer, "\"type\": ");
    emitReflectionTypeLayoutJSON(writer, var->getTypeLayout());

    emitReflectionModifierInfoJSON(writer, var->getVariable());

    emitReflectionVarBindingInfoJSON(writer, var);

    emitUserAttributes(writer, var->getVariable());
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

        CASE(Int8, int8);
        CASE(UInt8, uint8);
        CASE(Int16, int16);
        CASE(UInt16, uint16);
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

static void emitReflectionResourceTypeBaseInfoJSON(
    PrettyWriter&           writer,
    slang::TypeReflection*  type)
{
    auto shape  = type->getResourceShape();
    auto access = type->getResourceAccess();
    comma(writer);
    write(writer, "\"kind\": \"resource\"");
    comma(writer);
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
        comma(writer);
        write(writer, "\"array\": true");
    }
    if (shape & SLANG_TEXTURE_MULTISAMPLE_FLAG)
    {
        comma(writer);
        write(writer, "\"multisample\": true");
    }
    if (shape & SLANG_TEXTURE_FEEDBACK_FLAG)
    {
        comma(writer);
        write(writer, "\"feedback\": true");
    }

    if( access != SLANG_RESOURCE_ACCESS_READ )
    {
        comma(writer);
        write(writer, "\"access\": \"");
        switch(access)
        {
        default:
            write(writer, "unknown");
            assert(!"unhandled case");
            break;

        case SLANG_RESOURCE_ACCESS_READ:
            break;
        case SLANG_RESOURCE_ACCESS_WRITE:           write(writer, "write"); break;
        case SLANG_RESOURCE_ACCESS_READ_WRITE:      write(writer, "readWrite"); break;
        case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:  write(writer, "rasterOrdered"); break;
        case SLANG_RESOURCE_ACCESS_APPEND:          write(writer, "append"); break;
        case SLANG_RESOURCE_ACCESS_CONSUME:         write(writer, "consume"); break;
        }
        write(writer, "\"");
    }
}


static void emitReflectionTypeInfoJSON(
    PrettyWriter&           writer,
    slang::TypeReflection*  type)
{
    auto kind = type->getKind();
    switch(kind)
    {
    case slang::TypeReflection::Kind::SamplerState:
        comma(writer);
        write(writer, "\"kind\": \"samplerState\"");
        break;

    case slang::TypeReflection::Kind::Resource:
        {
            emitReflectionResourceTypeBaseInfoJSON(writer, type);

            // TODO: We should really print the result type for all resource
            // types, but current test output depends on the old behavior, so
            // we only add result type output for structured buffers at first.
            //
            auto shape  = type->getResourceShape();
            switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
            {
            default:
                break;

            case SLANG_STRUCTURED_BUFFER:
                if( auto resultType = type->getResourceResultType() )
                {
                    comma(writer);
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
        comma(writer);
        write(writer, "\"kind\": \"constantBuffer\"");
        comma(writer);
        write(writer, "\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::ParameterBlock:
        comma(writer);
        write(writer, "\"kind\": \"parameterBlock\"");
        comma(writer);
        write(writer, "\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::TextureBuffer:
        comma(writer);
        write(writer, "\"kind\": \"textureBuffer\"");
        comma(writer);
        write(writer, "\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::ShaderStorageBuffer:
        comma(writer);
        write(writer, "\"kind\": \"shaderStorageBuffer\"");
        comma(writer);
        write(writer, "\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::Scalar:
        comma(writer);
        write(writer, "\"kind\": \"scalar\"");
        comma(writer);
        emitReflectionScalarTypeInfoJSON(
            writer,
            SlangScalarType(type->getScalarType()));
        break;

    case slang::TypeReflection::Kind::Vector:
        comma(writer);
        write(writer, "\"kind\": \"vector\"");
        comma(writer);
        write(writer, "\"elementCount\": ");
        write(writer, int(type->getElementCount()));
        comma(writer);
        write(writer, "\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::Matrix:
        comma(writer);
        write(writer, "\"kind\": \"matrix\"");
        comma(writer);
        write(writer, "\"rowCount\": ");
        write(writer, type->getRowCount());
        comma(writer);
        write(writer, "\"columnCount\": ");
        write(writer, type->getColumnCount());
        comma(writer);
        write(writer, "\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::Array:
        {
            auto arrayType = type;
            comma(writer);
            write(writer, "\"kind\": \"array\"");
            comma(writer);
            write(writer, "\"elementCount\": ");
            write(writer, int(arrayType->getElementCount()));
            comma(writer);
            write(writer, "\"elementType\": ");
            emitReflectionTypeJSON(writer, arrayType->getElementType());
        }
        break;

    case slang::TypeReflection::Kind::Struct:
        {
            comma(writer);
            write(writer, "\"kind\": \"struct\"");
            comma(writer);
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
        comma(writer);
        write(writer, "\"kind\": \"GenericTypeParameter\"");
        comma(writer);
        emitReflectionNameInfoJSON(writer, type->getName());
        break;
    case slang::TypeReflection::Kind::Interface:
        comma(writer);
        write(writer, "\"kind\": \"Interface\"");
        comma(writer);
        emitReflectionNameInfoJSON(writer, type->getName());
        break;
    case slang::TypeReflection::Kind::Feedback:
        comma(writer);
        write(writer, "\"kind\": \"Feedback\"");
        comma(writer);
        emitReflectionNameInfoJSON(writer, type->getName());
        break;
    default:
        assert(!"unhandled case");
        break;
    }
    emitUserAttributes(writer, type);
}

static void emitReflectionParameterGroupTypeLayoutInfoJSON(
    PrettyWriter&                   writer,
    slang::TypeLayoutReflection*    typeLayout,
    const char*                     kind)
{
    write(writer, "\"kind\": \"");
    write(writer, kind);
    write(writer, "\"");

    write(writer, ",\n\"elementType\": ");
    emitReflectionTypeLayoutJSON(
        writer,
        typeLayout->getElementTypeLayout());

    // Note: There is a subtle detail below when it comes to the
    // container/element variable layouts that get nested inside
    // a parameter group type layout.
    //
    // A top-level parameter group type layout like `ConstantBuffer<Foo>`
    // needs to store both information about the `ConstantBuffer` part of
    // things (e.g., it might consume 1 `binding`), as well as the `Foo`
    // part (e.g., it might consume 4 bytes plus 1 `binding`), and there
    // is offset information for each.
    //
    // The "element" part is easy: it is a variable layout for a variable
    // of type `Foo`. The actual variable will be null, but everything else
    // will be filled in as a client would expect.
    //
    // The "container" part is thornier: what should the type and type
    // layout of the "container" variable be? The obvious answer (which
    // the Slang reflection implementation uses today) is that the type
    // is the type of the parameter group itself (e.g., `ConstantBuffer<Foo>`),
    // and the layout is a dummy `TypeLayout` that just reflects the
    // resource usage of the "container" part of things.
    //
    // That means that at runtime the "container var layout" will have
    // a parameter group type (e.g., `TYPE_KIND_CONSTANT_BUFFER`)
    // but its type layotu will be a base `TypeLayout` and not a
    // `ParameterGroupLayout` (since that would introduce infinite regress).
    //
    // We thus have to guard here against the recursive path where
    // we are emitting reflection info for the "container" part of things.
    //
    // TODO: We should probably 

    {
        CommaTrackerRAII commaTracker(writer);

        write(writer, ",\n\"containerVarLayout\": {\n");
        indent(writer);
        emitReflectionVarBindingInfoJSON(writer, typeLayout->getContainerVarLayout());
        dedent(writer);
        write(writer, "\n}");
    }

    write(writer, ",\n\"elementVarLayout\": ");
    emitReflectionVarLayoutJSON(
        writer,
        typeLayout->getElementVarLayout());
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
            comma(writer);
            write(writer, "\"kind\": \"array\"");

            comma(writer);
            write(writer, "\"elementCount\": ");
            write(writer, int(arrayTypeLayout->getElementCount()));

            comma(writer);
            write(writer, "\"elementType\": ");
            emitReflectionTypeLayoutJSON(
                writer,
                elementTypeLayout);

            if (arrayTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) != 0)
            {
                comma(writer);
                write(writer, "\"uniformStride\": ");
                write(writer, int(arrayTypeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM)));
            }
        }
        break;

    case slang::TypeReflection::Kind::Struct:
        {
            auto structTypeLayout = typeLayout;

            comma(writer);
            write(writer, "\"kind\": \"struct\"");
            if( auto name = structTypeLayout->getName() )
            {
                comma(writer);
                emitReflectionNameInfoJSON(writer, structTypeLayout->getName());
            }
            comma(writer);
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
            emitUserAttributes(writer, structTypeLayout->getType());
            
        }
        break;

    case slang::TypeReflection::Kind::ConstantBuffer:
        emitReflectionParameterGroupTypeLayoutInfoJSON(writer, typeLayout, "constantBuffer");
        break;

    case slang::TypeReflection::Kind::ParameterBlock:
        emitReflectionParameterGroupTypeLayoutInfoJSON(writer, typeLayout, "parameterBlock");
        break;

    case slang::TypeReflection::Kind::TextureBuffer:
        emitReflectionParameterGroupTypeLayoutInfoJSON(writer, typeLayout, "textureBuffer");
        break;

    case slang::TypeReflection::Kind::ShaderStorageBuffer:
        comma(writer);
        write(writer, "\"kind\": \"shaderStorageBuffer\"");

        comma(writer);
        write(writer, "\"elementType\": ");
        emitReflectionTypeLayoutJSON(
            writer,
            typeLayout->getElementTypeLayout());
        break;
    case slang::TypeReflection::Kind::GenericTypeParameter:
        comma(writer);
        write(writer, "\"kind\": \"GenericTypeParameter\"");

        comma(writer);
        emitReflectionNameInfoJSON(writer, typeLayout->getName());
        break;
    case slang::TypeReflection::Kind::Interface:
        comma(writer);
        write(writer, "\"kind\": \"Interface\"");

        comma(writer);
        emitReflectionNameInfoJSON(writer, typeLayout->getName());
        break;

    case slang::TypeReflection::Kind::Resource:
        {
            // Some resource types (notably structured buffers)
            // encode layout information for their result/element
            // type, but others don't. We need to check for
            // the relevant cases here.
            //
            auto type = typeLayout->getType();
            auto shape = type->getResourceShape();

            const auto baseType = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;

            if (baseType == SLANG_STRUCTURED_BUFFER)
            {
                emitReflectionResourceTypeBaseInfoJSON(writer, type);

                if( auto resultTypeLayout = typeLayout->getElementTypeLayout() )
                {
                    comma(writer);
                    write(writer, "\"resultType\": ");
                    emitReflectionTypeLayoutJSON(
                        writer,
                        resultTypeLayout);
                }
            }
            else if (shape & SLANG_TEXTURE_FEEDBACK_FLAG)
            {
                emitReflectionResourceTypeBaseInfoJSON(writer, type);

                if (auto resultType = typeLayout->getResourceResultType())
                {
                    comma(writer);
                    write(writer, "\"resultType\": ");
                    emitReflectionTypeJSON(writer, resultType);
                }
            }
            else
            {
                emitReflectionTypeInfoJSON(writer, type);
            }
        }
        break;
    }
}

static void emitReflectionTypeLayoutJSON(
    PrettyWriter&                   writer,
    slang::TypeLayoutReflection*    typeLayout)
{
    CommaTrackerRAII commaTracker(writer);
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
    CommaTrackerRAII commaTracker(writer);
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
    // TODO: This function is likely redundant with `emitReflectionVarLayoutJSON`
    // and we should try to collapse them into one.

    write(writer, "{\n");
    indent(writer);

    CommaTrackerRAII commaTracker(writer);

    if( auto name = param->getName() )
    {
        comma(writer);
        emitReflectionNameInfoJSON(writer, name);
    }

    emitReflectionModifierInfoJSON(writer, param->getVariable());

    emitReflectionVarBindingInfoJSON(writer, param);

    comma(writer);
    write(writer, "\"type\": ");
    emitReflectionTypeLayoutJSON(writer, param->getTypeLayout());

    dedent(writer);
    write(writer, "\n}");
}


static void emitEntryPointParamJSON(
    PrettyWriter&                       writer,
    slang::VariableLayoutReflection*    param,
    SlangCompileRequest*                request,
    int                                 entryPointIndex)
{
    write(writer, "{\n");
    indent(writer);

    if( auto name = param->getName() )
    {
        emitReflectionNameInfoJSON(writer, name);
    }

    emitReflectionVarBindingInfoJSON(writer, param, request, entryPointIndex);

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
        CommaTrackerRAII commaTracker(writer);
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
    SlangCompileRequest*            request,
    slang::ShaderReflection*        programReflection,
    int                             entryPointIndex)
{
    slang::EntryPointReflection* entryPoint = programReflection->getEntryPointByIndex(entryPointIndex);

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
    if( auto resultVarLayout = entryPoint->getResultVarLayout() )
    {
        write(writer, ",\n\"result:\": ");
        emitReflectionParamJSON(writer, resultVarLayout);
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

    // If code generation has been performed, print out the parameter usage by this entry point.
    if ((request->getCompileFlags() & SLANG_COMPILE_FLAG_NO_CODEGEN) == 0)
    {
        write(writer, ",\n\"bindings\": [\n");
        indent(writer);

        auto parameterCount = programReflection->getParameterCount();
        for( auto pp : range(parameterCount) )
        {
            if(pp != 0) write(writer, ",\n");

            auto parameter = programReflection->getParameterByIndex(pp);
            emitEntryPointParamJSON(writer, parameter, request, entryPointIndex);
        }

        dedent(writer);
        write(writer, "\n]");
    }

    dedent(writer);
    write(writer, "\n}");
}

static void emitReflectionJSON(
    PrettyWriter&               writer,
    SlangCompileRequest*        request,
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

            emitReflectionEntryPointJSON(writer, request, programReflection, (int)ee);
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

    {
        SlangUInt count = programReflection->getHashedStringCount();
        if (count)
        {
            write(writer, ",\n\"hashedStrings\": {\n");
            indent(writer);

            for (SlangUInt i = 0; i < count; ++i)
            {
                if (i)
                {
                    write(writer, ",\n");
                }

                size_t charsCount;
                const char* chars = programReflection->getHashedString(i, &charsCount);
                const int hash = spComputeStringHash(chars, charsCount);

                writeEscapedString(writer, chars, charsCount);
                write(writer, ": ");

                write(writer, hash);
            }

            dedent(writer);
            write(writer, "\n}\n");
        }
    }

    dedent(writer);
    write(writer, "\n}\n");
}

void emitReflectionJSON(
    SlangCompileRequest* request,
    SlangReflection*    reflection)
{
    auto programReflection = (slang::ShaderReflection*) reflection;

    PrettyWriter writer;
    
    emitReflectionJSON(writer, request, programReflection);
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

SlangResult performCompilationAndReflection(SlangCompileRequest* request, int argc, const char*const* argv)
{
    SLANG_RETURN_ON_FAIL(maybeDumpDiagnostic(spProcessCommandLineArguments(request, &argv[1], argc - 1), request));
    SLANG_RETURN_ON_FAIL(maybeDumpDiagnostic(spCompile(request), request));

    // Okay, let's go through and emit reflection info on whatever
    // we have.

    SlangReflection* reflection = spGetReflection(request);
    emitReflectionJSON(request, reflection);

    return SLANG_OK;
}

SLANG_TEST_TOOL_API SlangResult innerMain(Slang::StdWriters* stdWriters, SlangSession* session, int argc, const char*const* argv)
{
    Slang::StdWriters::setSingleton(stdWriters);
    
    SlangCompileRequest* request = spCreateCompileRequest(session);
    for (int i = 0; i < SLANG_WRITER_CHANNEL_COUNT_OF; ++i)
    {
        const auto channel = SlangWriterChannel(i);
        spSetWriter(request, channel, stdWriters->getWriter(channel));
    }

    char const* appName = "slang-reflection-test";
    if (argc > 0) appName = argv[0];

    SlangResult res = performCompilationAndReflection(request, argc, argv);

    spDestroyCompileRequest(request);

    return res;
}

int main(
    int argc,
    char** argv)
{
    using namespace Slang;

    SlangSession* session = spCreateSession(nullptr);

    auto stdWriters = StdWriters::initDefaultSingleton();
    
    SlangResult res = innerMain(stdWriters, session, argc, argv);
    spDestroySession(session);

    return SLANG_FAILED(res) ? 1 : 0;
}
