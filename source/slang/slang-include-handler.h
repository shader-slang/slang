#ifndef SLANG_INCLUDE_HANDLER_H
#define SLANG_INCLUDE_HANDLER_H
// slang-include-handler.h

#include "slang-source-loc.h"

namespace Slang
{

// A directory to be searched when looking for files (e.g., `#include`)
struct SearchDirectory
{
    SearchDirectory() = default;
    SearchDirectory(SearchDirectory const& other) = default;
    SearchDirectory(String const& path)
        : path(path)
    {}

    String path;
};

/// A list of directories to search for files (e.g., `#include`)
struct SearchDirectoryList
{
    // A parent list that should also be searched
    SearchDirectoryList*    parent = nullptr;

    // Directories to be searched
    List<SearchDirectory>   searchDirectories;
};

struct IncludeSystem
{
    IncludeSystem(SearchDirectoryList* searchDirectories, ISlangFileSystemExt* fileSystemExt, SourceManager* sourceManager = nullptr) :
        m_searchDirectories(searchDirectories),
        m_fileSystemExt(fileSystemExt),
        m_sourceManager(sourceManager)
    {
    }

    SlangResult findFile(const String& pathToInclude, const String& pathIncludedFrom, PathInfo& outPathInfo);
    SlangResult findFile(SlangPathType fromPathType, const String& fromPath, const String& path, PathInfo& outPathInfo);
    String simplifyPath(const String& path);
    SlangResult loadFile(const PathInfo& pathInfo, ComPtr<ISlangBlob>& outBlob);

    SlangResult findAndLoadFile(const String& pathToInclude, const String& pathIncludedFrom, PathInfo& outPathInfo, ComPtr<ISlangBlob>& outBlob);

    SearchDirectoryList* getSearchDirectoryList() const { return m_searchDirectories; }
    ISlangFileSystemExt* getFileSystem() const { return m_fileSystemExt; }
    SourceManager* getSourceManager() const { return m_sourceManager; }

protected:
    
    SearchDirectoryList* m_searchDirectories;
    ISlangFileSystemExt* m_fileSystemExt;
    SourceManager* m_sourceManager;                 ///< If not set, will not look up the content in the source manager
};

// Callback interface for the preprocessor to use when looking
// for files in `#include` directives.
struct IncludeHandler
{
    virtual SlangResult findFile(const String& pathToInclude,
        const String& pathIncludedFrom,
        PathInfo& pathInfoOut) = 0;

    virtual String simplifyPath(const String& path) = 0;
};

struct IncludeHandlerImpl : IncludeHandler
{
    virtual SlangResult findFile(
        String const& pathToInclude,
        String const& pathIncludedFrom,
        PathInfo& pathInfoOut) override;

    virtual String simplifyPath(const String& path) override;

    IncludeHandlerImpl(SearchDirectoryList* searchDirectories, ISlangFileSystemExt* fileSystemExt) :
        m_system(searchDirectories, fileSystemExt)
    {
    }

    IncludeSystem m_system;
};

}

#endif  // SLANG_INCLUDE_HANDLER_H
