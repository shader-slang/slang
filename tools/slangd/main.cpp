// main.cpp

// This file implements the entry point for `slangd`, the daemon process of Slang's language server.

#include "../../source/core/slang-basic.h"
#include "../../source/slang/slang-language-server.h"

#include <thread>

int main(int argc, const char* const* argv)
{
    bool isDebug = false;
    for (auto i = 1; i < argc; i++)
    {
        if (Slang::UnownedStringSlice(argv[i]) == "--debug")
        {
            isDebug = true;
        }
        else if (Slang::UnownedStringSlice(argv[i]) == "--print-builtin-module" && i < argc - 1)
        {
            Slang::UnownedStringSlice moduleName = Slang::UnownedStringSlice(argv[++i]);
            Slang::ComPtr<slang::IBlob> code;
            Slang::getBuiltinModuleSource(moduleName, code.writeRef());
            printf("%s\n", (const char*)code->getBufferPointer());
            return 0;
        }
    }
    if (isDebug)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    Slang::LanguageServerStartupOptions options;
    options.parse(argc, argv);
    auto result = Slang::runLanguageServer(options);
    slang::shutdown();
    return result;
}
