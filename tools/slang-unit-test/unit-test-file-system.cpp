// unit-test-file-system.cpp

#include "../../source/core/slang-file-system.h"

#include "../../source/core/slang-riff-file-system.h"
#include "../../source/core/slang-zip-file-system.h"

#include "../../source/core/slang-memory-file-system.h"

#include "../../source/core/slang-deflate-compression-system.h"
#include "../../source/core/slang-lz4-compression-system.h"

#include "../../source/core/slang-destroyable.h"

#include "../../source/core/slang-io.h"

#include "tools/unit-test/slang-unit-test.h"

using namespace Slang;

namespace { 

enum class FileSystemType
{
	Zip,
	RiffUncompressed,
	RiffDeflate,
	RiffLZ4,
	Memory,
	Relative,
	CountOf,
};

struct Entry 
{
	typedef Entry ThisType;

	bool operator<(const ThisType& rhs) const { return name < rhs.name; }
	bool operator==(const ThisType& rhs) const { return name == rhs.name && type == rhs.type; }
	bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

	SlangPathType type;
	String name;
};

} // 

static SlangResult _createAndCheckFile(ISlangMutableFileSystem* fileSystem, const char* path, const char* contents)
{
	UnownedStringSlice contentsSlice(contents);

	SLANG_RETURN_ON_FAIL(fileSystem->saveFile(path, contentsSlice.begin(), contentsSlice.getLength()));

	SlangPathType pathType;
	SLANG_RETURN_ON_FAIL(fileSystem->getPathType(path, &pathType));

	if (pathType != SLANG_PATH_TYPE_FILE)
	{
		return SLANG_FAIL;
	}

	ComPtr<ISlangBlob> blob;
	SLANG_RETURN_ON_FAIL(fileSystem->loadFile(path, blob.writeRef()));

	if (blob->getBufferSize() != contentsSlice.getLength())
	{
		return SLANG_FAIL;
	}
	if (contentsSlice != UnownedStringSlice((const char*)blob->getBufferPointer(), blob->getBufferSize()))
	{
		return SLANG_FAIL;
	}

	return SLANG_OK;
}

static SlangResult _createAndCheckDirectory(ISlangMutableFileSystem* fileSystem, const char* path)
{
	SLANG_RETURN_ON_FAIL(fileSystem->createDirectory(path));

	SlangPathType pathType;
	SLANG_RETURN_ON_FAIL(fileSystem->getPathType(path, &pathType));

	if (pathType != SLANG_PATH_TYPE_DIRECTORY)
	{
		return SLANG_FAIL;
	}

	return SLANG_OK;
}

static void _entryCallback(SlangPathType pathType, const char* name, void* userData)
{
	List<Entry>& out = *(List<Entry>*)userData;
	out.add(Entry{pathType, name});
}

static SlangResult _enumeratePath(ISlangFileSystemExt* fileSystem, const char* path, const ConstArrayView<Entry>& entries)
{
	List<Entry> contents;
	
	SLANG_RETURN_ON_FAIL(fileSystem->enumeratePathContents(path, _entryCallback, (void*)&contents));
	
	contents.sort();

	if (contents.getArrayView() != entries)
	{
		return SLANG_FAIL;
	}

	return SLANG_OK;
}

static SlangResult _checkSimplifiedPath(ISlangFileSystemExt* fileSystem, const char* path, const char* normalPath)
{
	ComPtr<ISlangBlob> simplifiedPathBlob;
	SLANG_RETURN_ON_FAIL(fileSystem->getPath(PathKind::Simplified, path, simplifiedPathBlob.writeRef()));

	auto simplifiedPath = StringUtil::getString(simplifiedPathBlob);

	if (simplifiedPath != normalPath)
	{
		return SLANG_FAIL;
	}

	return SLANG_OK;
}

SlangResult _getAllEntries(ISlangFileSystemExt* fileSystem, List<Entry>& outEntries)
{
	outEntries.clear();
	
	SLANG_RETURN_ON_FAIL(fileSystem->enumeratePathContents(".", _entryCallback, (void*)&outEntries));
	
	for (Index i = 0; i < outEntries.getCount(); ++i)
	{
		const Entry entry = outEntries[i];
		if (entry.type != SLANG_PATH_TYPE_DIRECTORY)
		{
			continue;
		}

		List<Entry> localEntries;
		SLANG_RETURN_ON_FAIL(fileSystem->enumeratePathContents(entry.name.getBuffer(), _entryCallback, (void*)&localEntries));

		for (const auto& localEntry : localEntries)
		{
			outEntries.add(Entry{ localEntry.type, Path::combine(entry.name, localEntry.name) });
		}
	}

	outEntries.sort();

	return SLANG_OK;
}

static SlangResult _checkEqual(ISlangFileSystemExt* a, ISlangFileSystemExt* b)
{
	List<Entry> aEntries, bEntries;

	SLANG_RETURN_ON_FAIL(_getAllEntries(a, aEntries));
	SLANG_RETURN_ON_FAIL(_getAllEntries(b, bEntries));

	if (aEntries != bEntries)
	{
		return SLANG_FAIL;
	}

	// For all the files check the contents is the same

	for (const auto& entry : aEntries)
	{
		if (entry.type != SLANG_PATH_TYPE_FILE)
		{
			continue;
		}

		ComPtr<ISlangBlob> blobA, blobB;

		SLANG_RETURN_ON_FAIL(a->loadFile(entry.name.getBuffer(), blobA.writeRef()));
		SLANG_RETURN_ON_FAIL(b->loadFile(entry.name.getBuffer(), blobB.writeRef()));

		if (blobA->getBufferSize() != blobB->getBufferSize())
		{
			return SLANG_FAIL;
		}

		if (::memcmp(blobA->getBufferPointer(), blobB->getBufferPointer(), blobA->getBufferSize()) != 0)
		{
			return SLANG_FAIL;
		}
	}

	return SLANG_OK;
}

