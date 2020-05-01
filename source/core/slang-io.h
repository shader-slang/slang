#ifndef SLANG_CORE_IO_H
#define SLANG_CORE_IO_H

#include "slang-string.h"
#include "slang-stream.h"
#include "slang-text-io.h"
#include "slang-secure-crt.h"

namespace Slang
{
	class File
	{
	public:
		static bool exists(const String& fileName);
		static String readAllText(const String& fileName);
		static List<unsigned char> readAllBytes(const String& fileName);
		static void writeAllText(const String& fileName, const String& text);
        static SlangResult remove(const String& fileName);

        static SlangResult makeExecutable(const String& fileName);

        static SlangResult generateTemporary(const UnownedStringSlice& prefix, String& outFileName);
	};

	class Path
	{
	public:
		static const char kPathDelimiter = '/';


            /// Returns -1 if no separator is found
        static Index findLastSeparatorIndex(String const& path);
            /// Finds the index of the last dot in a path, else returns -1
        static Index findExtIndex(String const& path);

		static String replaceExt(const String& path, const char* newExt);
		static String getFileName(const String& path);
		static String getPathWithoutExt(const String& path);
		static String getPathExt(const String& path);
		static String getParentDirectory(const String& path);

        static String getFileNameWithoutExt(const String& path);

		static String combine(const String& path1, const String& path2);
		static String combine(const String& path1, const String& path2, const String& path3);

            /// Combine path sections and store the result in outBuilder
        static void combineIntoBuilder(const UnownedStringSlice& path1, const UnownedStringSlice& path2, StringBuilder& outBuilder);

            /// Append a path, taking into account path separators onto the end of ioBuilder 
        static void append(StringBuilder& ioBuilder, const UnownedStringSlice& path);

		static bool createDirectory(const String& path);

            /// Accept either style of delimiter
        SLANG_FORCE_INLINE static bool isDelimiter(char c) { return c == '/' || c == '\\'; }

            /// True if the element appears to be a drive specification (where element is the prefix to a path that isn't a directory)
            /// @param pathPrefix The path prefix to test if it's a drive specification
        static bool isDriveSpecification(const UnownedStringSlice& pathPrefix);

            /// Splits the path into it's individual bits
        static void split(const UnownedStringSlice& path, List<UnownedStringSlice>& splitOut);
            /// Strips .. and . as much as it can 
        static String simplify(const UnownedStringSlice& path);
        static String simplify(const String& path) { return simplify(path.getUnownedSlice()); }

            /// Returns true if the path is absolute
        static bool isAbsolute(const UnownedStringSlice& path);
        static bool isAbsolute(const String& path) { return isAbsolute(path.getUnownedSlice()); }

            /// Returns true if path contains contains an element of . or ..
        static bool hasRelativeElement(const UnownedStringSlice& path);
        static bool hasRelativeElement(const String& path) { return hasRelativeElement(path.getUnownedSlice()); }

            /// Determines the type of file at the path
            /// @param path The path to test
            /// @param outPathType Holds the object type at the path on success
            /// @return SLANG_OK on success
        static SlangResult getPathType(const String& path, SlangPathType* outPathType);

            /// Determines the canonical equivalent path to path.
            /// The path returned should reference the identical object - and two different references to the same path should return the same canonical path
            /// @param path Path to get the canonical path for
            /// @param outCanonicalPath The canonical path for 'path' is call is successful
            /// @return SLANG_OK on success
        static SlangResult getCanonical(const String& path, String& outCanonicalPath);

            /// Returns the executable path
            /// @return The path in platform native format. Returns empty string if failed.
        static String getExecutablePath();

            /// Returns the first element of the path or an empty slice if there is none
            /// This broadly equivalent to returning the first element of split
            /// @param path Path to extract first element from
            /// @return The first element of the path, or empty 
        static UnownedStringSlice getFirstElement(const UnownedStringSlice& path);
	};

    // Helper class to clean up temporary files on dtor
    class TemporaryFileSet: public RefObject
    {
    public:
        typedef RefObject Super;
        typedef TemporaryFileSet ThisType;

        void remove(const String& path)
        {
            if (const Index index = m_paths.indexOf(path) >= 0)
            {
                m_paths.removeAt(index);
            }
        }

        void add(const String& path)
        {
            if (m_paths.indexOf(path) < 0)
            {
                m_paths.add(path);
            }
        }
        void add(const List<String>& paths)
        {
            for (const auto& path : paths)
            {
                add(path);
            }
        }
        void clear()
        {
            m_paths.clear();
        }

        void swapWith(ThisType& rhs)
        {
            m_paths.swapWith(rhs.m_paths);
        }

        ~TemporaryFileSet()
        {
            for (const auto& path : m_paths)
            {
                File::remove(path);
            }
        }
            /// Default Ctor
        TemporaryFileSet() {}

        List<String> m_paths;
    
    private:
        // Disable ctor/assignment
        TemporaryFileSet(const ThisType& rhs) = delete;
        void operator=(const ThisType& rhs) = delete;
    };
}

#endif
