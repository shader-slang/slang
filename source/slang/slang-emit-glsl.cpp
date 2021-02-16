// slang-emit-glsl.cpp
#include "slang-emit-glsl.h"

#include "../core/slang-writer.h"

#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include "slang-legalize-types.h"

#include <assert.h>

namespace Slang {

SlangResult GLSLSourceEmitter::init()
{
    SLANG_RETURN_ON_FAIL(Super::init());

    // Deal with cases where a particular stage requires certain GLSL versions
    // and/or extensions.
    switch (m_entryPointStage)
    {
        case Stage::AnyHit:
        case Stage::Callable:
        case Stage::ClosestHit:
        case Stage::Intersection:
        case Stage::Miss:
        case Stage::RayGeneration:
        {
            _requireRayTracing();   
            break;
        }
        default: break;
    }

    return SLANG_OK;
}

void GLSLSourceEmitter::_requireRayTracing()
{
    // There is more than one extension that provides ray-tracing capabilities,
    // and we need to pick which one to enable.
    //
    // By default, we will use the `GL_EXT_ray_tracing` extension, but if
    // the user has explicitly opted in to the `GL_NV_ray_tracing` extension
    // we will use that one instead.
    //
    if( getTargetCaps().implies(CapabilityAtom::GL_NV_ray_tracing) )
    {
        m_glslExtensionTracker->requireExtension(UnownedStringSlice::fromLiteral("GL_NV_ray_tracing"));
    }
    else
    {
        m_glslExtensionTracker->requireExtension(UnownedStringSlice::fromLiteral("GL_EXT_ray_tracing"));
        m_glslExtensionTracker->requireSPIRVVersion(SemanticVersion(1, 4));
    }

    m_glslExtensionTracker->requireVersion(ProfileVersion::GLSL_460);
}

void GLSLSourceEmitter::_requireGLSLExtension(const UnownedStringSlice& name)
{
    m_glslExtensionTracker->requireExtension(name);
}

void GLSLSourceEmitter::_requireGLSLVersion(ProfileVersion version)
{
    if (getSourceLanguage() != SourceLanguage::GLSL)
        return;

    m_glslExtensionTracker->requireVersion(version);
}

void GLSLSourceEmitter::_requireSPIRVVersion(const SemanticVersion& version)
{
    m_glslExtensionTracker->requireSPIRVVersion(version);
}

void GLSLSourceEmitter::_requireGLSLVersion(int version)
{
    switch (version)
    {
#define CASE(NUMBER) \
    case NUMBER: _requireGLSLVersion(ProfileVersion::GLSL_##NUMBER); break

        CASE(110);
        CASE(120);
        CASE(130);
        CASE(140);
        CASE(150);
        CASE(330);
        CASE(400);
        CASE(410);
        CASE(420);
        CASE(430);
        CASE(440);
        CASE(450);

#undef CASE
    }
}

void GLSLSourceEmitter::_emitGLSLStructuredBuffer(IRGlobalParam* varDecl, IRHLSLStructuredBufferTypeBase* structuredBufferType)
{
    // Shader storage buffer is an OpenGL 430 feature
    //
    // TODO: we should require either the extension or the version...
    _requireGLSLVersion(430);

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
    emitType(elementType, "_data[]");
    m_writer->emit(";\n");

    m_writer->dedent();
    m_writer->emit("} ");

    m_writer->emit(getName(varDecl));
    emitArrayBrackets(varDecl->getDataType());

    m_writer->emit(";\n");
}

void GLSLSourceEmitter::_emitGLSLByteAddressBuffer(IRGlobalParam* varDecl, IRByteAddressBufferTypeBase* byteAddressBufferType)
{
    // TODO: A lot of this logic is copy-pasted from `emitIRStructuredBuffer_GLSL`.
    // It might be worthwhile to share the common code to avoid regressions sneaking
    // in when one or the other, but not both, gets updated.

    // Shader storage buffer is an OpenGL 430 feature
    //
    // TODO: we should require either the extension or the version...
    _requireGLSLVersion(430);

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

    m_writer->emit(getName(varDecl));
    emitArrayBrackets(varDecl->getDataType());

    m_writer->emit(";\n");
}

void GLSLSourceEmitter::_emitGLSLParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    auto varLayout = getVarLayout(varDecl);
    SLANG_RELEASE_ASSERT(varLayout);

    EmitVarChain blockChain(varLayout);

    EmitVarChain containerChain = blockChain;
    EmitVarChain elementChain = blockChain;

    auto typeLayout = varLayout->getTypeLayout()->unwrapArray();
    if (auto parameterGroupTypeLayout = as<IRParameterGroupTypeLayout>(typeLayout))
    {
        containerChain = EmitVarChain(parameterGroupTypeLayout->getContainerVarLayout(), &blockChain);
        elementChain = EmitVarChain(parameterGroupTypeLayout->getElementVarLayout(), &blockChain);

        typeLayout = parameterGroupTypeLayout->getElementVarLayout()->getTypeLayout();
    }

    /*
    With resources backed by 'buffer' on glsl, we want to output 'readonly' if that is a good match
    for the underlying type. If uniform it's implicit it's readonly

    Here this only happens with isShaderRecord which is a 'constant buffer' (ie implicitly readonly)
    or IRGLSLShaderStorageBufferType which is read write.
    */

    _emitGLSLLayoutQualifier(LayoutResourceKind::DescriptorTableSlot, &containerChain);
    _emitGLSLLayoutQualifier(LayoutResourceKind::PushConstantBuffer, &containerChain);
    bool isShaderRecord = _emitGLSLLayoutQualifier(LayoutResourceKind::ShaderRecord, &containerChain);

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

    emitType(elementType, "_data");
    m_writer->emit(";\n");

    m_writer->dedent();
    m_writer->emit("} ");

    m_writer->emit(getName(varDecl));

    // If the underlying variable was an array (or array of arrays, etc.)
    // we need to emit all those array brackets here.
    emitArrayBrackets(varDecl->getDataType());

    m_writer->emit(";\n");
}

void GLSLSourceEmitter::_emitGLSLImageFormatModifier(IRInst* var, IRTextureType* resourceType)
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
            _requireGLSLExtension(UnownedStringSlice::fromLiteral("GL_EXT_shader_image_load_formatted"));
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
        _requireGLSLExtension(UnownedStringSlice::fromLiteral("GL_EXT_shader_image_load_formatted"));
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

bool GLSLSourceEmitter::_emitGLSLLayoutQualifier(LayoutResourceKind kind, EmitVarChain* chain)
{
    if (!chain)
        return false;
    if (!chain->varLayout->findOffsetAttr(kind))
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
                _requireGLSLExtension(UnownedStringSlice::fromLiteral("GL_ARB_enhanced_layouts"));

                m_writer->emit("layout(offset = ");
                m_writer->emit(index);
                m_writer->emit(")\n");
            }
        }
        break;

        case LayoutResourceKind::VaryingInput:
        case LayoutResourceKind::VaryingOutput:
            m_writer->emit("layout(location = ");
            m_writer->emit(index);
            if( space )
            {
                m_writer->emit(", index = ");
                m_writer->emit(space);
            }
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
            if( getTargetCaps().implies(CapabilityAtom::GL_NV_ray_tracing) )
            {
                m_writer->emit("layout(shaderRecordNV)\n");
            }
            else
            {
                m_writer->emit("layout(shaderRecordEXT)\n");
            }
            break;

    }
    return true;
}

