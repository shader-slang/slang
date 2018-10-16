// Preprocessor.h
#ifndef SLANG_PREPROCESSOR_H_INCLUDED
#define SLANG_PREPROCESSOR_H_INCLUDED

#include "../core/basic.h"
#include "../slang/lexer.h"

namespace Slang {

class DiagnosticSink;
class ModuleDecl;
class TranslationUnitRequest;

// Callback interface for the preprocessor to use when looking
// for files in `#include` directives.
struct IncludeHandler
{
    
    virtual SlangResult findFile(const String& pathToInclude,
        const String& pathIncludedFrom,
        PathInfo& pathInfoOut) = 0;

    virtual SlangResult readFile(const String& path, 
        ISlangBlob** blobOut) = 0;        
};

// Take a string of source code and preprocess it into a list of tokens.
TokenList preprocessSource(
    SourceFile*                 file,
    DiagnosticSink*             sink,
    IncludeHandler*             includeHandler,
    Dictionary<String, String>  defines,
    TranslationUnitRequest*     translationUnit);

} // namespace Slang

#endif
