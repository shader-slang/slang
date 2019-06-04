// slang-emit-glsl.cpp
#include "slang-emit-glsl.h"

#include "../core/slang-writer.h"

#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include "slang-legalize-types.h"

#include <assert.h>

namespace Slang {

void GLSLSourceEmitter::emitGLSLTextureType(IRTextureType* texType)
{
    switch (texType->getAccess())
    {
        case SLANG_RESOURCE_ACCESS_READ_WRITE:
        case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
            emitGLSLTextureOrTextureSamplerType(texType, "image");
            break;

        default:
            emitGLSLTextureOrTextureSamplerType(texType, "texture");
            break;
    }
}


void GLSLSourceEmitter::emitIRStructuredBuffer_GLSL(IRGlobalParam* varDecl, IRHLSLStructuredBufferTypeBase* structuredBufferType)
{
    // Shader storage buffer is an OpenGL 430 feature
    //
    // TODO: we should require either the extension or the version...
    requireGLSLVersion(430);

    m_writer->emit("layout(std430");

    auto layout = getVarLayout(varDecl);
    if (layout)
    {
        LayoutResourceKind kind = LayoutResourceKind::DescriptorTableSlot;
        EmitVarChain chain(layout);

        const UInt index = getBindingOffset(&chain, kind);
        const UInt space = getBindingSpace(&chain, kind);

        m_writer->emit(", binding = ");
        m_writer->emit(index);
        if (space)
        {
            m_writer->emit(", set = ");
            m_writer->emit(space);
        }
    }

    m_writer->emit(") ");

    /*
    If the output type is a buffer, and we can determine it is only readonly we can prefix before
    buffer with 'readonly'

    The actual structuredBufferType could be

    HLSLStructuredBufferType                        - This is unambiguously read only
    HLSLRWStructuredBufferType                      - Read write
    HLSLRasterizerOrderedStructuredBufferType       - Allows read/write access
    HLSLAppendStructuredBufferType                  - Write
    HLSLConsumeStructuredBufferType                 - TODO (JS): Its possible that this can be readonly, but we currently don't support on GLSL
    */

    if (as<IRHLSLStructuredBufferType>(structuredBufferType))
    {
        m_writer->emit("readonly ");
    }

    m_writer->emit("buffer ");

    // Generate a dummy name for the block
    m_writer->emit("_S");
    m_writer->emit(m_uniqueIDCounter++);

    m_writer->emit(" {\n");
    m_writer->indent();


    auto elementType = structuredBufferType->getElementType();
    emitIRType(elementType, "_data[]");
    m_writer->emit(";\n");

    m_writer->dedent();
    m_writer->emit("} ");

    m_writer->emit(getIRName(varDecl));
    emitArrayBrackets(varDecl->getDataType());

    m_writer->emit(";\n");
}

void GLSLSourceEmitter::emitIRByteAddressBuffer_GLSL(IRGlobalParam* varDecl, IRByteAddressBufferTypeBase* byteAddressBufferType)
{
    // TODO: A lot of this logic is copy-pasted from `emitIRStructuredBuffer_GLSL`.
    // It might be worthwhile to share the common code to avoid regressions sneaking
    // in when one or the other, but not both, gets updated.

    // Shader storage buffer is an OpenGL 430 feature
    //
    // TODO: we should require either the extension or the version...
    requireGLSLVersion(430);

    m_writer->emit("layout(std430");

    auto layout = getVarLayout(varDecl);
    if (layout)
    {
        LayoutResourceKind kind = LayoutResourceKind::DescriptorTableSlot;
        EmitVarChain chain(layout);

        const UInt index = getBindingOffset(&chain, kind);
        const UInt space = getBindingSpace(&chain, kind);

        m_writer->emit(", binding = ");
        m_writer->emit(index);
        if (space)
        {
            m_writer->emit(", set = ");
            m_writer->emit(space);
        }
    }

    m_writer->emit(") ");

    /*
    If the output type is a buffer, and we can determine it is only readonly we can prefix before
    buffer with 'readonly'

    HLSLByteAddressBufferType                   - This is unambiguously read only
    HLSLRWByteAddressBufferType                 - Read write
    HLSLRasterizerOrderedByteAddressBufferType  - Allows read/write access
    */

    if (as<IRHLSLByteAddressBufferType>(byteAddressBufferType))
    {
        m_writer->emit("readonly ");
    }

    m_writer->emit("buffer ");

    // Generate a dummy name for the block
    m_writer->emit("_S");
    m_writer->emit(m_uniqueIDCounter++);
    m_writer->emit("\n{\n");
    m_writer->indent();

    m_writer->emit("uint _data[];\n");

    m_writer->dedent();
    m_writer->emit("} ");

    m_writer->emit(getIRName(varDecl));
    emitArrayBrackets(varDecl->getDataType());

    m_writer->emit(";\n");
}


void GLSLSourceEmitter::emitGLSLParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    auto varLayout = getVarLayout(varDecl);
    SLANG_RELEASE_ASSERT(varLayout);