void GLSLSourceEmitter::_emitGLSLLayoutQualifiers(IRVarLayout* layout, EmitVarChain* inChain, LayoutResourceKind filter)
{
    if (!layout) return;

    switch (getSourceLanguage())
    {
        default:
            return;

        case SourceLanguage::GLSL:
            break;
    }

    EmitVarChain chain(layout, inChain);

    for (auto info : layout->getOffsetAttrs())
    {
        // Skip info that doesn't match our filter
        if (filter != LayoutResourceKind::None
            && filter != info->getResourceKind())
        {
            continue;
        }

        _emitGLSLLayoutQualifier(info->getResourceKind(), &chain);
    }
}

void GLSLSourceEmitter::_emitGLSLTextureOrTextureSamplerType(IRTextureTypeBase*  type, char const* baseName)
{
    if (type->getElementType()->getOp() == kIROp_HalfType)
    {
        // Texture access is always as float types if half is specified

    }
    else
    {
        _emitGLSLTypePrefix(type->getElementType(), true);
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

void GLSLSourceEmitter::_emitGLSLTypePrefix(IRType* type, bool promoteHalfToFloat)
{
    switch (type->getOp())
    {
        case kIROp_FloatType:
            // no prefix
            break;

        case kIROp_Int8Type:    m_writer->emit("i8");     break;
        case kIROp_Int16Type:   m_writer->emit("i16");    break;
        case kIROp_IntType:     m_writer->emit("i");      break;
        case kIROp_Int64Type:   
        {
            _requireBaseType(BaseType::Int64);
            m_writer->emit("i64");
            break;
        }

        case kIROp_UInt8Type:   m_writer->emit("u8");     break;
        case kIROp_UInt16Type:  m_writer->emit("u16");    break;
        case kIROp_UIntType:    m_writer->emit("u");      break;

        case kIROp_UInt64Type:
        {
            _requireBaseType(BaseType::UInt64);
            m_writer->emit("u64");
            break;
        }

        case kIROp_BoolType:    m_writer->emit("b");		break;

        case kIROp_HalfType:
        {
            _requireBaseType(BaseType::Half);
            if (promoteHalfToFloat)
            {
                // no prefix
            }
            else
            {
                m_writer->emit("f16");
            }
            break;
        }
        case kIROp_DoubleType:  m_writer->emit("d");		break;

        case kIROp_VectorType:
            _emitGLSLTypePrefix(cast<IRVectorType>(type)->getElementType(), promoteHalfToFloat);
            break;

        case kIROp_MatrixType:
            _emitGLSLTypePrefix(cast<IRMatrixType>(type)->getElementType(), promoteHalfToFloat);
            break;

        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled GLSL type prefix");
            break;
    }
}

void GLSLSourceEmitter::_requireBaseType(BaseType baseType)
{
    m_glslExtensionTracker->requireBaseTypeExtension(baseType);
}

void GLSLSourceEmitter::_maybeEmitGLSLFlatModifier(IRType* valueType)
{
    auto tt = valueType;
    if (auto vecType = as<IRVectorType>(tt))
        tt = vecType->getElementType();
    if (auto vecType = as<IRMatrixType>(tt))
        tt = vecType->getElementType();

    switch (tt->getOp())
    {
        default:
            break;

        case kIROp_IntType:
        case kIROp_UIntType:
        case kIROp_UInt64Type:
            m_writer->emit("flat ");
            break;
    }
}

void GLSLSourceEmitter::emitLoopControlDecorationImpl(IRLoopControlDecoration* decl)
{
    if (decl->getMode() == kIRLoopControl_Unroll)
    {
        // https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_control_flow_attributes.txt
        m_glslExtensionTracker->requireExtension(UnownedStringSlice::fromLiteral("GL_EXT_control_flow_attributes"));
        m_writer->emit("[[unroll]]\n");
    }
    else if (decl->getMode() == kIRLoopControl_Loop)
    {
        m_glslExtensionTracker->requireExtension(UnownedStringSlice::fromLiteral("GL_EXT_control_flow_attributes"));
        m_writer->emit("[[dont_unroll]]\n");
    }
}

void GLSLSourceEmitter::emitSimpleValueImpl(IRInst* inst) 
{
    switch (inst->getOp())
    {
        case kIROp_IntLit:
        {
            auto litInst = static_cast<IRConstant*>(inst);

            IRBasicType* type = as<IRBasicType>(inst->getDataType());
            if (type)
            {
                switch (type->getBaseType())
                {
                    default: 
                    
                    case BaseType::Int8:
                    case BaseType::Int16:
                    case BaseType::Int:
                    {
                        m_writer->emit(litInst->value.intVal);
                        return;
                    }
                    case BaseType::UInt8:
                    case BaseType::UInt16:
                    case BaseType::UInt:
                    {
                        m_writer->emit(UInt(litInst->value.intVal));
                        m_writer->emit("U");
                        return;
                    }
                    case BaseType::Int64:
                    {
                        m_writer->emitInt64(int64_t(litInst->value.intVal));
                        m_writer->emit("L");
                        return;
                    }
                    case BaseType::UInt64:
                    {
                        SLANG_COMPILE_TIME_ASSERT(sizeof(litInst->value.intVal) >= sizeof(uint64_t));
                        m_writer->emitUInt64(uint64_t(litInst->value.intVal));
                        m_writer->emit("UL");
                        return;
                    }

                }
            }
            break;   
        }
        case kIROp_FloatLit:
        {
            IRConstant* constantInst = static_cast<IRConstant*>(inst);

            IRConstant::FloatKind kind = constantInst->getFloatKind();

            switch (kind)
            {
                case IRConstant::FloatKind::Nan:
                {
                    m_writer->emit("(0.0 / 0.0)");
                    return;
                }
                case IRConstant::FloatKind::PositiveInfinity:
                {
                    m_writer->emit("(1.0 / 0.0)");
                    return;
                }
                case IRConstant::FloatKind::NegativeInfinity:
                {
                    m_writer->emit("(-1.0 / 0.0)");
                    return;
                }
                default: break;
            }
            break;
        }

        default: break;
    }

    Super::emitSimpleValueImpl(inst);
}


void GLSLSourceEmitter::emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    _emitGLSLParameterGroup(varDecl, type);
}

void GLSLSourceEmitter::emitEntryPointAttributesImpl(IRFunc* irFunc, IREntryPointDecoration* entryPointDecor)
{
    SLANG_ASSERT(entryPointDecor);

    auto profile = entryPointDecor->getProfile();
    auto stage = profile.getStage();

    switch (stage)
    {
        case Stage::Compute:
        {
            Int sizeAlongAxis[kThreadGroupAxisCount];
            getComputeThreadGroupSize(irFunc, sizeAlongAxis);

            m_writer->emit("layout(");
            char const* axes[] = { "x", "y", "z" };
            for (int ii = 0; ii < kThreadGroupAxisCount; ++ii)
            {
                if (ii != 0) m_writer->emit(", ");
                m_writer->emit("local_size_");
                m_writer->emit(axes[ii]);
                m_writer->emit(" = ");
                m_writer->emit(sizeAlongAxis[ii]);
            }
            m_writer->emit(") in;\n");
        }
        break;
        case Stage::Geometry:
        {
            if (auto decor = irFunc->findDecoration<IRMaxVertexCountDecoration>())
            {
                auto count = getIntVal(decor->getCount());
                m_writer->emit("layout(max_vertices = ");
                m_writer->emit(Int(count));
                m_writer->emit(") out;\n");
            }

            if (auto decor = irFunc->findDecoration<IRInstanceDecoration>())
            {
                auto count = getIntVal(decor->getCount());
                m_writer->emit("layout(invocations = ");
                m_writer->emit(Int(count));
                m_writer->emit(") in;\n");
            }

            // These decorations were moved from the parameters to the entry point by ir-glsl-legalize.
            // The actual parameters have become potentially multiple global parameters.
            if (auto decor = irFunc->findDecoration<IRGeometryInputPrimitiveTypeDecoration>())
            {
                switch (decor->getOp())
                {
                    case kIROp_TriangleInputPrimitiveTypeDecoration:       m_writer->emit("layout(triangles) in;\n"); break;
                    case kIROp_LineInputPrimitiveTypeDecoration:           m_writer->emit("layout(lines) in;\n"); break;
                    case kIROp_LineAdjInputPrimitiveTypeDecoration:        m_writer->emit("layout(lines_adjacency) in;\n"); break;
                    case kIROp_PointInputPrimitiveTypeDecoration:          m_writer->emit("layout(points) in;\n"); break;
                    case kIROp_TriangleAdjInputPrimitiveTypeDecoration:    m_writer->emit("layout(triangles_adjacency) in;\n"); break;
                    default:
                    {
                        SLANG_ASSERT(!"Unknown primitive type");
                    }
                }
            }

            if (auto decor = irFunc->findDecoration<IRStreamOutputTypeDecoration>())
            {
                IRType* type = decor->getStreamType();

                switch (type->getOp())
                {
                    case kIROp_HLSLPointStreamType:     m_writer->emit("layout(points) out;\n"); break;
                    case kIROp_HLSLLineStreamType:      m_writer->emit("layout(line_strip) out;\n"); break;
                    case kIROp_HLSLTriangleStreamType:  m_writer->emit("layout(triangle_strip) out;\n"); break;
                    default: SLANG_ASSERT(!"Unknown stream out type");
                }    
            }
        }
        break;
        case Stage::Pixel:
        {
            if (irFunc->findDecoration<IREarlyDepthStencilDecoration>())
            {
                // https://www.khronos.org/opengl/wiki/Early_Fragment_Test
                m_writer->emit("layout(early_fragment_tests) in;\n");
            }
            break;
        }
        // TODO: There are other stages that will need this kind of handling.
        default:
            break;
    }
}

bool GLSLSourceEmitter::tryEmitGlobalParamImpl(IRGlobalParam* varDecl, IRType* varType)
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
        _emitGLSLParameterGroup(varDecl, paramBlockType);
        return true;
    }
    if (auto structuredBufferType = as<IRHLSLStructuredBufferTypeBase>(unwrapArray(varType)))
    {
        _emitGLSLStructuredBuffer(varDecl, structuredBufferType);
        return true;
    }
    if (auto byteAddressBufferType = as<IRByteAddressBufferTypeBase>(unwrapArray(varType)))
    {
        _emitGLSLByteAddressBuffer(varDecl, byteAddressBufferType);
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
            _requireGLSLExtension(UnownedStringSlice::fromLiteral("GL_EXT_nonuniform_qualifier"));
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
                _emitGLSLImageFormatModifier(varDecl, resourceType);
            }
            break;

            default:
                break;
        }
    }
}

