#include "slang-win-find-vs-util.h"

#include "../slang-common.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#else

namespace Slang {

// Information on VS versioning can be found here
// https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B#Internal_version_numbering

namespace { // anonymous

struct RegistryInfo
{
    const char* regName;            ///< The name of the entry in the registry
    const char* pathFix;            ///< With the value from the registry how to fix the path
};

struct VersionInfo
{
    Version version;                ///< The version 
    const char* regKeyName;         ///< The name of the registry key 
};

typedef WinFindVisualStudioUtil::Version Version;
typedef WinFindVisualStudioUtil::VersionInfo VersionInfo;

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
    ret = RegQueryValueExA(key, keyName, nullptr, nullptr, (LPBYTE*)value, &size);
    RegCloseKey(key);

    if (ret != ERROR_SUCCESS)
    {
        return SLANG_FAIL;
    }

    outString = value;
    return SLANG_OK;
}

// Make easier to set up the array
static Version _makeVersion(int main, int dot = 0) { return WinFindVisualStudioUtil::makeVersion(main, dot); }

VersionInfo _makeVersionInfo(const char* regKeyName, int high, int dot = 0)
{
    VersionInfo info;
    info.regKeyName = regKeyName;
    info.version = WinFindVisualStudioUtil::makeVersion(high, dot);
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

/* static */void WinFindVisualStudioUtil::getVersions(List<Version>& outVersions)
{
    const int count = SLANG_COUNT_OF(s_versionInfos);
    outVersions.setCount(count);

    Version* dst = outVersions.begin();
    for (int i = 0; i < count; ++i)
    {
        dst[i] = s_versionInfos[i].version;
    }
}

/* static */WinFindVisualStudioUtil::Version WinFindVisualStudioUtil::getCompiledVersion()
{
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
        default: break;
    }

    if (version > 1920)
    {
        // Its an unknown newer version
        return Version::Future;
    }

    // Unknown version
    return Version::Unknown;
}

/* static */SlangResult WinFindVisualStudioUtil::find(String& outPath)
{
    int versionCount = SLANG_COUNT_OF(s_versionsInfos);


}

} // namespace Slang
