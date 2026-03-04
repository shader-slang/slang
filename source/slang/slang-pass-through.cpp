// slang-pass-through.cpp
#include "slang-pass-through.h"

#include "../core/slang-type-text-util.h"
#include "compiler-core/slang-slice-allocator.h"
#include "slang-compiler.h"

namespace Slang
{

void printDiagnosticArg(StringBuilder& sb, PassThroughMode val)
{
    sb << TypeTextUtil::getPassThroughName(SlangPassThrough(val));
}

SourceLanguage getDefaultSourceLanguageForDownstreamCompiler(PassThroughMode compiler)
{
    switch (compiler)
    {
    case PassThroughMode::None:
        {
            return SourceLanguage::Unknown;
        }
    case PassThroughMode::Fxc:
    case PassThroughMode::Dxc:
        {
            return SourceLanguage::HLSL;
        }
    case PassThroughMode::Glslang:
        {
            return SourceLanguage::GLSL;
        }
    case PassThroughMode::LLVM:
    case PassThroughMode::Clang:
    case PassThroughMode::VisualStudio:
    case PassThroughMode::Gcc:
    case PassThroughMode::GenericCCpp:
        {
            // These could ingest C, but we only have this function to work out a
            // 'default' language to ingest.
            return SourceLanguage::CPP;
        }
    case PassThroughMode::NVRTC:
        {
            return SourceLanguage::CUDA;
        }
    case PassThroughMode::Tint:
        {
            return SourceLanguage::WGSL;
        }
    case PassThroughMode::SpirvDis:
        {
            return SourceLanguage::SPIRV;
        }
    case PassThroughMode::MetalC:
        {
            return SourceLanguage::Metal;
        }
    default:
        break;
    }
    SLANG_ASSERT(!"Unknown compiler");
    return SourceLanguage::Unknown;
}

PassThroughMode getDownstreamCompilerRequiredForTarget(CodeGenTarget target)
{
    switch (target)
    {
    // Don't *require* a downstream compiler for source output
    case CodeGenTarget::GLSL:
    case CodeGenTarget::HLSL:
    case CodeGenTarget::CUDASource:
    case CodeGenTarget::CUDAHeader:
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CPPHeader:
    case CodeGenTarget::HostCPPSource:
    case CodeGenTarget::PyTorchCppBinding:
    case CodeGenTarget::CSource:
    case CodeGenTarget::Metal:
    case CodeGenTarget::WGSL:
    case CodeGenTarget::HostLLVMIR:
    case CodeGenTarget::ShaderLLVMIR:
    case CodeGenTarget::HostObjectCode:
    case CodeGenTarget::ShaderObjectCode:
        {
            return PassThroughMode::None;
        }
    case CodeGenTarget::None:
        {
            return PassThroughMode::None;
        }
    case CodeGenTarget::WGSLSPIRVAssembly:
    case CodeGenTarget::SPIRVAssembly:
    case CodeGenTarget::SPIRV:
        {
            return PassThroughMode::SpirvDis;
        }
    case CodeGenTarget::DXBytecode:
    case CodeGenTarget::DXBytecodeAssembly:
        {
            return PassThroughMode::Fxc;
        }
    case CodeGenTarget::DXIL:
    case CodeGenTarget::DXILAssembly:
        {
            return PassThroughMode::Dxc;
        }
    case CodeGenTarget::MetalLib:
    case CodeGenTarget::MetalLibAssembly:
        {
            return PassThroughMode::MetalC;
        }
    case CodeGenTarget::ShaderHostCallable:
    case CodeGenTarget::ShaderSharedLibrary:
    case CodeGenTarget::HostExecutable:
    case CodeGenTarget::HostHostCallable:
    case CodeGenTarget::HostSharedLibrary:
        {
            // We need some C/C++ compiler
            return PassThroughMode::GenericCCpp;
        }
    case CodeGenTarget::PTX:
        {
            return PassThroughMode::NVRTC;
        }
    case CodeGenTarget::WGSLSPIRV:
        {
            return PassThroughMode::Tint;
        }
    default:
        break;
    }

    SLANG_ASSERT(!"Unhandled target");
    return PassThroughMode::None;
}


void reportExternalCompileError(
    const char* compilerName,
    Severity severity,
    SlangResult res,
    const UnownedStringSlice& diagnostic,
    DiagnosticSink* sink)
{
    StringBuilder builder;
    if (compilerName)
    {
        builder << compilerName << ": ";
    }

    if (SLANG_FAILED(res) && res != SLANG_FAIL)
    {
        {
            char tmp[17];
            sprintf_s(tmp, SLANG_COUNT_OF(tmp), "0x%08x", uint32_t(res));
            builder << "Result(" << tmp << ") ";
        }

        PlatformUtil::appendResult(res, builder);
    }

    if (diagnostic.getLength() > 0)
    {
        builder.append(diagnostic);
        if (!diagnostic.endsWith("\n"))
        {
            builder.append("\n");
        }
    }

    sink->diagnoseRaw(severity, builder.getUnownedSlice());
}

void reportExternalCompileError(
    const char* compilerName,
    SlangResult res,
    const UnownedStringSlice& diagnostic,
    DiagnosticSink* sink)
{
    // TODO(tfoley): need a better policy for how we translate diagnostics
    // back into the Slang world (although we should always try to generate
    // HLSL that doesn't produce any diagnostics...)
    reportExternalCompileError(
        compilerName,
        SLANG_FAILED(res) ? Severity::Error : Severity::Warning,
        res,
        diagnostic,
        sink);
}

static Severity _getDiagnosticSeverity(ArtifactDiagnostic::Severity severity)
{
    switch (severity)
    {
    case ArtifactDiagnostic::Severity::Warning:
        return Severity::Warning;
    case ArtifactDiagnostic::Severity::Info:
        return Severity::Note;
    default:
        return Severity::Error;
    }
}

SlangResult passthroughDownstreamDiagnostics(
    DiagnosticSink* sink,
    IDownstreamCompiler* compiler,
    IArtifact* artifact)
{
    auto diagnostics = findAssociatedRepresentation<IArtifactDiagnostics>(artifact);

    if (!diagnostics)
        return SLANG_OK;

    if (diagnostics->getCount())
    {
        StringBuilder compilerText;
        DownstreamCompilerUtil::appendAsText(compiler->getDesc(), compilerText);

        StringBuilder builder;

        auto const diagnosticCount = diagnostics->getCount();
        for (Index i = 0; i < diagnosticCount; ++i)
        {
            const auto& diagnostic = *diagnostics->getAt(i);

            builder.clear();

            const Severity severity = _getDiagnosticSeverity(diagnostic.severity);

            if (diagnostic.filePath.count == 0 && diagnostic.location.line == 0 &&
                severity == Severity::Note)
            {
                // If theres no filePath line number and it's info, output severity and text alone
                builder << getSeverityName(severity) << " : ";
            }
            else
            {
                if (diagnostic.filePath.count)
                {
                    builder << asStringSlice(diagnostic.filePath);
                }

                if (diagnostic.location.line)
                {
                    builder << "(" << diagnostic.location.line << ")";
                }

                builder << ": ";

                if (diagnostic.stage == ArtifactDiagnostic::Stage::Link)
                {
                    builder << "link ";
                }

                builder << getSeverityName(severity);
                builder << " " << asStringSlice(diagnostic.code) << ": ";
            }

            builder << asStringSlice(diagnostic.text);
            reportExternalCompileError(
                compilerText.getBuffer(),
                severity,
                SLANG_OK,
                builder.getUnownedSlice(),
                sink);
        }
    }

    // If any errors are emitted, then we are done
    if (diagnostics->hasOfAtLeastSeverity(ArtifactDiagnostic::Severity::Error))
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}


} // namespace Slang
