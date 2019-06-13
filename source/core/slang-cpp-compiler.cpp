// slang-cpp-compiler.cpp
#include "slang-cpp-compiler.h"

#include "slang-common.h"
#include "../../slang-com-helper.h"
#include "slang-string-util.h"

#if SLANG_VC
#   include "windows/slang-win-visual-studio-util.h"
#else
#   include "unix/slang-unix-cpp-compiler-util.h"
#endif

namespace Slang
{

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! GenericCPPCompiler !!!!!!!!!!!!!!!!!!!!!!*/

SlangResult GenericCPPCompiler::compile(const CompileOptions& options, ExecuteResult& outResult)
{
    CommandLine cmdLine;

    // Calculate the command line options
    m_func(options, cmdLine);

    // Set the executable
    cmdLine.setExecutableFilename(m_exeName);

#if 0
    // Test
    {
        String line = ProcessUtil::getCommandLineString(cmdLine);
        printf("%s", line.getBuffer());
    }
#endif

    return ProcessUtil::execute(cmdLine, outResult);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPCompilerUtil !!!!!!!!!!!!!!!!!!!!!!*/

static bool _isDigit(char c)
{
    return c >= '0' && c <= '9';
}

static bool _isWhiteSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/* static */SlangResult CPPCompilerUtil::parseGccFamilyVersion(const UnownedStringSlice& text, const UnownedStringSlice& versionPrefix, CPPCompiler::Desc& outDesc)
{
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(text, lines);

    for (auto line : lines)
    {
        if (String(line).startsWith(versionPrefix))
        {
            UnownedStringSlice versionSlice(line.begin() + versionPrefix.size(), line.end());

            List<Int> digits;

            const char* cur = versionSlice.begin();
            const char* end = versionSlice.end();

            // Consume white space
            while (cur < end && _isWhiteSpace(*cur)) cur++;

            // Version is in format 0.0.0 
            while (true)
            {
                Int value = 0;
                const char* start = cur;
                while (cur < end && _isDigit(*cur))
                {
                    value = value * 10 + (*cur - '0');
                    cur++;
                }

                if (cur <= start)
                {
                    break;
                }

                digits.add(value);

                if (cur < end && *cur == '.')
                {
                    cur++;
                }
            }

            if (digits.getCount() < 2)
            {
                return SLANG_FAIL;
            }

            outDesc.majorVersion = digits[0];
            outDesc.minorVersion = digits[1];

            return SLANG_OK;
        }
    }

    return SLANG_FAIL;
}

SlangResult CPPCompilerUtil::calcGccFamilyVersion(const String& exeName, const UnownedStringSlice& versionPrefix, CPPCompiler::Desc& outDesc)
{
    CommandLine cmdLine;
    cmdLine.setExecutableFilename(exeName);
    cmdLine.addArg("-v");

    ExecuteResult exeRes;
    SLANG_RETURN_ON_FAIL(ProcessUtil::execute(cmdLine, exeRes));
    return parseGccFamilyVersion(exeRes.standardOutput.getUnownedSlice(), versionPrefix, outDesc);
}

static CPPCompiler::Desc _calcCompiledWithDesc()
{
    CPPCompiler::Desc desc = {};

#if SLANG_VC
    desc = WinVisualStudioUtil::getDesc(WinVisualStudioUtil::getCompiledVersion());
#elif SLANG_CLANG
    desc.type = CPPCompiler::Type::Clang;
#elif SLANG_SNC
    desc.type = CPPCompiler::Type::SNC;
#elif SLANG_GHS
    desc.type = CPPCompiler::Type::GHS;
#elif SLANG_GCC
    desc.type = CPPCompiler::Type::GCC;
#else
    desc.type = CPPCompiler::Type::Unknown;
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

    const CPPCompiler::Type type = desc.type;

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
    if (desc.type == CPPCompiler::Type::GCC || desc.type == CPPCompiler::Type::Clang)
    {
        CPPCompiler::Desc compatible = desc;
        compatible.type = (compatible.type == CPPCompiler::Type::Clang) ? CPPCompiler::Type::GCC : CPPCompiler::Type::Clang;

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
    {
        CPPCompiler::Desc desc(CPPCompiler::Type::Clang);
        if (SLANG_SUCCEEDED(calcGccFamilyVersion("clang", UnownedStringSlice::fromLiteral("clang version"), desc)))
        {
            RefPtr<CPPCompiler> compiler(new GenericCPPCompiler(desc, "clang", &UnixCPPCompilerUtil::calcArgs));
            set->addCompiler(compiler);
        }
    }
    {
        CPPCompiler::Desc desc(CPPCompiler::Type::GCC);
        if (SLANG_SUCCEEDED(calcGccFamilyVersion("g++", UnownedStringSlice::fromLiteral("gcc version"), desc)))
        {
            RefPtr<CPPCompiler> compiler(new GenericCPPCompiler(desc, "g++", &UnixCPPCompilerUtil::calcArgs));
            set->addCompiler(compiler);
        }
    }
#endif

    List<CPPCompiler*> compilers;
    set->getCompilers(compilers);

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
