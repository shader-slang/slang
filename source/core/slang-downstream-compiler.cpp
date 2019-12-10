// slang-downstream-compiler.cpp
#include "slang-downstream-compiler.h"

#include "slang-common.h"
#include "../../slang-com-helper.h"
#include "slang-string-util.h"

#include "slang-io.h"
#include "slang-shared-library.h"
#include "slang-blob.h"

// if Visual Studio import the visual studio platform specific header
#if SLANG_VC
#   include "windows/slang-win-visual-studio-util.h"
#endif

#include "slang-visual-studio-compiler-util.h"
#include "slang-gcc-compiler-util.h"
#include "slang-nvrtc-compiler.h"

namespace Slang
{

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompiler::Desc !!!!!!!!!!!!!!!!!!!!!!*/

void DownstreamCompiler::Desc::appendAsText(StringBuilder& out) const
{
    out << getCompilerTypeAsText(type);
    out << " ";
    out << majorVersion;
    out << ".";
    out << minorVersion;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamDiagnostic !!!!!!!!!!!!!!!!!!!!!!!!*/

/* static */UnownedStringSlice DownstreamDiagnostic::getTypeText(Type type)
{
    switch (type)
    {
        default:            return UnownedStringSlice::fromLiteral("Unknown");
        case Type::Info:    return UnownedStringSlice::fromLiteral("Info");
        case Type::Warning: return UnownedStringSlice::fromLiteral("Warning");
        case Type::Error:   return UnownedStringSlice::fromLiteral("Error");
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompiler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* static */UnownedStringSlice DownstreamCompiler::getCompilerTypeAsText(CompilerType type)
{
    switch (type)
    {
        default:
        case CompilerType::Unknown:     return UnownedStringSlice::fromLiteral("Unknown");
        case CompilerType::VisualStudio:return UnownedStringSlice::fromLiteral("Visual Studio");
        case CompilerType::GCC:         return UnownedStringSlice::fromLiteral("GCC");
        case CompilerType::Clang:       return UnownedStringSlice::fromLiteral("Clang");
        case CompilerType::SNC:         return UnownedStringSlice::fromLiteral("SNC");
        case CompilerType::GHS:         return UnownedStringSlice::fromLiteral("GHS");
        case CompilerType::NVRTC:       return UnownedStringSlice::fromLiteral("NVRTC");
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamDiagnostics !!!!!!!!!!!!!!!!!!!!!!*/

Index DownstreamDiagnostics::getCountByType(Diagnostic::Type type) const
{
    Index count = 0;
    for (const auto& msg : diagnostics)
    {
        count += Index(msg.type == type);
    }
    return count;
}

Int DownstreamDiagnostics::countByStage(Diagnostic::Stage stage, Index counts[Int(Diagnostic::Type::CountOf)]) const
{
    Int count = 0;
    ::memset(counts, 0, sizeof(Index) * Int(Diagnostic::Type::CountOf));
    for (const auto& diagnostic : diagnostics)
    {
        if (diagnostic.stage == stage)
        {
            count++;
            counts[Index(diagnostic.type)]++;
        }
    }
    return count++;
}

static void _appendCounts(const Index counts[Int(DownstreamDiagnostic::Type::CountOf)], StringBuilder& out)
{
    typedef DownstreamDiagnostic::Type Type;

    for (Index i = 0; i < Int(Type::CountOf); i++)
    {
        if (counts[i] > 0)
        {
            out << DownstreamDiagnostic::getTypeText(Type(i)) << "(" << counts[i] << ") ";
        }
    }
}

static void _appendSimplified(const Index counts[Int(DownstreamDiagnostic::Type::CountOf)], StringBuilder& out)
{
    typedef DownstreamDiagnostic::Type Type;
    for (Index i = 0; i < Int(Type::CountOf); i++)
    {
        if (counts[i] > 0)
        {
            out << DownstreamDiagnostic::getTypeText(Type(i)) << " ";
        }
    }
}

void DownstreamDiagnostics::appendSummary(StringBuilder& out) const
{
    Index counts[Int(Diagnostic::Type::CountOf)];
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
    Index counts[Int(Diagnostic::Type::CountOf)];
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

void DownstreamDiagnostics::removeByType(Diagnostic::Type type)
{
    Index count = diagnostics.getCount();
    for (Index i = 0; i < count; ++i)
    {
        if (diagnostics[i].type == type)
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
        
        if (options.sourceContents.getLength() != 0)
        {
            String compileSourcePath = modulePath;

            // NOTE: Strictly speaking producing filenames by modifying the generateTemporary path that may introduce a temp filename clash, but in practice is extraordinary unlikely
            compileSourcePath.append("-src");

            // Make the temporary filename have the appropriate extension.
            if (options.sourceType == DownstreamCompiler::SourceType::C)
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

            // There is no source contents
            options.sourceContents = String();
        }
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
    DownstreamCompiler::Desc desc = {};

#if SLANG_VC
    desc = WinVisualStudioUtil::getDesc(WinVisualStudioUtil::getCompiledVersion());
#elif SLANG_CLANG
    desc.type = DownstreamCompiler::CompilerType::Clang;
    desc.majorVersion = Int(__clang_major__);
    desc.minorVersion = Int(__clang_minor__);
#elif SLANG_SNC
    desc.type = DownstreamCompiler::CompilerType::SNC;
#elif SLANG_GHS
    desc.type = DownstreamCompiler::CompilerType::GHS;
#elif SLANG_GCC
    desc.type = DownstreamCompiler::CompilerType::GCC;
    desc.majorVersion = Int(__GNUC__);
    desc.minorVersion = Int(__GNUC_MINOR__);
#else
    desc.type = DownstreamCompiler::CompilerType::Unknown;
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
    Int bestIndex = -1;

    const DownstreamCompiler::CompilerType type = desc.type;

    Int maxVersionValue = 0;
    Int minVersionDiff = 0x7fffffff;

    const auto descVersionValue = desc.getVersionValue();

    for (Index i = 0; i < compilers.getCount(); ++i)
    {
        DownstreamCompiler* compiler = compilers[i];
        auto compilerDesc = compiler->getDesc();

        if (type == compilerDesc.type)
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
    if (desc.type == DownstreamCompiler::CompilerType::GCC || desc.type == DownstreamCompiler::CompilerType::Clang)
    {
        DownstreamCompiler::Desc compatible = desc;
        compatible.type = (compatible.type == DownstreamCompiler::CompilerType::Clang) ? DownstreamCompiler::CompilerType::GCC : DownstreamCompiler::CompilerType::Clang;

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

static void _addGCCFamilyCompiler(const String& path, const String& inExeName, DownstreamCompilerSet* compilerSet)
{
    String exeName(inExeName);
    if (path.getLength() > 0)
    {
        exeName = Path::combine(path, inExeName);
    }

    DownstreamCompiler::Desc desc;
    if (SLANG_SUCCEEDED(GCCDownstreamCompilerUtil::calcVersion(exeName, desc)))
    {
        RefPtr<CommandLineDownstreamCompiler> compiler(new GCCDownstreamCompiler(desc));
        compiler->m_cmdLine.setExecutableFilename(exeName);
        compilerSet->addCompiler(compiler);
    }
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

/* static */SlangResult DownstreamCompilerUtil::initializeSet(const InitializeSetDesc& desc, DownstreamCompilerSet* set)
{
#if SLANG_WINDOWS_FAMILY
    WinVisualStudioUtil::find(set);
#endif

    _addGCCFamilyCompiler(desc.getPath(CompilerType::Clang), "clang", set);
    _addGCCFamilyCompiler(desc.getPath(CompilerType::GCC), "g++", set);

    {
        DownstreamCompiler* cppCompiler = findClosestCompiler(set, getCompiledWithDesc());

        // Set the default to the compiler closest to how this source was compiled
        set->setDefaultCompiler(DownstreamCompiler::SourceType::CPP, cppCompiler);
        set->setDefaultCompiler(DownstreamCompiler::SourceType::C, cppCompiler);
    }

    // Lets see if we have NVRTC. 
    {
        ISlangSharedLibrary* sharedLibrary = desc.sharedLibraries[int(CompilerType::NVRTC)];
        if (sharedLibrary)
        {
            RefPtr<DownstreamCompiler> compiler;
            if (SLANG_SUCCEEDED(NVRTCDownstreamCompilerUtil::createCompiler(sharedLibrary, compiler)))
            {
                set->addCompiler(compiler);

                set->setDefaultCompiler(DownstreamCompiler::SourceType::CUDA, compiler);
            }
        }
    }

    return SLANG_OK;
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

bool DownstreamCompilerSet::hasCompiler(DownstreamCompiler::CompilerType compilerType) const
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
