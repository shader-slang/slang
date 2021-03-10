// slang-downstream-compiler.cpp
#include "slang-downstream-compiler.h"

#include "slang-common.h"
#include "../../slang-com-helper.h"
#include "slang-string-util.h"

#include "slang-type-text-util.h"

#include "slang-io.h"
#include "slang-shared-library.h"
#include "slang-blob.h"

#ifdef SLANG_VC
#   include "windows/slang-win-visual-studio-util.h"
#endif

#include "slang-visual-studio-compiler-util.h"
#include "slang-gcc-compiler-util.h"
#include "slang-nvrtc-compiler.h"

namespace Slang
{

static DownstreamCompiler::Infos _calcInfos()
{
    typedef DownstreamCompiler::Info Info;
    typedef DownstreamCompiler::SourceLanguageFlag SourceLanguageFlag;
    typedef DownstreamCompiler::SourceLanguageFlags SourceLanguageFlags;

    DownstreamCompiler::Infos infos;

    infos.infos[int(SLANG_PASS_THROUGH_CLANG)] = Info(SourceLanguageFlag::CPP | SourceLanguageFlag::C);
    infos.infos[int(SLANG_PASS_THROUGH_VISUAL_STUDIO)] = Info(SourceLanguageFlag::CPP | SourceLanguageFlag::C);
    infos.infos[int(SLANG_PASS_THROUGH_GCC)] = Info(SourceLanguageFlag::CPP | SourceLanguageFlag::C);

    infos.infos[int(SLANG_PASS_THROUGH_NVRTC)] = Info(SourceLanguageFlag::CUDA);

    infos.infos[int(SLANG_PASS_THROUGH_DXC)] = Info(SourceLanguageFlag::HLSL);
    infos.infos[int(SLANG_PASS_THROUGH_FXC)] = Info(SourceLanguageFlag::HLSL);
    infos.infos[int(SLANG_PASS_THROUGH_GLSLANG)] = Info(SourceLanguageFlag::GLSL);

    return infos;
}

/* static */DownstreamCompiler::Infos DownstreamCompiler::s_infos = _calcInfos();

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompiler::Desc !!!!!!!!!!!!!!!!!!!!!!*/

void DownstreamCompiler::Desc::appendAsText(StringBuilder& out) const
{
    out << TypeTextUtil::getPassThroughAsHumanText(type);

    // Append the version if there is a version
    if (majorVersion || minorVersion)
    {
        out << " ";
        out << majorVersion;
        out << ".";
        out << minorVersion;
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamDiagnostic !!!!!!!!!!!!!!!!!!!!!!!!*/

/* static */UnownedStringSlice DownstreamDiagnostic::getSeverityText(Severity severity)
{
    switch (severity)
    {
        default:                return UnownedStringSlice::fromLiteral("Unknown");
        case Severity::Info:    return UnownedStringSlice::fromLiteral("Info");
        case Severity::Warning: return UnownedStringSlice::fromLiteral("Warning");
        case Severity::Error:   return UnownedStringSlice::fromLiteral("Error");
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompiler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/* static */bool DownstreamCompiler::canCompile(SlangPassThrough compiler, SlangSourceLanguage sourceLanguage)
{
    const auto& info = getInfo(compiler);
    return (info.sourceLanguageFlags & (SourceLanguageFlags(1) << int(sourceLanguage))) != 0;
}

/* static */SlangCompileTarget DownstreamCompiler::getCompileTarget(SlangSourceLanguage sourceLanguage)
{
    switch (sourceLanguage)
    {
        case SLANG_SOURCE_LANGUAGE_HLSL:    return SLANG_HLSL;
        case SLANG_SOURCE_LANGUAGE_GLSL:    return SLANG_GLSL;
        case SLANG_SOURCE_LANGUAGE_C:       return SLANG_C_SOURCE;
        case SLANG_SOURCE_LANGUAGE_CPP:     return SLANG_CPP_SOURCE;
        case SLANG_SOURCE_LANGUAGE_CUDA:    return SLANG_CUDA_SOURCE;
        default:                            return SLANG_TARGET_UNKNOWN;
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamDiagnostics !!!!!!!!!!!!!!!!!!!!!!*/

Index DownstreamDiagnostics::getCountBySeverity(Diagnostic::Severity severity) const
{
    Index count = 0;
    for (const auto& msg : diagnostics)
    {
        count += Index(msg.severity == severity);
    }
    return count;
}

Int DownstreamDiagnostics::countByStage(Diagnostic::Stage stage, Index counts[Int(Diagnostic::Severity::CountOf)]) const
{
    Int count = 0;
    ::memset(counts, 0, sizeof(Index) * Int(Diagnostic::Severity::CountOf));
    for (const auto& diagnostic : diagnostics)
    {
        if (diagnostic.stage == stage)
        {
            count++;
            counts[Index(diagnostic.severity)]++;
        }
    }
    return count++;
}

static void _appendCounts(const Index counts[Int(DownstreamDiagnostic::Severity::CountOf)], StringBuilder& out)
{
    typedef DownstreamDiagnostic::Severity Severity;

    for (Index i = 0; i < Int(Severity::CountOf); i++)
    {
        if (counts[i] > 0)
        {
            out << DownstreamDiagnostic::getSeverityText(Severity(i)) << "(" << counts[i] << ") ";
        }
    }
}

static void _appendSimplified(const Index counts[Int(DownstreamDiagnostic::Severity::CountOf)], StringBuilder& out)
{
    typedef DownstreamDiagnostic::Severity Severity;
    for (Index i = 0; i < Int(Severity::CountOf); i++)
    {
        if (counts[i] > 0)
        {
            out << DownstreamDiagnostic::getSeverityText(Severity(i)) << " ";
        }
    }
}

void DownstreamDiagnostics::appendSummary(StringBuilder& out) const
{
    Index counts[Int(Diagnostic::Severity::CountOf)];
    if (countByStage(Diagnostic::Stage::Compile, counts) > 0)
    {
        out << "Compile: ";
        _appendCounts(counts, out);
        out << "\n";
    }
    if (countByStage(Diagnostic::Stage::Link, counts) > 0)
    {
        out << "Link: ";
        _appendCounts(counts, out);
        out << "\n";
    }
}

void DownstreamDiagnostics::appendSimplifiedSummary(StringBuilder& out) const
{
    Index counts[Int(Diagnostic::Severity::CountOf)];
    if (countByStage(Diagnostic::Stage::Compile, counts) > 0)
    {
        out << "Compile: ";
        _appendSimplified(counts, out);
        out << "\n";
    }
    if (countByStage(Diagnostic::Stage::Link, counts) > 0)
    {
        out << "Link: ";
        _appendSimplified(counts, out);
        out << "\n";
    }
}

void DownstreamDiagnostics::removeBySeverity(Diagnostic::Severity severity)
{
    Index count = diagnostics.getCount();
    for (Index i = 0; i < count; ++i)
    {
        if (diagnostics[i].severity == severity)
        {
            diagnostics.removeAt(i);
            i--;
            count--;
        }
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CommandLineDownstreamCompileResult !!!!!!!!!!!!!!!!!!!!!!*/

SlangResult CommandLineDownstreamCompileResult::getHostCallableSharedLibrary(ComPtr<ISlangSharedLibrary>& outLibrary)
{
    if (m_hostCallableSharedLibrary)
    {
        outLibrary = m_hostCallableSharedLibrary;
        return SLANG_OK;
    }

    // Okay we want to load
    // Try loading the shared library
    SharedLibrary::Handle handle;
    if (SLANG_FAILED(SharedLibrary::loadWithPlatformPath(m_moduleFilePath.getBuffer(), handle)))
    {
        return SLANG_FAIL;
    }
    // The shared library needs to keep temp files in scope
    RefPtr<TemporarySharedLibrary> sharedLib(new TemporarySharedLibrary(handle, m_moduleFilePath));
    sharedLib->m_temporaryFileSet = m_temporaryFiles;

    m_hostCallableSharedLibrary = sharedLib;
    outLibrary = m_hostCallableSharedLibrary;
    return SLANG_OK;
}

SlangResult CommandLineDownstreamCompileResult::getBinary(ComPtr<ISlangBlob>& outBlob)
{
    if (m_binaryBlob)
    {
        outBlob = m_binaryBlob;
        return SLANG_OK;
    }

    // Read the binary
    try
    {
        // Read the contents of the binary
        List<uint8_t> contents = File::readAllBytes(m_moduleFilePath);

        m_binaryBlob = new ScopeRefObjectBlob(ListBlob::moveCreate(contents), m_temporaryFiles);
        outBlob = m_binaryBlob;
        return SLANG_OK;
    }
    catch (const Slang::IOException&)
    {
        return SLANG_FAIL;
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CommandLineDownstreamCompiler !!!!!!!!!!!!!!!!!!!!!!*/

static bool _isContentsInFile(const DownstreamCompiler::CompileOptions& options)
{
    if (options.sourceContentsPath.getLength() <= 0)
    {
        return false;
    }

    // We can see if we can load it
    if (File::exists(options.sourceContentsPath))
    {
        // Here we look for the file on the regular file system (as opposed to using the 
        // ISlangFileSystem. This is unfortunate but necessary - because when we call out
        // to the compiler all it is able to (currently) see are files on the file system.
        //
        // Note that it could be coincidence that the filesystem has a file that's identical in
        // contents/name. That being the case though, any includes wouldn't work for a generated
        // file either from some specialized ISlangFileSystem, so this is probably as good as it gets
        // until we can integrate directly to a C/C++ compiler through say a shared library where we can control
        // file system access.
        try
        {
            String readContents = File::readAllText(options.sourceContentsPath);
            // We should see if they are the same
            return options.sourceContents == readContents.getUnownedSlice();
        }
        catch (const Slang::IOException&)
        {
        }
    }
    return false;
}

SlangResult CommandLineDownstreamCompiler::compile(const CompileOptions& inOptions, RefPtr<DownstreamCompileResult>& out)
{
    // Copy the command line options
    CommandLine cmdLine(m_cmdLine);

    CompileOptions options(inOptions);

    // Find all the files that will be produced
    RefPtr<TemporaryFileSet> productFileSet(new TemporaryFileSet);
    
    if (options.modulePath.getLength() == 0 || options.sourceContents.getLength() != 0)
    {
        String modulePath = options.modulePath;

        // If there is no module path, generate one.
        if (modulePath.getLength() == 0)
        {
            SLANG_RETURN_ON_FAIL(File::generateTemporary(UnownedStringSlice::fromLiteral("slang-generated"), modulePath));
            options.modulePath = modulePath;
        }

        if (_isContentsInFile(options))
        {
            options.sourceFiles.add(options.sourceContentsPath);
        }
        else
        {
            String compileSourcePath = modulePath;

            // NOTE: Strictly speaking producing filenames by modifying the generateTemporary path that may introduce a temp filename clash, but in practice is extraordinary unlikely
            compileSourcePath.append("-src");

            // Make the temporary filename have the appropriate extension.
            if (options.sourceLanguage == SLANG_SOURCE_LANGUAGE_C)
            {
                compileSourcePath.append(".c");
            }
            else
            {
                compileSourcePath.append(".cpp");
            }

            // Write it out
            try
            {
                productFileSet->add(compileSourcePath);

                File::writeAllText(compileSourcePath, options.sourceContents);
            }
            catch (...)
            {
                return SLANG_FAIL;
            }

            // Add it as a source file
            options.sourceFiles.add(compileSourcePath);
        }

        // There is no source contents
        options.sourceContents = String();
        options.sourceContentsPath = String();
    }

    // Append command line args to the end of cmdLine using the target specific function for the specified options
    SLANG_RETURN_ON_FAIL(calcArgs(options, cmdLine));

    String moduleFilePath;

    {
        StringBuilder builder;
        SLANG_RETURN_ON_FAIL(calcModuleFilePath(options, builder));
        moduleFilePath = builder.ProduceString();
    }

    {
        List<String> paths;
        SLANG_RETURN_ON_FAIL(calcCompileProducts(options, DownstreamCompiler::ProductFlag::All, paths));
        productFileSet->add(paths);
    }

    ExecuteResult exeRes;

#if 0
    // Test
    {
        String line = ProcessUtil::getCommandLineString(cmdLine);
        printf("%s", line.getBuffer());
    }
#endif

    SLANG_RETURN_ON_FAIL(ProcessUtil::execute(cmdLine, exeRes));

#if 0
    {
        printf("stdout=\"%s\"\nstderr=\"%s\"\nret=%d\n", exeRes.standardOutput.getBuffer(), exeRes.standardError.getBuffer(), int(exeRes.resultCode));
    }
#endif

    DownstreamDiagnostics diagnostics;
    SLANG_RETURN_ON_FAIL(parseOutput(exeRes, diagnostics));

    
    out = new CommandLineDownstreamCompileResult(diagnostics, moduleFilePath, productFileSet);
    
    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompiler::Desc !!!!!!!!!!!!!!!!!!!!!!*/

static DownstreamCompiler::Desc _calcCompiledWithDesc()
{
    DownstreamCompiler::Desc desc;

#if SLANG_VC
    desc = WinVisualStudioUtil::getDesc(WinVisualStudioUtil::getCompiledVersion());
#elif SLANG_CLANG
    desc.type = SLANG_PASS_THROUGH_CLANG;
    desc.majorVersion = Int(__clang_major__);
    desc.minorVersion = Int(__clang_minor__);
#elif SLANG_GCC
    desc.type = SLANG_PASS_THROUGH_GCC;
    desc.majorVersion = Int(__GNUC__);
    desc.minorVersion = Int(__GNUC_MINOR__);
#else
    // TODO(JS): Hmmm None is not quite the same as unknown. It works for now, but we might want to have a distinct enum for unknown.
    desc.type = SLANG_PASS_THROUGH_NONE;        
#endif

    return desc;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompilerUtil !!!!!!!!!!!!!!!!!!!!!!*/

const DownstreamCompiler::Desc& DownstreamCompilerUtil::getCompiledWithDesc()
{
    static DownstreamCompiler::Desc s_desc = _calcCompiledWithDesc();
    return s_desc;
}

/* static */DownstreamCompiler* DownstreamCompilerUtil::findCompiler(const DownstreamCompilerSet* set, MatchType matchType, const DownstreamCompiler::Desc& desc)
{
    List<DownstreamCompiler*> compilers;
    set->getCompilers(compilers);
    return findCompiler(compilers, matchType, desc);
}

/* static */DownstreamCompiler* DownstreamCompilerUtil::findCompiler(const List<DownstreamCompiler*>& compilers, MatchType matchType, const DownstreamCompiler::Desc& desc)
{
    if (compilers.getCount() <= 0)
    {
        return nullptr;
    }

    Int bestIndex = -1;

    const SlangPassThrough compilerType = desc.type;

    Int maxVersionValue = 0;
    Int minVersionDiff = 0x7fffffff;

    Int descVersionValue = desc.getVersionValue();

    // If we don't have version set, then anything 0 or above is good enough, and just take newest
    if (descVersionValue == 0)
    {
        maxVersionValue = -1;
        matchType = MatchType::Newest;
    }

    for (Index i = 0; i < compilers.getCount(); ++i)
    {
        DownstreamCompiler* compiler = compilers[i];
        auto compilerDesc = compiler->getDesc();

        if (compilerType == compilerDesc.type)
        {
            const Int versionValue = compilerDesc.getVersionValue();
            switch (matchType)
            {
                case MatchType::MinGreaterEqual:
                {
                    auto diff = descVersionValue - versionValue;
                    if (diff >= 0 && diff < minVersionDiff)
                    {
                        bestIndex = i;
                        minVersionDiff = diff;
                    }
                    break;
                }
                case MatchType::MinAbsolute:
                {
                    auto diff = descVersionValue - versionValue;
                    diff = (diff >= 0) ? diff : -diff;
                    if (diff < minVersionDiff)
                    {
                        bestIndex = i;
                        minVersionDiff = diff;
                    }
                    break;
                }
                case MatchType::Newest:
                {
                    if (versionValue > maxVersionValue)
                    {
                        maxVersionValue = versionValue;
                        bestIndex = i;
                    }
                    break;
                }
            }
        }
    }

    return (bestIndex >= 0) ? compilers[bestIndex] : nullptr;
}

/* static */DownstreamCompiler* DownstreamCompilerUtil::findClosestCompiler(const List<DownstreamCompiler*>& compilers, const DownstreamCompiler::Desc& desc)
{
    DownstreamCompiler* compiler;

    compiler = findCompiler(compilers, MatchType::MinGreaterEqual, desc);
    if (compiler)
    {
        return compiler;
    }
    compiler = findCompiler(compilers, MatchType::MinAbsolute, desc);
    if (compiler)
    {
        return compiler;
    }

    // If we are gcc, we can try clang and vice versa
    if (desc.type == SLANG_PASS_THROUGH_GCC || desc.type == SLANG_PASS_THROUGH_CLANG)
    {
        DownstreamCompiler::Desc compatible = desc;
        compatible.type = (compatible.type == SLANG_PASS_THROUGH_CLANG) ? SLANG_PASS_THROUGH_GCC : SLANG_PASS_THROUGH_CLANG;

        compiler = findCompiler(compilers, MatchType::MinGreaterEqual, compatible);
        if (compiler)
        {
            return compiler;
        }
        compiler = findCompiler(compilers, MatchType::MinAbsolute, compatible);
        if (compiler)
        {
            return compiler;
        }
    }

    return nullptr;
}

/* static */DownstreamCompiler* DownstreamCompilerUtil::findClosestCompiler(const DownstreamCompilerSet* set, const DownstreamCompiler::Desc& desc)
{
    DownstreamCompiler* compiler = set->getCompiler(desc);
    if (compiler)
    {
        return compiler;
    }
    List<DownstreamCompiler*> compilers;
    set->getCompilers(compilers);
    return findClosestCompiler(compilers, desc);
}

/* static */void DownstreamCompilerUtil::updateDefault(DownstreamCompilerSet* set, SlangSourceLanguage sourceLanguage)
{
    DownstreamCompiler* compiler = nullptr;

    switch (sourceLanguage)
    {
        case SLANG_SOURCE_LANGUAGE_CPP:
        case SLANG_SOURCE_LANGUAGE_C:
        {
            compiler = findClosestCompiler(set, getCompiledWithDesc());
            break;
        }
        case SLANG_SOURCE_LANGUAGE_CUDA:
        {
            DownstreamCompiler::Desc desc;
            desc.type = SLANG_PASS_THROUGH_NVRTC;
            compiler = findCompiler(set, MatchType::Newest, desc);
            break;
        }
        default: break;
    }

    set->setDefaultCompiler(sourceLanguage, compiler);
}

/* static */void DownstreamCompilerUtil::updateDefaults(DownstreamCompilerSet* set)
{
    for (Index i = 0; i < Index(SLANG_SOURCE_LANGUAGE_COUNT_OF); ++i)
    {
        updateDefault(set, SlangSourceLanguage(i));
    }
}

static SlangResult _locateDXCCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
    // First try dxil, so it's loaded from the same path if it's there
    ComPtr<ISlangSharedLibrary> dxil;
    DefaultSharedLibraryLoader::load(loader, path, "dxil", dxil.writeRef());

    ComPtr<ISlangSharedLibrary> sharedLibrary;
    if (SLANG_SUCCEEDED(DefaultSharedLibraryLoader::load(loader, path, "dxcompiler", sharedLibrary.writeRef())))
    {
        // Can we determine the version?
        DownstreamCompiler::Desc desc(SLANG_PASS_THROUGH_DXC);
        RefPtr<DownstreamCompiler> compiler(new SharedLibraryDownstreamCompiler(desc, sharedLibrary));

        set->addCompiler(compiler);
    }
    return SLANG_OK;
}

static SlangResult _locateFXCCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
    ComPtr<ISlangSharedLibrary> sharedLibrary;
    if (SLANG_SUCCEEDED(DefaultSharedLibraryLoader::load(loader, path, "d3dcompiler_47", sharedLibrary.writeRef())))
    {
        // Can we determine the version?
        DownstreamCompiler::Desc desc(SLANG_PASS_THROUGH_FXC);
        RefPtr<DownstreamCompiler> compiler(new SharedLibraryDownstreamCompiler(desc, sharedLibrary));
        set->addCompiler(compiler);
    }
    return SLANG_OK;
}

static SlangResult _locateGlslangCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
#if SLANG_UNIX_FAMILY
    // On unix systems we need to ensure pthread is loaded first.
    ComPtr<ISlangSharedLibrary> pthreadLibrary;
    DefaultSharedLibraryLoader::load(loader, path, "pthread", pthreadLibrary.writeRef());
#endif
    ComPtr<ISlangSharedLibrary> sharedLibrary;
    if (SLANG_SUCCEEDED(DefaultSharedLibraryLoader::load(loader, path, "slang-glslang", sharedLibrary.writeRef())))
    {
        // Can we determine the version?
        DownstreamCompiler::Desc desc(SLANG_PASS_THROUGH_GLSLANG);
        RefPtr<DownstreamCompiler> compiler(new SharedLibraryDownstreamCompiler(desc, sharedLibrary));
        set->addCompiler(compiler);
    }
    return SLANG_OK;
}

/* static */void DownstreamCompilerUtil::setDefaultLocators(DownstreamCompilerLocatorFunc outFuncs[int(SLANG_PASS_THROUGH_COUNT_OF)])
{
    outFuncs[int(SLANG_PASS_THROUGH_VISUAL_STUDIO)] = &VisualStudioCompilerUtil::locateCompilers;
    outFuncs[int(SLANG_PASS_THROUGH_CLANG)] = &GCCDownstreamCompilerUtil::locateClangCompilers;
    outFuncs[int(SLANG_PASS_THROUGH_GCC)] = &GCCDownstreamCompilerUtil::locateGCCCompilers;
    outFuncs[int(SLANG_PASS_THROUGH_NVRTC)] = &NVRTCDownstreamCompilerUtil::locateCompilers;
    outFuncs[int(SLANG_PASS_THROUGH_DXC)] = &_locateDXCCompilers;
    outFuncs[int(SLANG_PASS_THROUGH_FXC)] = &_locateFXCCompilers;
    outFuncs[int(SLANG_PASS_THROUGH_GLSLANG)] = &_locateGlslangCompilers;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompilerSet !!!!!!!!!!!!!!!!!!!!!!*/

void DownstreamCompilerSet::getCompilerDescs(List<DownstreamCompiler::Desc>& outCompilerDescs) const
{
    outCompilerDescs.clear();
    for (DownstreamCompiler* compiler : m_compilers)
    {
        outCompilerDescs.add(compiler->getDesc());
    }
}

Index DownstreamCompilerSet::_findIndex(const DownstreamCompiler::Desc& desc) const
{
    const Index count = m_compilers.getCount();
    for (Index i = 0; i < count; ++i)
    { 
        if (m_compilers[i]->getDesc() == desc)
        {
            return i;
        }
    }
    return -1;
}

DownstreamCompiler* DownstreamCompilerSet::getCompiler(const DownstreamCompiler::Desc& compilerDesc) const
{
    const Index index = _findIndex(compilerDesc);
    return index >= 0 ? m_compilers[index] : nullptr;
}

void DownstreamCompilerSet::getCompilers(List<DownstreamCompiler*>& outCompilers) const
{
    outCompilers.clear();
    outCompilers.addRange((DownstreamCompiler*const*)m_compilers.begin(), m_compilers.getCount());
}

bool DownstreamCompilerSet::hasCompiler(SlangPassThrough compilerType) const
{
    for (DownstreamCompiler* compiler : m_compilers)
    {
        const auto& desc = compiler->getDesc();
        if (desc.type == compilerType)
        {
            return true;
        }
    }
    return false;
}


void DownstreamCompilerSet::remove(SlangPassThrough compilerType)
{
    for (Index i = 0; i < m_compilers.getCount(); ++i)
    {
        DownstreamCompiler* compiler = m_compilers[i];
        if (compiler->getDesc().type == compilerType)
        {
            m_compilers.fastRemoveAt(i);
            i--;
        }
    }
}

void DownstreamCompilerSet::addCompiler(DownstreamCompiler* compiler)
{
    const Index index = _findIndex(compiler->getDesc());
    if (index >= 0)
    {
        m_compilers[index] = compiler;
    }
    else
    {
        m_compilers.add(compiler);
    }
}

}
