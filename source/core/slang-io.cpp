#include "slang-io.h"
#include "slang-exception.h"

#include "../../slang-com-helper.h"

#ifndef __STDC__
#   define __STDC__ 1
#endif

#include <sys/stat.h>

#ifdef _WIN32
#   include <direct.h>

#   define WIN32_LEAN_AND_MEAN
#   define VC_EXTRALEAN
#   include <Windows.h>
#endif

#if defined(__linux__) || defined(__CYGWIN__) || SLANG_APPLE_FAMILY
#   include <unistd.h>
#endif

#if SLANG_APPLE_FAMILY
#   include <mach-o/dyld.h>
#endif

#include <limits.h> /* PATH_MAX */
#include <stdio.h>
#include <stdlib.h>

namespace Slang
{

    /* static */SlangResult File::remove(const String& fileName)
    {
#ifdef _WIN32
        // https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-deletefilea
        if (DeleteFileA(fileName.getBuffer()))
        {
            return SLANG_OK;
        }
        return SLANG_FAIL;
#else
        // https://linux.die.net/man/3/remove
        if (::remove(fileName.getBuffer()) == 0)
        {
            return SLANG_OK;
        }
        return SLANG_FAIL;
#endif
    }


#ifdef _WIN32
    /* static */SlangResult File::generateTemporary(const UnownedStringSlice& inPrefix, Slang::String& outFileName)
    {
        // https://docs.microsoft.com/en-us/windows/win32/fileio/creating-and-using-a-temporary-file

        String tempPath;
        {
            int count = MAX_PATH + 1;
            while (true)
            {
                char* chars = tempPath.prepareForAppend(count);
                //  Gets the temp path env string (no guarantee it's a valid path).
                // https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-gettemppatha
                DWORD ret = ::GetTempPathA(count - 1, chars);
                if (ret == 0)
                {
                    return SLANG_FAIL;
                }
                if (ret > DWORD(count - 1))
                {
                    count = ret + 1;
                    continue;
                }
                tempPath.appendInPlace(chars, count);
                break;
            }
        }

        if (!File::exists(tempPath))
        {
            return SLANG_FAIL;
        }

        const String prefix(inPrefix);
        String tempFileName;

        {
            int count = MAX_PATH + 1;
            char* chars = tempFileName.prepareForAppend(count);

            // https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-gettempfilenamea
            //  Generates a temporary file name. 
            DWORD ret = ::GetTempFileNameA(tempPath.getBuffer(), prefix.getBuffer(), 0, chars);

            if (ret == 0)
            {
                return SLANG_FAIL;
            }
            tempFileName.appendInPlace(chars, ::strlen(chars));
        }

        outFileName = tempFileName;
        return SLANG_OK;
    }
#else
    /* static */SlangResult File::generateTemporary(const UnownedStringSlice& inPrefix, Slang::String& outFileName)
    {
        StringBuilder builder;
        builder << "/tmp/" << inPrefix << "-XXXXXX";

        List<char> buffer;
        buffer.setCount(builder.getLength() + 1);
        ::memcpy(buffer.getBuffer(), builder.getBuffer(), builder.getLength());
        buffer[builder.getLength()] = 0;

        int handle = mkstemp(buffer.getBuffer());
        if (handle == -1)
        {
            return SLANG_FAIL;
        }

        // Close the handle..
        close(handle);

        outFileName = buffer.getBuffer();
        return SLANG_OK;
    }
#endif

    /* static */SlangResult File::makeExecutable(const String& fileName)
    {
#ifdef _WIN32
        SLANG_UNUSED(fileName);
        // As long as file extension is executable, it can be executed
        return SLANG_OK;
#else
        const int ret = ::chmod(fileName.getBuffer(), S_IXUSR);
        return (ret == 0) ? SLANG_OK : SLANG_FAIL;
#endif
    }


