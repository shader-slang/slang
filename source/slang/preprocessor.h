// Preprocessor.h
#ifndef SLANG_PREPROCESSOR_H_INCLUDED
#define SLANG_PREPROCESSOR_H_INCLUDED

#include "../core/basic.h"
#include "../slang/lexer.h"

namespace Slang{ namespace Compiler {

class DiagnosticSink;
class ProgramSyntaxNode;

// Callback interface for the preprocessor to use when looking
// for files in `#include` directives.
struct IncludeHandler
{
    virtual bool TryToFindIncludeFile(
        CoreLib::String const& pathToInclude,
        CoreLib::String const& pathIncludedFrom,
        CoreLib::String* outFoundPath,
        CoreLib::String* outFoundSource) = 0;
};

// Take a string of source code and preprocess it into a list of tokens.
TokenList preprocessSource(
    CoreLib::String const& source,
    CoreLib::String const& fileName,
    DiagnosticSink* sink,
    IncludeHandler* includeHandler,
    CoreLib::Dictionary<CoreLib::String, CoreLib::String>  defines,
    ProgramSyntaxNode*  syntax);

}}

#endif
