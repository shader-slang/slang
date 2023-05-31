// slang-reflection-test-main.cpp

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <slang.h>
#include <slang-com-helper.h>

#include "../../source/core/slang-string-escape-util.h"
#include "../../source/core/slang-char-util.h"

#include "../../source/core/slang-test-tool-util.h"

using namespace Slang;

struct PrettyWriter
{
    struct CommaTrackerRAII;

    struct CommaState
    {
        bool needComma = false;
    };

    void writeRaw(const UnownedStringSlice& out) { m_writer->write(out.begin(), out.getLength()); }
    void writeRaw(char const* begin, char const* end);
    void writeRaw(PrettyWriter& writer, char const* begin) { writeRaw(UnownedStringSlice(begin)); }
    
    void writeRawChar(int c);

    void writeHexChar(int c) { writeRawChar(CharUtil::getHexChar(Index(c))); }

        /// Adjusts indentation if at start of a line
    void adjust();

        /// Increase indentation
    void indent() { m_indent++; }
        /// Decreate indentation
    void dedent();

        /// Write taking into account any CR that might be in a slice
    void write(const UnownedStringSlice& slice);
    void write(char const* text) { write(UnownedStringSlice(text)); }
    void write(char const* text, size_t length) { write(UnownedStringSlice(text, length)); }

        /// Write the slice as an escaped string
    void writeEscapedString(const UnownedStringSlice& slice);

        /// Call before items in a comma-separated JSON list to emit the comma if/when needed
    void maybeComma();

        /// Get the output writer as a helper 
    WriterHelper getOut() const { return WriterHelper(m_writer); }
        /// Get the output writer
    ISlangWriter* getOutWriter() const { return m_writer; }

    bool m_startOfLine = true;
    int m_indent = 0;
    CommaState* m_commaState = nullptr;
    ISlangWriter* m_writer = StdWriters::getSingleton()->getWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT);
};

void PrettyWriter::writeRaw(char const* begin, char const* end)
{
    SLANG_ASSERT(end >= begin);
    writeRaw(UnownedStringSlice(begin, end));
}

void PrettyWriter::writeRawChar(int c)
{
    const char ch = char(c);
    m_writer->write(&ch, 1);
}

void PrettyWriter::adjust()
{
    // Only indent if at start of a line
    if (m_startOfLine)
    {
        // Output current indentation
        const auto indentSlice = toSlice("    ");
        for (int ii = 0; ii < m_indent; ++ii)
            writeRaw(indentSlice);

        m_startOfLine = false;      
    }
}

void PrettyWriter::dedent()
{
    SLANG_ASSERT(m_indent > 0);
    m_indent--;
}

void PrettyWriter::write(const UnownedStringSlice& slice)
{
    const auto end = slice.end();
    auto start = slice.begin();

    while (start < end)
    {
        const char* cur = start;

        // Search for \n if there is one
        while (cur < end && *cur != '\n') cur++;

        // If there were some chars, adjust and write
        if (cur > start)
        {
            adjust();
            writeRaw(UnownedStringSlice(start, cur));
        }

        if (cur < end && *cur == '\n')
        {
            writeRawChar('\n');
            // Skip the CR
            cur++;
            // Mark we are at the start of a line
            m_startOfLine = true;
        }

        start = cur;
    }
}

void PrettyWriter::writeEscapedString(const UnownedStringSlice& slice)
{
    adjust();

    auto handler = StringEscapeUtil::getHandler(StringEscapeUtil::Style::Cpp);

    StringBuilder buf;
    StringEscapeUtil::appendQuoted(handler, slice, buf);
    writeRaw(buf.getUnownedSlice());
}

void PrettyWriter::maybeComma()
{
    if (auto state = m_commaState)
    {
        if (!state->needComma)
        {
            state->needComma = true;
            return;
        }
    }

    write(toSlice(",\n"));
}

static void write(PrettyWriter& writer, uint64_t val)
{
    writer.adjust();
    writer.getOut().print("%llu", (unsigned long long)val);
}

static void write(PrettyWriter& writer, int64_t val)
{
    writer.adjust();
    writer.getOut().print("%lld", (long long)val);
}

