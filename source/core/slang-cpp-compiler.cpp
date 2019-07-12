// slang-cpp-compiler.cpp
#include "slang-cpp-compiler.h"

#include "slang-common.h"
#include "../../slang-com-helper.h"
#include "slang-string-util.h"

#include "slang-io.h"
#include "slang-shared-library.h"

// if Visual Studio import the visual studio platform specific header
#if SLANG_VC
#   include "windows/slang-win-visual-studio-util.h"
#endif

#include "slang-visual-studio-compiler-util.h"
#include "slang-gcc-compiler-util.h"

namespace Slang
{

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPCompiler::Desc !!!!!!!!!!!!!!!!!!!!!!*/

void CPPCompiler::Desc::appendAsText(StringBuilder& out) const
{
    out << getCompilerTypeAsText(type);
    out << " ";
    out << majorVersion;
    out << ".";
    out << minorVersion;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPCompiler::OutputMessage !!!!!!!!!!!!!!!!!!!!!!*/

/* static */UnownedStringSlice CPPCompiler::OutputMessage::getTypeText(OutputMessage::Type type)
{
    typedef OutputMessage::Type Type;
    switch (type)
    {
        default:            return UnownedStringSlice::fromLiteral("Unknown");
        case Type::Info:    return UnownedStringSlice::fromLiteral("Info");
        case Type::Warning: return UnownedStringSlice::fromLiteral("Warning");
        case Type::Error:   return UnownedStringSlice::fromLiteral("Error");
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPCompiler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* static */UnownedStringSlice CPPCompiler::getCompilerTypeAsText(CompilerType type)
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
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPCompiler::Output !!!!!!!!!!!!!!!!!!!!!!*/

Index CPPCompiler::Output::getCountByType(OutputMessage::Type type) const
{
    Index count = 0;
    for (const auto& msg : messages)
    {
        count += Index(msg.type == type);
    }
    return count;
}

Int CPPCompiler::Output::countByStage(OutputMessage::Stage stage, Index counts[Int(OutputMessage::Type::CountOf)]) const
{
    Int count = 0;
    ::memset(counts, 0, sizeof(Index) * Int(OutputMessage::Type::CountOf));
    for (const auto& msg : messages)
    {
        if (msg.stage == stage)
        {
            count++;
            counts[Index(msg.type)]++;
        }
    }
    return count++;
}

static void _appendCounts(const Index counts[Int(CPPCompiler::OutputMessage::Type::CountOf)], StringBuilder& out)
{
    typedef CPPCompiler::OutputMessage::Type Type;

    for (Index i = 0; i < Int(Type::CountOf); i++)
    {
        if (counts[i] > 0)
        {
            out << CPPCompiler::OutputMessage::getTypeText(Type(i)) << "(" << counts[i] << ") ";
        }
    }
}

static void _appendSimplified(const Index counts[Int(CPPCompiler::OutputMessage::Type::CountOf)], StringBuilder& out)
{
    typedef CPPCompiler::OutputMessage::Type Type;
    for (Index i = 0; i < Int(Type::CountOf); i++)
    {
        if (counts[i] > 0)
        {
            out << CPPCompiler::OutputMessage::getTypeText(Type(i)) << " ";
        }
    }
}

void CPPCompiler::Output::appendSummary(StringBuilder& out) const
{
    Index counts[Int(OutputMessage::Type::CountOf)];
    if (countByStage(OutputMessage::Stage::Compile, counts) > 0)
    {
        out << "Compile: ";
        _appendCounts(counts, out);
        out << "\n";
    }
    if (countByStage(OutputMessage::Stage::Link, counts) > 0)
    {
        out << "Link: ";
        _appendCounts(counts, out);
        out << "\n";
    }
}

void CPPCompiler::Output::appendSimplifiedSummary(StringBuilder& out) const
{
    Index counts[Int(OutputMessage::Type::CountOf)];
    if (countByStage(OutputMessage::Stage::Compile, counts) > 0)
    {
        out << "Compile: ";
        _appendSimplified(counts, out);
        out << "\n";
    }
    if (countByStage(OutputMessage::Stage::Link, counts) > 0)
    {
        out << "Link: ";
        _appendSimplified(counts, out);
        out << "\n";
    }
}

void CPPCompiler::Output::removeByType(OutputMessage::Type type)
{
    Index count = messages.getCount();
    for (Index i = 0; i < count; ++i)
    {
        if (messages[i].type == type)
        {
            messages.removeAt(i);
            i--;
            count--;
        }
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! GenericCPPCompiler !!!!!!!!!!!!!!!!!!!!!!*/

SlangResult GenericCPPCompiler::compile(const CompileOptions& options, Output& outOutput)
{
    outOutput.reset();

    // Copy the command line options
    CommandLine cmdLine(m_cmdLine);

    // Append command line args to the end of cmdLine using the target specific function for the specified options
    m_calcArgsFunc(options, cmdLine);

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

    return m_parseOutputFunc(exeRes, outOutput);
}

SlangResult GenericCPPCompiler::calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath)
{
    return m_calcModuleFilePathFunc(options, outPath);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPCompilerUtil !!!!!!!!!!!!!!!!!!!!!!*/

static CPPCompiler::Desc _calcCompiledWithDesc()
{
    CPPCompiler::Desc desc = {};

#if SLANG_VC
    desc = WinVisualStudioUtil::getDesc(WinVisualStudioUtil::getCompiledVersion());
#elif SLANG_CLANG
    desc.type = CPPCompiler::CompilerType::Clang;
    desc.majorVersion = Int(__clang_major__);
    desc.minorVersion = Int(__clang_minor__);
#elif SLANG_SNC
    desc.type = CPPCompiler::CompilerType::SNC;
#elif SLANG_GHS
    desc.type = CPPCompiler::CompilerType::GHS;
#elif SLANG_GCC
    desc.type = CPPCompiler::CompilerType::GCC;
    desc.majorVersion = Int(__GNUC__);
    desc.minorVersion = Int(__GNUC_MINOR__);
#else
    desc.type = CPPCompiler::CompilerType::Unknown;
#endif

    return desc;
}

const CPPCompiler::Desc& CPPCompilerUtil::getCompiledWithDesc()
{
    static CPPCompiler::Desc s_desc = _calcCompiledWithDesc();
    return s_desc;
}

/* static */CPPCompiler* CPPCompilerUtil::findCompiler(const CPPCompilerSet* set, MatchType matchType, const CPPCompiler::Desc& desc)
{
    List<CPPCompiler*> compilers;
    set->getCompilers(compilers);
    return findCompiler(compilers, matchType, desc);
}

/* static */CPPCompiler* CPPCompilerUtil::findCompiler(const List<CPPCompiler*>& compilers, MatchType matchType, const CPPCompiler::Desc& desc)
{
    Int bestIndex = -1;

    const CPPCompiler::CompilerType type = desc.type;

    Int maxVersionValue = 0;
    Int minVersionDiff = 0x7fffffff;

    const auto descVersionValue = desc.getVersionValue();

    for (Index i = 0; i < compilers.getCount(); ++i)
    {
        CPPCompiler* compiler = compilers[i];
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

/* static */CPPCompiler* CPPCompilerUtil::findClosestCompiler(const List<CPPCompiler*>& compilers, const CPPCompiler::Desc& desc)
{
    CPPCompiler* compiler;

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
    if (desc.type == CPPCompiler::CompilerType::GCC || desc.type == CPPCompiler::CompilerType::Clang)
    {
        CPPCompiler::Desc compatible = desc;
        compatible.type = (compatible.type == CPPCompiler::CompilerType::Clang) ? CPPCompiler::CompilerType::GCC : CPPCompiler::CompilerType::Clang;

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

// Have to do this conditionally because unreferenced static functions are a warning on VC, and warnings are errors.
#if !SLANG_WINDOWS_FAMILY
static void _addGCCFamilyCompiler(const String& exeName, CPPCompilerSet* compilerSet)
{
    CPPCompiler::Desc desc;
    if (SLANG_SUCCEEDED(GCCCompilerUtil::calcVersion(exeName, desc)))
    {
        RefPtr<CPPCompiler> compiler(new GenericCPPCompiler(desc, exeName, &GCCCompilerUtil::calcArgs, &GCCCompilerUtil::parseOutput, GCCCompilerUtil::calcModuleFilePath));
        compilerSet->addCompiler(compiler);
    }
}
#endif

/* static */CPPCompiler* CPPCompilerUtil::findClosestCompiler(const CPPCompilerSet* set, const CPPCompiler::Desc& desc)
{
    CPPCompiler* compiler = set->getCompiler(desc);
    if (compiler)
    {
        return compiler;
    }
    List<CPPCompiler*> compilers;
    set->getCompilers(compilers);
    return findClosestCompiler(compilers, desc);
}

/* static */SlangResult CPPCompilerUtil::initializeSet(CPPCompilerSet* set)
{
#if SLANG_WINDOWS_FAMILY
    WinVisualStudioUtil::find(set);
#else
    _addGCCFamilyCompiler("clang", set);
    _addGCCFamilyCompiler("g++", set);
#endif

    // Set the default to the compiler closest to how this source was compiled
    set->setDefaultCompiler(findClosestCompiler(set, getCompiledWithDesc()));
    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPCompilerFactory !!!!!!!!!!!!!!!!!!!!!!*/


void CPPCompilerSet::getCompilerDescs(List<CPPCompiler::Desc>& outCompilerDescs) const
{
    outCompilerDescs.clear();
    for (CPPCompiler* compiler : m_compilers)
    {
        outCompilerDescs.add(compiler->getDesc());
    }
}

Index CPPCompilerSet::_findIndex(const CPPCompiler::Desc& desc) const
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

CPPCompiler* CPPCompilerSet::getCompiler(const CPPCompiler::Desc& compilerDesc) const
{
    const Index index = _findIndex(compilerDesc);
    return index >= 0 ? m_compilers[index] : nullptr;
}

void CPPCompilerSet::getCompilers(List<CPPCompiler*>& outCompilers) const
{
    outCompilers.clear();
    outCompilers.addRange((CPPCompiler*const*)m_compilers.begin(), m_compilers.getCount());
}

bool CPPCompilerSet::hasCompiler(CPPCompiler::CompilerType compilerType) const
{
    for (CPPCompiler* compiler : m_compilers)
    {
        const auto& desc = compiler->getDesc();
        if (desc.type == compilerType)
        {
            return true;
        }
    }
    return false;
}

void CPPCompilerSet::addCompiler(CPPCompiler* compiler)
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