    EmitVarChain blockChain(varLayout);

    EmitVarChain containerChain = blockChain;
    EmitVarChain elementChain = blockChain;

    auto typeLayout = varLayout->typeLayout->unwrapArray();
    if (auto parameterGroupTypeLayout = as<ParameterGroupTypeLayout>(typeLayout))
    {
        containerChain = EmitVarChain(parameterGroupTypeLayout->containerVarLayout, &blockChain);
        elementChain = EmitVarChain(parameterGroupTypeLayout->elementVarLayout, &blockChain);

        typeLayout = parameterGroupTypeLayout->elementVarLayout->typeLayout;
    }

    /*
    With resources backed by 'buffer' on glsl, we want to output 'readonly' if that is a good match
    for the underlying type. If uniform it's implicit it's readonly

    Here this only happens with isShaderRecord which is a 'constant buffer' (ie implicitly readonly)
    or IRGLSLShaderStorageBufferType which is read write.
    */

    emitGLSLLayoutQualifier(LayoutResourceKind::DescriptorTableSlot, &containerChain);
    emitGLSLLayoutQualifier(LayoutResourceKind::PushConstantBuffer, &containerChain);
    bool isShaderRecord = emitGLSLLayoutQualifier(LayoutResourceKind::ShaderRecord, &containerChain);

    if (isShaderRecord)
    {
        // TODO: A shader record in vk can be potentially read-write. Currently slang doesn't support write access
        // and readonly buffer generates SPIRV validation error.
        m_writer->emit("buffer ");
    }
    else if (as<IRGLSLShaderStorageBufferType>(type))
    {
        // Is writable 
        m_writer->emit("layout(std430) buffer ");
    }
    // TODO: what to do with HLSL `tbuffer` style buffers?
    else
    {
        // uniform is implicitly read only
        m_writer->emit("layout(std140) uniform ");
    }

    // Generate a dummy name for the block
    m_writer->emit("_S");
    m_writer->emit(m_uniqueIDCounter++);

    m_writer->emit("\n{\n");
    m_writer->indent();

    auto elementType = type->getElementType();

    emitIRType(elementType, "_data");
    m_writer->emit(";\n");

    m_writer->dedent();
    m_writer->emit("} ");

    m_writer->emit(getIRName(varDecl));

    // If the underlying variable was an array (or array of arrays, etc.)
    // we need to emit all those array brackets here.
    emitArrayBrackets(varDecl->getDataType());

    m_writer->emit(";\n");
}