static void write(PrettyWriter& writer, int32_t val)
{
    writer.adjust();
    writer.getOut().print("%d", int(val));
}

static void write(PrettyWriter& writer, uint32_t val)
{
    writer.adjust();
    writer.getOut().print("%u", (unsigned int)val);
}

static void write(PrettyWriter& writer, float val)
{
    writer.adjust();
    writer.getOut().print("%f", val);
}

    /// Type for tracking whether a comma is needed in a comma-separated JSON list
struct PrettyWriter::CommaTrackerRAII
{
    CommaTrackerRAII(PrettyWriter& writer)
        : m_writer(&writer)
        , m_previousState(writer.m_commaState)
    {
        writer.m_commaState = &m_state;
    }

    ~CommaTrackerRAII()
    {
        m_writer->m_commaState = m_previousState;
    }

private:
    CommaState m_state;
    PrettyWriter* m_writer;
    CommaState* m_previousState;
};


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
        writer.write("\"kind\": \"uniform\"");
        writer.write(", ");
        writer.write("\"offset\": ");
        write(writer, index);
        writer.write(", ");
        writer.write("\"size\": ");
        write(writer, count);
    }
    else
    {
        writer.write("\"kind\": \"");
        switch( category )
        {
    #define CASE(NAME, KIND) case SLANG_PARAMETER_CATEGORY_##NAME: writer.write(toSlice(#KIND)); break
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
            writer.write("unknown");
            assert(!"unhandled case");
            break;
        }
        writer.write("\"");
        if( space && category != SLANG_PARAMETER_CATEGORY_REGISTER_SPACE)
        {
            writer.write(", ");
            writer.write("\"space\": ");
            write(writer, space);
        }
        writer.write(", ");
        writer.write("\"index\": ");
        write(writer, index);
        if( count != 1)
        {
            writer.write(", ");
            writer.write("\"count\": ");
            if( count == SLANG_UNBOUNDED_SIZE )
            {
                writer.write("\"unbounded\"");
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
        writer.maybeComma();
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

        writer.write("\"stage\": \"");
        writer.write(stageName);
        writer.write("\"");
    }

    auto typeLayout = var->getTypeLayout();
    auto categoryCount = var->getCategoryCount();

    if (categoryCount)
    {
        writer.maybeComma();
        if( categoryCount != 1 )
        {
            writer.write("\"bindings\": [\n");
        }
        else
        {
            writer.write("\"binding\": ");
        }
        writer.indent();

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

            if (cc != 0) writer.write(",\n");

            writer.write("{");
            
            emitReflectionVarBindingInfoJSON(
                writer,
                category,
                index,
                count,
                space);
                
            if (usedAvailable)
            {
                writer.write(", \"used\": ");
                write(writer, used);
            }

            writer.write("}");
        }

        writer.dedent();
        if( categoryCount != 1 )
        {
            writer.write("\n]");
        }
    }

    if (auto semanticName = var->getSemanticName())
    {
        writer.maybeComma();
        writer.write("\"semanticName\": \"");
        writer.write(semanticName);
        writer.write("\"");

        if (auto semanticIndex = var->getSemanticIndex())
        {
            writer.maybeComma();
            writer.write("\"semanticIndex\": ");
            write(writer, int(semanticIndex));
        }
    }
}

static void emitReflectionNameInfoJSON(
    PrettyWriter&   writer,
    char const*     name)
{
    // TODO: deal with escaping special characters if/when needed
    writer.write("\"name\": \"");
    writer.write(name);
    writer.write("\"");
}

static void emitReflectionModifierInfoJSON(
    PrettyWriter&               writer,
    slang::VariableReflection*  var)
{
    if( var->findModifier(slang::Modifier::Shared) )
    {
        writer.maybeComma();
        writer.write("\"shared\": true");
    }
}

static void emitUserAttributeJSON(PrettyWriter& writer, slang::UserAttribute* userAttribute)
{
    writer.write("{\n");
    writer.indent();
    writer.write("\"name\": \"");
    writer.write(userAttribute->getName());
    writer.write("\",\n");
    writer.write("\"arguments\": [\n");
    writer.indent();
    for (unsigned int i = 0; i < userAttribute->getArgumentCount(); i++)
    {
        int intVal;
        float floatVal;
        size_t bufSize = 0;
        if (i > 0)
            writer.write(",\n");
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
            writer.write(str, bufSize);
        }
        else
            writer.write("\"invalid value\"");
    }
    writer.dedent();
    writer.write("\n]\n");
    writer.dedent();
    writer.write("}\n");
}

