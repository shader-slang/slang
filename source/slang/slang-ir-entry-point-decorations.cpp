#include "slang-ir-entry-point-decorations.h"

#include "compiler-core/slang-diagnostic-sink.h"
#include "core/slang-signal.h"
#include "core/slang-string.h"
#include "core/slang-type-text-util.h"
#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-options.h"
#include "slang-profile.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

class CheckEntryPointDecorationsContext
{
public:
    CheckEntryPointDecorationsContext(IRModule* module, CodeGenTarget target, DiagnosticSink* sink)
        : m_module(module), m_target(target), m_sink(sink)
    {
    }

    void check()
    {
        for (auto inst : m_module->getGlobalInsts())
        {
            const auto func = as<IRFunc>(inst);
            if (!func)
                continue;
            const auto entryPointDecoration = func->findDecoration<IREntryPointDecoration>();
            if (!entryPointDecoration)
                continue;

            checkEntryPoint(func, entryPointDecoration->getProfile().getStage());
        }
    }

private:
    void checkEntryPoint(IRFunc* entryPoint, Stage stage)
    {
        for (auto decoration : entryPoint->getDecorations())
        {
            if (auto outputTopologyDecoration = as<IROutputTopologyDecoration>(decoration))
            {
                checkOutputTopologyDecoration(outputTopologyDecoration, stage);
            }
            else if (auto postDepthCoverage = as<IRPostDepthCoverageDecoration>(decoration))
            {
                checkPostDepthCoverageDecoration(postDepthCoverage);
            }
            else if (auto waveSize = as<IRWaveSizeDecoration>(decoration))
            {
                checkWaveSizeDecoration(entryPoint, waveSize);
            }
        }
    }

    // A CUDA warp is always 32 lanes wide.
    static constexpr Int kCudaWarpSize = 32;

    // Diagnose `[WaveSize(N)]` on the targets that cannot request a wave size N. The decoration is
    // checked here, right after linking, rather than in the target emitters, so that the
    // diagnostic uses the location recorded when the attribute was lowered instead of depending on
    // the source location of an entry point that later passes may have rewritten.
    //
    // Consider this example:
    //
    //     [WaveSize(64)]
    //     [numthreads(64, 1, 1)]
    //     void computeMain() { ... }
    //
    // HLSL emits it as `[WaveSize(64)]`, WGSL as `@subgroup_size(64)`, and SPIR-V leaves the size
    // to the pipeline creation. Metal has no way to request a SIMD-group width, and a CUDA warp
    // is always 32 lanes wide, so on those two targets the attribute is dropped. Dropping it
    // silently would let code that sizes its groupshared memory by the requested width compute
    // wrong results, so we warn instead. WGSL needs one more check: the workgroup's x dimension
    // must be a multiple of the subgroup size.
    //
    // Only a literal wave size is checked here. Like `[numthreads]`, `[WaveSize]` accepts a
    // link-time constant expression such as `[WaveSize(W * 2)]`, and its operand stays an
    // arithmetic instruction until `simplifyIR` folds it, which happens after this pass. The
    // passes that inspect `[numthreads]` this early (e.g. `emitCalcGroupExtents`) likewise skip
    // an operand that is not an `IRIntLit`, and the emitters read the folded value later.
    void checkWaveSizeDecoration(IRFunc* entryPoint, IRWaveSizeDecoration* decoration)
    {
        // Metal drops the attribute whatever its value is.
        if (isMetalTarget(m_target))
        {
            m_sink->diagnose(Diagnostics::WaveSizeUnsupportedOnTarget{
                .target = getTargetName(),
                .location = decoration->sourceLoc});
            return;
        }

        auto waveSizeLiteral = as<IRIntLit>(decoration->getOperand(0));
        if (!waveSizeLiteral)
            return;
        const Int waveSize = Int(waveSizeLiteral->getValue());

        if (isCUDATarget(m_target))
        {
            if (waveSize != kCudaWarpSize)
            {
                m_sink->diagnose(Diagnostics::WaveSizeMismatchOnTarget{
                    .waveSize = int(waveSize),
                    .target = getTargetName(),
                    .targetWaveSize = int(kCudaWarpSize),
                    .location = decoration->sourceLoc});
            }
        }
        else if (isWGPUTarget(m_target))
        {
            checkWaveSizeDividesWorkgroupX(entryPoint, waveSize, decoration);
        }
    }

    // WGSL makes it a pipeline-creation error if the x dimension of `@workgroup_size` is not a
    // multiple of `@subgroup_size`, so we report it at compile time. A workgroup size that is a
    // specialization constant is not an `IRIntLit` and is only known at pipeline creation, so we
    // cannot check it here.
    void checkWaveSizeDividesWorkgroupX(
        IRFunc* entryPoint,
        Int waveSize,
        IRWaveSizeDecoration* decoration)
    {
        // The emitter writes a missing `[numthreads]` as a workgroup size of 1 along each axis.
        Int workgroupSizeX = 1;
        if (auto numThreads = entryPoint->findDecoration<IRNumThreadsDecoration>())
        {
            auto xLiteral = as<IRIntLit>(numThreads->getOperand(0));
            if (!xLiteral)
                return;
            workgroupSizeX = Int(xLiteral->getValue());
        }

        if (workgroupSizeX % waveSize != 0)
        {
            m_sink->diagnose(Diagnostics::WaveSizeNotDivisorOfWorkgroupX{
                .workgroupSizeX = int(workgroupSizeX),
                .waveSize = int(waveSize),
                .target = getTargetName(),
                .location = decoration->sourceLoc});
        }
    }

