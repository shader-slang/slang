#ifndef SLANG_VISUAL_STUDIO_COMPILER_UTIL_H
#define SLANG_VISUAL_STUDIO_COMPILER_UTIL_H

#include "slang-downstream-compiler.h"

namespace Slang
{


struct VisualStudioCompilerUtil : public DownstreamCompilerBaseUtil
{
        /// Calculate Visual Studio family compilers cmdLine arguments from options
    static SlangResult calcArgs(const CompileOptions& options, CommandLine& cmdLine);
        /// Parse Visual Studio exeRes into CPPCompiler::Output
    static SlangResult parseOutput(const ExecuteResult& exeRes, DownstreamDiagnostics& outOutput);

    static SlangResult calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath);

    static SlangResult calcCompileProducts(const CompileOptions& options, ProductFlags flags, List<String>& outPaths);

    static SlangResult locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set);
};

class VisualStudioDownstreamCompiler : public CommandLineDownstreamCompiler
{
public:
    typedef CommandLineDownstreamCompiler Super;
    typedef VisualStudioCompilerUtil Util;

    // CommandLineDownstreamCompiler impl  - just forwards to the Util
    virtual SlangResult calcArgs(const CompileOptions& options, CommandLine& cmdLine) SLANG_OVERRIDE { return Util::calcArgs(options, cmdLine); }
    virtual SlangResult parseOutput(const ExecuteResult& exeResult, DownstreamDiagnostics& output) SLANG_OVERRIDE { return Util::parseOutput(exeResult, output); }
    virtual SlangResult calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath) SLANG_OVERRIDE { return Util::calcModuleFilePath(options, outPath); }
    virtual SlangResult calcCompileProducts(const CompileOptions& options, ProductFlags productFlags, List<String>& outPaths) SLANG_OVERRIDE { return Util::calcCompileProducts(options, productFlags, outPaths); }

    VisualStudioDownstreamCompiler(const Desc& desc):Super(desc) {}
};


}

#endif
