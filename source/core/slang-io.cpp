#include "slang-io.h"
#include "slang-exception.h"

#include "../../slang-com-helper.h"

#include "slang-string-util.h"
#include "slang-char-util.h"

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
// For Path::find
#   include <fnmatch.h>

#   include <dirent.h>
#   include <sys/stat.h>
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
            // Generates a temporary file name.
            // Will create a file with this name.
            DWORD ret = ::GetTempFileNameA(tempPath.getBuffer(), prefix.getBuffer(), 0, chars);

            if (ret == 0)
            {
                return SLANG_FAIL;
            }
            tempFileName.appendInPlace(chars, ::strlen(chars));
        }

        SLANG_ASSERT(File::exists(tempFileName));

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
        SLANG_ASSERT(File::exists(outFileName));

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

    String Path::replaceExt(const String& path, const char* newExt)
    {
        StringBuilder sb(path.getLength() + 10);
        Index dotPos = findExtIndex(path);

        if (dotPos < 0)
            dotPos = path.getLength();
        sb.Append(path.getBuffer(), dotPos);
        sb.Append('.');
        sb.Append(newExt);
        return sb.ProduceString();
    }

    /* static */ Index Path::findLastSeparatorIndex(UnownedStringSlice const& path)
    {
        const char* chars = path.begin();
        for (Index i = path.getLength() - 1; i >= 0; --i)
        {
            const char c = chars[i];
            if (c == '/' || c == '\\')
            {
                return i;
            }
        }
        return -1;
    }

    /* static */Index Path::findExtIndex(UnownedStringSlice const& path)
    {
        const Index sepIndex = findLastSeparatorIndex(path);

        const Index dotIndex = path.lastIndexOf('.');
        if (sepIndex >= 0)
        {
            // Index has to be in the last part of the path
            return (dotIndex > sepIndex) ? dotIndex : -1;
        }
        else
        {
            return dotIndex;
        }
    }

    String Path::getFileName(const String& path)
    {
        Index pos = findLastSeparatorIndex(path);
        if (pos >= 0)
        {
            pos = pos + 1;
            return path.subString(pos, path.getLength() - pos);
        }
        else
        {
            return path;
        }
    }

    /* static */String Path::getFileNameWithoutExt(const String& path)
    {
        Index sepIndex = findLastSeparatorIndex(path);
        sepIndex = (sepIndex < 0) ? 0 : (sepIndex + 1);
        Index dotIndex = findExtIndex(path);
        dotIndex = (dotIndex < 0) ? path.getLength() : dotIndex;

        return path.subString(sepIndex, dotIndex - sepIndex);
    }

    /* static*/ String Path::getPathWithoutExt(const String& path)
    {
        Index dotPos = findExtIndex(path);
        if (dotPos >= 0)
            return path.subString(0, dotPos);
        else
            return path;
    }

    UnownedStringSlice Path::getPathExt(const UnownedStringSlice& path)
    {
        const Index dotPos = findExtIndex(path);
        if (dotPos >= 0)
        {
            return path.subString(dotPos + 1, path.getLength() - dotPos - 1);
        }
        else
        {
            // Note that the caller can identify if path has no extension or just a .
            // as if it's a dot a zero length slice is returned in path
            // If it's not then a default slice is returned (which doesn't point into path).
            //
            // Granted this is a little obscure and perhaps should be improved.
            return UnownedStringSlice();
        }
    }

    String Path::getParentDirectory(const String& path)
    {
        Index pos = findLastSeparatorIndex(path);
        if (pos >= 0)
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
        if (path.getLength() > 0)
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
        switch (element.getLength())
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
        if (path.getLength() > 0 && isDelimiter(path[0]))
        {
            return true;
        }

#if SLANG_WINDOWS_FAMILY
        // Check for the \\ network drive style
        if (path.getLength() >= 2 && path[0] == '\\' && path[1] == '\\')
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
        if (splitOut.getCount() > 1 && splitOut.getLast().getLength() == 0)
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

    /* static */void Path::simplify(List<UnownedStringSlice>& ioSplit)
    {
        // Strictly speaking we could do something about case on platforms like window, but here we won't worry about that
        for (Index i = 0; i < ioSplit.getCount(); i++)
        {
            const UnownedStringSlice& cur = ioSplit[i];
            if (cur == "." && ioSplit.getCount() > 1)
            {
                // Just remove it 
                ioSplit.removeAt(i);
                i--;
            }
            else if (cur == ".." && i > 0)
            {
                // Can we remove this and the one before ?
                UnownedStringSlice& before = ioSplit[i - 1];
                if (before == ".." || (i == 1 && isDriveSpecification(before)))
                {
                    // Can't do it, but we allow relative, so just leave for now
                    continue;
                }
                ioSplit.removeRange(i - 1, 2);
                i -= 2;
            }
        }
    }

    /* static */void Path::join(const UnownedStringSlice* slices, Index count, StringBuilder& out)
    {
        out.Clear();

        if (count == 0)
        {
            out << ".";
            return;
        }

        StringUtil::join(slices, count, kPathDelimiter, out);
    }


    /* static */String Path::simplify(const UnownedStringSlice& path)
    {
        List<UnownedStringSlice> splitPath;
        split(path, splitPath);
        simplify(splitPath);

        // Reconstruct the string
        StringBuilder builder;
        join(splitPath.getBuffer(), splitPath.getCount(), builder);
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

    SlangResult Path::remove(const String& path)
    {
#ifdef _WIN32
        // Need to determine if its a file or directory

        SlangPathType pathType;
        SLANG_RETURN_ON_FAIL(getPathType(path, &pathType));

        
        switch (pathType)
        {
            case SLANG_PATH_TYPE_FILE:
            {
                // https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-deletefilea
                if (DeleteFileA(path.getBuffer()))
                {
                    return SLANG_OK;
                }
                break;
            }
            case SLANG_PATH_TYPE_DIRECTORY:
            {
                // https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-removedirectorya
                if (RemoveDirectoryA(path.getBuffer()))
                {
                    return SLANG_OK;
                }
                break;
            }
            default: break;
        }

        return SLANG_FAIL;
#else
        // https://linux.die.net/man/3/remove
        if (::remove(path.getBuffer()) == 0)
        {
            return SLANG_OK;
        }
        return SLANG_FAIL;
#endif
    }

#if defined(_WIN32)
    /* static */SlangResult Path::find(const String& directoryPath, const char* pattern, Visitor* visitor)
    {
        pattern = pattern ? pattern : "*";
        String searchPath = Path::combine(directoryPath, pattern);

        WIN32_FIND_DATAW fileData;

        HANDLE findHandle = FindFirstFileW(searchPath.toWString(), &fileData);
        if (!findHandle)
        {
            return SLANG_E_NOT_FOUND;
        }

        do
        {
            if (!((wcscmp(fileData.cFileName, L".") == 0) ||
                  (wcscmp(fileData.cFileName, L"..") == 0)))
            {
                const Type type = (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? Type::Directory : Type::File;

                String filename = String::fromWString(fileData.cFileName);
                visitor->accept(type, filename.getUnownedSlice());
            }
        }
        while (FindNextFileW(findHandle, &fileData) != 0);

         ::FindClose(findHandle);
         return SLANG_OK;
    }
#else
    /* static */SlangResult Path::find(const String& directoryPath, const char* pattern, Visitor* visitor)
    {
        DIR* directory = opendir(directoryPath.getBuffer());
        
        if (!directory)
        {
            return SLANG_E_NOT_FOUND;
        }

        StringBuilder builder;
        for (;;)
        {
            dirent* entry = readdir(directory);
            if (entry == nullptr)
            {
                break;
            }

            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }

            // If there is a pattern, check if it matches, and if it doesn't ignore it
            if (pattern && fnmatch(pattern, entry->d_name, 0) != 0)
            {
                continue;
            }

            const UnownedStringSlice filename(entry->d_name);

            // Produce the full path, to do stat
            Path::combineIntoBuilder(directoryPath.getUnownedSlice(), filename, builder);

            //    fprintf(stderr, "stat(%s)\n", path.getBuffer());
            struct stat fileInfo;
            if (stat(builder.getBuffer(), &fileInfo) != 0)
            {
                continue;
            }

            Type type = Type::Unknown;
            if (S_ISDIR(fileInfo.st_mode))
            {
                type = Type::Directory;
            }
            else if (S_ISREG(fileInfo.st_mode))
            {
                type = Type::File;
            }

            visitor->accept(type, filename);
        }

        closedir(directory);
        return SLANG_OK;
    }
#endif

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

    SlangResult File::readAllText(const Slang::String& fileName, String& outText)
    {
        RefPtr<FileStream> stream(new FileStream);
        SLANG_RETURN_ON_FAIL(stream->init(fileName, FileMode::Open, FileAccess::Read, FileShare::ReadWrite));

        StreamReader reader;
        SLANG_RETURN_ON_FAIL(reader.init(stream));
        SLANG_RETURN_ON_FAIL(reader.readToEnd(outText));

        return SLANG_OK;
    }

    SlangResult File::readAllBytes(const Slang::String& path, Slang::List<unsigned char>& out)
    {
        FileStream stream;
        SLANG_RETURN_ON_FAIL(stream.init(path, FileMode::Open, FileAccess::Read, FileShare::ReadWrite));

        const Int64 start = stream.getPosition();
        stream.seek(SeekOrigin::End, 0);
        const Int64 end = stream.getPosition();
        stream.seek(SeekOrigin::Start, start);

        const Int64 positionSizeInBytes = end - start;

        if (UInt64(positionSizeInBytes) > UInt64(kMaxIndex))
        {
            // It's too large to fit in memory.
            return SLANG_FAIL;
        }

        const Index sizeInBytes = Index(positionSizeInBytes);

        out.setCount(sizeInBytes);

        size_t readSizeInBytes;
        SLANG_RETURN_ON_FAIL(stream.read(out.getBuffer(), sizeInBytes, readSizeInBytes));

        // If not all read just return an error
        return (size_t(sizeInBytes) == readSizeInBytes) ? SLANG_OK : SLANG_FAIL;
    }

    SlangResult File::readAllBytes(const String& path, ScopedAllocation& out)
    {
        FileStream stream;
        SLANG_RETURN_ON_FAIL(stream.init(path, FileMode::Open, FileAccess::Read, FileShare::ReadWrite));

        const Int64 start = stream.getPosition();
        stream.seek(SeekOrigin::End, 0);
        const Int64 end = stream.getPosition();
        stream.seek(SeekOrigin::Start, start);

        const Int64 positionSizeInBytes = end - start;

        if (UInt64(positionSizeInBytes) > UInt64(~size_t(0)))
        {
            // It's too large to fit in memory.
            return SLANG_FAIL;
        }

        const size_t sizeInBytes = size_t(positionSizeInBytes);

        void* data = out.allocate(sizeInBytes);
        if (!data)
        {
            return SLANG_E_OUT_OF_MEMORY;
        }

        size_t readSizeInBytes;
        SLANG_RETURN_ON_FAIL(stream.read(data, sizeInBytes, readSizeInBytes));

        // If not all read just return an error
        return (sizeInBytes == readSizeInBytes) ? SLANG_OK : SLANG_FAIL;
    }

    SlangResult File::writeAllBytes(const String& path, const void* data, size_t size)
    {
        FileStream stream;
        SLANG_RETURN_ON_FAIL(stream.init(path, FileMode::Create, FileAccess::Write, FileShare::ReadWrite));
        SLANG_RETURN_ON_FAIL(stream.write(data, size));
        return SLANG_OK;
    }
    
    SlangResult File::writeAllText(const Slang::String& fileName, const Slang::String& text)
    {
        RefPtr<FileStream> stream = new FileStream;
        SLANG_RETURN_ON_FAIL(stream->init(fileName, FileMode::Create));

        StreamWriter writer;
        SLANG_RETURN_ON_FAIL(writer.init(stream));
        SLANG_RETURN_ON_FAIL(writer.write(text));

        return SLANG_OK;
    }
    
    String URI::getPath() const
    {
        Index startIndex = uri.indexOf("://");
        if (startIndex == -1)
            return String();
        startIndex += 3;
        Index endIndex = uri.indexOf('?');
        if (endIndex == -1)
            endIndex = uri.getLength();
        StringBuilder sb;
#if SLANG_WINDOWS_FAMILY
        if (uri[startIndex] == '/')
            startIndex++;
#endif
        for (Index i = startIndex; i < endIndex;)
        {
            auto ch = uri[i];
            if (ch == '%')
            {
                Int charVal = CharUtil::getHexDigitValue(uri[i + 1]) * 16 +
                              CharUtil::getHexDigitValue(uri[i + 2]);
                sb.appendChar((char)charVal);
                i += 3;
            }
            else
            {
                sb.appendChar(uri[i]);
                i++;
            }
        }
        return sb.ProduceString();
    }

    StringSlice URI::getProtocol() const
    {
        Index separatorIndex = uri.indexOf("://");
        if (separatorIndex != -1)
            return uri.subString(0, separatorIndex);
        return StringSlice();
    }

    bool URI::isSafeURIChar(char ch)
    {
        return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
               ch == '-' || ch == '_' || ch == '/' || ch == '.';
    }

    URI URI::fromLocalFilePath(UnownedStringSlice path)
    {
        URI uri;
        StringBuilder sb;
        sb << "file://";

#if SLANG_WINDOWS_FAMILY
        sb << "/";
#endif

        for (auto ch : path)
        {
            if (isSafeURIChar(ch))
            {
                sb.appendChar(ch);
            }
            else if (ch == '\\')
            {
                sb.appendChar('/');
            }
            else
            {
                char buffer[32];
                int length = IntToAscii(buffer, (int)ch, 16);
                ReverseInternalAscii(buffer, length);
                sb << "%" << buffer;
            }
        }
        return URI::fromString(sb.getUnownedSlice());
    }

    URI URI::fromString(UnownedStringSlice uriString)
    {
        URI uri;
        uri.uri = uriString;
        return uri;
    }

}