static void emitUserAttributes(PrettyWriter& writer, slang::TypeReflection* type)
{
    auto attribCount = type->getUserAttributeCount();
    if (attribCount)
    {
        writer.write(",\n\"userAttribs\": [");
        for (unsigned int i = 0; i < attribCount; i++)
        {
            if (i > 0)
                writer.write(",\n");
            auto attrib = type->getUserAttributeByIndex(i);
            emitUserAttributeJSON(writer, attrib);
        }
        writer.write("]");
    }
}
static void emitUserAttributes(PrettyWriter& writer, slang::VariableReflection* var)
{
    auto attribCount = var->getUserAttributeCount();
    if (attribCount)
    {
        writer.write(",\n\"userAttribs\": [");
        for (unsigned int i = 0; i < attribCount; i++)
        {
            if (i > 0)
                writer.write(",\n");
            auto attrib = var->getUserAttributeByIndex(i);
            emitUserAttributeJSON(writer, attrib);
        }
        writer.write("]");
    }
}

static void emitReflectionVarLayoutJSON(
    PrettyWriter&                       writer,
    slang::VariableLayoutReflection*    var)
{
    writer.write("{\n");
    writer.indent();

    PrettyWriter::CommaTrackerRAII commaTracker(writer);

    if( auto name = var->getName() )
    {
        writer.maybeComma();
        emitReflectionNameInfoJSON(writer, name);
    }

    writer.maybeComma();
    writer.write("\"type\": ");
    emitReflectionTypeLayoutJSON(writer, var->getTypeLayout());

    emitReflectionModifierInfoJSON(writer, var->getVariable());

    emitReflectionVarBindingInfoJSON(writer, var);

    emitUserAttributes(writer, var->getVariable());
    writer.dedent();
    writer.write("\n}");
}

