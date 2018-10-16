#include "slang-io.h"
#include "exception.h"

#ifndef __STDC__
#   define __STDC__ 1
#endif

#include <sys/stat.h>

#ifdef _WIN32
#   include <direct.h>
#endif

#include <limits.h> /* PATH_MAX */
#include <stdio.h>
#include <stdlib.h>

namespace Slang
{
	bool File::Exists(const String & fileName)
	{
#ifdef _WIN32
		struct _stat32 statVar;
		return ::_wstat32(((String)fileName).ToWString(), &statVar) != -1;
#else
		struct stat statVar;
		return ::stat(fileName.Buffer(), &statVar) == 0;
#endif
	}

	String Path::TruncateExt(const String & path)
	{
		UInt dotPos = path.LastIndexOf('.');
		if (dotPos != -1)
			return path.SubString(0, dotPos);
		else
			return path;
	}
	String Path::ReplaceExt(const String & path, const char * newExt)
	{
		StringBuilder sb(path.Length()+10);
		UInt dotPos = path.LastIndexOf('.');
		if (dotPos == -1)
			dotPos = path.Length();
		sb.Append(path.Buffer(), dotPos);
		sb.Append('.');
		sb.Append(newExt);
		return sb.ProduceString();
	}

    static UInt findLastSeparator(String const& path)
    {
		UInt slashPos = path.LastIndexOf('/');
        UInt backslashPos = path.LastIndexOf('\\');

        if (slashPos == -1) return backslashPos;
        if (backslashPos == -1) return slashPos;

        UInt pos = slashPos;
        if (backslashPos > slashPos)
            pos = backslashPos;

        return pos;
    }

	String Path::GetFileName(const String & path)
	{
        UInt pos = findLastSeparator(path);
        if (pos != -1)
        {
            pos = pos + 1;
            return path.SubString(pos, path.Length() - pos);
        }
        else
        {
            return path;
        }
	}
	String Path::GetFileNameWithoutEXT(const String & path)
	{
        String fileName = GetFileName(path);
		UInt dotPos = fileName.LastIndexOf('.');
		if (dotPos == -1)
            return fileName;
		return fileName.SubString(0, dotPos);
	}
	String Path::GetFileExt(const String & path)
	{
		UInt dotPos = path.LastIndexOf('.');
		if (dotPos != -1)
			return path.SubString(dotPos+1, path.Length()-dotPos-1);
		else
			return "";
	}
	String Path::GetDirectoryName(const String & path)
	{
        UInt pos = findLastSeparator(path);
		if (pos != -1)
			return path.SubString(0, pos);
		else
			return "";
	}
	String Path::Combine(const String & path1, const String & path2)
	{
		if (path1.Length() == 0) return path2;
		StringBuilder sb(path1.Length()+path2.Length()+2);
		sb.Append(path1);
		if (!path1.EndsWith('\\') && !path1.EndsWith('/'))
			sb.Append(PathDelimiter);
		sb.Append(path2);
		return sb.ProduceString();
	}
	String Path::Combine(const String & path1, const String & path2, const String & path3)
	{
		StringBuilder sb(path1.Length()+path2.Length()+path3.Length()+3);
		sb.Append(path1);
		if (!path1.EndsWith('\\') && !path1.EndsWith('/'))
			sb.Append(PathDelimiter);
		sb.Append(path2);
		if (!path2.EndsWith('\\') && !path2.EndsWith('/'))
			sb.Append(PathDelimiter);
		sb.Append(path3);
		return sb.ProduceString();
	}

	bool Path::CreateDir(const String & path)
	{
#if defined(_WIN32)
		return _wmkdir(path.ToWString()) == 0;
#else 
		return mkdir(path.Buffer(), 0777) == 0;
#endif
	}

    /* static */SlangResult Path::GetCanonical(const String & path, String & canonicalPathOut)
    {
#if defined(_WIN32)
        // https://msdn.microsoft.com/en-us/library/506720ff.aspx
        wchar_t* absPath = ::_wfullpath(nullptr, path.ToWString(), 0);
        if (!absPath)
        {
            return SLANG_FAIL;
        }  

        canonicalPathOut =  String::FromWString(absPath);
        ::free(absPath);
        return SLANG_OK;
#else
        // http://man7.org/linux/man-pages/man3/realpath.3.html
        char* canonicalPath = ::realpath(path.begin(), nullptr);
        if (canonicalPath)
        {
            canonicalPathOut = canonicalPath;
            ::free(canonicalPath);
            return SLANG_OK;
        }
        return SLANG_FAIL;
#endif
    }

	Slang::String File::ReadAllText(const Slang::String & fileName)
	{
		StreamReader reader(new FileStream(fileName, FileMode::Open, FileAccess::Read, FileShare::ReadWrite));
		return reader.ReadToEnd();
	}

	Slang::List<unsigned char> File::ReadAllBytes(const Slang::String & fileName)
	{
		RefPtr<FileStream> fs = new FileStream(fileName, FileMode::Open, FileAccess::Read, FileShare::ReadWrite);
		List<unsigned char> buffer;
		while (!fs->IsEnd())
		{
			unsigned char ch;
			int read = (int)fs->Read(&ch, 1);
			if (read)
				buffer.Add(ch);
			else
				break;
		}
		return _Move(buffer);
	}

	void File::WriteAllText(const Slang::String & fileName, const Slang::String & text)
	{
		StreamWriter writer(new FileStream(fileName, FileMode::Create));
		writer.Write(text);
	}


}

