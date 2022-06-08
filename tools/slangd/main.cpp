// main.cpp

// This file implements the entry point for `slangd`, the daemon process of Slang's language server.

#include <thread>

#include "../../source/core/slang-basic.h"
#include "../../source/slang/slang-language-server.h"

int main(int argc, const char* const* argv)
{
    bool isDebug = false;
    for (auto i = 1; i < argc; i++)
    {
        if (Slang::UnownedStringSlice(argv[i]) == "--debug")
        {
            isDebug = true;
        }
    }
    if (isDebug)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    return Slang::runLanguageServer();
}