static void emitReflectionScalarTypeInfoJSON(
    PrettyWriter&   writer,
    SlangScalarType scalarType)
{
    writer.write("\"scalarType\": \"");
    switch (scalarType)
    {
    default:
        writer.write("unknown");
        assert(!"unhandled case");
        break;
#define CASE(TAG, ID) case static_cast<SlangScalarType>(slang::TypeReflection::ScalarType::TAG): writer.write(toSlice(#ID)); break
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
    writer.write("\"");
}

static void emitReflectionResourceTypeBaseInfoJSON(
    PrettyWriter&           writer,
    slang::TypeReflection*  type)
{
    auto shape  = type->getResourceShape();
    auto access = type->getResourceAccess();
    writer.maybeComma();
    writer.write("\"kind\": \"resource\"");
    writer.maybeComma();
    writer.write("\"baseShape\": \"");
    switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
    {
    default:
        writer.write("unknown");
        assert(!"unhandled case");
        break;

#define CASE(SHAPE, NAME) case SLANG_##SHAPE: writer.write(toSlice(#NAME)); break
        CASE(TEXTURE_1D, texture1D);
        CASE(TEXTURE_2D, texture2D);
        CASE(TEXTURE_3D, texture3D);
        CASE(TEXTURE_CUBE, textureCube);
        CASE(TEXTURE_BUFFER, textureBuffer);
        CASE(STRUCTURED_BUFFER, structuredBuffer);
        CASE(BYTE_ADDRESS_BUFFER, byteAddressBuffer);
#undef CASE
    }
    writer.write("\"");
    if (shape & SLANG_TEXTURE_ARRAY_FLAG)
    {
        writer.maybeComma();
        writer.write("\"array\": true");
    }
    if (shape & SLANG_TEXTURE_MULTISAMPLE_FLAG)
    {
        writer.maybeComma();
        writer.write("\"multisample\": true");
    }
    if (shape & SLANG_TEXTURE_FEEDBACK_FLAG)
    {
        writer.maybeComma();
        writer.write("\"feedback\": true");
    }

    if( access != SLANG_RESOURCE_ACCESS_READ )
    {
        writer.maybeComma();
        writer.write("\"access\": \"");
        switch(access)
        {
        default:
            writer.write("unknown");
            assert(!"unhandled case");
            break;

        case SLANG_RESOURCE_ACCESS_READ:
            break;
        case SLANG_RESOURCE_ACCESS_WRITE:           writer.write("write"); break;
        case SLANG_RESOURCE_ACCESS_READ_WRITE:      writer.write("readWrite"); break;
        case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:  writer.write("rasterOrdered"); break;
        case SLANG_RESOURCE_ACCESS_APPEND:          writer.write("append"); break;
        case SLANG_RESOURCE_ACCESS_CONSUME:         writer.write("consume"); break;
        }
        writer.write("\"");
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
        writer.maybeComma();
        writer.write("\"kind\": \"samplerState\"");
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
                    writer.maybeComma();
                    writer.write("\"resultType\": ");
                    emitReflectionTypeJSON(
                        writer,
                        resultType);
                }
                break;
            }
        }
        break;

    case slang::TypeReflection::Kind::ConstantBuffer:
        writer.maybeComma();
        writer.write("\"kind\": \"constantBuffer\"");
        writer.maybeComma();
        writer.write("\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::ParameterBlock:
        writer.maybeComma();
        writer.write("\"kind\": \"parameterBlock\"");
        writer.maybeComma();
        writer.write("\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::TextureBuffer:
        writer.maybeComma();
        writer.write("\"kind\": \"textureBuffer\"");
        writer.maybeComma();
        writer.write("\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::ShaderStorageBuffer:
        writer.maybeComma();
        writer.write("\"kind\": \"shaderStorageBuffer\"");
        writer.maybeComma();
        writer.write("\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::Scalar:
        writer.maybeComma();
        writer.write("\"kind\": \"scalar\"");
        writer.maybeComma();
        emitReflectionScalarTypeInfoJSON(
            writer,
            SlangScalarType(type->getScalarType()));
        break;

    case slang::TypeReflection::Kind::Vector:
        writer.maybeComma();
        writer.write("\"kind\": \"vector\"");
        writer.maybeComma();
        writer.write("\"elementCount\": ");
        write(writer, int(type->getElementCount()));
        writer.maybeComma();
        writer.write("\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::Matrix:
        writer.maybeComma();
        writer.write("\"kind\": \"matrix\"");
        writer.maybeComma();
        writer.write("\"rowCount\": ");
        write(writer, type->getRowCount());
        writer.maybeComma();
        writer.write("\"columnCount\": ");
        write(writer, type->getColumnCount());
        writer.maybeComma();
        writer.write("\"elementType\": ");
        emitReflectionTypeJSON(
            writer,
            type->getElementType());
        break;

    case slang::TypeReflection::Kind::Array:
        {
            auto arrayType = type;
            writer.maybeComma();
            writer.write("\"kind\": \"array\"");
            writer.maybeComma();
            writer.write("\"elementCount\": ");
            write(writer, int(arrayType->getElementCount()));
            writer.maybeComma();
            writer.write("\"elementType\": ");
            emitReflectionTypeJSON(writer, arrayType->getElementType());
        }
        break;

    case slang::TypeReflection::Kind::Struct:
        {
            writer.maybeComma();
            writer.write("\"kind\": \"struct\"");
            writer.maybeComma();
            writer.write("\"fields\": [\n");
            writer.indent();

            auto structType = type;
            auto fieldCount = structType->getFieldCount();
            for( uint32_t ff = 0; ff < fieldCount; ++ff )
            {
                if (ff != 0) writer.write(",\n");
                emitReflectionVarInfoJSON(
                    writer,
                    structType->getFieldByIndex(ff));
            }
            writer.dedent();
            writer.write("\n]");
        }
        break;

    case slang::TypeReflection::Kind::GenericTypeParameter:
        writer.maybeComma();
        writer.write("\"kind\": \"GenericTypeParameter\"");
        writer.maybeComma();
        emitReflectionNameInfoJSON(writer, type->getName());
        break;
    case slang::TypeReflection::Kind::Interface:
        writer.maybeComma();
        writer.write("\"kind\": \"Interface\"");
        writer.maybeComma();
        emitReflectionNameInfoJSON(writer, type->getName());
        break;
    case slang::TypeReflection::Kind::Feedback:
        writer.maybeComma();
        writer.write("\"kind\": \"Feedback\"");
        writer.maybeComma();
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
    writer.write("\"kind\": \"");
    writer.write(kind);
    writer.write("\"");

    writer.write(",\n\"elementType\": ");
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
        PrettyWriter::CommaTrackerRAII commaTracker(writer);

        writer.write(",\n\"containerVarLayout\": {\n");
        writer.indent();
        emitReflectionVarBindingInfoJSON(writer, typeLayout->getContainerVarLayout());
        writer.dedent();
        writer.write("\n}");
    }

    writer.write(",\n\"elementVarLayout\": ");
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
            writer.maybeComma();
            writer.write("\"kind\": \"array\"");

            writer.maybeComma();
            writer.write("\"elementCount\": ");
            write(writer, int(arrayTypeLayout->getElementCount()));

            writer.maybeComma();
            writer.write("\"elementType\": ");
            emitReflectionTypeLayoutJSON(
                writer,
                elementTypeLayout);

            if (arrayTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) != 0)
            {
                writer.maybeComma();
                writer.write("\"uniformStride\": ");
                write(writer, int(arrayTypeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM)));
            }
        }
        break;

    case slang::TypeReflection::Kind::Struct:
        {
            auto structTypeLayout = typeLayout;

            writer.maybeComma();
            writer.write("\"kind\": \"struct\"");
            if( auto name = structTypeLayout->getName() )
            {
                writer.maybeComma();
                emitReflectionNameInfoJSON(writer, structTypeLayout->getName());
            }
            writer.maybeComma();
            writer.write("\"fields\": [\n");
            writer.indent();

            auto fieldCount = structTypeLayout->getFieldCount();
            for( uint32_t ff = 0; ff < fieldCount; ++ff )
            {
                if (ff != 0) writer.write(",\n");
                emitReflectionVarLayoutJSON(
                    writer,
                    structTypeLayout->getFieldByIndex(ff));
            }
            writer.dedent();
            writer.write("\n]");
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
        writer.maybeComma();
        writer.write("\"kind\": \"shaderStorageBuffer\"");

        writer.maybeComma();
        writer.write("\"elementType\": ");
        emitReflectionTypeLayoutJSON(
            writer,
            typeLayout->getElementTypeLayout());
        break;
    case slang::TypeReflection::Kind::GenericTypeParameter:
        writer.maybeComma();
        writer.write("\"kind\": \"GenericTypeParameter\"");

        writer.maybeComma();
        emitReflectionNameInfoJSON(writer, typeLayout->getName());
        break;
    case slang::TypeReflection::Kind::Interface:
        writer.maybeComma();
        writer.write("\"kind\": \"Interface\"");

        writer.maybeComma();
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
                    writer.maybeComma();
                    writer.write("\"resultType\": ");
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
                    writer.maybeComma();
                    writer.write("\"resultType\": ");
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
    PrettyWriter::CommaTrackerRAII commaTracker(writer);
    writer.write("{\n");
    writer.indent();
    emitReflectionTypeLayoutInfoJSON(writer, typeLayout);
    writer.dedent();
    writer.write("\n}");
}

static void emitReflectionTypeJSON(
    PrettyWriter&           writer,
    slang::TypeReflection*  type)
{
    PrettyWriter::CommaTrackerRAII commaTracker(writer);
    writer.write("{\n");
    writer.indent();
    emitReflectionTypeInfoJSON(writer, type);
    writer.dedent();
    writer.write("\n}");
}

static void emitReflectionVarInfoJSON(
    PrettyWriter&               writer,
    slang::VariableReflection*  var)
{
    emitReflectionNameInfoJSON(writer, var->getName());

    emitReflectionModifierInfoJSON(writer, var);

    writer.write(",\n");
    writer.write("\"type\": ");
    emitReflectionTypeJSON(writer, var->getType());
}

static void emitReflectionParamJSON(
    PrettyWriter&                       writer,
    slang::VariableLayoutReflection*    param)
{
    // TODO: This function is likely redundant with `emitReflectionVarLayoutJSON`
    // and we should try to collapse them into one.

    writer.write("{\n");
    writer.indent();

    PrettyWriter::CommaTrackerRAII commaTracker(writer);

    if( auto name = param->getName() )
    {
        writer.maybeComma();
        emitReflectionNameInfoJSON(writer, name);
    }

    emitReflectionModifierInfoJSON(writer, param->getVariable());

    emitReflectionVarBindingInfoJSON(writer, param);

    writer.maybeComma();
    writer.write("\"type\": ");
    emitReflectionTypeLayoutJSON(writer, param->getTypeLayout());

    writer.dedent();
    writer.write("\n}");
}


static void emitEntryPointParamJSON(
    PrettyWriter&                       writer,
    slang::VariableLayoutReflection*    param,
    SlangCompileRequest*                request,
    int                                 entryPointIndex)
{
    writer.write("{\n");
    writer.indent();

    if( auto name = param->getName() )
    {
        emitReflectionNameInfoJSON(writer, name);
    }

    emitReflectionVarBindingInfoJSON(writer, param, request, entryPointIndex);

    writer.dedent();
    writer.write("\n}");
}

template<typename T>
struct Range
{
public:
    Range(
        T begin,
        T end)
        : m_begin(begin)
        , m_end(end)
    {}

    struct Iterator
    {
    public:
        explicit Iterator(T value)
            : m_value(value)
        {}

        T operator*() const { return m_value; }
        void operator++() { m_value++; }

        bool operator!=(Iterator const& other)
        {
            return m_value != other.m_value;
        }

    private:
        T m_value;
    };

    Iterator begin() const { return Iterator(m_begin); }
    Iterator end()   const { return Iterator(m_end); }

private:
    T m_begin;
    T m_end;
};

template<typename T>
Range<T> makeRange(T begin, T end)
{
    return Range<T>(begin, end);
}

template<typename T>
Range<T> makeRange(T end)
{
    return Range<T>(T(0), end);
}

static void emitReflectionTypeParamJSON(
    PrettyWriter&                   writer,
    slang::TypeParameterReflection* typeParam)
{
    writer.write("{\n");
    writer.indent();
    emitReflectionNameInfoJSON(writer, typeParam->getName());
    writer.write(",\n");
    writer.write("constraints: \n");
    writer.write("[\n");
    writer.indent();
    auto constraintCount = typeParam->getConstraintCount();
    for (auto ee : makeRange(constraintCount))
    {
        if (ee != 0) writer.write(",\n");
        writer.write("{\n");
        writer.indent();
        PrettyWriter::CommaTrackerRAII commaTracker(writer);
        emitReflectionTypeInfoJSON(writer, typeParam->getConstraintByIndex(ee));
        writer.dedent();
        writer.write("\n}");
    }
    writer.dedent();
    writer.write("\n]");
    writer.dedent();
    writer.write("\n}");
}

static void emitReflectionEntryPointJSON(
    PrettyWriter&                   writer,
    SlangCompileRequest*            request,
    slang::ShaderReflection*        programReflection,
    int                             entryPointIndex)
{
    slang::EntryPointReflection* entryPoint = programReflection->getEntryPointByIndex(entryPointIndex);

    writer.write("{\n");
    writer.indent();

    emitReflectionNameInfoJSON(writer, entryPoint->getName());

    switch (entryPoint->getStage())
    {
    case SLANG_STAGE_VERTEX:    writer.write(",\n\"stage:\": \"vertex\"");     break;
    case SLANG_STAGE_HULL:      writer.write(",\n\"stage:\": \"hull\"");       break;
    case SLANG_STAGE_DOMAIN:    writer.write(",\n\"stage:\": \"domain\"");     break;
    case SLANG_STAGE_GEOMETRY:  writer.write(",\n\"stage:\": \"geometry\"");   break;
    case SLANG_STAGE_FRAGMENT:  writer.write(",\n\"stage:\": \"fragment\"");   break;
    case SLANG_STAGE_COMPUTE:   writer.write(",\n\"stage:\": \"compute\"");    break;
    default:
        break;
    }

    auto parameterCount = entryPoint->getParameterCount();
    if (parameterCount)
    {
        writer.write(",\n\"parameters\": [\n");
        writer.indent();

        for( auto pp : makeRange(parameterCount) )
        {
            if(pp != 0) writer.write(",\n");

            auto parameter = entryPoint->getParameterByIndex(pp);
            emitReflectionParamJSON(writer, parameter);
        }

        writer.dedent();
        writer.write("\n]");
    }
    if (entryPoint->usesAnySampleRateInput())
    {
        writer.write(",\n\"usesAnySampleRateInput\": true");
    }
    if( auto resultVarLayout = entryPoint->getResultVarLayout() )
    {
        writer.write(",\n\"result:\": ");
        emitReflectionParamJSON(writer, resultVarLayout);
    }

    if (entryPoint->getStage() == SLANG_STAGE_COMPUTE)
    {
        SlangUInt threadGroupSize[3];
        entryPoint->getComputeThreadGroupSize(3, threadGroupSize);

        writer.write(",\n\"threadGroupSize\": [");
        for (int ii = 0; ii < 3; ++ii)
        {
            if (ii != 0) writer.write(", ");
            write(writer, threadGroupSize[ii]);
        }
        writer.write("]");
    }

    // If code generation has been performed, print out the parameter usage by this entry point.
    if ((request->getCompileFlags() & SLANG_COMPILE_FLAG_NO_CODEGEN) == 0)
    {
        writer.write(",\n\"bindings\": [\n");
        writer.indent();

        auto parameterCount = programReflection->getParameterCount();
        for( auto pp : makeRange(parameterCount) )
        {
            if(pp != 0) writer.write(",\n");

            auto parameter = programReflection->getParameterByIndex(pp);
            emitEntryPointParamJSON(writer, parameter, request, entryPointIndex);
        }

        writer.dedent();
        writer.write("\n]");
    }

    writer.dedent();
    writer.write("\n}");
}

static void emitReflectionJSON(
    PrettyWriter&               writer,
    SlangCompileRequest*        request,
    slang::ShaderReflection*    programReflection)
{
    writer.write("{\n");
    writer.indent();
    writer.write("\"parameters\": [\n");
    writer.indent();

    auto parameterCount = programReflection->getParameterCount();
    for( auto pp : makeRange(parameterCount) )
    {
        if(pp != 0) writer.write(",\n");

        auto parameter = programReflection->getParameterByIndex(pp);
        emitReflectionParamJSON(writer, parameter);
    }

    writer.dedent();
    writer.write("\n]");

    auto entryPointCount = programReflection->getEntryPointCount();
    if (entryPointCount)
    {
        writer.write(",\n\"entryPoints\": [\n");
        writer.indent();
    
        for (auto ee : makeRange(entryPointCount))
        {
            if (ee != 0) writer.write(",\n");

            emitReflectionEntryPointJSON(writer, request, programReflection, (int)ee);
        }

        writer.dedent();
        writer.write("\n]");
    }

    auto genParamCount = programReflection->getTypeParameterCount();
    if (genParamCount)
    {
        writer.write(",\n\"typeParams\":\n");
        writer.write("[\n");
        writer.indent();
        for (auto ee : makeRange(genParamCount))
        {
            if (ee != 0) writer.write(",\n");

            auto typeParam = programReflection->getTypeParameterByIndex(ee);
            emitReflectionTypeParamJSON(writer, typeParam);
        }
        writer.dedent();
        writer.write("\n]");
    }

    {
        SlangUInt count = programReflection->getHashedStringCount();
        if (count)
        {
            writer.write(",\n\"hashedStrings\": {\n");
            writer.indent();

            for (SlangUInt i = 0; i < count; ++i)
            {
                if (i)
                {
                    writer.write(",\n");
                }

                size_t charsCount;
                const char* chars = programReflection->getHashedString(i, &charsCount);
                const int hash = spComputeStringHash(chars, charsCount);

                writer.writeEscapedString(UnownedStringSlice(chars, charsCount));
                writer.write(": ");

                write(writer, hash);
            }

            writer.dedent();
            writer.write("\n}\n");
        }
    }

    writer.dedent();
    writer.write("\n}\n");
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