void GLSLSourceEmitter::emitLayoutQualifiersImpl(IRVarLayout* layout)
{
    // Layout-related modifiers need to come before the declaration,
    // so deal with them here.
    _emitGLSLLayoutQualifiers(layout, nullptr);

    // try to emit an appropriate leading qualifier
    for (auto rr : layout->getOffsetAttrs())
    {
        switch (rr->getResourceKind())
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
                if( getTargetCaps().implies(CapabilityAtom::GL_NV_ray_tracing) )
                {
                    m_writer->emit("rayPayloadInNV ");
                }
                else
                {
                    m_writer->emit("rayPayloadInEXT ");
                }
            }
            break;

            case LayoutResourceKind::CallablePayload:
            {
                if( getTargetCaps().implies(CapabilityAtom::GL_NV_ray_tracing) )
                {
                    m_writer->emit("callableDataInNV ");
                }
                else
                {
                    m_writer->emit("callableDataInEXT ");
                }
            }
            break;

            case LayoutResourceKind::HitAttributes:
            {
                if( getTargetCaps().implies(CapabilityAtom::GL_NV_ray_tracing) )
                {
                    m_writer->emit("hitAttributeNV ");
                }
                else
                {
                    m_writer->emit("hitAttributeEXT ");
                }
            }
            break;

            default:
                continue;
        }

        break;
    }
}

