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

/* Holds the platform specific mutable state of a find files operation.
NOTE! That FindFiles can find files and/or directories */
class FindFilesState : public Slang::RefObject
{
public:
        /// Moves to the next thing found. If nothing more is found will return 0, and hasResult will return false.
        /// findNext will return false if hasResult is false.
        /// To put it another way if findNext returns true, then hasResult *must* return true, and getPath/getFoundType are valid.
    virtual bool findNext() = 0;
        /// True if holds a result. If true, getPath/getFoundType are valid, otherwise they are undefined
    virtual bool hasResult() = 0;

        /// Starts a find. 
        /// The pattern can *only* match the files/directories in the specified directory. Pass '*' to match everything.
        /// NOTE! The specifics of the pattern matching is currently target specific.
        /// The allowedTypes flags controls what types (files and/or directories) should be searched for.
        /// NOTE! Just because startFind returns with a success code, does *not* mean the FindFileState has a result - this will happen with an empty directory.
        /// @return is SLANG_E_NOT_FOUND if the directoryPath is not found
    virtual SlangResult startFind(const Slang::String& directoryPath, const Slang::String& pattern, FindTypeFlags allowedTypes) = 0;

        /// Get the path of the thing found.
    const Slang::String& getPath() const { return m_foundPath; }

        /// Get the type the path relates to
    FindType getFoundType() const { return m_foundType; }

        /// Create a find files state. Can only be used after an 'start'. Note that a start function can be called
        /// on a FindFilesState in any state.
        /// Also start method return error codes which can be useful.
    static Slang::RefPtr<FindFilesState> create();

protected:

    Slang::String m_directoryPath;                  ///< The directory being searched
    Slang::String m_foundPath;                      ///< The path to what has been found (if anything)
    FindType m_foundType = FindType::Unknown;       ///< The type of thing at the foundPath
    FindTypeFlags m_allowedTypes = 0;               ///< Set of all types allowed
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
        // for( auto subdir : FindFilesResult::findChildDirectories(dir))
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
        // for( auto file : FindFilesResult::findFilesInDirectoryMatchingPattern(dir, "*.txt"))
        // { ... }
        //
        // Each element in the range is a `Slang::String` representing the
        // path to a file in the directory.
    static FindFilesResult findFilesInDirectoryMatchingPattern(const Slang::String& directoryPath, const Slang::String& pattern);

        // Enumerate files in the given `directoryPath`  and return a logical
        // collection of the results that can be iterated with a range-based
        // `for` loop:
        //
        // for( auto file : FindFilesResult::findFilesInDirectory(dir))
        // { ... }
        //
        // Each element in the range is a `Slang::String` representing the
        // path to a file in the directory.
    static FindFilesResult findFilesInDirectory(const Slang::String& directoryPath);
    
protected:
    Slang::RefPtr<FindFilesState> m_state;
};

#endif // SLANG_OS_H
