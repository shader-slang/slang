#include <vector>
#include <string>
#include <windows.h>
#include <inttypes.h>
#include <dbghelp.h>
#include "common.h"

#define SLANG_EXAMPLE_LOG_ERROR(...)                \
    printf("error: %s: %d: ", __FILE__, __LINE__);  \
    print(__VA_ARGS__);                             \
    printf("\n");

static void print() {}
static void print(unsigned int n)
{
    printf("%u", n);
}


static bool getModuleFileNameAtAddress(DWORD64 const address, std::string& fileName)
{
    HMODULE module = NULL;
    {
        BOOL result =
            GetModuleHandleEx(
                              GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                              (LPCTSTR)address,
                              &module
                              );
        if(result == 0)
        {
            SLANG_EXAMPLE_LOG_ERROR(GetLastError());
            return false;
        }
        if(module == NULL)
        {
            SLANG_EXAMPLE_LOG_ERROR();
            return false;
        }
    }

    std::vector<char> buffer(1U << 8U);
    uint32_t constexpr maxBufferSize = 1U << 20;
    while(buffer.size() < maxBufferSize)
    {
        DWORD result = GetModuleFileNameA(module, buffer.data(), buffer.size());
        if(result == 0)
        {
            SLANG_EXAMPLE_LOG_ERROR(GetLastError());
            return false;
        }
        else if(result == ERROR_INSUFFICIENT_BUFFER)
        {
            buffer.resize(buffer.size() << 1U);
        }
        else
        {
            break;
        }
    }
    if(buffer.size() == maxBufferSize)
    {
        SLANG_EXAMPLE_LOG_ERROR();
        return false;
    }

    fileName = std::string(buffer.data(), buffer.data() + buffer.size());
    return true;
}

// NOTE: This function is not thread-safe, due to usage of StackWalk64 and static buffers.
static bool printStack(HANDLE process, HANDLE thread, CONTEXT const& context)
{
#if defined(_M_AMD64)
    DWORD constexpr machineType = IMAGE_FILE_MACHINE_AMD64;
#else
#error Unsupported machine type
#endif

    static char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];

    // StackWalk64 may modify the context record
    CONTEXT contextCopy;
    memcpy(&contextCopy, &context, sizeof(CONTEXT));

    STACKFRAME64 frame = {};
    constexpr uint32_t maxFrameCount = 1U << 10;
    uint32_t frameIndex = 0U;
    while(frameIndex < maxFrameCount)
    {
        // Use the default routine
        PREAD_PROCESS_MEMORY_ROUTINE64 readMemoryRoutine = NULL;
        // Not sure what this is for, but documentation says most callers can pass NULL
        PTRANSLATE_ADDRESS_ROUTINE64 translateAddressRoutine = NULL;
        {
            BOOL result =
                StackWalk64(
                            machineType,
                            process,
                            thread,
                            &frame,
                            &contextCopy,
                            readMemoryRoutine,
                            SymFunctionTableAccess64,
                            SymGetModuleBase64,
                            translateAddressRoutine
                            );
            if(result == FALSE)
                break;
        }

        PSYMBOL_INFO maybeSymbol = (PSYMBOL_INFO)symbolBuffer;
        {
            maybeSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            maybeSymbol->MaxNameLen = MAX_SYM_NAME;
            DWORD64 address = frame.AddrPC.Offset;
            // Not required, we want to look up the symbol exactly at the address
            PDWORD64 displacement = NULL;
            BOOL result =
                SymFromAddr(process, address, displacement, maybeSymbol);
            if(result == FALSE)
            {
                SLANG_EXAMPLE_LOG_ERROR(GetLastError());
                maybeSymbol = NULL;
            }
        }

        printf("%u", frameIndex);

        std::string moduleFileName;
        if(getModuleFileNameAtAddress(frame.AddrPC.Offset, moduleFileName))
            printf(": %s", moduleFileName.c_str());

        if(maybeSymbol)
        {
            PSYMBOL_INFO& symbol = maybeSymbol;

            IMAGEHLP_LINE64 line = {};
            line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

            DWORD displacement;
            if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &displacement, &line))
            {
                printf(": %s: %s: %lu",
                       symbol->Name, line.FileName, line.LineNumber);
            }
            else
            {
                printf(": %s", symbol->Name);
            }

            printf(": 0x%.16" PRIXPTR, symbol->Address);
        }
        printf("\n");

        frameIndex++;
    }

    return frameIndex < maxFrameCount;
}

int exceptionFilter(_EXCEPTION_POINTERS* exception)
{
    printf("error: Exception 0x%x occurred. Stack trace:\n",
           exception->ExceptionRecord->ExceptionCode);

    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    bool symbolsLoaded = false;
    {
        // The default search paths should suffice
        PCSTR symbolFileSearchPath = NULL;
        BOOL loadSymbolsOfLoadedModules = TRUE;
        BOOL result =
            SymInitialize(process, symbolFileSearchPath, loadSymbolsOfLoadedModules);
        if(result == FALSE)
        {
            printf("warning: Failed to load symbols\n");
        }
        else
        {
            symbolsLoaded = true;
        }
    }

    if(!printStack(process, thread, *exception->ContextRecord))
    {
        printf("warning: Failed to print complete stack trace!\n");
    }

    if(symbolsLoaded)
    {
        BOOL result = SymCleanup(process);
        if(result == FALSE)
        {
            SLANG_EXAMPLE_LOG_ERROR(GetLastError());
        }
    }

    return EXCEPTION_EXECUTE_HANDLER;
}