static const char* _getGLSLVectorCompareFunctionName(IROp op)
{
    // Glsl vector comparisons use functions...
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/equal.xhtml

    switch (op)
    {
        case kIROp_Eql:     return "equal";
        case kIROp_Neq:     return "notEqual";
        case kIROp_Greater: return "greaterThan";
        case kIROp_Less:    return "lessThan";
        case kIROp_Geq:     return "greaterThanEqual";
        case kIROp_Leq:     return "lessThanEqual";
        default:    return nullptr;
    }
}

void GLSLSourceEmitter::_maybeEmitGLSLCast(IRType* castType, IRInst* inst)
{
    // Wrap in cast if a cast type is specified
    if (castType)
    {
        emitType(castType);
        m_writer->emit("(");

        // Emit the operand
        emitOperand(inst, getInfo(EmitOp::General));

        m_writer->emit(")");
    }
    else
    {
        // Emit the operand
        emitOperand(inst, getInfo(EmitOp::General));
    }
}

void GLSLSourceEmitter::_emitLegalizedBoolVectorBinOp(IRInst* inst, IRVectorType* type, const EmitOpInfo& op, const EmitOpInfo& inOuterPrec)
{
    auto elementCount = type->getElementCount();

    EmitOpInfo outerPrec = inOuterPrec;
    auto prec = getInfo(EmitOp::Postfix);
    bool needClose = maybeEmitParens(outerPrec, prec);

    emitType(type);
    m_writer->emit("(uvec");
    emitSimpleValue(elementCount);
    m_writer->emit("(");
    emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
    m_writer->emit(")");
    m_writer->emit(op.op);
    m_writer->emit("uvec");
    emitSimpleValue(elementCount);
    m_writer->emit("(");
    emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
    m_writer->emit("))");

    maybeCloseParens(needClose);
}

bool GLSLSourceEmitter::_tryEmitLogicalBinOp(IRInst* inst, const EmitOpInfo& bitOp, const EmitOpInfo& inOuterPrec)
{
    // Logical operation on scalar `bool` values are directly
    // supported by GLSL. They have short-circuiting behavior,
    // but we need not worry about that because our logic
    // for folding sub-expressions into their use sites will
    // never fold a sub-expression that would have side effects.
    //
    // Thus we fall back to the default handling for scalar
    // cases (which should only arise for `bool` operands).
    //
    IRType* type = inst->getDataType();
    auto vectorType = as<IRVectorType>(type);
    if(!vectorType)
        return false;

    // For vector cases, we need to convert the operands to
    // a type that supports vector operations, and then use
    // bit operations there.
    //
    _emitLegalizedBoolVectorBinOp(inst, vectorType, bitOp, inOuterPrec);
    return true;
}

bool GLSLSourceEmitter::_tryEmitBitBinOp(IRInst* inst, const EmitOpInfo& bitOp, const EmitOpInfo& boolOp, const EmitOpInfo& inOuterPrec)
{
    // The bitwise binary operations are supported in GLSL,
    // but do not support `bool` or vector-of-`bool` operands.
    //
    // We start by checking if we have a `bool`-based case,
    // and fall back to the default emit logic if not.
    //
    IRType* type = inst->getDataType();
    IRType* elementType = type;
    auto vectorType = as<IRVectorType>(type);
    if(vectorType)
        elementType = vectorType->getElementType();
    if(!as<IRBoolType>(elementType))
        return false;

    // If we have a vector case, then it will be handled
    // by casting the `bool` vectors to vectors of
    // integers and doing the bitwise op there, where
    // it should yield an equivalent result.
    //
    if(vectorType)
    {
        _emitLegalizedBoolVectorBinOp(inst, vectorType, bitOp, inOuterPrec);
    }
    else
    {
        // In the scalar case, we will translate
        // bitwise operations on `bool` values to
        // the equivalent logical operation, knowing
        // that our appraoch to folding of sub-expressions
        // into use sites will avoid any potential issues
        // around short-circuiting behavior.
        //
        auto prec = boolOp;
        EmitOpInfo outerPrec = inOuterPrec;
        bool needClose = maybeEmitParens(outerPrec, prec);

        emitOperand(inst->getOperand(0), leftSide(outerPrec, prec));
        m_writer->emit(prec.op);
        emitOperand(inst->getOperand(1), rightSide(outerPrec, prec));

        maybeCloseParens(needClose);
    }
    return true;

}

