// Preprocessor.h
#ifndef SLANG_PREPROCESSOR_H_INCLUDED
#define SLANG_PREPROCESSOR_H_INCLUDED

#include "../core/slang-basic.h"

#include "slang-lexer.h"

#include "slang-include-system.h"

namespace Slang {

class DiagnosticSink;
class Linkage;
class Module;
class ModuleDecl;

// Take a string of source code and preprocess it into a list of tokens.
TokenList preprocessSource(
    SourceFile*                 file,
    DiagnosticSink*             sink,
    IncludeSystem*              includeSystem,
    Dictionary<String, String>  defines,
    Linkage*                    linkage,
    Module*                     parentModule);

} // namespace Slang

#endif