void GLSLSourceEmitter::emitGLSLImageFormatModifier(IRInst* var, IRTextureType* resourceType)
{
    // If the user specified a format manually, using `[format(...)]`,
    // then we will respect that format and emit a matching `layout` modifier.
    //
    if (auto formatDecoration = var->findDecoration<IRFormatDecoration>())
    {
        auto format = formatDecoration->getFormat();
        if (format == ImageFormat::unknown)
        {
            // If the user explicitly opts out of having a format, then
            // the output shader will require the extension to support
            // load/store from format-less images.
            //
            // TODO: We should have a validation somewhere in the compiler
            // that atomic operations are only allowed on images with
            // explicit formats (and then only on specific formats).
            // This is really an argument that format should be part of
            // the image *type* (with a "base type" for images with
            // unknown format).
            //
            requireGLSLExtension("GL_EXT_shader_image_load_formatted");
        }
        else
        {
            // If there is an explicit format specified, then we
            // should emit a `layout` modifier using the GLSL name
            // for the format.
            //
            m_writer->emit("layout(");
            m_writer->emit(getGLSLNameForImageFormat(format));
            m_writer->emit(")\n");
        }

        // No matter what, if an explicit `[format(...)]` was given,
        // then we don't need to emit anything else.
        //
        return;
    }


    // When no explicit format is specified, we need to either
    // emit the image as having an unknown format, or else infer
    // a format from the type.
    //
    // For now our default behavior is to infer (so that unmodified
    // HLSL input is more likely to generate valid SPIR-V that
    // runs anywhere), but we provide a flag to opt into
    // treating images without explicit formats as having
    // unknown format.
    //
    if (m_compileRequest->useUnknownImageFormatAsDefault)
    {
        requireGLSLExtension("GL_EXT_shader_image_load_formatted");
        return;
    }

    // At this point we have a resource type like `RWTexture2D<X>`
    // and we want to infer a reasonable format from the element
    // type `X` that was specified.
    //
    // E.g., if `X` is `float` then we can infer a format like `r32f`,
    // and so forth. The catch of course is that it is possible to
    // specify a shader parameter with a type like `RWTexture2D<float4>` but
    // provide an image at runtime with a format like `rgba8`, so
    // this inference is never guaranteed to give perfect results.
    //
    // If users don't like our inferred result, they need to use a
    // `[format(...)]` attribute to manually specify what they want.
    //
    // TODO: We should consider whether we can expand the space of
    // allowed types for `X` in `RWTexture2D<X>` to include special
    // pseudo-types that act just like, e.g., `float4`, but come
    // with attached/implied format information.
    //
    auto elementType = resourceType->getElementType();
    Int vectorWidth = 1;
    if (auto elementVecType = as<IRVectorType>(elementType))
    {
        if (auto intLitVal = as<IRIntLit>(elementVecType->getElementCount()))
        {
            vectorWidth = (Int)intLitVal->getValue();
        }
        else
        {
            vectorWidth = 0;
        }
        elementType = elementVecType->getElementType();
    }
    if (auto elementBasicType = as<IRBasicType>(elementType))
    {
        m_writer->emit("layout(");
        switch (vectorWidth)
        {
            default: m_writer->emit("rgba");  break;

            case 3:
            {
                // TODO: GLSL doesn't support 3-component formats so for now we are going to
                // default to rgba
                //
                // The SPIR-V spec (https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.pdf)
                // section 3.11 on Image Formats it does not list rgbf32.
                //
                // It seems SPIR-V can support having an image with an unknown-at-compile-time
                // format, so long as the underlying API supports it. Ideally this would mean that we can
                // just drop all these qualifiers when emitting GLSL for Vulkan targets.
                //
                // This raises the question of what to do more long term. For Vulkan hopefully we can just
                // drop the layout. For OpenGL targets it would seem reasonable to have well-defined rules
                // for inferring the format (and just document that 3-component formats map to 4-component formats,
                // but that shouldn't matter because the API wouldn't let the user allocate those 3-component formats anyway),
                // and add an attribute for specifying the format manually if you really want to override our
                // inference (e.g., to specify r11fg11fb10f).

                m_writer->emit("rgba");
                //Emit("rgb");                                
                break;
            }

            case 2:  m_writer->emit("rg");    break;
            case 1:  m_writer->emit("r");     break;
        }
        switch (elementBasicType->getBaseType())
        {
            default:
            case BaseType::Float:   m_writer->emit("32f");  break;
            case BaseType::Half:    m_writer->emit("16f");  break;
            case BaseType::UInt:    m_writer->emit("32ui"); break;
            case BaseType::Int:     m_writer->emit("32i"); break;

                // TODO: Here are formats that are available in GLSL,
                // but that are not handled by the above cases.
                //
                // r11f_g11f_b10f
                //
                // rgba16
                // rgb10_a2
                // rgba8
                // rg16
                // rg8
                // r16
                // r8
                //
                // rgba16_snorm
                // rgba8_snorm
                // rg16_snorm
                // rg8_snorm
                // r16_snorm
                // r8_snorm
                //
                // rgba16i
                // rgba8i
                // rg16i
                // rg8i
                // r16i
                // r8i
                //
                // rgba16ui
                // rgb10_a2ui
                // rgba8ui
                // rg16ui
                // rg8ui
                // r16ui
                // r8ui
        }
        m_writer->emit(")\n");
    }
}