bool GLSLSourceEmitter::tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec)
{
    switch (inst->getOp())
    {
        case kIROp_constructVectorFromScalar:
        {
            // Simple constructor call
            EmitOpInfo outerPrec = inOuterPrec;
            bool needClose = false;

            auto prec = getInfo(EmitOp::Postfix);
            needClose = maybeEmitParens(outerPrec, prec);

            emitType(inst->getDataType());
            m_writer->emit("(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(")");

            maybeCloseParens(needClose);
            // Handled
            return true;
        }
        case kIROp_Mul:
        {
            // Component-wise multiplication needs to be special cased,
            // because GLSL uses infix `*` to express inner product
            // when working with matrices.

            // Are we targetting GLSL, and are both operands matrices?
            if (as<IRMatrixType>(inst->getOperand(0)->getDataType())
                && as<IRMatrixType>(inst->getOperand(1)->getDataType()))
            {
                m_writer->emit("matrixCompMult(");
                emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
                m_writer->emit(", ");
                emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
                m_writer->emit(")");
                return true;
            }
            break;
        }
        case kIROp_Select:
        {
            if (inst->getOperand(0)->getDataType()->getOp() != kIROp_BoolType)
            {
                // For GLSL, emit a call to `mix` if condition is a vector
                m_writer->emit("mix(");
                emitOperand(inst->getOperand(2), leftSide(getInfo(EmitOp::General), getInfo(EmitOp::General)));
                m_writer->emit(", ");
                emitOperand(inst->getOperand(1), leftSide(getInfo(EmitOp::General), getInfo(EmitOp::General)));
                m_writer->emit(", ");
                emitOperand(inst->getOperand(0), leftSide(getInfo(EmitOp::General), getInfo(EmitOp::General)));
                m_writer->emit(")");
                return true;
            }
            break;
        }
        case kIROp_BitCast:
        {
            auto toType = extractBaseType(inst->getDataType());
            switch (toType)
            {
                default:
                    diagnoseUnhandledInst(inst);
                    break;

                case BaseType::UInt:
                    break;

                case BaseType::Int:
                    emitType(inst->getDataType());
                    break;

                case BaseType::Float:
                    m_writer->emit("uintBitsToFloat");
                    break;
            }

            m_writer->emit("(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(")");

            return true;
        }
        case kIROp_And:
            return _tryEmitLogicalBinOp(inst, getInfo(EmitOp::BitAnd), inOuterPrec);
        case kIROp_Or:
            return _tryEmitLogicalBinOp(inst, getInfo(EmitOp::BitOr), inOuterPrec);
        case kIROp_Not:
        {
            IRInst* operand = inst->getOperand(0);
            if (auto vectorType = as<IRVectorType>(operand->getDataType()))
            {
                EmitOpInfo outerPrec = inOuterPrec;
                bool needClose = false;

                // Handle as a function call
                auto prec = getInfo(EmitOp::Postfix);
                needClose = maybeEmitParens(outerPrec, prec);

                m_writer->emit("not(");
                emitOperand(operand, getInfo(EmitOp::General));
                m_writer->emit(")");

                maybeCloseParens(needClose);
                return true;
            }
            return false;
        }

        // When emitting a bitwise operation in GLSL, we need to special-case the handling
        // of `bool` and vectors of `bool` so that they produce valid results by operating
        // on the single-bit truth value.
        //
        // In the case of a vector we will convert to `uint` vectors and perform the
        // bitwise op on them before converting back to `bool` vectors.
        //
        // In the scalar case we will apply the corresponding logical operation to
        // the `bool` operands.
        //
        case kIROp_BitAnd:
            return _tryEmitBitBinOp(inst, getInfo(EmitOp::BitAnd), getInfo(EmitOp::And), inOuterPrec);
        case kIROp_BitOr:
            return _tryEmitBitBinOp(inst, getInfo(EmitOp::BitOr), getInfo(EmitOp::Or), inOuterPrec);
        case kIROp_BitXor:
            // Note: on scalar `bool` operands, a bitwise XOR (`^`) is equivalent to a not-equal (`!=`) comparison.
            return _tryEmitBitBinOp(inst, getInfo(EmitOp::BitXor), getInfo(EmitOp::Neq), inOuterPrec);

        // Comparisons
        case kIROp_Eql:
        case kIROp_Neq:
        case kIROp_Greater:
        case kIROp_Less:
        case kIROp_Geq:
        case kIROp_Leq:
        {
            // If the comparison is between vectors use GLSL vector comparisons
            IRInst* left = inst->getOperand(0);
            IRInst* right = inst->getOperand(1);

            auto leftVectorType = as<IRVectorType>(left->getDataType());
            auto rightVectorType = as<IRVectorType>(right->getDataType());

            // If either side is a vector handle as a vector
            if (leftVectorType || rightVectorType)
            {
                const char* funcName = _getGLSLVectorCompareFunctionName(inst->getOp());
                SLANG_ASSERT(funcName);

                // Determine the vector type
                const auto vecType = leftVectorType ? leftVectorType : rightVectorType;

                // Handle as a function call
                auto prec = getInfo(EmitOp::Postfix);

                EmitOpInfo outerPrec = inOuterPrec;
                bool needClose = maybeEmitParens(outerPrec, outerPrec);

                m_writer->emit(funcName);
                m_writer->emit("(");
                _maybeEmitGLSLCast((leftVectorType ? nullptr : vecType), left);
                m_writer->emit(",");
                _maybeEmitGLSLCast((rightVectorType ? nullptr : vecType), right);
                m_writer->emit(")");

                maybeCloseParens(needClose);

                return true;
            }

            // Use the default
            break;
        }
        case kIROp_FRem:
        {
            IRInst* left = inst->getOperand(0);
            IRInst* right = inst->getOperand(1);

            // Handle as a function call
            auto prec = getInfo(EmitOp::Postfix);

            EmitOpInfo outerPrec = inOuterPrec;
            bool needClose = maybeEmitParens(outerPrec, outerPrec);

            // TODO: the GLSL `mod` function amounts to a floating-point
            // modulus rather than a floating-point remainder. We need
            // to fix this to emit the right SPIR-V opcode, but there is
            // no built-in GLSL function that maps to the opcode we want.
            //
            m_writer->emit("mod(");
            emitOperand(left, getInfo(EmitOp::General));
            m_writer->emit(",");
            emitOperand(right, getInfo(EmitOp::General));
            m_writer->emit(")");

            maybeCloseParens(needClose);

            return true;
        }
        // TODO: We should also special-case `kIROp_IRem` here,
        // so that we emit a remainder instead of a modulus. As for
        // `FRem` there is no direct GLSL translation, so we will
        // leave things with the default behavior for now.

        case kIROp_StringLit:
        {
            IRStringLit* lit = cast<IRStringLit>(inst);
            const UnownedStringSlice slice = lit->getStringSlice();
            m_writer->emit(int32_t(getStableHashCode32(slice.begin(), slice.getLength())));
            return true;
        }
        case kIROp_GetStringHash:
        {
            // On GLSL target, the `String` type is just an `int`
            // that is the hash of the string, so we can emit
            // the first operand to `getStringHash` directly.
            //
            EmitOpInfo outerPrec = inOuterPrec;
            emitOperand(inst->getOperand(0), outerPrec);
            return true;
        }
        case kIROp_StructuredBufferLoad:
        {
            auto outerPrec = inOuterPrec;
            auto prec = getInfo(EmitOp::Postfix);
            bool needClose = maybeEmitParens(outerPrec, prec);

            emitOperand(inst->getOperand(0), leftSide(outerPrec, prec));
            m_writer->emit("._data[");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit("]");

            maybeCloseParens(needClose);
            return true;
        }
        case kIROp_StructuredBufferStore:
        {
            auto outerPrec = inOuterPrec;

            auto assignPrec = getInfo(EmitOp::Assign);
            bool assignNeedsClose = maybeEmitParens(outerPrec, assignPrec);

            {
                auto subscriptPrec = getInfo(EmitOp::Postfix);
                bool subscriptNeedsClose = maybeEmitParens(assignPrec, subscriptPrec);

                emitOperand(inst->getOperand(0), leftSide(assignPrec, subscriptPrec));
                m_writer->emit("._data[");
                emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
                m_writer->emit("]");

                maybeCloseParens(subscriptNeedsClose);
            }

            m_writer->emit(" = ");
            emitOperand(inst->getOperand(2), rightSide(assignPrec, outerPrec));
            maybeCloseParens(assignNeedsClose);
            return true;
        }
        default: break;
    }

    // Not handled
    return false;
}

