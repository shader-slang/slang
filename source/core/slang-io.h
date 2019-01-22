#ifndef CORE_LIB_IO_H
#define CORE_LIB_IO_H

#include "slang-string.h"
#include "stream.h"
#include "text-io.h"
#include "secure-crt.h"

namespace Slang
{
	class File
	{
	public:
		static bool Exists(const Slang::String & fileName);
		static Slang::String ReadAllText(const Slang::String & fileName);
		static Slang::List<unsigned char> ReadAllBytes(const Slang::String & fileName);
		static void WriteAllText(const Slang::String & fileName, const Slang::String & text);
	};

	class Path
	{
	public:
		static const char PathDelimiter = '/';

		static String TruncateExt(const String & path);
		static String ReplaceExt(const String & path, const char * newExt);
		static String GetFileName(const String & path);
		static String GetFileNameWithoutEXT(const String & path);
		static String GetFileExt(const String & path);
		static String GetDirectoryName(const String & path);
		static String Combine(const String & path1, const String & path2);
		static String Combine(const String & path1, const String & path2, const String & path3);
		static bool CreateDir(const String & path);

            /// Accept either style of delimiter
        SLANG_FORCE_INLINE static bool IsDelimiter(char c) { return c == '/' || c == '\\'; }

        static bool IsDriveSpecification(const UnownedStringSlice& element);

            /// Splits the path into it's individual bits
        static void Split(const UnownedStringSlice& path, List<UnownedStringSlice>& splitOut);
            /// Strips .. and . as much as it can 
        static String Simplify(const UnownedStringSlice& path);
        static String Simplify(const String& path) { return Simplify(path.getUnownedSlice()); }

            /// Returns true if a path contains a . or ..
        static bool IsRelative(const UnownedStringSlice& path);
        static bool IsRelative(const String& path) { return IsRelative(path.getUnownedSlice()); }

        static SlangResult GetPathType(const String & path, SlangPathType* pathTypeOut);

        static SlangResult GetCanonical(const String & path, String& canonicalPathOut);
	};
}

#endif
