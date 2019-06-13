#include "slang-unix-cpp-compiler-util.h"

#include "../slang-common.h"
#include "../slang-process-util.h"
#include "../slang-string-util.h"

#include "../slang-shared-library.h"

#include "../slang-io.h"

// The method used to invoke VS was originally inspired by some ideas in
// https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus/

namespace Slang {


/* static */void UnixCPPCompilerUtil::calcArgs(const CPPCompileOptions& options, CommandLine& cmdLine)
{
    typedef CPPCompileOptions::OptimizationLevel OptimizationLevel;
    typedef CPPCompileOptions::TargetType TargetType;
    typedef CPPCompileOptions::DebugInfoType DebugInfoType;

    cmdLine.addArg("-fvisibility=hidden");
    // Use shared libraries
    //cmdLine.addArg("-shared");

    switch (options.optimizationLevel)
    {
        case OptimizationLevel::Debug:
        {
            // No optimization
            cmdLine.addArg("-O0");
            break;
        }
        case OptimizationLevel::Normal:
        {
            cmdLine.addArg("-Os");
            break;
        }
        default: break;
    }

    if (options.debugInfoType != DebugInfoType::None)
    {
        cmdLine.addArg("-g");
    }

    switch (options.targetType)
    {
        case TargetType::SharedLibrary:
        {
            // Position independent
            cmdLine.addArg("-fPIC");

            String sharedLibraryPath;

            // Work out the shared library name
            {
                String moduleDir = Path::getParentDirectory(options.modulePath);
                String moduleFilename = Path::getFileName(options.modulePath);

                StringBuilder sharedLibraryFilename;
                SharedLibrary::appendPlatformFileName(moduleFilename.getUnownedSlice(), sharedLibraryFilename);

                if (moduleDir.getLength() > 0)
                {
                    sharedLibraryPath = Path::combine(moduleDir, sharedLibraryFilename);
                }
                else
                {
                    sharedLibraryPath = sharedLibraryFilename;
                }
            }

            cmdLine.addArg("-o");
            cmdLine.addArg(sharedLibraryPath);
            break;
        }
        case TargetType::Executable:
        {
            cmdLine.addArg("-o");

            StringBuilder builder;
            builder << options.modulePath;
            builder << ProcessUtil::getExecutableSuffix();
            
            cmdLine.addArg(options.modulePath);
            break;
        }
        case TargetType::Object:
        {
            // Don't link, just produce object file
            cmdLine.addArg("-c");
            break;
        }
        default: break;
    }

    // Add defines
    for (const auto& define : options.defines)
    {
        StringBuilder builder;
        builder << define.nameWithSig;
        if (define.value.getLength())
        {
            builder << "=" << define.value;
        }

        cmdLine.addArg(builder);
    }

    // Add includes
    for (const auto& include : options.includePaths)
    {
        cmdLine.addArg("-I");
        cmdLine.addArg(include);
    }

    // Link options
    if (0)
    {
        StringBuilder linkOptions;
        linkOptions << "Wl,";
        cmdLine.addArg(linkOptions);
    }

    // Files to compile
    for (const auto& sourceFile : options.sourceFiles)
    {
        cmdLine.addArg(sourceFile);
    }

    for (const auto& libPath : options.libraryPaths)
    {
        // Note that any escaping of the path is handled in the ProcessUtil::
        cmdLine.addArg("-L");
        cmdLine.addArg(libPath);
        cmdLine.addArg("-F");
        cmdLine.addArg(libPath);
    }
}

/* static */SlangResult UnixCPPCompilerUtil::executeCompiler(const CommandLine& commandLine, ExecuteResult& outResult)
{
    CommandLine cmdLine;
    // We'll assume g++ for now
    cmdLine.setExecutableFilename("g++");

    // Append the command line options
    cmdLine.addArgs(commandLine.m_args.getBuffer(), commandLine.m_args.getCount());

#if 0
    // Test
    {
        String line = ProcessUtil::getCommandLineString(cmdLine);
        printf("%s", line.getBuffer());
    }
#endif

    return ProcessUtil::execute(cmdLine, outResult);
}

} // namespace Slang