void GLSLSourceEmitter::handleRequiredCapabilitiesImpl(IRInst* inst)
{
    // Does this function declare any requirements on GLSL version or
    // extensions, which should affect our output?

    for (auto decoration : inst->getDecorations())
    {
        switch (decoration->getOp())
        {
            default:
                break;

            case kIROp_RequireGLSLExtensionDecoration:
            {
                _requireGLSLExtension(((IRRequireGLSLExtensionDecoration*)decoration)->getExtensionName());
                break;
            }
            case kIROp_RequireGLSLVersionDecoration:
            {
                _requireGLSLVersion(int(((IRRequireGLSLVersionDecoration*)decoration)->getLanguageVersion()));
                break;
            }
            case kIROp_RequireSPIRVVersionDecoration:
            {
                auto intValue = static_cast<IRRequireSPIRVVersionDecoration*>(decoration)->getSPIRVVersion();
                SemanticVersion version;
                version.setFromInteger(SemanticVersion::IntegerType(intValue));
                _requireSPIRVVersion(version);
                break;
            }

        }
    }
}

void GLSLSourceEmitter::emitPreprocessorDirectivesImpl()
{
    auto effectiveProfile = m_effectiveProfile;
    if (effectiveProfile.getFamily() == ProfileFamily::GLSL)
    {
        _requireGLSLVersion(effectiveProfile.getVersion());
    }

    // HACK: We aren't picking GLSL versions carefully right now,
    // and so we might end up only requiring the initial 1.10 version,
    // even though even basic functionality needs a higher version.
    //
    // For now, we'll work around this by just setting the minimum required
    // version to a high one:
    //
    // TODO: Either correctly compute a minimum required version, or require
    // the user to specify a version as part of the target.
    m_glslExtensionTracker->requireVersion(ProfileVersion::GLSL_450);

    auto requiredProfileVersion = m_glslExtensionTracker->getRequiredProfileVersion();
    switch (requiredProfileVersion)
    {
#define CASE(TAG, VALUE)    \
case ProfileVersion::TAG: m_writer->emit("#version " #VALUE "\n"); return

        CASE(GLSL_110, 110);
        CASE(GLSL_120, 120);
        CASE(GLSL_130, 130);
        CASE(GLSL_140, 140);
        CASE(GLSL_150, 150);
        CASE(GLSL_330, 330);
        CASE(GLSL_400, 400);
        CASE(GLSL_410, 410);
        CASE(GLSL_420, 420);
        CASE(GLSL_430, 430);
        CASE(GLSL_440, 440);
        CASE(GLSL_450, 450);
        CASE(GLSL_460, 460);
#undef CASE

        default:
            break;
    }

    // No information is available for us to guess a profile,
    // so it seems like we need to pick one out of thin air.
    //
    // Ideally we should infer a minimum required version based
    // on the constructs we have seen used in the user's code
    //
    // For now we just fall back to a reasonably recent version.

    m_writer->emit("#version 420\n");
}

void GLSLSourceEmitter::emitLayoutDirectivesImpl(TargetRequest* targetReq)
{
    // Reminder: the meaning of row/column major layout
    // in our semantics is the *opposite* of what GLSL
    // calls them, because what they call "columns"
    // are what we call "rows."
    //
    switch (targetReq->getDefaultMatrixLayoutMode())
    {
        case kMatrixLayoutMode_RowMajor:
        default:
            m_writer->emit("layout(column_major) uniform;\n");
            m_writer->emit("layout(column_major) buffer;\n");
            break;

        case kMatrixLayoutMode_ColumnMajor:
            m_writer->emit("layout(row_major) uniform;\n");
            m_writer->emit("layout(row_major) buffer;\n");
            break;
    }
}

void GLSLSourceEmitter::emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
{
    if (elementCount > 1)
    {
        _emitGLSLTypePrefix(elementType);
        m_writer->emit("vec");
        m_writer->emit(elementCount);
    }
    else
    {
        emitSimpleType(elementType);
    }
}


void GLSLSourceEmitter::emitSimpleTypeImpl(IRType* type)
{
    switch (type->getOp())
    {
        case kIROp_Int64Type:
        {
            _requireBaseType(BaseType::Int64);
            m_writer->emit(getDefaultBuiltinTypeName(type->getOp()));
            return;
        }
        case kIROp_UInt64Type:
        {
            _requireBaseType(BaseType::UInt64);
            m_writer->emit(getDefaultBuiltinTypeName(type->getOp()));
            return;
        }
        case kIROp_VoidType:
        case kIROp_BoolType:
        case kIROp_Int8Type:
        case kIROp_Int16Type:
        case kIROp_IntType:
        case kIROp_UInt8Type:
        case kIROp_UInt16Type:
        case kIROp_UIntType:
        case kIROp_FloatType:   
        case kIROp_DoubleType:
        {
            _requireBaseType(cast<IRBasicType>(type)->getBaseType());
            m_writer->emit(getDefaultBuiltinTypeName(type->getOp()));
            return;
        }
        case kIROp_HalfType:
        {
            _requireBaseType(BaseType::Half);
            m_writer->emit("float16_t");
            return;
        }
        case kIROp_StructType:
            m_writer->emit(getName(type));
            return;

        case kIROp_VectorType:
        {
            auto vecType = (IRVectorType*)type;
            emitVectorTypeNameImpl(vecType->getElementType(), getIntVal(vecType->getElementCount()));
            return;
        }
        case kIROp_MatrixType:
        {
            auto matType = (IRMatrixType*)type;

            _emitGLSLTypePrefix(matType->getElementType());
            m_writer->emit("mat");
            emitVal(matType->getRowCount(), getInfo(EmitOp::General));
            // TODO(tfoley): only emit the next bit
            // for non-square matrix
            m_writer->emit("x");
            emitVal(matType->getColumnCount(), getInfo(EmitOp::General));
            return;
        }
        case kIROp_SamplerStateType:
        case kIROp_SamplerComparisonStateType:
        {
            auto samplerStateType = cast<IRSamplerStateTypeBase>(type);
            switch (samplerStateType->getOp())
            {
                case kIROp_SamplerStateType:			m_writer->emit("sampler");		break;
                case kIROp_SamplerComparisonStateType:	m_writer->emit("samplerShadow");	break;
                default:
                    SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled sampler state flavor");
                    break;
            }
            return;
        }
        case kIROp_StringType: m_writer->emit("int"); return;
        default: break;
    }

    // TODO: Ideally the following should be data-driven,
    // based on meta-data attached to the definitions of
    // each of these IR opcodes.
    if (auto texType = as<IRTextureType>(type))
    {
        switch (texType->getAccess())
        {
            case SLANG_RESOURCE_ACCESS_READ_WRITE:
            case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
                _emitGLSLTextureOrTextureSamplerType(texType, "image");
                break;

            default:
                _emitGLSLTextureOrTextureSamplerType(texType, "texture");
                break;
        }
        return;
    }
    else if (auto textureSamplerType = as<IRTextureSamplerType>(type))
    {
        _emitGLSLTextureOrTextureSamplerType(textureSamplerType, "sampler");
        return;
    }
    else if (auto imageType = as<IRGLSLImageType>(type))
    {
        _emitGLSLTextureOrTextureSamplerType(imageType, "image");
        return;
    }
    else if (auto structuredBufferType = as<IRHLSLStructuredBufferTypeBase>(type))
    {
        // TODO: We desugar global variables with structured-buffer type into GLSL
        // `buffer` declarations, but we don't currently handle structured-buffer types
        // in other contexts (e.g., as function parameters). The simplest thing to do
        // would be to emit a `StructuredBuffer<Foo>` as `Foo[]` and `RWStructuredBuffer<Foo>`
        // as `in out Foo[]`, but that is starting to get into the realm of transformations
        // that should really be handled during legalization, rather than during emission.
        //
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "structured buffer type used unexpectedly");
        return;
    }
    else if (auto untypedBufferType = as<IRUntypedBufferResourceType>(type))
    {
        switch (untypedBufferType->getOp())
        {
            case kIROp_RaytracingAccelerationStructureType:
            {
                _requireRayTracing();

                if( getTargetCaps().implies(CapabilityAtom::GL_NV_ray_tracing) )
                {
                    m_writer->emit("accelerationStructureNV");
                }
                else
                {
                    m_writer->emit("accelerationStructureEXT");
                }
                break;
            }

                // TODO: These "translations" are obviously wrong for GLSL.
            case kIROp_HLSLByteAddressBufferType:                   m_writer->emit("ByteAddressBuffer");                  break;
            case kIROp_HLSLRWByteAddressBufferType:                 m_writer->emit("RWByteAddressBuffer");                break;
            case kIROp_HLSLRasterizerOrderedByteAddressBufferType:  m_writer->emit("RasterizerOrderedByteAddressBuffer"); break;

            default:
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled buffer type");
                break;
        }

        return;
    }

    SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled type");
}

