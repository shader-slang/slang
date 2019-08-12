#ifndef SLANG_VISUAL_STUDIO_COMPILER_UTIL_H
#define SLANG_VISUAL_STUDIO_COMPILER_UTIL_H

#include "slang-cpp-compiler.h"

namespace Slang
{


struct VisualStudioCompilerUtil : public CPPCompilerBaseUtil
{
        /// Calculate Visual Studio family compilers cmdLine arguments from options
    static SlangResult calcArgs(const CompileOptions& options, CommandLine& cmdLine);
        /// Parse Visual Studio exeRes into CPPCompiler::Output
    static SlangResult parseOutput(const ExecuteResult& exeRes, CPPCompiler::Output& outOutput);

    static SlangResult calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath);

    static SlangResult calcCompileProducts(const CompileOptions& options, ProductFlags flags, List<String>& outPaths);
};

class VisualStudioCPPCompiler : public CommandLineCPPCompiler
{
public:
    typedef CommandLineCPPCompiler Super;
    typedef VisualStudioCompilerUtil Util;

    // CommandLineCPPCompiler impl  - just forwards to the Util
    virtual SlangResult calcArgs(const CompileOptions& options, CommandLine& cmdLine) SLANG_OVERRIDE { return Util::calcArgs(options, cmdLine); }
    virtual SlangResult parseOutput(const ExecuteResult& exeResult, Output& output) SLANG_OVERRIDE { return Util::parseOutput(exeResult, output); }
    virtual SlangResult calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath) SLANG_OVERRIDE { return Util::calcModuleFilePath(options, outPath); }
    virtual SlangResult calcCompileProducts(const CompileOptions& options, ProductFlags productFlags, List<String>& outPaths) SLANG_OVERRIDE { return Util::calcCompileProducts(options, productFlags, outPaths); }

    VisualStudioCPPCompiler(const Desc& desc):Super(desc) {}
};


}

#endif
