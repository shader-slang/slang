#ifndef SLANG_OS_H
#define SLANG_OS_H

// os.h

#include "../../source/core/slang-io.h"

class CombinePathVisitor : public Slang::Path::Visitor
{
public:
    virtual void accept(Slang::Path::Type type, const Slang::UnownedStringSlice& filename) SLANG_OVERRIDE
    {
        using namespace Slang;
        const Path::TypeFlags flags = Path::TypeFlags(1) << int(type);
        if (flags & m_allowedFlags)
        {
            m_paths.add(Path::combine(m_directoryPath, filename));    
        }
    }

        /// Ctor
    CombinePathVisitor(const Slang::String& directoryPath, Slang::Path::TypeFlags allowedFlags):
        m_directoryPath(directoryPath),
        m_allowedFlags(allowedFlags)
    {
    }

    Slang::List<Slang::String> m_paths;

protected:

    Slang::Path::TypeFlags m_allowedFlags;
    Slang::String m_directoryPath;
};

/* A helper class for holding results of a find. Allows for easy iteration via begin/end */
class FindFilesUtil
{
public:
    
        /// Enumerate subdirectories in the given `directoryPath`, storing in outPaths.
        /// @return SLANG_OK or SLANG_E_NOT_FOUND if directory is not found.
    static SlangResult findChildDirectories(const Slang::String& directoryPath, Slang::List<Slang::String>& outPaths);

        /// Enumerate files in the given `directoryPath` that match the provided
        /// `pattern` as a simplified regex for files to return (e.g., "*.txt")
        /// @return SLANG_OK or SLANG_E_NOT_FOUND if directory is not found.
    static SlangResult findFilesInDirectoryMatchingPattern(const Slang::String& directoryPath, const char* pattern, Slang::List<Slang::String>& outPaths);

        /// Enumerate files in the given `directoryPath`, storing in outPaths.
        /// @return SLANG_OK or SLANG_E_NOT_FOUND if directory is not found.
    static SlangResult findFilesInDirectory(const Slang::String& directoryPath, Slang::List<Slang::String>& outPaths);
};

#endif // SLANG_OS_H