	bool File::exists(const String& fileName)
	{
#ifdef _WIN32
		struct _stat32 statVar;
		return ::_wstat32(((String)fileName).toWString(), &statVar) != -1;
#else
		struct stat statVar;
		return ::stat(fileName.getBuffer(), &statVar) == 0;
#endif
	}

	String Path::truncateExt(const String& path)
	{
		UInt dotPos = path.lastIndexOf('.');
		if (dotPos != -1)
			return path.subString(0, dotPos);
		else
			return path;
	}
	String Path::replaceExt(const String& path, const char* newExt)
	{
		StringBuilder sb(path.getLength()+10);
		UInt dotPos = path.lastIndexOf('.');
		if (dotPos == -1)
			dotPos = path.getLength();
		sb.Append(path.getBuffer(), dotPos);
		sb.Append('.');
		sb.Append(newExt);
		return sb.ProduceString();
	}

    static UInt findLastSeparator(String const& path)
    {
		UInt slashPos = path.lastIndexOf('/');
        UInt backslashPos = path.lastIndexOf('\\');

        if (slashPos == -1) return backslashPos;
        if (backslashPos == -1) return slashPos;

        UInt pos = slashPos;
        if (backslashPos > slashPos)
            pos = backslashPos;

        return pos;
    }

	String Path::getFileName(const String& path)
	{
        UInt pos = findLastSeparator(path);
        if (pos != -1)
        {
            pos = pos + 1;
            return path.subString(pos, path.getLength() - pos);
        }
        else
        {
            return path;
        }
	}
	String Path::getFileNameWithoutExt(const String& path)
	{
        String fileName = getFileName(path);
		UInt dotPos = fileName.lastIndexOf('.');
		if (dotPos == -1)
            return fileName;
		return fileName.subString(0, dotPos);
	}
	String Path::getFileExt(const String& path)
	{
		UInt dotPos = path.lastIndexOf('.');
		if (dotPos != -1)
			return path.subString(dotPos+1, path.getLength()-dotPos-1);
		else
			return "";
	}
	String Path::getParentDirectory(const String& path)
	{
        UInt pos = findLastSeparator(path);
		if (pos != -1)
			return path.subString(0, pos);
		else
			return "";
	}
	
    /* static */void Path::append(StringBuilder& ioBuilder, const UnownedStringSlice& path)
    {
        if (ioBuilder.getLength() == 0)
        {
            ioBuilder.append(path);
            return;
        }
        if (path.size() > 0)
        {
            // If ioBuilder doesn't end in a delimiter, add one
            if (!isDelimiter(ioBuilder[ioBuilder.getLength() - 1]))
            {
                ioBuilder.append(kPathDelimiter);
            }
            // Check that path doesn't start with a path delimiter 
            SLANG_ASSERT(!isDelimiter(path[0]));
            // Append the path
            ioBuilder.append(path);
        }
    }

    /* static */void Path::combineIntoBuilder(const UnownedStringSlice& path1, const UnownedStringSlice& path2, StringBuilder& outBuilder)
    {
        outBuilder.Clear();
        outBuilder.Append(path1);
        append(outBuilder, path2);
    }

    String Path::combine(const String& path1, const String& path2)
    {
        if (path1.getLength() == 0)
        {
            return path2;
        }

        StringBuilder sb;
        combineIntoBuilder(path1.getUnownedSlice(), path2.getUnownedSlice(), sb);
        return sb.ProduceString();
    }
	String Path::combine(const String& path1, const String& path2, const String& path3)
	{
		StringBuilder sb;
        sb.append(path1);
        append(sb, path2.getUnownedSlice());
        append(sb, path3.getUnownedSlice());
		return sb.ProduceString();
	}

    /* static */ bool Path::isDriveSpecification(const UnownedStringSlice& element)
    {
        switch (element.size())
        {
            case 0:     
            {
                // We'll just assume it is
                return true;
            }
            case 2:
            {
                // Look for a windows like drive spec
                const char firstChar = element[0]; 
                return element[1] == ':' && ((firstChar >= 'a' && firstChar <= 'z') || (firstChar >= 'A' && firstChar <= 'Z'));
            }
            default:    return false;
        }
    }

