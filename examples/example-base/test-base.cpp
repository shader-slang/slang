#include "test-base.h"

#ifdef _WIN32
// clang-format off
// include ordering sensitive
#    include <windows.h>
#    include <shellapi.h>
#    include <io.h>
#    include <fcntl.h>
// clang-format on
#include <iostream>
#include <string>
#endif

static rhi::DeviceType parseApiString(const char* apiStr)
{
    static const struct
    {
        const char* name;
        rhi::DeviceType type;
    } apiTable[] = {
#ifdef _WIN32
        {"d3d11", rhi::DeviceType::D3D11},
        {"d3d12", rhi::DeviceType::D3D12},
#endif
        {"vulkan", rhi::DeviceType::Vulkan},
#ifdef __APPLE__
        {"metal", rhi::DeviceType::Metal},
#endif
        {"cpu", rhi::DeviceType::CPU},
        {"cuda", rhi::DeviceType::CUDA},
        {"webgpu", rhi::DeviceType::WGPU}};

    for (auto& api : apiTable)
    {
        if (strcmp(apiStr, api.name) == 0)
        {
            return api.type;
        }
    }
    return rhi::DeviceType::Default; // Invalid/unknown
}

#ifdef _WIN32
static std::string wstringToString(const std::wstring& wstr)
{
    char buffer[256];
    wcstombs(buffer, wstr.c_str(), sizeof(buffer));
    return std::string(buffer);
}
#endif

// Simple output function that works for both console and WIN32 apps
static void printOutput(const char* text)
{
    printf("%s", text);
#ifdef _WIN32
    OutputDebugStringA(text);
#endif
}

#ifdef _WIN32
// For WIN32 apps, allocate a console window to show help text
static bool ensureConsoleVisible()
{
    // Check if we already have a console (running from cmd.exe or already allocated)
    if (GetConsoleWindow() != NULL)
    {
        // We already have a console, just use it
        return false; // No new window created
    }

    // Check if stdout is connected to a console or redirected
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut != INVALID_HANDLE_VALUE)
    {
        DWORD fileType = GetFileType(hStdOut);
        switch (fileType)
        {
        case FILE_TYPE_CHAR: // Console
        case FILE_TYPE_DISK: // File redirection
        case FILE_TYPE_PIPE: // Pipe redirection
            // Output goes to console or is redirected, don't create a console
            return false;
        }
        // FILE_TYPE_UNKNOWN or other cases: proceed to create console
    }

    // No console exists and output isn't redirected, so allocate a new one
    if (AllocConsole())
    {
        // Redirect stdout to the console
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
        freopen_s((FILE**)stdin, "CONIN$", "r", stdin);

        // Make sure cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
        // point to console as well
        std::ios::sync_with_stdio(true);

        // Set console title
        SetConsoleTitleA("Triangle Example - Help");
        return true; // New window was created
    }

    return false; // Failed to create console
}
#endif

int TestBase::parseOption(int argc, char** argv)
{
    // Parse command line arguments for help, API selection, and test mode
#ifdef _WIN32
    wchar_t** szArglist;
    szArglist = CommandLineToArgvW(GetCommandLineW(), &argc);
#endif

    for (int i = 0; i < argc; i++)
    {
#ifdef _WIN32
        std::string arg = wstringToString(szArglist[i]);
#else
        std::string arg = argv[i];
#endif

        if (arg == "--test-mode" || arg == "-test-mode")
        {
            m_isTestMode = true;
        }
        else if (arg == "-h" || arg == "--help")
        {
            m_showHelp = true;
        }
        else if ((arg == "-api" || arg == "--api") && i + 1 < argc)
        {
            i++; // Move to the next argument which should be the API name
#ifdef _WIN32
            std::string apiStr = wstringToString(szArglist[i]);
#else
            std::string apiStr = argv[i];
#endif

            rhi::DeviceType newType = parseApiString(apiStr.c_str());
            if (newType != rhi::DeviceType::Default)
            {
                m_deviceType = newType;
            }
            else
            {
#ifdef _WIN32
                // For WIN32 apps, ensure we have a visible console window for errors too
                ensureConsoleVisible();
#endif
                std::string errorMsg = "Unknown API: " + apiStr + "\n";
                printOutput(errorMsg.c_str());
                m_showHelp = true;
            }
        }
    }

#ifdef _WIN32
    LocalFree(szArglist);
#endif

    return 0;
}

void TestBase::printEntrypointHashes(
    int entryPointCount,
    int targetCount,
    ComPtr<slang::IComponentType>& composedProgram)
{
    for (int targetIndex = 0; targetIndex < targetCount; targetIndex++)
    {
        for (int entryPointIndex = 0; entryPointIndex < entryPointCount; entryPointIndex++)
        {
            ComPtr<slang::IBlob> entryPointHashBlob;
            composedProgram->getEntryPointHash(
                entryPointIndex,
                targetIndex,
                entryPointHashBlob.writeRef());

            Slang::StringBuilder strBuilder;
            strBuilder << "callIdx: " << m_globalCounter << ", entrypoint: " << entryPointIndex
                       << ", target: " << targetIndex << ", hash: ";
            m_globalCounter++;

            uint8_t* buffer = (uint8_t*)entryPointHashBlob->getBufferPointer();
            for (size_t i = 0; i < entryPointHashBlob->getBufferSize(); i++)
            {
                strBuilder << Slang::StringUtil::makeStringWithFormat("%.2X", buffer[i]);
            }
            fprintf(stdout, "%s\n", strBuilder.begin());
        }
    }
}

void TestBase::printUsage(const char* programName) const
{
#ifdef _WIN32
    // For WIN32 apps, ensure we have a visible console window
    bool createdNewWindow = ensureConsoleVisible();
#endif

    // Use printOutput to ensure output appears in both console and debug output (for WIN32 apps)
    std::string usage = "Usage: " + std::string(programName) + " [options]\n\n";
    printOutput(usage.c_str());
    printOutput("Options:\n");
    printOutput(" -h, --help                     Show this help message\n");
    printOutput(" -api (");
#ifdef _WIN32
    printOutput("d3d11|d3d12|");
#endif
    printOutput("vulkan");
#ifdef __APPLE__
    printOutput("|metal");
#endif
    printOutput("|cpu|cuda|webgpu)\n");
#ifdef _WIN32
    printOutput("                                Use a given rendering API (Default: d3d12)\n");
#elif defined(__APPLE__)
    printOutput("                                Use a given rendering API (Default: metal)\n");
#else
    printOutput("                                Use a given rendering API (Default: vulkan)\n");
#endif
    printOutput(" -test-mode                     Print hash values of compiled shader entry points "
                "and skip rendering\n");
    printOutput("\n");
    printOutput("Supported APIs:\n");
#ifdef _WIN32
    printOutput("  d3d11    - Direct3D 11\n");
    printOutput("  d3d12    - Direct3D 12\n");
#endif
    printOutput("  vulkan   - Vulkan\n");
#ifdef __APPLE__
    printOutput("  metal    - Metal (macOS/iOS)\n");
#endif
    printOutput("  cpu      - CPU execution\n");
    printOutput("  cuda     - CUDA\n");
    printOutput("  webgpu   - WebGPU\n");

#ifdef _WIN32
    // For WIN32 apps, only prompt if we created a new console window
    if (createdNewWindow)
    {
        printOutput("\nPress Enter to continue or close this window...\n");
        getchar(); // Wait for user input
    }
#endif
}
