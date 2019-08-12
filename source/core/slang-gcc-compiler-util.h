#ifndef SLANG_GCC_COMPILER_UTIL_H
#define SLANG_GCC_COMPILER_UTIL_H

#include "slang-cpp-compiler.h"

namespace Slang
{

/* Utility for processing input and output of gcc-like compilers, including clang */
struct GCCCompilerUtil : public CPPCompilerBaseUtil
{
        /// Extracts version number into desc from text (assumes gcc/clang -v layout with a line with version)
    static SlangResult parseVersion(const UnownedStringSlice& text, const UnownedStringSlice& prefix, CPPCompiler::Desc& outDesc);

        /// Runs the exeName, and extracts the version info into outDesc
    static SlangResult calcVersion(const String& exeName, CPPCompiler::Desc& outDesc);

        /// Calculate gcc family compilers (including clang) cmdLine arguments from options
    static SlangResult calcArgs(const CompileOptions& options, CommandLine& cmdLine);

        /// Parse ExecuteResult into Output
    static SlangResult parseOutput(const ExecuteResult& exeRes, CPPCompiler::Output& outOutput);

        /// Calculate the output module filename 
    static SlangResult calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath);

        /// Given options, calculate paths to products produced for a compilation
    static SlangResult calcCompileProducts(const CompileOptions& options, ProductFlags flags, List<String>& outPaths);
};

class GCCCPPCompiler : public CommandLineCPPCompiler
{
public:
    typedef CommandLineCPPCompiler Super;
    typedef GCCCompilerUtil Util;

    // CommandLineCPPCompiler impl  - just forwards to the Util
    virtual SlangResult calcArgs(const CompileOptions& options, CommandLine& cmdLine) SLANG_OVERRIDE { return Util::calcArgs(options, cmdLine); }
    virtual SlangResult parseOutput(const ExecuteResult& exeResult, Output& output) SLANG_OVERRIDE { return Util::parseOutput(exeResult, output); }
    virtual SlangResult calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath) SLANG_OVERRIDE { return Util::calcModuleFilePath(options, outPath); }
    virtual SlangResult calcCompileProducts(const CompileOptions& options, ProductFlags flags,  List<String>& outPaths) SLANG_OVERRIDE { return Util::calcCompileProducts(options, flags, outPaths); }

    GCCCPPCompiler(const Desc& desc):Super(desc) {}
};

}

#endif