void GLSLSourceEmitter::emitRateQualifiersImpl(IRRate* rate)
{
    if (as<IRConstExprRate>(rate))
    {
        m_writer->emit("const ");
        
    }
    else if (as<IRGroupSharedRate>(rate))
    {
        m_writer->emit("shared ");
    }
}

static UnownedStringSlice _getInterpolationModifierText(IRInterpolationMode mode)
{
    switch (mode)
    {
        case IRInterpolationMode::NoInterpolation:      return UnownedStringSlice::fromLiteral("flat");
        case IRInterpolationMode::NoPerspective:        return UnownedStringSlice::fromLiteral("noperspective");
        case IRInterpolationMode::Linear:               return UnownedStringSlice::fromLiteral("smooth");
        case IRInterpolationMode::Sample:               return UnownedStringSlice::fromLiteral("sample");
        case IRInterpolationMode::Centroid:             return UnownedStringSlice::fromLiteral("centroid");
        default:                                        return UnownedStringSlice();
    }
}

void GLSLSourceEmitter::emitInterpolationModifiersImpl(IRInst* varInst, IRType* valueType, IRVarLayout* layout)
{
    bool anyModifiers = false;

    for (auto dd : varInst->getDecorations())
    {
        if (dd->getOp() != kIROp_InterpolationModeDecoration)
            continue;

        auto decoration = (IRInterpolationModeDecoration*)dd;
        const UnownedStringSlice slice = _getInterpolationModifierText(decoration->getMode());

        if (slice.getLength())
        {
            m_writer->emit(slice);
            m_writer->emitChar(' ');
            anyModifiers = true;
        }
    }

    // If the user didn't explicitly qualify a varying
    // with integer type, then we need to explicitly
    // add the `flat` modifier for GLSL.
    if (!anyModifiers)
    {
        // Only emit a default `flat` for fragment
        // stage varying inputs.
        //
        // TODO: double-check that this works for
        // signature matching even if the producing
        // stage didn't use `flat`.
        //
        // If this ends up being a problem we can instead
        // output everything with `flat` except for
        // fragment *outputs* (and maybe vertex inputs).
        //
        if (layout && layout->getStage() == Stage::Fragment
            && layout->usesResourceKind(LayoutResourceKind::VaryingInput))
        {
            _maybeEmitGLSLFlatModifier(valueType);
        }
    }
}

