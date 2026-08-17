#include "slang-compiler-options.h"

#include "core/slang-type-text-util.h"
#include "core/slang-writer.h"
#include "slang-compiler.h"

#include <cstdio>

namespace Slang
{
void CompilerOptionSet::load(uint32_t count, const slang::CompilerOptionEntry* entries)
{
    for (uint32_t i = 0; i < count; i++)
    {
        CompilerOptionValue value;
        value.kind = entries[i].value.kind;
        value.intValue = entries[i].value.intValue0;
        value.intValue2 = entries[i].value.intValue1;
        if (value.kind == CompilerOptionValueKind::String)
        {
            value.stringValue = entries[i].value.stringValue0;
            value.stringValue2 = entries[i].value.stringValue1;
        }
        add(entries[i].name, value);

        // When we see option EmitSpirvDirectly or EmitSpirvViaGLSL, we will need to
        // translate them to EmitSpirvMethod.
        if (entries[i].name == slang::CompilerOptionName::EmitSpirvDirectly)
        {
            set(slang::CompilerOptionName::EmitSpirvMethod,
                value.intValue != 0 ? SLANG_EMIT_SPIRV_DIRECTLY : SLANG_EMIT_SPIRV_VIA_GLSL);
        }
        else if (entries[i].name == slang::CompilerOptionName::EmitSpirvViaGLSL)
        {
            SlangEmitSpirvMethod current =
                getEnumOption<SlangEmitSpirvMethod>(slang::CompilerOptionName::EmitSpirvMethod);
            if (current == SLANG_EMIT_SPIRV_DEFAULT && value.intValue)
            {
                set(CompilerOptionName::EmitSpirvMethod, SLANG_EMIT_SPIRV_VIA_GLSL);
            }
        }
    }
}

void CompilerOptionSet::writeCommandLineArgs(Session* globalSession, StringBuilder& sb)
{
    for (auto& option : options)
    {
        // Most emitted options resolve their flag name from the command-option catalog; a few keys
        // that are not registered as their own command option (e.g. DebugInformationFormat,
        // EmitSpirvMethod) build the flag inline in their case below, so a missing registration is
        // not itself a reason to skip.
        UnownedStringSlice name;
        if (auto optionInfoIndex = globalSession->m_commandOptions.findOptionByUserValue(
                CommandOptions::UserValue(option.key));
            optionInfoIndex != -1)
        {
            auto optionInfo = globalSession->m_commandOptions.getOptionAt(optionInfoIndex);
            auto nameCommaIndex = optionInfo.names.indexOf(',');
            if (nameCommaIndex == -1)
                nameCommaIndex = optionInfo.names.getLength();
            name = optionInfo.names.head(nameCommaIndex);
        }
        switch (option.key)
        {
        case CompilerOptionName::Capability:
            {
                StringBuilder subBuilder;
                for (auto v : option.value)
                {
                    if (subBuilder.getLength() != 0)
                        subBuilder << "+";
                    if (v.kind == CompilerOptionValueKind::Int)
                        subBuilder << capabilityNameToString((CapabilityName)v.intValue);
                    else
                        subBuilder << v.stringValue;
                }
                if (subBuilder.getLength())
                    sb << " " << name << " " << subBuilder.produceString();
                break;
            }
        case CompilerOptionName::Include:
            for (auto v : option.value)
            {
                sb << " -I \"" << v.stringValue << "\"";
            }
            break;
        case CompilerOptionName::MacroDefine:
            for (auto v : option.value)
            {
                sb << " -D" << v.stringValue;
                if (v.stringValue2.getLength())
                    sb << "=" << v.stringValue2;
            }
            break;
        case CompilerOptionName::VulkanBindShift: // intValue0 (higher 8 bits): kind;
                                                  // intValue0(higher bits): set; intValue1:
                                                  // shift
            for (auto v : option.value)
            {
                uint8_t kind;
                int set, shift;
                v.unpackInt3(kind, set, shift);
                switch ((HLSLToVulkanLayoutOptions::Kind)(kind))
                {
                case HLSLToVulkanLayoutOptions::Kind::UnorderedAccess:
                    sb << " -fvk-u-shift";
                    break;
                case HLSLToVulkanLayoutOptions::Kind::Sampler:
                    sb << " -fvk-s-shift";
                    break;
                case HLSLToVulkanLayoutOptions::Kind::ShaderResource:
                    sb << " -fvk-t-shift";
                    break;
                case HLSLToVulkanLayoutOptions::Kind::ConstantBuffer:
                    sb << " -fvk-b-shift";
                    break;
                default:
                    continue;
                }
                sb << " " << shift << " " << set;
            }
            break;
        case CompilerOptionName::VulkanBindShiftAll: // intValue0: kind; intValue1: shift
            // Produced by `-fvk-<kind>-shift <shift> all`, where the `all` keyword replaces the
            // per-space operand of the regular VulkanBindShift form; round-trip to that spelling
            // (there is no `-fvk-all-shift` flag).
            for (auto v : option.value)
            {
                switch ((HLSLToVulkanLayoutOptions::Kind)v.intValue)
                {
                case HLSLToVulkanLayoutOptions::Kind::UnorderedAccess:
                    sb << " -fvk-u-shift";
                    break;
                case HLSLToVulkanLayoutOptions::Kind::Sampler:
                    sb << " -fvk-s-shift";
                    break;
                case HLSLToVulkanLayoutOptions::Kind::ShaderResource:
                    sb << " -fvk-t-shift";
                    break;
                case HLSLToVulkanLayoutOptions::Kind::ConstantBuffer:
                    sb << " -fvk-b-shift";
                    break;
                default:
                    continue;
                }
                sb << " " << v.intValue2 << " all";
            }
            break;
        case CompilerOptionName::VulkanBindGlobals: // intValue0: index; intValue1: set
            for (auto v : option.value)
            {
                sb << " " << name << " " << v.intValue << " " << v.intValue2;
            }
            break;
        case CompilerOptionName::TraceCoverageBindless: // intValue0: space; intValue1: array index
        case CompilerOptionName::TraceCoverageBinding:  // intValue0: index; intValue1: space
            for (auto v : option.value)
            {
                sb << " " << name << " " << v.intValue << " " << v.intValue2;
            }
            break;
        case CompilerOptionName::TraceCoverageReservedSpace: // intValue0: space
            for (auto v : option.value)
            {
                sb << " " << name << " " << v.intValue;
            }
            break;
        case CompilerOptionName::Optimization:
            for (auto v : option.value)
            {
                sb << " -O" << v.intValue;
            }
            break;
        case CompilerOptionName::DownstreamArgs:
            for (auto v : option.value)
            {
                List<UnownedStringSlice> lines;
                StringUtil::split(v.stringValue2.getUnownedSlice(), '\n', lines);
                for (auto l : lines)
                {
                    sb << " -X" << v.stringValue << " " << l.trim();
                }
            }
            break;
        case CompilerOptionName::DebugInformation:
            // The `-g` flag's registered name is the placeholder "-g..." (it accepts a level and/or
            // a format suffix), so the flag is built inline rather than from the resolved `name`.
            for (auto v : option.value)
            {
                sb << " -g" << v.intValue;
            }
            break;
        case CompilerOptionName::DebugInformationFormat:
            // The debug format is the second half of the `-g` flag (`-gdwarf` etc.). It is stored
            // under its own key and emitted independently of the level so a format supplied without
            // a level still appears; a bare `-gdwarf` sets only this key (see
            // `_parseDebugInformation`).
            for (auto v : option.value)
            {
                if (v.intValue != SLANG_DEBUG_INFO_FORMAT_DEFAULT)
                    sb << " -g"
                       << TypeTextUtil::getDebugInfoFormatName((SlangDebugInfoFormat)v.intValue);
            }
            break;
        case CompilerOptionName::Language:
            for (auto v : option.value)
            {
                sb << " " << name << " "
                   << NameValueUtil::findName(
                          TypeTextUtil::getLanguageInfos(),
                          v.intValue,
                          toSlice("unknown"));
            }
            break;
        case CompilerOptionName::Profile:
            for (auto v : option.value)
            {
                Profile profile((Profile::RawVal)v.intValue);
                if (profile.raw != Profile::Unknown)
                    sb << " " << name << " " << profile.getName();
            }
            break;
        case CompilerOptionName::LineDirectiveMode:
            for (auto v : option.value)
            {
                sb << " " << name << " "
                   << NameValueUtil::findName(
                          TypeTextUtil::getLineDirectiveInfos(),
                          v.intValue,
                          toSlice("default"));
            }
            break;
        case CompilerOptionName::FloatingPointMode:
            for (auto v : option.value)
            {
                sb << " " << name << " "
                   << NameValueUtil::findName(
                          TypeTextUtil::getFloatingPointModeInfos(),
                          v.intValue,
                          toSlice("precise"));
            }
            break;
        case CompilerOptionName::DenormalModeFp16:
        case CompilerOptionName::DenormalModeFp32:
        case CompilerOptionName::DenormalModeFp64:
            for (auto v : option.value)
            {
                sb << " " << name << " "
                   << NameValueUtil::findName(
                          TypeTextUtil::getFpDenormalModeInfos(),
                          v.intValue,
                          toSlice("any"));
            }
            break;
        case CompilerOptionName::LanguageVersion:
            for (auto v : option.value)
            {
                sb << " " << name << " "
                   << NameValueUtil::findName(
                          TypeTextUtil::getLanguageVersionInfos(),
                          v.intValue,
                          toSlice("latest"));
            }
            break;
        case CompilerOptionName::PassThrough:
            for (auto v : option.value)
            {
                if (v.intValue != SLANG_PASS_THROUGH_NONE)
                    sb << " " << name << " "
                       << TypeTextUtil::getPassThroughName((SlangPassThrough)v.intValue);
            }
            break;
        case CompilerOptionName::TypeConformance:
        case CompilerOptionName::LLVMTargetTriple:
        case CompilerOptionName::LLVMCPU:
        case CompilerOptionName::LLVMFeatures:
            for (auto v : option.value)
            {
                sb << " " << name << " " << v.stringValue;
            }
            break;
        case CompilerOptionName::EmitSpirvMethod:
            // `-emit-spirv-directly` / `-emit-spirv-via-glsl` parse into this derived enum, so map
            // the stored value back to whichever source flag produced it (the default is implicit).
            for (auto v : option.value)
            {
                if (v.intValue == SLANG_EMIT_SPIRV_DIRECTLY)
                    sb << " -emit-spirv-directly";
                else if (v.intValue == SLANG_EMIT_SPIRV_VIA_GLSL)
                    sb << " -emit-spirv-via-glsl";
            }
            break;
        case CompilerOptionName::EmitCPUMethod:
            for (auto v : option.value)
            {
                if (v.intValue == SLANG_EMIT_CPU_VIA_LLVM)
                    sb << " -emit-cpu-via-llvm";
                else if (v.intValue == SLANG_EMIT_CPU_VIA_CPP)
                    sb << " -emit-cpu-via-cpp";
            }
            break;
        case CompilerOptionName::BindlessSpaceIndex:
        case CompilerOptionName::SPIRVResourceHeapStride:
        case CompilerOptionName::SPIRVSamplerHeapStride:
            for (auto v : option.value)
            {
                sb << " " << name << " " << v.intValue;
            }
            break;
        case CompilerOptionName::TraceCoverageCounterByteWidth:
            // Stored as a byte width (4 or 8); the CLI flag takes a bit count, so convert back.
            for (auto v : option.value)
            {
                sb << " " << name << " " << (v.intValue * 8);
            }
            break;
        case CompilerOptionName::GLSLForceScalarLayout:
        case CompilerOptionName::ForceDXLayout:
        case CompilerOptionName::ForceCLayout:
        case CompilerOptionName::MatrixLayoutRow:
        case CompilerOptionName::MatrixLayoutColumn:
        case CompilerOptionName::VulkanInvertY:
        case CompilerOptionName::VulkanUseDxPositionW:
        case CompilerOptionName::VulkanUseEntryPointName:
        case CompilerOptionName::VulkanUseGLLayout:
        case CompilerOptionName::VulkanEmitReflection:
        case CompilerOptionName::EnableEffectAnnotations:
        case CompilerOptionName::DefaultImageFormatUnknown:
        case CompilerOptionName::DisableDynamicDispatch:
        case CompilerOptionName::DisableSpecialization:
        case CompilerOptionName::DumpIntermediates:
        case CompilerOptionName::MinimumSlangOptimization:
        case CompilerOptionName::SkipSPIRVValidation:
        case CompilerOptionName::ZeroInitialize:
        case CompilerOptionName::IgnoreCapabilities:
        case CompilerOptionName::RestrictiveCapabilityCheck:
        case CompilerOptionName::DisableShortCircuit:
        case CompilerOptionName::DisableNonEssentialValidations:
        case CompilerOptionName::DisableSourceMap:
        case CompilerOptionName::UnscopedEnum:
        case CompilerOptionName::PreserveParameters:
        case CompilerOptionName::Obfuscate:
        case CompilerOptionName::IncompleteLibrary:
        case CompilerOptionName::EnableExperimentalDynamicDispatch:
        case CompilerOptionName::GenerateWholeProgram:
        case CompilerOptionName::UseMSVCStyleBitfieldPacking:
        case CompilerOptionName::ExperimentalFeature:
        case CompilerOptionName::EmitSeparateDebug:
        case CompilerOptionName::TraceCoverage:
        case CompilerOptionName::TraceFunctionCoverage:
        case CompilerOptionName::TraceBranchCoverage:
        case CompilerOptionName::TraceCoverageBoolean:
        case CompilerOptionName::SPIRVUnifiedDescriptorHeapStride:
        case CompilerOptionName::DebugInfoIncludeSource:
        case CompilerOptionName::EmbedDownstreamIR:
        case CompilerOptionName::NoMangle:
        case CompilerOptionName::NoHLSLBinding:
        case CompilerOptionName::NoHLSLPackConstantBufferElements:
        case CompilerOptionName::EnableExperimentalPasses:
        case CompilerOptionName::TrackLiveness:
        case CompilerOptionName::LoopInversion:
        case CompilerOptionName::AllowGLSL:
            if (option.value.getCount() && option.value[0].intValue != 0)
                sb << " " << name;
            break;
        default:
            // Other option kinds are currently omitted.
            break;
        }
    }
}

// Append a string to the digest with a length prefix so it is self-delimiting. Without the prefix,
// concatenated strings are ambiguous: MacroDefine("AB","C") and MacroDefine("A","BC") both feed the
// byte stream "ABC" and would collide.
static void appendDelimitedString(DigestBuilder<SHA1>& builder, const String& str)
{
    builder.append(str.getLength());
    builder.append(str);
}

void CompilerOptionSet::buildHash(DigestBuilder<SHA1>& builder)
{
    // Hash keys in a fixed (sorted-by-enum) order so the digest depends only on the option set, not
    // on the order options happened to be inserted; otherwise the same logical options assembled in
    // a different order would produce a spurious cache miss.
    List<CompilerOptionName> keys;
    for (auto& kv : options)
        keys.add(kv.key);
    keys.sort();

    for (auto key : keys)
    {
        // These are output-policy sidecar paths, not generated shader code. Locked by
        // _testCoverageManifestOutputDoesNotAffectCompilerOptionHash and
        // _testSeparateDebugInfoOutputDoesNotAffectCompilerOptionHash; re-including them would
        // invalidate persistent module caches on every sidecar-path change.
        if (key == CompilerOptionName::CoverageManifestOutput ||
            key == CompilerOptionName::SeparateDebugInfoOutput)
            continue;

        // This is a load-time acceptance-policy knob, not generated shader code: it only decides
        // whether loadModule runs isBinaryModuleUpToDate. There is no CLI spelling for it, so an
        // offline `slangc -o *.slang-module` bakes a digest with the flag absent; a loader that
        // enables it (its sole purpose) would otherwise fold it into the recompute and never match
        // that baked digest, making the freshness check unable to accept any default-compiled
        // module (issue #6557). Excluding it keeps the write/read digest symmetric.
        if (key == CompilerOptionName::UseUpToDateBinaryModule)
            continue;

        auto values = options.tryGetValue(key);
        builder.append(key);
        builder.append(values->getCount());
        for (auto& v : *values)
        {
            builder.append(v.kind);
            if (v.kind == CompilerOptionValueKind::Int)
            {
                builder.append(v.intValue);
                builder.append(v.intValue2);
            }
            else
            {
                appendDelimitedString(builder, v.stringValue);
                appendDelimitedString(builder, v.stringValue2);
            }
        }
    }
}

bool CompilerOptionSet::allowDuplicate(CompilerOptionName name)
{
    switch (name)
    {
    case CompilerOptionName::Include:
    case CompilerOptionName::MacroDefine:
    case CompilerOptionName::WarningsAsErrors:
    case CompilerOptionName::DisableWarning:
    case CompilerOptionName::DisableWarnings:
    case CompilerOptionName::EnableWarning:
    case CompilerOptionName::WarningLevel:
    case CompilerOptionName::Capability:
    case CompilerOptionName::DownstreamArgs:
    case CompilerOptionName::VulkanBindShift:
    case CompilerOptionName::VulkanBindShiftAll:
    case CompilerOptionName::TypeConformance:
    case CompilerOptionName::DumpIRBefore:
    case CompilerOptionName::DumpIRAfter:
    case CompilerOptionName::TraceCoverageReservedSpace:
        return true;
    }
    return false;
}
CompilerOptionValue Slang::CompilerOptionSet::getDefault(CompilerOptionName name)
{
    switch (name)
    {
    case CompilerOptionName::Optimization:
        return CompilerOptionValue::fromEnum(OptimizationLevel::Default);
    case CompilerOptionName::LanguageVersion:
        return CompilerOptionValue::fromEnum(SLANG_LANGUAGE_VERSION_DEFAULT);
    case CompilerOptionName::DebugInformation:
        return CompilerOptionValue::fromEnum(DebugInfoLevel::None);
    default:
        return CompilerOptionValue();
    }
}

SlangTargetFlags CompilerOptionSet::getTargetFlags()
{
    SlangTargetFlags result = 0;
    if (shouldDumpIR())
        result |= SLANG_TARGET_FLAG_DUMP_IR;
    if (getBoolOption(CompilerOptionName::GenerateWholeProgram))
        result |= SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM;
    if (getBoolOption(CompilerOptionName::ParameterBlocksUseRegisterSpaces))
        result |= SLANG_TARGET_FLAG_PARAMETER_BLOCKS_USE_REGISTER_SPACES;
    if (shouldEmitSPIRVDirectly())
        result |= SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;
    return result;
}

void CompilerOptionSet::setTargetFlags(SlangTargetFlags flags)
{
    set(CompilerOptionName::DumpIr, (flags & SLANG_TARGET_FLAG_DUMP_IR) != 0);
    set(CompilerOptionName::GenerateWholeProgram,
        (flags & SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM) != 0);

    if ((flags & SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY) != 0)
        set(CompilerOptionName::EmitSpirvMethod, SLANG_EMIT_SPIRV_DIRECTLY);
    else
    {
        // We allow to set this flag only when users are not setting the
        // the spirv emit method via CompilerOptionName.
        SlangEmitSpirvMethod current =
            getEnumOption<SlangEmitSpirvMethod>(CompilerOptionName::EmitSpirvMethod);
        if (current != SLANG_EMIT_SPIRV_DIRECTLY)
            set(CompilerOptionName::EmitSpirvMethod, SLANG_EMIT_SPIRV_VIA_GLSL);
    }

    set(CompilerOptionName::ParameterBlocksUseRegisterSpaces,
        (flags & SLANG_TARGET_FLAG_PARAMETER_BLOCKS_USE_REGISTER_SPACES) != 0);
}

void CompilerOptionSet::addTargetFlags(SlangTargetFlags flags)
{
    if ((flags & SLANG_TARGET_FLAG_DUMP_IR))
        set(CompilerOptionName::DumpIr, true);

    if ((flags & SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM) != 0)
        set(CompilerOptionName::GenerateWholeProgram, true);

    if ((flags & SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY) != 0)
        set(CompilerOptionName::EmitSpirvMethod, SLANG_EMIT_SPIRV_DIRECTLY);

    if ((flags & SLANG_TARGET_FLAG_PARAMETER_BLOCKS_USE_REGISTER_SPACES) != 0)
        set(CompilerOptionName::ParameterBlocksUseRegisterSpaces, true);
}
MatrixLayoutMode CompilerOptionSet::getMatrixLayoutMode()
{
    if (getBoolOption(CompilerOptionName::MatrixLayoutRow))
        return kMatrixLayoutMode_RowMajor;
    if (getBoolOption(CompilerOptionName::MatrixLayoutColumn))
        return kMatrixLayoutMode_ColumnMajor;

    return (MatrixLayoutMode)kMatrixLayoutMode_RowMajor;
}

void CompilerOptionSet::setMatrixLayoutMode(MatrixLayoutMode mode)
{
    options.remove(CompilerOptionName::MatrixLayoutColumn);
    options.remove(CompilerOptionName::MatrixLayoutRow);
    if (mode == kMatrixLayoutMode_ColumnMajor)
        set(CompilerOptionName::MatrixLayoutColumn, true);
    if (mode == kMatrixLayoutMode_RowMajor)
        set(CompilerOptionName::MatrixLayoutRow, true);
}

Profile CompilerOptionSet::getProfile()
{
    if (auto profileRaw = getEnumOption<Profile::RawEnum>(CompilerOptionName::Profile))
        return Profile(profileRaw);
    return Profile();
}

void CompilerOptionSet::setProfile(Profile profile)
{
    set(CompilerOptionName::Profile, (int)profile.raw);
}

ProfileVersion CompilerOptionSet::getProfileVersion()
{
    if (auto profileRaw = getEnumOption<Profile::RawEnum>(CompilerOptionName::Profile))
        return Profile(profileRaw).getVersion();
    return ProfileVersion::Unknown;
}

void CompilerOptionSet::setProfileVersion(ProfileVersion version)
{
    Profile profile;
    if (auto profileRaw = getEnumOption<Profile::RawEnum>(CompilerOptionName::Profile))
        profile = Profile(profileRaw);
    profile.setVersion(version);
    set(CompilerOptionName::Profile, (int)profile.raw);
}

void CompilerOptionSet::addCapabilityAtom(CapabilityName cap)
{
    add(CompilerOptionName::Capability, cap);
}

List<String> CompilerOptionSet::getDownstreamArgs(String downstreamToolName)
{
    List<String> result;
    auto downstreamArgsArray = getArray(CompilerOptionName::DownstreamArgs);
    for (auto& argSet : downstreamArgsArray)
    {
        if (argSet.stringValue == downstreamToolName)
        {
            CommandLineArgs args;
            args.deserialize(argSet.stringValue2);
            for (auto arg : args.m_args)
                result.add(arg.value);
            break;
        }
    }
    return result;
}

void CompilerOptionSet::serialize(SerializedOptionsData* outData)
{
    for (auto& option : options)
    {
        for (auto val : option.value)
        {
            slang::CompilerOptionEntry entry = {};
            entry.name = option.key;
            entry.value.kind = val.kind;
            entry.value.intValue0 = val.intValue;
            entry.value.intValue1 = val.intValue2;
            outData->stringPool.add(val.stringValue);
            entry.value.stringValue0 = val.stringValue.getBuffer();
            outData->stringPool.add(val.stringValue2);
            entry.value.stringValue1 = val.stringValue2.getBuffer();
            outData->entries.add(entry);
        }
    }
}

void applySettingsToDiagnosticSink(
    DiagnosticSink* targetSink,
    DiagnosticSink* outputSink,
    CompilerOptionSet& options)
{
    auto disableArray = options.getArray(CompilerOptionName::DisableWarning);
    for (auto& element : disableArray)
    {
        overrideDiagnostic(
            targetSink,
            outputSink,
            element.stringValue.getUnownedSlice(),
            Severity::Warning,
            Severity::Disable);
    }
    disableArray = options.getArray(CompilerOptionName::DisableWarnings);
    for (auto& element : disableArray)
    {
        overrideDiagnostics(
            targetSink,
            outputSink,
            element.stringValue.getUnownedSlice(),
            Severity::Warning,
            Severity::Disable);
    }
    auto enableArray = options.getArray(CompilerOptionName::EnableWarning);
    for (auto& element : enableArray)
    {
        overrideDiagnostics(
            targetSink,
            outputSink,
            element.stringValue.getUnownedSlice(),
            Severity::Warning,
            Severity::Warning);
    }
    auto warningsAsErrorsArray = options.getArray(CompilerOptionName::WarningsAsErrors);
    for (auto& element : warningsAsErrorsArray)
    {
        if (element.stringValue == "all")
            targetSink->setFlag(DiagnosticSink::Flag::TreatWarningsAsErrors);
        else
            overrideDiagnostics(
                targetSink,
                outputSink,
                element.stringValue.getUnownedSlice(),
                Severity::Warning,
                Severity::Error);
    }
    // Enable each requested warning group (-Wall/-Wextra/-Wpedantic). These are additive, so a
    // diagnostic tagged with any enabled group becomes visible. `intValue` is embedder-controlled
    // through the public WarningLevel option, so validate it here at the API boundary and ignore
    // anything outside the known groups (Default is the always-on baseline and needs no enabling).
    auto warningLevelArray = options.getArray(CompilerOptionName::WarningLevel);
    for (auto& element : warningLevelArray)
    {
        switch (element.intValue)
        {
        case SLANG_WARNING_LEVEL_ALL:
        case SLANG_WARNING_LEVEL_EXTRA:
        case SLANG_WARNING_LEVEL_PEDANTIC:
            targetSink->enableWarningLevel((WarningLevel)element.intValue);
            break;
        default:
            break;
        }
    }
    if (options.shouldEmitRichDiagnostics())
    {
        targetSink->setFlag(DiagnosticSink::Flag::AlwaysGenerateRichDiagnostics);
    }
    if (options.shouldEmitMachineReadableDiagnostics())
    {
        targetSink->setFlag(DiagnosticSink::Flag::MachineReadableDiagnostics);
    }

    // Handle diagnostic color setting.
    // A sink may have settings applied from several option sets in sequence (e.g. a linkage option
    // set followed by a component-type option set). Only apply the color mode when this set
    // actually carries the option, so a set that does not specify it does not overwrite a mode a
    // prior set already applied (which would reset it to the AUTO default).
    // The sink will handle AUTO by checking writer->isConsole().
    if (options.hasOption(CompilerOptionName::DiagnosticColor))
    {
        targetSink->setDiagnosticColorMode(
            (SlangDiagnosticColor)options.getIntOption(CompilerOptionName::DiagnosticColor));
    }
}
} // namespace Slang
