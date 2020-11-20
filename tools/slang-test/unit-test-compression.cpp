// unit-compression.cpp

#include "test-context.h"

#include "../../source/core/slang-zip-file-system.h"

using namespace Slang;

static void compressionUnitTest()
{
    // Lets load a zip

    ScopedAllocation alloc;
    File::readAllBytes("mdl.zip", alloc);

    ComPtr<ZipFileSystem> fileSystem;

    ZipFileSystem::create(alloc.getData(), alloc.getSizeInBytes(), fileSystem);

    // Enumerate the files

    StringBuilder buffer;

    fileSystem->enumeratePathContents("", [](SlangPathType pathType, const char* name, void* userData) {
        StringBuilder& buf = *(StringBuilder*)userData;
        buf << name << "\n";
    }, &buffer);

    ComPtr<ISlangBlob> blob;
    fileSystem->loadFile("mdl/AmbientOcclusionParams.h", blob.writeRef());

    {
        const char* chars = (const char*)blob->getBufferPointer();

        SLANG_UNUSED(chars);
    }

    // Create a directory

    fileSystem->createDirectory("hello");

    fileSystem->createDirectory("hello2");

    fileSystem->remove("hello");
    fileSystem->createDirectory("hello");

    // Try reading again
    fileSystem->loadFile("mdl/AmbientOcclusionParams.h", blob.writeRef());

    {
        auto& archive = fileSystem->getArchive();

        // Write it out
        File::writeAllBytes("mdl2.zip", archive.getBuffer(), archive.getCount());
    }

    //ZipCompressionUtil::unitTest();
}

SLANG_UNIT_TEST("Compression", compressionUnitTest);