    String getTargetName() const
    {
        return String(TypeTextUtil::getCompileTargetName(SlangCompileTarget(m_target)));
    }

    // `[postdepthcoverage]` only lowers to an execution mode on the Khronos targets (SPIR-V and
    // GLSL). On any other target the decoration is silently dropped, which would leave a shader
    // that reads `SV_Coverage` running with ordinary pre-depth coverage — a different result with
    // no compile-time signal. Warn so that mismatch is visible rather than silent.
    void checkPostDepthCoverageDecoration(IRPostDepthCoverageDecoration* decoration)
    {
        if (isKhronosTarget(m_target))
            return;
        m_sink->diagnose(Diagnostics::PostDepthCoverageTargetNotSupported{
            .target = String(TypeTextUtil::getCompileTargetName(SlangCompileTarget(m_target))),
            .location = decoration->sourceLoc});
    }

    void checkOutputTopologyDecoration(IROutputTopologyDecoration* decoration, Stage stage)
    {
        const auto outputTopologyType = OutputTopologyType(decoration->getTopologyType());
        if (stage == Stage::Domain || stage == Stage::Hull)
        {
            if (outputTopologyType != OutputTopologyType::Point &&
                outputTopologyType != OutputTopologyType::Line &&
                outputTopologyType != OutputTopologyType::TriangleCW &&
                outputTopologyType != OutputTopologyType::TriangleCCW)
            {
                diagnoseInvalidStageOutputTopology(
                    decoration,
                    stage,
                    "'point', 'line', 'triangle_cw', 'triangle_ccw'");
            }
            return;
        }

        if (stage == Stage::Mesh)
        {
            if (isTargetGLSL() || isTargetSPIRV() || isTargetMetal())
            {
                if (outputTopologyType != OutputTopologyType::Point &&
                    outputTopologyType != OutputTopologyType::Line &&
                    outputTopologyType != OutputTopologyType::Triangle)
                {
                    diagnoseInvalidMeshStageOutputTopology(
                        decoration,
                        "'point', 'line', 'triangle'");
                }
            }
            else if (isTargetHLSL())
            {
                if (outputTopologyType != OutputTopologyType::Line &&
                    outputTopologyType != OutputTopologyType::Triangle)
                {
                    diagnoseInvalidMeshStageOutputTopology(decoration, "'line', 'triangle'");
                }
            }
            else
            {
                SLANG_UNEXPECTED("Invalid compilation target for mesh stage");
            }
        }
    }

    void diagnoseInvalidStageOutputTopology(
        IROutputTopologyDecoration* decoration,
        Stage stage,
        String validTopologies)
    {
        auto stageName = getStageName(stage);
        m_sink->diagnose(Diagnostics::InvalidStageOutputTopology{
            .topology = String(decoration->getTopology()->getStringSlice()),
            .stage = String(stageName ? stageName : "unknown"),
            .validTopologies = validTopologies,
            .location = decoration->sourceLoc});
    }

    void diagnoseInvalidMeshStageOutputTopology(
        IROutputTopologyDecoration* decoration,
        String validTopologies)
    {
        m_sink->diagnose(Diagnostics::InvalidMeshStageOutputTopology{
            .topology = String(decoration->getTopology()->getStringSlice()),
            .target = String(TypeTextUtil::getCompileTargetName(SlangCompileTarget(m_target))),
            .validTopologies = validTopologies,
            .location = decoration->sourceLoc});
    }

    bool isTargetHLSL() const { return m_target == CodeGenTarget::HLSL; }

    bool isTargetGLSL() const { return m_target == CodeGenTarget::GLSL; }

    bool isTargetSPIRV() const
    {
        return m_target == CodeGenTarget::SPIRV || m_target == CodeGenTarget::SPIRVAssembly;
    }

    bool isTargetMetal() const
    {
        return m_target == CodeGenTarget::Metal || m_target == CodeGenTarget::MetalLib ||
               m_target == CodeGenTarget::MetalLibAssembly;
    }

    IRModule* m_module;
    const CodeGenTarget m_target;
    DiagnosticSink* m_sink;
};

void checkEntryPointDecorations(IRModule* module, CodeGenTarget target, DiagnosticSink* sink)
{
    CheckEntryPointDecorationsContext(module, target, sink).check();
}

OutputTopologyType convertOutputTopologyStringToEnum(String rawOutputTopology)
{
    auto name = rawOutputTopology.toLower();

    OutputTopologyType outputTopologyType = OutputTopologyType::Unknown;

#define CASE(ID, NAME)                               \
    if (name == String(#NAME).toLower())             \
    {                                                \
        outputTopologyType = OutputTopologyType::ID; \
    }                                                \
    else

    OUTPUT_TOPOLOGY_TYPES(CASE)
#undef CASE
    {
        outputTopologyType = OutputTopologyType::Unknown;
        // no match
    }
    return outputTopologyType;
}

} // namespace Slang