static SlangResult _test(FileSystemType type)
{
	ComPtr<ISlangMutableFileSystem> fileSystem;

	switch (type)
	{
		case FileSystemType::Zip:
		{
			SLANG_RETURN_ON_FAIL(ZipFileSystem::create(fileSystem));
			break;
		}
		case FileSystemType::RiffUncompressed:
		{
			fileSystem = new RiffFileSystem(nullptr);
			break;
		}
		case FileSystemType::RiffDeflate:
		{
			fileSystem = new RiffFileSystem(DeflateCompressionSystem::getSingleton());
			break;
		}
		case FileSystemType::RiffLZ4:
		{
			fileSystem = new RiffFileSystem(LZ4CompressionSystem::getSingleton());
			break;
		}
		case FileSystemType::Memory:
		{
			fileSystem = new MemoryFileSystem;
			break;
		}
		case FileSystemType::Relative:
		{
			ComPtr<ISlangMutableFileSystem> memoryFileSystem(new MemoryFileSystem);
			memoryFileSystem->createDirectory("base");

			fileSystem = new RelativeFileSystem(memoryFileSystem, "base");
			break;
		}
	}

	SLANG_RETURN_ON_FAIL(_createAndCheckFile(fileSystem, "a", "someText"));
	SLANG_RETURN_ON_FAIL(_createAndCheckFile(fileSystem, "b", "A longer bit of text...."));

	SLANG_RETURN_ON_FAIL(_createAndCheckDirectory(fileSystem, "d"));
	SLANG_RETURN_ON_FAIL(_createAndCheckFile(fileSystem, "d/a", "Some more silly stuff"));
	SLANG_RETURN_ON_FAIL(_createAndCheckFile(fileSystem, "d\\b", "Lets go empty"));

	// Lets find all the files in the directory

	{
		const Entry entries[] =  { {SLANG_PATH_TYPE_FILE, "a" }, {SLANG_PATH_TYPE_FILE, "b" } };
		SLANG_RETURN_ON_FAIL(_enumeratePath(fileSystem, "d", makeConstArrayView(entries)));
	}

	{
		const Entry entries[] = { {SLANG_PATH_TYPE_FILE, "a" }, {SLANG_PATH_TYPE_FILE, "b" }, {SLANG_PATH_TYPE_DIRECTORY, "d" } };
		SLANG_RETURN_ON_FAIL(_enumeratePath(fileSystem, ".", makeConstArrayView(entries)));
	}

	{
		SLANG_RETURN_ON_FAIL(_checkSimplifiedPath(fileSystem, "d/../a", "a"));
	}
	

	// If we have an archive file system check out it's behavior
	if (IArchiveFileSystem* archiveFileSystem = as<IArchiveFileSystem>(fileSystem))
	{
		// Load and check its okay

		ComPtr<ISlangBlob> archiveBlob;
		SLANG_RETURN_ON_FAIL(archiveFileSystem->storeArchive(false, archiveBlob.writeRef()));

		ComPtr<ISlangFileSystemExt> loadedFileSystem;
		SLANG_RETURN_ON_FAIL(loadArchiveFileSystem(archiveBlob->getBufferPointer(), archiveBlob->getBufferSize(), loadedFileSystem));

		// Check the file systems contents are the same
		SLANG_RETURN_ON_FAIL(_checkEqual(loadedFileSystem, fileSystem));
	}

	SLANG_RETURN_ON_FAIL(fileSystem->remove("d/a"));
	{
		const Entry entries[] = { {SLANG_PATH_TYPE_FILE, "b" } };
		SLANG_RETURN_ON_FAIL(_enumeratePath(fileSystem, "d", makeConstArrayView(entries)));
	}
	SLANG_RETURN_ON_FAIL(fileSystem->remove("d\\b"));
	{
		SLANG_RETURN_ON_FAIL(_enumeratePath(fileSystem, "d", makeConstArrayView((const Entry*)nullptr, 0)));
	}

	// If it's removed it can't be removed again
	SLANG_CHECK(SLANG_FAILED(fileSystem->remove("d\\b")));

	// Remove the directory
	SLANG_RETURN_ON_FAIL(fileSystem->remove("d"));

	{
		const Entry entries[] = { {SLANG_PATH_TYPE_FILE, "a" }, {SLANG_PATH_TYPE_FILE, "b" } };
		SLANG_RETURN_ON_FAIL(_enumeratePath(fileSystem, ".", makeConstArrayView(entries)));
	}


	return SLANG_OK;
}

SLANG_UNIT_TEST(fileSystem)
{
	for (Index i = 0; i < Count(FileSystemType::CountOf); ++i)
	{
		const auto type = FileSystemType(i);

		auto const res = _test(type);

		SLANG_CHECK(SLANG_SUCCEEDED(res));
	}
}