    UnownedStringSlice Path::getFirstElement(const UnownedStringSlice& in)
    {
        const char* end = in.end();
        const char* cur = in.begin();
        // Find delimiter or the end
        while (cur < end && !Path::isDelimiter(*cur)) ++cur;
        return UnownedStringSlice(in.begin(), cur);
    }

    /* static */bool Path::isAbsolute(const UnownedStringSlice& path)
    {
        if (path.size() > 0 && isDelimiter(path[0]))
        {
            return true;
        }

#if SLANG_WINDOWS_FAMILY
        // Check for the \\ network drive style
        if (path.size() >= 2 && path[0] == '\\' && path[1] == '\\')
        {
            return true;
        }

        // Check for drive 
        if (isDriveSpecification(getFirstElement(path)))
        {
            return true;
        }
#endif

        return false;
    }

    /* static */void Path::split(const UnownedStringSlice& path, List<UnownedStringSlice>& splitOut)
    {
        splitOut.clear();

        const char* start = path.begin();
        const char* end = path.end();

        while (start < end)
        {
            const char* cur = start;
            // Find the split
            while (cur < end && !isDelimiter(*cur)) cur++;

            splitOut.add(UnownedStringSlice(start, cur));
           
            // Next
            start = cur + 1;
        }

        // Okay if the end is empty. And we aren't with a spec like // or c:/ , then drop the final slash 
        if (splitOut.getCount() > 1 && splitOut.getLast().size() == 0)
        {
            if (splitOut.getCount() == 2 && isDriveSpecification(splitOut[0]))
            {
                return;
            }
            // Remove the last 
            splitOut.removeLast();
        }
    }

    /* static */bool Path::hasRelativeElement(const UnownedStringSlice& path)
    {
        List<UnownedStringSlice> splitPath;
        split(path, splitPath);

        for (const auto& cur : splitPath)
        {
            if (cur == "." || cur == "..")
            {
                return true;
            }
        }
        return false;
    }

    /* static */String Path::simplify(const UnownedStringSlice& path)
    {
        List<UnownedStringSlice> splitPath;
        split(path, splitPath);

        // Strictly speaking we could do something about case on platforms like window, but here we won't worry about that
        for (Index i = 0; i < splitPath.getCount(); i++)
        {
            const UnownedStringSlice& cur = splitPath[i];
            if (cur == "." && splitPath.getCount() > 1)
            {
                // Just remove it 
                splitPath.removeAt(i);
                i--;
            }
            else if (cur == ".." && i > 0)
            {
                // Can we remove this and the one before ?
                UnownedStringSlice& before = splitPath[i - 1];
                if (before == ".." || (i == 1 && isDriveSpecification(before)))
                {
                    // Can't do it
                    continue;
                }
                splitPath.removeRange(i - 1, 2);
                i -= 2;
            }
        }

        // If its empty it must be .
        if (splitPath.getCount() == 0)
        {
            splitPath.add(UnownedStringSlice::fromLiteral("."));
        }
   
        // Reconstruct the string
        StringBuilder builder;
        for (Index i = 0; i < splitPath.getCount(); i++)
        {
            if (i > 0)
            {
                builder.Append(kPathDelimiter);
            }
            builder.Append(splitPath[i]);
        }

        return builder.ToString();
    }

	bool Path::createDirectory(const String& path)
	{
#if defined(_WIN32)
		return _wmkdir(path.toWString()) == 0;
#else 
		return mkdir(path.getBuffer(), 0777) == 0;
#endif
	}

