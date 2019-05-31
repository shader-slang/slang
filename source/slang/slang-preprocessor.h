// Preprocessor.h
#ifndef SLANG_PREPROCESSOR_H_INCLUDED
#define SLANG_PREPROCESSOR_H_INCLUDED

#include "../core/slang-basic.h"
#include "../slang/slang-lexer.h"

namespace Slang {

class DiagnosticSink;
class Linkage;
class Module;
class ModuleDecl;

// Callback interface for the preprocessor to use when looking
// for files in `#include` directives.
struct IncludeHandler
{
    
    virtual SlangResult findFile(const String& pathToInclude,
        const String& pathIncludedFrom,
        PathInfo& pathInfoOut) = 0;

    virtual String simplifyPath(const String& path) = 0;
};

// Take a string of source code and preprocess it into a list of tokens.
TokenList preprocessSource(
    SourceFile*                 file,
    DiagnosticSink*             sink,
    IncludeHandler*             includeHandler,
    Dictionary<String, String>  defines,
    Linkage*                    linkage,
    Module*                     parentModule);

} // namespace Slang

#endif