void GLSLSourceEmitter::emitVarDecorationsImpl(IRInst* varDecl)
{
    // Deal with Vulkan raytracing layout stuff *before* we
    // do the check for whether `layout` is null, because
    // the payload won't automatically get a layout applied
    // (it isn't part of the user-visible interface...)
    //
    if (varDecl->findDecoration<IRVulkanRayPayloadDecoration>())
    {
        m_writer->emit("layout(location = ");
        m_writer->emit(getRayPayloadLocation(varDecl));
        m_writer->emit(")\n");
        if( getTargetCaps().implies(CapabilityAtom::GL_NV_ray_tracing) )
        {
            m_writer->emit("rayPayloadNV\n");
        }
        else
        {
            m_writer->emit("rayPayloadEXT\n");
        }
    }
    if (varDecl->findDecoration<IRVulkanCallablePayloadDecoration>())
    {
        m_writer->emit("layout(location = ");
        m_writer->emit(getCallablePayloadLocation(varDecl));
        m_writer->emit(")\n");
        if( getTargetCaps().implies(CapabilityAtom::GL_NV_ray_tracing) )
        {
            m_writer->emit("callableDataNV\n");
        }
        else
        {
            m_writer->emit("callableDataEXT\n");
        }
    }

    if (varDecl->findDecoration<IRVulkanHitAttributesDecoration>())
    {
        if( getTargetCaps().implies(CapabilityAtom::GL_NV_ray_tracing) )
        {
            m_writer->emit("hitAttributeNV\n");
        }
        else
        {
            m_writer->emit("hitAttributeEXT\n");
        }
    }

    if (varDecl->findDecoration<IRGloballyCoherentDecoration>())
    {
        m_writer->emit("coherent\n");
    }
}

void GLSLSourceEmitter::emitMatrixLayoutModifiersImpl(IRVarLayout* layout)
{
    // When a variable has a matrix type, we want to emit an explicit
    // layout qualifier based on what the layout has been computed to be.
    //

    auto typeLayout = layout->getTypeLayout()->unwrapArray();

    if (auto matrixTypeLayout = as<IRMatrixTypeLayout>(typeLayout))
    {
        // Reminder: the meaning of row/column major layout
        // in our semantics is the *opposite* of what GLSL
        // calls them, because what they call "columns"
        // are what we call "rows."
        //
        switch (matrixTypeLayout->getMode())
        {
            case kMatrixLayoutMode_ColumnMajor:
                m_writer->emit("layout(row_major)\n");
                break;

            case kMatrixLayoutMode_RowMajor:
                m_writer->emit("layout(column_major)\n");
                break;
        }
    }
}


} // namespace Slang
