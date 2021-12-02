#include "slang-win-visual-studio-util.h"

#include "../../core/slang-common.h"
#include "../../core/slang-process-util.h"
#include "../../core/slang-string-util.h"

#include "../slang-visual-studio-compiler-util.h"

#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include <Windows.h>
#   undef WIN32_LEAN_AND_MEAN
#   undef NOMINMAX

#   include <Shlobj.h>
#pragma comment(lib, "advapi32")
#pragma comment(lib, "Shell32")
#endif

// The method used to invoke VS was originally inspired by some ideas in
// https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus/

namespace Slang {

// Information on VS versioning can be found here
// https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B#Internal_version_numbering


namespace { // anonymous

typedef WinVisualStudioUtil::Version Version;

struct RegistryInfo
{
    const char* regName;            ///< The name of the entry in the registry
    const char* pathFix;            ///< With the value from the registry how to fix the path
};

struct VersionInfo
{
    Version version;                ///< The version
    const char* name;         ///< The name of the registry key 
};

} // anonymous

static SlangResult _readRegistryKey(const char* path, const char* keyName, String& outString)
{
    // https://docs.microsoft.com/en-us/windows/desktop/api/winreg/nf-winreg-regopenkeyexa
    HKEY  key;
    LONG ret = RegOpenKeyExA(HKEY_LOCAL_MACHINE, path, 0,  KEY_READ | KEY_WOW64_32KEY, &key);
    if (ret != ERROR_SUCCESS)
    {
        return SLANG_FAIL;
    }

    char value[MAX_PATH];
    DWORD size = MAX_PATH;

    // https://docs.microsoft.com/en-us/windows/desktop/api/winreg/nf-winreg-regqueryvalueexa
    ret = RegQueryValueExA(key, keyName, nullptr, nullptr, (LPBYTE)value, &size);
    RegCloseKey(key);

    if (ret != ERROR_SUCCESS)
    {
        return SLANG_FAIL;
    }

    outString = value;
    return SLANG_OK;
}

// Make easier to set up the array
static Version _makeVersion(int main, int dot = 0) { return WinVisualStudioUtil::makeVersion(main, dot); }

VersionInfo _makeVersionInfo(const char* name, int high, int dot = 0)
{
    VersionInfo info;
    info.name = name;
    info.version = WinVisualStudioUtil::makeVersion(high, dot);
    return info;
}

static const VersionInfo s_versionInfos[] = 
{
    _makeVersionInfo("VS 2005", 8),
    _makeVersionInfo("VS 2008", 9),
    _makeVersionInfo("VS 2010", 10),
    _makeVersionInfo("VS 2012", 11),
    _makeVersionInfo("VS 2013", 12),
    _makeVersionInfo("VS 2015", 14),
    _makeVersionInfo("VS 2017", 15),
    _makeVersionInfo("VS 2019", 16),
};

// When trying to figure out how this stuff works by running regedit - care is needed, 
// because what regedit displays varies on which version of regedit is used. 
// In order to use the registry paths used here it's necessary to use Start/Run with 
// %systemroot%\syswow64\regedit to view 32 bit keys

static const RegistryInfo s_regInfos[] =
{
    {"SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VC7", "" },
    {"SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VS7", "VC\\Auxiliary\\Build\\" },
};

static bool _canUseVSWhere(Version version)
{
    // If greater than 15.0 we can use vswhere tool
    return (int(version) >= int(_makeVersion(15)));
}

static int _getRegistryKeyIndex(Version version)
{
    if (int(version) >= int(_makeVersion(15)))
    {
        return 1;
    }
    return 0;
}

/* static */void WinVisualStudioUtil::getVersions(List<Version>& outVersions)
{
    const int count = SLANG_COUNT_OF(s_versionInfos);
    outVersions.setCount(count);

    Version* dst = outVersions.begin();
    for (int i = 0; i < count; ++i)
    {
        dst[i] = s_versionInfos[i].version;
    }
}

/* static */WinVisualStudioUtil::Version WinVisualStudioUtil::getCompiledVersion()
{
    // Get the version of visual studio used to compile this source
    const uint32_t version = _MSC_VER;

    switch (version)
    {
        case 1400:	    return _makeVersion(8); 
        case 1500:	    return _makeVersion(9);
        case 1600:	    return _makeVersion(10);
        case 1700:	    return _makeVersion(11);
        case 1800:	    return _makeVersion(12);
        case 1900:
        {
            return _makeVersion(14);
        }
        case 1911:
        case 1912:
        case 1913:
        case 1914:
        case 1915:
        case 1916:
        {
            return _makeVersion(15);
        }
        case 1920:
        {
            return _makeVersion(16);
        }
        default:
        {
            int lastKnownVersion = 1920;
            if (version > lastKnownVersion)
            {
                // Its an unknown newer version
                return Version::Future;
            }
            break;
        }
    }

    // Unknown version
    return Version::Unknown;
}

static SlangResult _find(int versionIndex, WinVisualStudioUtil::VersionPath& outPath)
{
    const auto& versionInfo = s_versionInfos[versionIndex];

    auto version = versionInfo.version;

    outPath.version = version;
    outPath.vcvarsPath = String();

    if (_canUseVSWhere(version))
    {
        CommandLine cmd;

        // Lookup directly %ProgramFiles(x86)% path
        // https://docs.microsoft.com/en-us/windows/desktop/api/shlobj_core/nf-shlobj_core-shgetfolderpatha
        HWND hwnd = GetConsoleWindow();

        char programFilesPath[_MAX_PATH];
        SHGetFolderPathA(hwnd, CSIDL_PROGRAM_FILESX86, NULL, 0, programFilesPath);

        String vswherePath = programFilesPath;
        vswherePath.append("\\Microsoft Visual Studio\\Installer\\vswhere");

        cmd.setExecutableLocation(ExecutableLocation(vswherePath));

        StringBuilder versionName;
        WinVisualStudioUtil::append(version, versionName);

        String args[] = { "-version", versionName, "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath" };
        cmd.addArgs(args, SLANG_COUNT_OF(args));

        ExecuteResult exeRes;
        if (SLANG_SUCCEEDED(ProcessUtil::execute(cmd, exeRes)))
        {
            // We need to chopoff CR/LF if there is one
            List<UnownedStringSlice> lines;
            StringUtil::calcLines(exeRes.standardOutput.getUnownedSlice(), lines);

            if (lines.getCount())
            {
                outPath.vcvarsPath = lines[0];
                outPath.vcvarsPath.append("\\VC\\Auxiliary\\Build\\");
                return SLANG_OK;
            }
        }
    }

    const Int keyIndex = _getRegistryKeyIndex(version);
    if (keyIndex >= 0)
    {
        SLANG_ASSERT(keyIndex < SLANG_COUNT_OF(s_regInfos));

        // Try reading the key
        const auto& keyInfo = s_regInfos[keyIndex];

        StringBuilder keyName;
        WinVisualStudioUtil::append(versionInfo.version, keyName);

        String value;
        if (SLANG_SUCCEEDED(_readRegistryKey(keyInfo.regName, keyName.getBuffer(), value)))
        {
            outPath.vcvarsPath = value;
            return SLANG_OK;
        }
    }

    return SLANG_FAIL;
}

/* static */SlangResult WinVisualStudioUtil::find(List<VersionPath>& outVersionPaths)
{
    outVersionPaths.clear();

    const int versionCount = SLANG_COUNT_OF(s_versionInfos);

    for (int i = versionCount - 1; i >= 0; --i)
    {
        VersionPath versionPath;
        if (SLANG_SUCCEEDED(_find(i, versionPath)))
        {
            outVersionPaths.add(versionPath);
        }
    }

    return SLANG_OK;
}

/* static */SlangResult WinVisualStudioUtil::find(Version version, VersionPath& outPath)
{
    const int versionCount = SLANG_COUNT_OF(s_versionInfos);

    for (int i = 0; i < versionCount; ++i)
    {
        const auto& versionInfo = s_versionInfos[i];
        if (versionInfo.version == version)
        {
            return _find(i, outPath);
        }
    }
    return SLANG_FAIL;
}

/* static */SlangResult WinVisualStudioUtil::find(DownstreamCompilerSet* set)
{
    const int versionCount = SLANG_COUNT_OF(s_versionInfos);

    for (int i = versionCount - 1; i >= 0; --i)
    {
        const auto& versionInfo = s_versionInfos[i];
        auto desc = getDesc(versionInfo.version);

        VersionPath versionPath;
        if (!set->getCompiler(desc) && SLANG_SUCCEEDED(_find(i, versionPath)))
        {
            RefPtr<CommandLineDownstreamCompiler> compiler = new VisualStudioDownstreamCompiler(desc);
            calcExecuteCompilerArgs(versionPath, compiler->m_cmdLine);
            set->addCompiler(compiler);
        }
    }

    return SLANG_OK;
}

/* static */void WinVisualStudioUtil::calcExecuteCompilerArgs(const VersionPath& versionPath, CommandLine& outCmdLine)
{
    // To invoke cl we need to run the suitable vcvars. In order to run this we have to have MS CommandLine.
    // So here we build up a cl command line that is run by first running vcvars, and then executing cl with the parameters as passed to commandLine

    // https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessa
    // To run a batch file, you must start the command interpreter; set lpApplicationName to cmd.exe and set lpCommandLine to the
    // following arguments: /c plus the name of the batch file.

    CommandLine cmdLine;
    cmdLine.setExecutableLocation(ExecutableLocation(ExecutableLocation::Type::Name, "cmd.exe"));
    
    {
        String options[] = { "/q", "/c", "@prompt", "$" };
        cmdLine.addArgs(options, SLANG_COUNT_OF(options));
    }

    cmdLine.addArg("&&");
    cmdLine.addArg(Path::combine(versionPath.vcvarsPath, "vcvarsall.bat"));

#if SLANG_PTR_IS_32
    cmdLine.addArg("x86");
#else
    cmdLine.addArg("x86_amd64");
#endif

    cmdLine.addArg("&&");
    cmdLine.addArg("cl");

    outCmdLine = cmdLine;
}

/* static */SlangResult WinVisualStudioUtil::executeCompiler(const VersionPath& versionPath, const CommandLine& commandLine, ExecuteResult& outResult)
{
    CommandLine cmdLine;
    calcExecuteCompilerArgs(versionPath, cmdLine);
    // Append the command line options
    cmdLine.addArgs(commandLine.m_args.getBuffer(), commandLine.m_args.getCount());
    return ProcessUtil::execute(cmdLine, outResult);
}

/* static */void WinVisualStudioUtil::append(Version version, StringBuilder& outBuilder)
{
    switch (version)
    {
        case Version::Unknown:
        {
            outBuilder << "unknown";
        }
        case Version::Future:
        {
            outBuilder << "future";
            break;
        }
        default:
        {
            outBuilder << (int(version) / 10) << "." << (int(version) % 10);
            break;
        }
    }
}

} // namespace Slang
