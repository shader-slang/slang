#ifndef SLANG_VISUAL_STUDIO_COMPILER_UTIL_H
#define SLANG_VISUAL_STUDIO_COMPILER_UTIL_H

#include "slang-cpp-compiler.h"

namespace Slang
{

struct VisualStudioCompilerUtil
{
    typedef CPPCompiler::CompileOptions CompileOptions;
    typedef CPPCompiler::OptimizationLevel OptimizationLevel;
    typedef CPPCompiler::TargetType TargetType;
    typedef CPPCompiler::DebugInfoType DebugInfoType;
    typedef CPPCompiler::SourceType SourceType;
    typedef CPPCompiler::OutputMessage OutputMessage;
    typedef CPPCompiler::FloatingPointMode FloatingPointMode;

        /// Calculate Visual Studio family compilers cmdLine arguments from options
    static void calcArgs(const CompileOptions& options, CommandLine& cmdLine);
        /// Parse Visual Studio exeRes into CPPCompiler::Output
    static SlangResult parseOutput(const ExecuteResult& exeRes, CPPCompiler::Output& outOutput);

    static SlangResult calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath);

};

}

#endif
