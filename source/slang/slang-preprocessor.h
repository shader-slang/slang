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

// Callback interface for the preprocessor to use when looking
// for files in `#include` directives.
struct IncludeHandler
{
    virtual SlangResult findFile(const String& pathToInclude, const String& pathIncludedFrom, PathInfo& outPathInfo) = 0;
    virtual String simplifyPath(const String& path) = 0;
};

// A default implementation that uses IncludeSystem to implement functionality
struct IncludeHandlerImpl : IncludeHandler
{
    virtual SlangResult findFile(const String& pathToInclude, const String& pathIncludedFrom, PathInfo& outPathInfo) override
    {
        return m_system.findFile(pathToInclude, pathIncludedFrom, outPathInfo);
    }
    virtual String simplifyPath(const String& path) override { return m_system.simplifyPath(path); }

    IncludeHandlerImpl(SearchDirectoryList* searchDirectories, ISlangFileSystemExt* fileSystemExt, SourceManager* sourceManager = nullptr) :
        m_system(searchDirectories, fileSystemExt, sourceManager)
    {
    }
protected:
    IncludeSystem m_system;
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
