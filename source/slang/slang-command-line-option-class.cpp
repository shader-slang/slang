// slang-command-line-option-class.cpp
//
// Kept in its own translation unit, depending only on the `CompilerOptionName` enum, so the same
// source compiles into both the slang library and the unit-test module (the classification is not
// part of the exported ABI). See tools/CMakeLists.txt.

#include "slang-compiler-options.h"

namespace Slang
{
// Single source of truth for how `writeCommandLineArgs` treats each option. The `default` arm
// returns `Unclassified` so a newly-added enumerator is caught by the exhaustiveness unit test
// rather than silently omitted from the reproduction command line.
CommandLineOptionClass classifyCommandLineOption(CompilerOptionName name)
{
    switch (name)
    {
    // Descriptive options worth recording for reproduction fidelity: they either influence the
    // generated artifact (codegen, layout, capabilities, language rules, debug info, optimization,
    // floating-point behavior) or document how it was produced (e.g. `SkipSPIRVValidation`,
    // `DisableNonEssentialValidations`, `DumpIntermediates` do not change the emitted bytes but
    // belong on a faithful reproduction command line), and all have a stable CLI spelling.
    case CompilerOptionName::MacroDefine:
    case CompilerOptionName::Include:
    case CompilerOptionName::Language:
    case CompilerOptionName::MatrixLayoutColumn:
    case CompilerOptionName::MatrixLayoutRow:
    case CompilerOptionName::ZeroInitialize:
    case CompilerOptionName::IgnoreCapabilities:
    case CompilerOptionName::RestrictiveCapabilityCheck:
    case CompilerOptionName::Profile:
    case CompilerOptionName::SkipSPIRVValidation:
    case CompilerOptionName::DisableShortCircuit:
    case CompilerOptionName::MinimumSlangOptimization:
    case CompilerOptionName::DisableNonEssentialValidations:
    case CompilerOptionName::DisableSourceMap:
    case CompilerOptionName::UnscopedEnum:
    case CompilerOptionName::PreserveParameters:
    case CompilerOptionName::Capability:
    case CompilerOptionName::DefaultImageFormatUnknown:
    case CompilerOptionName::DisableDynamicDispatch:
    case CompilerOptionName::DisableSpecialization:
    case CompilerOptionName::FloatingPointMode:
    case CompilerOptionName::DebugInformation:
    case CompilerOptionName::DebugInformationFormat:
    case CompilerOptionName::LineDirectiveMode:
    case CompilerOptionName::Optimization:
    case CompilerOptionName::Obfuscate:
    case CompilerOptionName::VulkanBindShift:
    case CompilerOptionName::VulkanBindShiftAll:
    case CompilerOptionName::VulkanBindGlobals:
    case CompilerOptionName::VulkanInvertY:
    case CompilerOptionName::VulkanUseDxPositionW:
    case CompilerOptionName::VulkanUseEntryPointName:
    case CompilerOptionName::VulkanUseGLLayout:
    case CompilerOptionName::VulkanEmitReflection:
    case CompilerOptionName::GLSLForceScalarLayout:
    case CompilerOptionName::EnableEffectAnnotations:
    case CompilerOptionName::IncompleteLibrary:
    case CompilerOptionName::DownstreamArgs:
    case CompilerOptionName::BindlessSpaceIndex:
    case CompilerOptionName::SPIRVResourceHeapStride:
    case CompilerOptionName::SPIRVSamplerHeapStride:
    case CompilerOptionName::LanguageVersion:
    case CompilerOptionName::TypeConformance:
    case CompilerOptionName::EnableExperimentalDynamicDispatch:
    case CompilerOptionName::GenerateWholeProgram:
    case CompilerOptionName::ForceDXLayout:
    case CompilerOptionName::DenormalModeFp16:
    case CompilerOptionName::DenormalModeFp32:
    case CompilerOptionName::DenormalModeFp64:
    case CompilerOptionName::UseMSVCStyleBitfieldPacking:
    case CompilerOptionName::ForceCLayout:
    case CompilerOptionName::ExperimentalFeature:
    case CompilerOptionName::EmitSeparateDebug:
    case CompilerOptionName::TraceCoverage:
    case CompilerOptionName::TraceCoverageBinding:
    case CompilerOptionName::TraceCoverageReservedSpace:
    case CompilerOptionName::TraceFunctionCoverage:
    case CompilerOptionName::TraceBranchCoverage:
    case CompilerOptionName::TraceCoverageCounterByteWidth:
    case CompilerOptionName::TraceCoverageBoolean:
    // `TraceCoverageBindlessIndex` is a compile-time constant baked into the artifact (it selects
    // the descriptor-array index the synthesized coverage buffer is indexed at), so it describes
    // the compile and is serialized like the other coverage-binding options.
    case CompilerOptionName::TraceCoverageBindlessIndex:
    case CompilerOptionName::SPIRVUnifiedDescriptorHeapStride:
    case CompilerOptionName::DebugInfoIncludeSource:
    case CompilerOptionName::DumpIntermediates:
    case CompilerOptionName::EmitSpirvMethod:
    case CompilerOptionName::EmitCPUMethod:
    case CompilerOptionName::EmbedDownstreamIR:
    case CompilerOptionName::NoMangle:
    case CompilerOptionName::NoHLSLBinding:
    case CompilerOptionName::NoHLSLPackConstantBufferElements:
    case CompilerOptionName::EnableExperimentalPasses:
    case CompilerOptionName::TrackLiveness:
    case CompilerOptionName::LoopInversion:
    case CompilerOptionName::LLVMTargetTriple:
    case CompilerOptionName::LLVMCPU:
    case CompilerOptionName::LLVMFeatures:
    case CompilerOptionName::AllowGLSL:
    case CompilerOptionName::PassThrough:
        return CommandLineOptionClass::Serialize;

    // The source flags for the emit-method options. The command-line parser folds each into the
    // corresponding serialized method key (`-emit-spirv-directly`/`-emit-spirv-via-glsl` ->
    // `EmitSpirvMethod`, `-emit-cpu-via-cpp`/`-emit-cpu-via-llvm` -> `EmitCPUMethod`), so emitting
    // the source flag as well would double-count the choice the method key already carries.
    case CompilerOptionName::EmitSpirvViaGLSL:
    case CompilerOptionName::EmitSpirvDirectly:
    case CompilerOptionName::EmitCPUViaCPP:
    case CompilerOptionName::EmitCPUViaLLVM:
        return CommandLineOptionClass::RepresentedElsewhere;

    // Options intentionally excluded from the reproduction command line. Most describe context or
    // side channels rather than the artifact's contents: the entry-point context contributed by the
    // caller (target/stage/entry are appended by the SPIR-V emit site itself), input/output paths
    // and module identity, output-policy sidecar paths, repro tooling, dump/introspection,
    // diagnostics routing and reporting, downstream toolchain selection/paths, and
    // deprecated/removed/sentinel values. A few (e.g. the API-only `SkipDownstreamLinking`) can
    // affect emission but have no command-line spelling to reconstruct, so they are omitted here
    // rather than misrepresented.
    case CompilerOptionName::DepFile:
    case CompilerOptionName::EntryPointName:
    case CompilerOptionName::Specialize:
    case CompilerOptionName::Help:
    case CompilerOptionName::HelpStyle:
    case CompilerOptionName::ModuleName:
    case CompilerOptionName::Output:
    case CompilerOptionName::Stage:
    case CompilerOptionName::Target:
    case CompilerOptionName::Version:
    case CompilerOptionName::WarningsAsErrors:
    case CompilerOptionName::DisableWarnings:
    case CompilerOptionName::EnableWarning:
    case CompilerOptionName::DisableWarning:
    case CompilerOptionName::WarningLevel:
    case CompilerOptionName::DumpWarningDiagnostics:
    case CompilerOptionName::InputFilesRemain:
    case CompilerOptionName::EmitIr:
    case CompilerOptionName::ReportDownstreamTime:
    case CompilerOptionName::ReportPerfBenchmark:
    case CompilerOptionName::ReportCheckpointIntermediates:
    case CompilerOptionName::SourceEmbedStyle:
    case CompilerOptionName::SourceEmbedName:
    case CompilerOptionName::SourceEmbedLanguage:
    case CompilerOptionName::SPIRVCoreGrammarJSON:
    case CompilerOptionName::CompilerPath:
    case CompilerOptionName::DefaultDownstreamCompiler:
    case CompilerOptionName::DumpRepro:
    case CompilerOptionName::DumpReproOnError:
    case CompilerOptionName::ExtractRepro:
    case CompilerOptionName::LoadRepro:
    case CompilerOptionName::LoadReproDirectory:
    case CompilerOptionName::ReproFallbackDirectory:
    case CompilerOptionName::DumpAst:
    case CompilerOptionName::DumpIntermediatePrefix:
    case CompilerOptionName::DumpIr:
    case CompilerOptionName::DumpIrIds:
    case CompilerOptionName::PreprocessorOutput:
    case CompilerOptionName::OutputIncludes:
    case CompilerOptionName::ReproFileSystem:
    case CompilerOptionName::REMOVED_SerialIR:
    case CompilerOptionName::SkipCodeGen:
    case CompilerOptionName::ValidateIr:
    case CompilerOptionName::VerbosePaths:
    case CompilerOptionName::VerifyDebugSerialIr:
    case CompilerOptionName::NoCodeGen:
    case CompilerOptionName::FileSystem:
    case CompilerOptionName::Heterogeneous:
    case CompilerOptionName::ValidateUniformity:
    case CompilerOptionName::ArchiveType:
    case CompilerOptionName::CompileCoreModule:
    case CompilerOptionName::Doc:
    case CompilerOptionName::IrCompression:
    case CompilerOptionName::LoadCoreModule:
    case CompilerOptionName::ReferenceModule:
    case CompilerOptionName::SaveCoreModule:
    case CompilerOptionName::SaveCoreModuleBinSource:
    case CompilerOptionName::ParameterBlocksUseRegisterSpaces:
    case CompilerOptionName::EmitReflectionJSON:
    case CompilerOptionName::CountOfParsableOptions:
    case CompilerOptionName::UseUpToDateBinaryModule:
    case CompilerOptionName::SaveGLSLModuleBinSource:
    case CompilerOptionName::SkipDownstreamLinking:
    case CompilerOptionName::DumpModule:
    case CompilerOptionName::GetModuleInfo:
    case CompilerOptionName::GetSupportedModuleVersions:
    case CompilerOptionName::ReportDetailedPerfBenchmark:
    case CompilerOptionName::ValidateIRDetailed:
    case CompilerOptionName::DumpIRBefore:
    case CompilerOptionName::DumpIRAfter:
    case CompilerOptionName::EnableRichDiagnostics:
    case CompilerOptionName::ReportDynamicDispatchSites:
    case CompilerOptionName::EnableMachineReadableDiagnostics:
    case CompilerOptionName::DiagnosticColor:
    case CompilerOptionName::CompilerVersion:
    case CompilerOptionName::CoverageManifestOutput:
    case CompilerOptionName::SeparateDebugInfoOutput:
    // `CountOf` is the terminal sentinel, not a real option; `writeCommandLineArgs` never passes it
    // and the exhaustiveness test iterates only `[0, CountOf)`. It is listed here purely so the
    // switch is visibly total over the enum (and thus classifies as a benign `Omit` rather than
    // reaching the `Unclassified` default) should a caller ever pass it.
    case CompilerOptionName::CountOf:
        return CommandLineOptionClass::Omit;

    default:
        return CommandLineOptionClass::Unclassified;
    }
}

} // namespace Slang