bool GLSLSourceEmitter::emitGLSLLayoutQualifier(LayoutResourceKind kind, EmitVarChain* chain)
{
    if (!chain)
        return false;
    if (!chain->varLayout->FindResourceInfo(kind))
        return false;

    UInt index = getBindingOffset(chain, kind);
    UInt space = getBindingSpace(chain, kind);
    switch (kind)
    {
        case LayoutResourceKind::Uniform:
        {
            // Explicit offsets require a GLSL extension (which
            // is not universally supported, it seems) or a new
            // enough GLSL version (which we don't want to
            // universally require), so for right now we
            // won't actually output explicit offsets for uniform
            // shader parameters.
            //
            // TODO: We should fix this so that we skip any
            // extra work for parameters that are laid out as
            // expected by the default rules, but do *something*
            // for parameters that need non-default layout.
            //
            // Using the `GL_ARB_enhanced_layouts` feature is one
            // option, but we should also be able to do some
            // things by introducing padding into the declaration
            // (padding insertion would probably be best done at
            // the IR level).
            bool useExplicitOffsets = false;
            if (useExplicitOffsets)
            {
                requireGLSLExtension("GL_ARB_enhanced_layouts");

                m_writer->emit("layout(offset = ");
                m_writer->emit(index);
                m_writer->emit(")\n");
            }
        }
        break;

        case LayoutResourceKind::VertexInput:
        case LayoutResourceKind::FragmentOutput:
            m_writer->emit("layout(location = ");
            m_writer->emit(index);
            m_writer->emit(")\n");
            break;

        case LayoutResourceKind::SpecializationConstant:
            m_writer->emit("layout(constant_id = ");
            m_writer->emit(index);
            m_writer->emit(")\n");
            break;

        case LayoutResourceKind::ConstantBuffer:
        case LayoutResourceKind::ShaderResource:
        case LayoutResourceKind::UnorderedAccess:
        case LayoutResourceKind::SamplerState:
        case LayoutResourceKind::DescriptorTableSlot:
            m_writer->emit("layout(binding = ");
            m_writer->emit(index);
            if (space)
            {
                m_writer->emit(", set = ");
                m_writer->emit(space);
            }
            m_writer->emit(")\n");
            break;

        case LayoutResourceKind::PushConstantBuffer:
            m_writer->emit("layout(push_constant)\n");
            break;
        case LayoutResourceKind::ShaderRecord:
            m_writer->emit("layout(shaderRecordNV)\n");
            break;

    }
    return true;
}

void GLSLSourceEmitter::emitGLSLLayoutQualifiers(RefPtr<VarLayout> layout, EmitVarChain* inChain, LayoutResourceKind filter)
{
    if (!layout) return;

    switch (getSourceStyle())
    {
        default:
            return;

        case SourceStyle::GLSL:
            break;
    }

    EmitVarChain chain(layout, inChain);

    for (auto info : layout->resourceInfos)
    {
        // Skip info that doesn't match our filter
        if (filter != LayoutResourceKind::None
            && filter != info.kind)
        {
            continue;
        }

        emitGLSLLayoutQualifier(info.kind, &chain);
    }
}

void GLSLSourceEmitter::emitGLSLTextureOrTextureSamplerType(IRTextureTypeBase*  type, char const* baseName)
{
    if (type->getElementType()->op == kIROp_HalfType)
    {
        // Texture access is always as float types if half is specified

    }
    else
    {
        emitGLSLTypePrefix(type->getElementType(), true);
    }

    m_writer->emit(baseName);
    switch (type->GetBaseShape())
    {
        case TextureFlavor::Shape::Shape1D:		m_writer->emit("1D");		break;
        case TextureFlavor::Shape::Shape2D:		m_writer->emit("2D");		break;
        case TextureFlavor::Shape::Shape3D:		m_writer->emit("3D");		break;
        case TextureFlavor::Shape::ShapeCube:	m_writer->emit("Cube");	break;
        case TextureFlavor::Shape::ShapeBuffer:	m_writer->emit("Buffer");	break;
        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource shape");
            break;
    }

    if (type->isMultisample())
    {
        m_writer->emit("MS");
    }
    if (type->isArray())
    {
        m_writer->emit("Array");
    }
}


void GLSLSourceEmitter::emitGLSLTextureSamplerType(IRTextureSamplerType* type)
{
    emitGLSLTextureOrTextureSamplerType(type, "sampler");
}

void GLSLSourceEmitter::emitGLSLImageType(IRGLSLImageType* type)
{
    emitGLSLTextureOrTextureSamplerType(type, "image");
}

void GLSLSourceEmitter::emitIRParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    emitGLSLParameterGroup(varDecl, type);
}

void GLSLSourceEmitter::emitIREntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout)
{
    emitIREntryPointAttributes_GLSL(irFunc, entryPointLayout);
}

void GLSLSourceEmitter::emitTextureTypeImpl(IRTextureType* texType)
{
    emitGLSLTextureType(texType);
}

void GLSLSourceEmitter::emitImageTypeImpl(IRGLSLImageType* type)
{
    emitGLSLImageType(type);
}

void GLSLSourceEmitter::emitTextureSamplerTypeImpl(IRTextureSamplerType* type)
{
    emitGLSLTextureSamplerType(type);
}

