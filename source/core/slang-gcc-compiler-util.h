#ifndef SLANG_GCC_COMPILER_UTIL_H
#define SLANG_GCC_COMPILER_UTIL_H

#include "slang-cpp-compiler.h"

namespace Slang
{

/* Utility for processing input and output of gcc-like compilers, including clang */
struct GCCCompilerUtil
{
    typedef CPPCompiler::CompileOptions CompileOptions;
    typedef CPPCompiler::OptimizationLevel OptimizationLevel;
    typedef CPPCompiler::TargetType TargetType;
    typedef CPPCompiler::DebugInfoType DebugInfoType;
    typedef CPPCompiler::SourceType SourceType;
    typedef CPPCompiler::FloatingPointMode FloatingPointMode;

        /// Extracts version number into desc from text (assumes gcc/clang -v layout with a line with version)
    static SlangResult parseVersion(const UnownedStringSlice& text, const UnownedStringSlice& prefix, CPPCompiler::Desc& outDesc);

        /// Runs the exeName, and extracts the version info into outDesc
    static SlangResult calcVersion(const String& exeName, CPPCompiler::Desc& outDesc);

        /// Calculate gcc family compilers (including clang) cmdLine arguments from options
    static void calcArgs(const CompileOptions& options, CommandLine& cmdLine);

        /// Parse ExecuteResult into Output
    static SlangResult parseOutput(const ExecuteResult& exeRes, CPPCompiler::Output& outOutput);

        /// Calculate the output module filename 
    static SlangResult calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath);

};

}

#endif