    /* static */SlangResult Path::getPathType(const String& path, SlangPathType* pathTypeOut)
    {
#ifdef _WIN32
        // https://msdn.microsoft.com/en-us/library/14h5k7ff.aspx
        struct _stat32 statVar;
        if (::_wstat32(String(path).toWString(), &statVar) == 0)
        {
            if (statVar.st_mode & _S_IFDIR)
            {
                *pathTypeOut = SLANG_PATH_TYPE_DIRECTORY;
                return SLANG_OK;
            }
            else if (statVar.st_mode & _S_IFREG)
            {
                *pathTypeOut = SLANG_PATH_TYPE_FILE;
                return SLANG_OK;
            }
            return SLANG_FAIL;
        }

        return SLANG_E_NOT_FOUND;
#else
        struct stat statVar;
        if (::stat(path.getBuffer(), &statVar) == 0)
        {
            if (S_ISDIR(statVar.st_mode))
            {
                *pathTypeOut = SLANG_PATH_TYPE_DIRECTORY;
                return SLANG_OK;
            }
            if (S_ISREG(statVar.st_mode))
            {
                *pathTypeOut = SLANG_PATH_TYPE_FILE;
                return SLANG_OK;
            }
            return SLANG_FAIL;
        }

        return SLANG_E_NOT_FOUND;
#endif
    }


    /* static */SlangResult Path::getCanonical(const String& path, String& canonicalPathOut)
    {
#if defined(_WIN32)
        // https://msdn.microsoft.com/en-us/library/506720ff.aspx
        wchar_t* absPath = ::_wfullpath(nullptr, path.toWString(), 0);
        if (!absPath)
        {
            return SLANG_FAIL;
        }  

        canonicalPathOut =  String::fromWString(absPath);
        ::free(absPath);
        return SLANG_OK;
#else
#   if 1
        
        // http://man7.org/linux/man-pages/man3/realpath.3.html
        char* canonicalPath = ::realpath(path.begin(), nullptr);
        if (canonicalPath)
        {
            canonicalPathOut = canonicalPath;
            ::free(canonicalPath);
            return SLANG_OK;
        }
        return SLANG_FAIL;
#   else
        // This is a mechanism to get an approximation of canonical path if we don't have 'realpath'
        // We only can get if the file exists. This checks that the ../. etc are really valid
        SlangPathType pathType;
        SLANG_RETURN_ON_FAIL(getPathType(path, &pathType));
        if (isAbsolute(path))
        {
            // If it's absolute, we can just simplify as is
            canonicalPathOut = Path::simplify(path);
            return SLANG_OK;
        }
        else
        {
            char buffer[PATH_MAX];
            // https://linux.die.net/man/3/getcwd
            const char* getCwdPath = getcwd(buffer, SLANG_COUNT_OF(buffer));
            if (!getCwdPath)
            {
                return SLANG_FAIL;
            }

            // Okay combine the paths
            String combinedPaths = Path::combine(String(getCwdPath), path);
            // Simplify
            canonicalPathOut = Path::simplify(combinedPaths);
            return SLANG_OK;
        }
#   endif
#endif
    }

