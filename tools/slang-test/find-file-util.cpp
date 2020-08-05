// find-file-util.cpp
#include "find-file-util.h"

#include "slang-com-helper.h"

using namespace Slang;

/* static */SlangResult FindFileUtil::findChildDirectories(const Slang::String& directoryPath, Slang::List<Slang::String>& outPaths)
{
    outPaths.clear();
    CombinePathVisitor visitor(directoryPath, Path::TypeFlag::Directory);
    SLANG_RETURN_ON_FAIL(Path::find(directoryPath, nullptr, &visitor));
    outPaths.swapWith(visitor.m_paths);
    return SLANG_OK;
}

/* static */SlangResult FindFileUtil::findFilesInDirectoryMatchingPattern(const Slang::String& directoryPath, const char* pattern, Slang::List<Slang::String>& outPaths)
{
    outPaths.clear();
    CombinePathVisitor visitor(directoryPath, Path::TypeFlag::File);
    SLANG_RETURN_ON_FAIL(Path::find(directoryPath, pattern, &visitor));
    outPaths.swapWith(visitor.m_paths);
    return SLANG_OK;
}

/* static */SlangResult FindFileUtil::findFilesInDirectory(const Slang::String& directoryPath, Slang::List<Slang::String>& outPaths)
{
    return findFilesInDirectoryMatchingPattern(directoryPath, nullptr, outPaths);
}
