#ifndef SLANG_OS_H
#define SLANG_OS_H

// os.h

#include "../../source/core/slang-io.h"

enum class FindType
{
    Unknown,
    File,
    Directory,
};

typedef uint32_t FindTypeFlags;
struct FindTypeFlag
{
    enum Enum : FindTypeFlags
    {
        File = FindTypeFlags(1) << int(FindType::File),
        Directory = FindTypeFlags(1) << int(FindType::Directory),
    };
};

/* Holds the platform specific mutable state of a find files operation */
class FindFilesState : public Slang::RefObject
{
public:
    virtual bool findNext() = 0;
    virtual bool hasResult() = 0;

    virtual SlangResult startFind(const Slang::String& directoryPath, const Slang::String& pattern, FindTypeFlags allowedTypes) = 0;

        /// Create a find files state. Can only be used after an 'start'. Note that a start function can be called
        /// on a FindFilesState in any state.
        /// Also start methods return error codes which can be useful.
    static Slang::RefPtr<FindFilesState> create();

        /// Get the current path. Only valid if hasResult is true.
    const Slang::String& getPath() const { return m_filePath; }

protected:

    Slang::String m_directoryPath;
    Slang::String m_filePath;
    FindType m_findType = FindType::Unknown;
    FindTypeFlags m_allowedTypes = 0;
};

/* A helper class for holding results of a find. Allows for easy iteration via begin/end */
class FindFilesResult
{
public:
    struct Iterator
    {
        typedef Iterator ThisType;

        /// True if there is either no result, or no results in the result
        bool atEnd() const { return m_state == nullptr || !m_state->hasResult(); }

        /// Equality is only for testing if at the end
        bool operator==(const ThisType& other) const { return atEnd() == other.atEnd(); }
        bool operator!=(const ThisType& other) const { return !(*this == other); }

        void operator++()
        {
            if (!m_state->findNext())
            {
                m_state.setNull();
            }
        }
        const Slang::String& operator*() const
        {
            SLANG_ASSERT(m_state);
            return m_state->getPath();
        }

        Iterator(FindFilesState* state) : m_state(state) {}
    protected:
        Slang::RefPtr<FindFilesState> m_state;
    };

    Iterator begin() { return Iterator(m_state); }
    Iterator end() { return Iterator(nullptr); }

    FindFilesResult(FindFilesState* state):
        m_state(state)
    {
    }

        // Enumerate subdirectories in the given `directoryPath` and return a logical
        // collection of the results that can be iterated with a range-based
        // `for` loop:
        //
        // for( auto subdir : findChildDirectories(dir))
        // { ... }
        //
        // Each element in the range is a `Slang::String` representing the
        // path to a subdirectory of the directory.
    static FindFilesResult findChildDirectories(const Slang::String& directoryPath);

        // Enumerate files in the given `directoryPath` that match the provided
        // `pattern` as a simplified regex for files to return (e.g., "*.txt")
        // and return a logical collection of the results
        // that can be iterated with a range-based `for` loop:
        //
        // for( auto file : osFindFilesInDirectoryMatchingPattern(dir, "*.txt"))
        // { ... }
        //
        // Each element in the range is a `Slang::String` representing the
        // path to a file in the directory.
    static FindFilesResult findFilesInDirectoryMatchingPattern(const Slang::String& directoryPath, const Slang::String& pattern);

        // Enumerate files in the given `directoryPath`  and return a logical
        // collection of the results that can be iterated with a range-based
        // `for` loop:
        //
        // for( auto file : osFindFilesInDirectory(dir))
        // { ... }
        //
        // Each element in the range is a `Slang::String` representing the
        // path to a file in the directory.
    static FindFilesResult findFilesInDirectory(const Slang::String& directoryPath);
    
protected:
    Slang::RefPtr<FindFilesState> m_state;
};

#endif // SLANG_OS_H