    /// Gets the path to the executable that was invoked that led to the current threads execution
    /// If run from a shared library/dll will be the path of the executable that loaded said library
    /// @param outPath Pointer to buffer to hold the path.
    /// @param ioPathSize Size of the buffer to hold the path (including zero terminator). 
    /// @return SLANG_OK on success, SLANG_E_BUFFER_TOO_SMALL if buffer is too small. If ioPathSize is changed it will be the required size
    static SlangResult _calcExectuablePath(char* outPath, size_t* ioSize)
    {
        SLANG_ASSERT(ioSize);
        const size_t bufferSize = *ioSize;
        SLANG_ASSERT(bufferSize > 0);

#if SLANG_WINDOWS_FAMILY
        // https://docs.microsoft.com/en-us/windows/desktop/api/libloaderapi/nf-libloaderapi-getmodulefilenamea
        
        DWORD res = ::GetModuleFileNameA(::GetModuleHandle(nullptr), outPath, DWORD(bufferSize));
        // If it fits it's the size not including terminator. So must be less than bufferSize
        if (res < bufferSize)
        {
            return SLANG_OK;
        }
        return SLANG_E_BUFFER_TOO_SMALL;
#elif SLANG_LINUX_FAMILY

#   if defined(__linux__) || defined(__CYGWIN__)
        // https://linux.die.net/man/2/readlink
        // Mark last byte with 0, so can check overrun
        ssize_t resSize = ::readlink("/proc/self/exe", outPath, bufferSize);
        if (resSize < 0)
        {
            return SLANG_FAIL;
        }
        if (resSize >= bufferSize)
        {
            return SLANG_E_BUFFER_TOO_SMALL;
        }
        // Zero terminate
        outPath[resSize - 1] = 0;
        return SLANG_OK;
#   else        
        String text = Slang::File::readAllText("/proc/self/maps");
        Index startIndex = text.indexOf('/');
        if (startIndex == Index(-1))
        {
            return SLANG_FAIL;
        }
        Index endIndex = text.indexOf("\n", startIndex);
        endIndex = (endIndex == Index(-1)) ? text.getLength() : endIndex;

        auto path = text.subString(startIndex, endIndex - startIndex);

        if (path.getLength() < bufferSize)
        {
            ::memcpy(outPath, path.begin(), path.getLength());
            outPath[path.getLength()] = 0;
            return SLANG_OK;
        }

        *ioSize = path.getLength() + 1;
        return SLANG_E_BUFFER_TOO_SMALL;
#   endif

#elif SLANG_APPLE_FAMILY
        // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/dyld.3.html
        uint32_t size = uint32_t(*ioSize);
        switch (_NSGetExecutablePath(outPath, &size))
        {
            case 0:           return SLANG_OK;
            case -1:
            {
                *ioSize = size;
                return SLANG_E_BUFFER_TOO_SMALL;
            }
            default: break;
        }
        return SLANG_FAIL;
#else
        return SLANG_E_NOT_IMPLEMENTED;
#endif
    }

    static String _getExecutablePath()
    {
        List<char> buffer;
        // Guess an initial buffer size
        buffer.setCount(1024);

        while (true)
        {
            const size_t size = size_t(buffer.getCount());
            size_t bufferSize = size;
            SlangResult res = _calcExectuablePath(buffer.getBuffer(), &bufferSize);

            if (SLANG_SUCCEEDED(res))
            {
                return String(buffer.getBuffer());
            }

            if (res != SLANG_E_BUFFER_TOO_SMALL)
            {
                // Couldn't determine the executable string
                return String();
            }

            // If bufferSize changed it should be the exact fit size, else we just make the buffer bigger by a guess (50% bigger)
            bufferSize = (bufferSize > size) ? bufferSize : (bufferSize + bufferSize / 2);
            buffer.setCount(Index(bufferSize));
        }
    }

    /* static */String Path::getExecutablePath()
    {
        // TODO(JS): It would be better if we lazily evaluated this, and then returned the same string on subsequent calls, because it has to do
        // a fair amount of work depending on target.
        // This was how previous code worked, with a static variable. Unfortunately this led to a memory leak being reported - because reporting
        // is done before a global variable is released.
        // It would be good to have a mechanism that allows 'core' library source free memory in some controlled manner.
        return _getExecutablePath();
    }

	Slang::String File::readAllText(const Slang::String& fileName)
	{
		StreamReader reader(new FileStream(fileName, FileMode::Open, FileAccess::Read, FileShare::ReadWrite));
		return reader.ReadToEnd();
	}

	Slang::List<unsigned char> File::readAllBytes(const Slang::String& fileName)
	{
		RefPtr<FileStream> fs = new FileStream(fileName, FileMode::Open, FileAccess::Read, FileShare::ReadWrite);
		List<unsigned char> buffer;
		while (!fs->IsEnd())
		{
			unsigned char ch;
			int read = (int)fs->Read(&ch, 1);
			if (read)
				buffer.add(ch);
			else
				break;
		}
		return _Move(buffer);
	}

	void File::writeAllText(const Slang::String& fileName, const Slang::String& text)
	{
		StreamWriter writer(new FileStream(fileName, FileMode::Create));
		writer.Write(text);
	}


}

