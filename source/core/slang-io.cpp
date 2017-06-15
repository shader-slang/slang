#include "slang-io.h"
#include "exception.h"
#ifndef __STDC__
#define __STDC__ 1
#endif
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

namespace Slang
{
	CommandLineWriter * currentCommandWriter = nullptr;

	void SetCommandLineWriter(CommandLineWriter * writer)
	{
		currentCommandWriter = writer;
	}

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
		int dotPos = path.LastIndexOf('.');
		if (dotPos != -1)
			return path.SubString(0, dotPos);
		else
			return path;
	}
	String Path::ReplaceExt(const String & path, const char * newExt)
	{
		StringBuilder sb(path.Length()+10);
		int dotPos = path.LastIndexOf('.');
		if (dotPos == -1)
			dotPos = path.Length();
		sb.Append(path.Buffer(), dotPos);
		sb.Append('.');
		sb.Append(newExt);
		return sb.ProduceString();
	}
	String Path::GetFileName(const String & path)
	{
		int pos = path.LastIndexOf('/');
		pos = Math::Max(path.LastIndexOf('\\'), pos) + 1;
		return path.SubString(pos, path.Length()-pos);
	}
	String Path::GetFileNameWithoutEXT(const String & path)
	{
		int pos = path.LastIndexOf('/');
		pos = Math::Max(path.LastIndexOf('\\'), pos) + 1;
		int dotPos = path.LastIndexOf('.');
		if (dotPos <= pos)
			dotPos = path.Length();
		return path.SubString(pos, dotPos - pos);
	}
	String Path::GetFileExt(const String & path)
	{
		int dotPos = path.LastIndexOf('.');
		if (dotPos != -1)
			return path.SubString(dotPos+1, path.Length()-dotPos-1);
		else
			return "";
	}
	String Path::GetDirectoryName(const String & path)
	{
		int pos = path.LastIndexOf('/');
		pos = Math::Max(path.LastIndexOf('\\'), pos);
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
