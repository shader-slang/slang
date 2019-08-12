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
    static SlangResult calcArgs(const CompileOptions& options, CommandLine& cmdLine);
        /// Parse Visual Studio exeRes into CPPCompiler::Output
    static SlangResult parseOutput(const ExecuteResult& exeRes, CPPCompiler::Output& outOutput);

    static SlangResult calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath);

};

class VisualStudioCPPCompiler : public CommandLineCPPCompiler
{
public:
    typedef CommandLineCPPCompiler Super;

    virtual SlangResult calcArgs(const CompileOptions& options, CommandLine& cmdLine) SLANG_OVERRIDE
    {
        return VisualStudioCompilerUtil::calcArgs(options, cmdLine);
    }
    virtual SlangResult parseOutput(const ExecuteResult& exeResult, Output& output) SLANG_OVERRIDE
    {
        return VisualStudioCompilerUtil::parseOutput(exeResult, output);
    }
    virtual SlangResult calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath) SLANG_OVERRIDE
    {
        return VisualStudioCompilerUtil::calcModuleFilePath(options, outPath);
    }
    VisualStudioCPPCompiler(const Desc& desc):Super(desc) {}
};


}

#endif
