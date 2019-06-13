// slang-cpp-compiler.cpp
#include "slang-cpp-compiler.h"

#include "slang-common.h"
#include "../../slang-com-helper.h"
#include "slang-string-util.h"

#ifdef SLANG_VC
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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPCompilerFactory !!!!!!!!!!!!!!!!!!!!!!*/

CPPCompilerSystem::CPPCompilerSystem()
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
#elif
    desc.type = CPPCompiler::Type::GCC;
#else
    desc.type = CPPCompiler::Type::Unknown;
#endif

    m_compiledWith = desc;
}

static bool _isDigit(char c)
{
    return c >= '0' && c <= '9';
}

static bool _isWhiteSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/* static */SlangResult CPPCompilerSystem::parseGccFamilyVersion(const UnownedStringSlice& text, const UnownedStringSlice& versionPrefix, CPPCompiler::Desc& outDesc)
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

SlangResult CPPCompilerSystem::calcGccFamilyVersion(const String& exeName, const UnownedStringSlice& versionPrefix, CPPCompiler::Desc& outDesc)
{
    CommandLine cmdLine;
    cmdLine.setExecutableFilename(exeName);
    cmdLine.addArg("-v");

    ExecuteResult exeRes;
    SLANG_RETURN_ON_FAIL(ProcessUtil::execute(cmdLine, exeRes));

    return CPPCompilerSystem::parseGccFamilyVersion(exeRes.standardOutput.getUnownedSlice(), versionPrefix, outDesc);
}

SlangResult CPPCompilerSystem::_init()
{
#if SLANG_WINDOWS_FAMILY
    List<WinVisualStudioUtil::VersionPath> versionPaths;
    WinVisualStudioUtil::find(versionPaths);

    for (const auto& versionPath : versionPaths)
    {
        RefPtr<CPPCompiler> compiler(new WinVisualStudioCompiler(WinVisualStudioUtil::getDesc(versionPath.version), versionPath));
        addCompiler(compiler);
    }
#else
    {
        CPPCompiler::Desc desc = { CPPCompiler::Type::Clang };
        if (SLANG_SUCCEEDED(calcGccFamilyVersion("clang", UnownedStringSlice::fromLiteral("clang version"), desc)))
        {
            RefPtr<CPPCompiler> compiler(new GenericCPPCompiler(desc, "clang", &UnixCPPCompilerUtil::calcArgs));
            addCompiler(compiler);
        }
    }
    {
        CPPCompiler::Desc desc = { CPPCompiler::Type::GCC };
        if (SLANG_SUCCEEDED(calcGccFamilyVersion("g++", UnownedStringSlice::fromLiteral("gcc version"), desc)))
        {
            RefPtr<CPPCompiler> compiler(new GenericCPPCompiler(desc, "g++", &UnixCPPCompilerUtil::calcArgs));
            addCompiler(compiler);
        }
    }
#endif

    // Need to find the 'best' compiler relative to what built this binary
    m_closestRuntimeCompiler = findClosestRuntimeCompiler();

    //
    return SLANG_OK;
}

CPPCompiler* CPPCompilerSystem::findClosestRuntimeCompiler()
{
    CPPCompiler::Desc compiledWith = getCompiledWithDesc();

    CPPCompiler* compiler;
    compiler = findCompiler(MatchType::MinGreaterEqual, compiledWith);
    if (compiler)
    {
        return compiler;
    }
    compiler = findCompiler(MatchType::MinAbsolute, compiledWith);
    if (compiler)
    {
        return compiler;
    }

    // If we are gcc, we can try clang and vice versa
    if (compiledWith.type == CPPCompiler::Type::GCC || compiledWith.type == CPPCompiler::Type::Clang)
    {
        CPPCompiler::Desc compatible = compiledWith;
        compatible.type = (compatible.type == CPPCompiler::Type::Clang) ? CPPCompiler::Type::GCC : CPPCompiler::Type::Clang;

        compiler = findCompiler(MatchType::MinGreaterEqual, compatible);
        if (compiler)
        {
            return compiler;
        }
        compiler = findCompiler(MatchType::MinAbsolute, compatible);
        if (compiler)
        {
            return compiler;
        }
    }

    return nullptr;
}

CPPCompiler* CPPCompilerSystem::findCompiler(MatchType matchType, CPPCompiler::Desc& desc) const
{
    Int bestIndex = -1;

    const CPPCompiler::Type type = desc.type;

    Int maxVersionValue = 0;
    Int minVersionDiff = 0x7fffffff;

    const auto descVersionValue = desc.getVersionValue();

    for (Index i = 0; i < m_compilers.getCount(); ++i)
    {
        CPPCompiler* compiler = m_compilers[i];
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
    
    return (bestIndex >= 0) ? getCompilerByIndex(bestIndex) : nullptr;
}

void CPPCompilerSystem::getCompilerDescs(List<CPPCompiler::Desc>& outCompilerDescs) const
{
    outCompilerDescs.clear();
    for (CPPCompiler* compiler : m_compilers)
    {
        outCompilerDescs.add(compiler->getDesc());
    }
}

CPPCompiler* CPPCompilerSystem::getCompiler(const CPPCompiler::Desc& compilerDesc)
{
    for (CPPCompiler* compiler : m_compilers)
    {
        if (compiler->getDesc() == compilerDesc)
        {
            return compiler;
        }
    }
    return nullptr;
}

void CPPCompilerSystem::addCompiler(CPPCompiler* compiler)
{
    for (Index i = 0; i < m_compilers.getCount(); ++i)
    {
        CPPCompiler* cur = m_compilers[i];
        if (cur->getDesc() == compiler->getDesc())
        {
            m_compilers[i] = compiler;
            return;
        }
    }
    m_compilers.add(compiler);
}

/* static */RefPtr<CPPCompilerSystem> CPPCompilerSystem::create()
{
    RefPtr<CPPCompilerSystem> factory;
    if (SLANG_FAILED(factory->_init()))
    {
        return nullptr;
    }
    return factory;
}

}
