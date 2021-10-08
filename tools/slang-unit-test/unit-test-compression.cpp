// unit-compression.cpp

#include "tools/unit-test/slang-unit-test.h"

#include "source/core/slang-io.h"
#include "../../source/core/slang-zip-file-system.h"

#include "../../source/core/slang-lz4-compression-system.h"
#include "../../source/core/slang-deflate-compression-system.h"

using namespace Slang;

static bool _equals(const void* data, size_t size, ISlangBlob* blob)
{
    return blob && blob->getBufferSize() == size && memcmp(blob->getBufferPointer(), data, size) == 0;
}

template <size_t SIZE>
static bool _equals(const char (&text)[SIZE], ISlangBlob* blob)
{
    return _equals(text, SIZE, blob);
}

static List<String> _getContents(ISlangFileSystemExt* fileSystem, const char* path)
{
    List<String> objs;

    fileSystem->enumeratePathContents(path, [](SlangPathType pathType, const char* name, void* userData) {
        List<String>& out = *(List<String>*)userData;
        out.add(name);
    }, &objs);

    return objs;
}

SLANG_UNIT_TEST(compression)
{
    const SlangArchiveType archiveTypes[] =
    {
        SLANG_ARCHIVE_TYPE_RIFF,
        SLANG_ARCHIVE_TYPE_RIFF_DEFLATE,
        SLANG_ARCHIVE_TYPE_RIFF_LZ4,
        SLANG_ARCHIVE_TYPE_ZIP
    };

    for (auto archiveType : archiveTypes)
    {
        // Test out archive file systems
        RefPtr<ArchiveFileSystem> archiveFileSystem;
        SLANG_CHECK(SLANG_SUCCEEDED(createArchiveFileSystem(archiveType, archiveFileSystem)));
        
        const char contents[] = "I'm compressed";
        const char contents2[] = "Some more stuff";
        const char contents3[] = "Replace it";

        {
            ISlangMutableFileSystem* fileSystem = archiveFileSystem;

            SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->createDirectory("hello")));
            SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->createDirectory("hello2")));
            SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->remove("hello")));
            SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->createDirectory("hello")));

            SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->saveFile("file.txt", contents, SLANG_COUNT_OF(contents))));

            SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->saveFile("file2.txt", contents2, SLANG_COUNT_OF(contents2))));

            ComPtr<ISlangBlob> blob;
            SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->loadFile("file.txt", blob.writeRef())));
            SLANG_CHECK(_equals(contents, blob));

            SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->loadFile("file2.txt", blob.writeRef())));
            SLANG_CHECK(_equals(contents2, blob));

            SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->saveFile("file2.txt", contents3, SLANG_COUNT_OF(contents3))));

            SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->loadFile("file2.txt", blob.writeRef())));
            SLANG_CHECK(_equals(contents3, blob));

            // Check the path type
            {
                SlangPathType pathType;
                SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->getPathType("file2.txt", &pathType)));
                SLANG_CHECK(pathType == SLANG_PATH_TYPE_FILE);

                SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->getPathType("hello", &pathType)));
                SLANG_CHECK(pathType == SLANG_PATH_TYPE_DIRECTORY);
            }

            // Enumerate
            {
                for (const auto& obj : _getContents(fileSystem, ""))
                {
                    // All of these should exist
                    SlangPathType pathType;
                    SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->getPathType(obj.getBuffer(), &pathType)));
                }
            }

            SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->saveFile("implicit-path/file2.txt", contents3, SLANG_COUNT_OF(contents3))));

            {
                SlangPathType pathType;
                SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->getPathType("implicit-path", &pathType)));

                SLANG_CHECK(pathType == SLANG_PATH_TYPE_DIRECTORY);

                List<String> objs = _getContents(fileSystem, "implicit-path");

                // It contains a file
                SLANG_CHECK(objs.getCount() == 1);

                for (const auto& obj : objs)
                {
                    String path = Path::combine("implicit-path", obj);

                    // All of these should exist
                    SlangPathType pathType;
                    SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->getPathType(path.getBuffer(), &pathType)));
                }

                // Make an explicit path, and see whe have the same results
                fileSystem->createDirectory("implicit-path");

                objs = _getContents(fileSystem, "implicit-path");
                SLANG_CHECK(objs.getCount() == 1);
            }
        }
    

        // Load and check its okay
 
        {
            ComPtr<ISlangBlob> archiveBlob;
            SLANG_CHECK(SLANG_SUCCEEDED(archiveFileSystem->storeArchive(false, archiveBlob.writeRef())));


            RefPtr<ArchiveFileSystem> fileSystem;
#if 0
            SLANG_CHECK(SLANG_SUCCEEDED(createArchiveFileSystem(archiveType, fileSystem)));

            SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->loadArchive(archiveBlob->getBufferPointer(), archiveBlob->getBufferSize())));
#else
            SLANG_CHECK(SLANG_SUCCEEDED(loadArchiveFileSystem(archiveBlob->getBufferPointer(), archiveBlob->getBufferSize(), fileSystem)));
#endif

            ComPtr<ISlangBlob> blob;

            SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->loadFile("file.txt", blob.writeRef())));
            SLANG_CHECK(_equals(contents, blob));

            SLANG_CHECK(SLANG_SUCCEEDED(fileSystem->loadFile("file2.txt", blob.writeRef())));
            SLANG_CHECK(_equals(contents3, blob));
        }
    }

    // Test out compression systems
    for (Index i = 0; i < 2; ++i)
    {
        // Lets try lz4

        ICompressionSystem* system = nullptr;
        if (i == 0)
        {
            system = LZ4CompressionSystem::getSingleton();
        }
        else
        {
            system = DeflateCompressionSystem::getSingleton();
        }

        const char src[] = "Some text to compress";
        size_t srcSize = sizeof(src);

        ComPtr<ISlangBlob> compressedBlob;

        CompressionStyle style;

        SLANG_CHECK(SLANG_SUCCEEDED(system->compress(&style, src, srcSize, compressedBlob.writeRef()))); 
        
        // Now lets decompress

        List<char> decompressedData;
        decompressedData.setCount(srcSize);

        SLANG_CHECK(SLANG_SUCCEEDED(system->decompress(compressedBlob->getBufferPointer(), compressedBlob->getBufferSize(), srcSize, decompressedData.getBuffer())));
        SLANG_CHECK(memcmp(src, decompressedData.getBuffer(), srcSize) == 0);
    }
}
