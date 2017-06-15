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
#ifdef _WIN32
		static const char PathDelimiter = '\\';
#else
		static const char PathDelimiter = '/';
#endif
		static String TruncateExt(const String & path);
		static String ReplaceExt(const String & path, const char * newExt);
		static String GetFileName(const String & path);
		static String GetFileNameWithoutEXT(const String & path);
		static String GetFileExt(const String & path);
		static String GetDirectoryName(const String & path);
		static String Combine(const String & path1, const String & path2);
		static String Combine(const String & path1, const String & path2, const String & path3);
		static bool CreateDir(const String & path);
	};
}

#endif