bool GLSLSourceEmitter::tryEmitIRGlobalParamImpl(IRGlobalParam* varDecl, IRType* varType)
{
    // There are a number of types that are (or can be)
        // "first-class" in D3D HLSL, but are second-class in GLSL in
        // that they require explicit global declarations for each value/object,
        // and don't support declaration as ordinary variables.
        //
        // This includes constant buffers (`uniform` blocks) and well as
        // structured and byte-address buffers (both mapping to `buffer` blocks).
        //
        // We intercept these types, and arrays thereof, to produce the required
        // global declarations. This assumes that earlier "legalization" passes
        // already performed the work of pulling fields with these types out of
        // aggregates.
        //
        // Note: this also assumes that these types are not used as function
        // parameters/results, local variables, etc. Additional legalization
        // steps are required to guarantee these conditions.
        //
    if (auto paramBlockType = as<IRUniformParameterGroupType>(unwrapArray(varType)))
    {
        emitGLSLParameterGroup(varDecl, paramBlockType);
        return true;
    }
    if (auto structuredBufferType = as<IRHLSLStructuredBufferTypeBase>(unwrapArray(varType)))
    {
        emitIRStructuredBuffer_GLSL(varDecl, structuredBufferType);
        return true;
    }
    if (auto byteAddressBufferType = as<IRByteAddressBufferTypeBase>(unwrapArray(varType)))
    {
        emitIRByteAddressBuffer_GLSL(varDecl, byteAddressBufferType);
        return true;
    }

    // We want to skip the declaration of any system-value variables
    // when outputting GLSL (well, except in the case where they
    // actually *require* redeclaration...).
    //
    // Note: these won't be variables the user declare explicitly
    // in their code, but rather variables that we generated as
    // part of legalizing the varying input/output signature of
    // an entry point for GL/Vulkan.
    //
    // TODO: This could be handled more robustly by attaching an
    // appropriate decoration to these variables to indicate their
    // purpose.
    //
    if (auto linkageDecoration = varDecl->findDecoration<IRLinkageDecoration>())
    {
        if (linkageDecoration->getMangledName().startsWith("gl_"))
        {
            // The variable represents an OpenGL system value,
            // so we will assume that it doesn't need to be declared.
            //
            // TODO: handle case where we *should* declare the variable.
            return true;
        }
    }

    // When emitting unbounded-size resource arrays with GLSL we need
    // to use the `GL_EXT_nonuniform_qualifier` extension to ensure
    // that they are not treated as "implicitly-sized arrays" which
    // are arrays that have a fixed size that just isn't specified
    // at the declaration site (instead being inferred from use sites).
    //
    // While the extension primarily introduces the `nonuniformEXT`
    // qualifier that we use to implement `NonUniformResourceIndex`,
    // it also changes the GLSL language semantics around (resource) array
    // declarations that don't specify a size.
    //
    if (as<IRUnsizedArrayType>(varType))
    {
        if (isResourceType(unwrapArray(varType)))
        {
            requireGLSLExtension("GL_EXT_nonuniform_qualifier");
        }
    }

    // Do the default thing
    return false;
}

void GLSLSourceEmitter::emitImageFormatModifierImpl(IRInst* varDecl, IRType* varType)
{
    // As a special case, if we are emitting a GLSL declaration
    // for an HLSL `RWTexture*` then we need to emit a `format` layout qualifier.

    if(auto resourceType = as<IRTextureType>(unwrapArray(varType)))
    {
        switch (resourceType->getAccess())
        {
            case SLANG_RESOURCE_ACCESS_READ_WRITE:
            case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
            {
                emitGLSLImageFormatModifier(varDecl, resourceType);
            }
            break;

            default:
                break;
        }
    }
}

void GLSLSourceEmitter::emitLayoutQualifiersImpl(VarLayout* layout)
{
    // Layout-related modifiers need to come before the declaration,
    // so deal with them here.
    emitGLSLLayoutQualifiers(layout, nullptr);

    // try to emit an appropriate leading qualifier
    for (auto rr : layout->resourceInfos)
    {
        switch (rr.kind)
        {
            case LayoutResourceKind::Uniform:
            case LayoutResourceKind::ShaderResource:
            case LayoutResourceKind::DescriptorTableSlot:
                m_writer->emit("uniform ");
                break;

            case LayoutResourceKind::VaryingInput:
            {
                m_writer->emit("in ");
            }
            break;

            case LayoutResourceKind::VaryingOutput:
            {
                m_writer->emit("out ");
            }
            break;

            case LayoutResourceKind::RayPayload:
            {
                m_writer->emit("rayPayloadInNV ");
            }
            break;

            case LayoutResourceKind::CallablePayload:
            {
                m_writer->emit("callableDataInNV ");
            }
            break;

            case LayoutResourceKind::HitAttributes:
            {
                m_writer->emit("hitAttributeNV ");
            }
            break;

            default:
                continue;
        }

        break;
    }
}


} // namespace Slang
