// slang-downstream-compiler.cpp
#include "slang-downstream-compiler.h"

#include "../core/slang-common.h"
#include "../../slang-com-helper.h"
#include "../core/slang-string-util.h"

#include "../core/slang-type-text-util.h"

#include "../core/slang-io.h"
#include "../core/slang-shared-library.h"
#include "../core/slang-blob.h"
#include "../core/slang-char-util.h"

#include "../core/slang-castable-util.h"

#include "slang-artifact-impl.h"
#include "slang-artifact-representation-impl.h"
#include "slang-artifact-associated-impl.h"
#include "slang-artifact-util.h"
#include "slang-artifact-helper.h"
#include "slang-artifact-desc-util.h"

namespace Slang
{

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompilerBase !!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

SlangResult DownstreamCompilerBase::convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact)
{
    SLANG_UNUSED(from);
    SLANG_UNUSED(to);
    SLANG_UNUSED(outArtifact);
    
    return SLANG_E_NOT_AVAILABLE;
}

void* DownstreamCompilerBase::castAs(const Guid& guid)
{
    if (auto ptr = getInterface(guid))
    {
        return ptr;
    }
    return getObject(guid);
}

void* DownstreamCompilerBase::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ICastable::getTypeGuid() ||
        guid == IDownstreamCompiler::getTypeGuid())
    {
        return static_cast<IDownstreamCompiler*>(this);
    }

    return nullptr;
}

void* DownstreamCompilerBase::getObject(const Guid& guid)
{
    SLANG_UNUSED(guid);
    return nullptr;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CommandLineDownstreamArtifactRepresentation !!!!!!!!!!!!!!!!!!!!!!*/

void* CommandLineDownstreamArtifactRepresentation::castAs(const Guid& guid)
{
    if (auto ptr = getInterface(guid))
    {
        return ptr;
    }
    return getObject(guid);
}

void* CommandLineDownstreamArtifactRepresentation::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ICastable::getTypeGuid() ||
        guid == IArtifactRepresentation::getTypeGuid())
    {
        IArtifactRepresentation* rep = this;
        return rep;
    }

    return nullptr;
}

void* CommandLineDownstreamArtifactRepresentation::getObject(const Guid& guid)
{
    SLANG_UNUSED(guid);
    return nullptr;
}

SlangResult CommandLineDownstreamArtifactRepresentation::createRepresentation(const Guid& typeGuid, ICastable** outCastable)
{
    if (typeGuid == ISlangSharedLibrary::getTypeGuid())
    {
        // Okay we want to load
        // Try loading the shared library
        SharedLibrary::Handle handle;
        if (SLANG_FAILED(SharedLibrary::loadWithPlatformPath(m_moduleFilePath.getBuffer(), handle)))
        {
            return SLANG_FAIL;
        }

        // The shared library needs to keep temp files in scope
        ComPtr<ISlangSharedLibrary> lib(new ScopeSharedLibrary(handle, m_artifactList));
        *outCastable = lib.detach();
        return SLANG_OK;
    }
    else if (typeGuid == ISlangBlob::getTypeGuid())
    {
        List<uint8_t> contents;
        // Read the binary
            // Read the contents of the binary
        SLANG_RETURN_ON_FAIL(File::readAllBytes(m_moduleFilePath, contents));

        auto blob = ScopeBlob::create(ListBlob::moveCreate(contents), m_artifactList);

        *outCastable = CastableUtil::getCastable(blob).detach();
        return SLANG_OK;
    }

    return SLANG_E_NOT_AVAILABLE;
}

bool CommandLineDownstreamArtifactRepresentation::exists()
{
    return File::exists(m_moduleFilePath);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CommandLineDownstreamCompiler !!!!!!!!!!!!!!!!!!!!!!*/

static bool _isContentsInFile(const DownstreamCompileOptions& options)
{
    if (options.sourceContentsPath.getLength() <= 0)
    {
        return false;
    }

    // We can see if we can load it
    if (File::exists(options.sourceContentsPath))
    {
        // Here we look for the file on the regular file system (as opposed to using the 
        // ISlangFileSystem. This is unfortunate but necessary - because when we call out
        // to the compiler all it is able to (currently) see are files on the file system.
        //
        // Note that it could be coincidence that the filesystem has a file that's identical in
        // contents/name. That being the case though, any includes wouldn't work for a generated
        // file either from some specialized ISlangFileSystem, so this is probably as good as it gets
        // until we can integrate directly to a C/C++ compiler through say a shared library where we can control
        // file system access.
        String readContents;

        if (SLANG_SUCCEEDED(File::readAllText(options.sourceContentsPath, readContents)))
        {
            return options.sourceContents == readContents.getUnownedSlice();
        }
    }
    return false;
}

SlangResult CommandLineDownstreamCompiler::compile(const CompileOptions& inOptions, IArtifact** outArtifact)
{
    // Copy the command line options
    CommandLine cmdLine(m_cmdLine);

    CompileOptions options(inOptions);

    auto helper = DefaultArtifactHelper::getSingleton();

    // Find all the files that will be produced

    auto artifactList = ArtifactList::create();

    ComPtr<IFileArtifactRepresentation> lockFile;

    if (options.modulePath.getLength() == 0 || options.sourceContents.getLength() != 0)
    {
        

        String modulePath = options.modulePath;

        // If there is no module path, generate one.
        if (modulePath.getLength() == 0)
        {
            SLANG_RETURN_ON_FAIL(helper->createLockFile("slang-generated", nullptr, lockFile.writeRef()));

            auto lockArtifact = Artifact::create(ArtifactDesc::make(ArtifactKind::Base, ArtifactPayload::Lock, ArtifactStyle::None));
            lockArtifact->addRepresentation(lockFile);

            artifactList->add(lockArtifact);

            modulePath = lockFile->getPath();
            options.modulePath = modulePath;
        }

        if (_isContentsInFile(options))
        {
            options.sourceFiles.add(options.sourceContentsPath);
        }
        else
        {
            String compileSourcePath = modulePath;

            compileSourcePath.append("-src");

            // Make the temporary filename have the appropriate extension.
            if (options.sourceLanguage == SLANG_SOURCE_LANGUAGE_C)
            {
                compileSourcePath.append(".c");
            }
            else
            {
                compileSourcePath.append(".cpp");
            }

            // Write it out
            SLANG_RETURN_ON_FAIL(File::writeAllText(compileSourcePath, options.sourceContents));
            
            // Create the reference to the file 
            auto fileRep = FileArtifactRepresentation::create(IFileArtifactRepresentation::Kind::Owned, compileSourcePath.getUnownedSlice(), lockFile, nullptr);
            auto fileArtifact = ArtifactUtil::createArtifact(ArtifactDescUtil::makeDescForSourceLanguage(options.sourceLanguage));
            fileArtifact->addRepresentation(fileRep);

            artifactList->add(fileArtifact);

            // Add it as a source file
            options.sourceFiles.add(compileSourcePath);
        }

        // There is no source contents
        options.sourceContents = String();
        options.sourceContentsPath = String();
    }

    // Append command line args to the end of cmdLine using the target specific function for the specified options
    SLANG_RETURN_ON_FAIL(calcArgs(options, cmdLine));

    String moduleFilePath;

    {
        StringBuilder builder;
        SLANG_RETURN_ON_FAIL(calcModuleFilePath(options, builder));
        moduleFilePath = builder.ProduceString();
    }

    {
        // TODO(JS):
        // We should make this output artifacts(!)
        List<String> paths;
        SLANG_RETURN_ON_FAIL(calcCompileProducts(options, DownstreamProductFlag::All, paths));

        for (const auto& path : paths)
        {
            auto fileRep = FileArtifactRepresentation::create(IFileArtifactRepresentation::Kind::Owned, path.getUnownedSlice(), lockFile, nullptr);
            auto artifact = ArtifactUtil::createArtifact(ArtifactDesc::make(ArtifactKind::Unknown, ArtifactPayload::Unknown, ArtifactStyle::Unknown));
            artifact->addRepresentation(fileRep);
            artifactList->add(artifact);
        }
    }

    ExecuteResult exeRes;

#if 0
    // Test
    {
        String line = ProcessUtil::getCommandLineString(cmdLine);
        printf("%s", line.getBuffer());
    }
#endif

    SLANG_RETURN_ON_FAIL(ProcessUtil::execute(cmdLine, exeRes));

#if 0
    {
        printf("stdout=\"%s\"\nstderr=\"%s\"\nret=%d\n", exeRes.standardOutput.getBuffer(), exeRes.standardError.getBuffer(), int(exeRes.resultCode));
    }
#endif

    auto artifact = ArtifactUtil::createArtifactForCompileTarget(options.targetType);

    auto diagnostics = ArtifactDiagnostics::create();

    SLANG_RETURN_ON_FAIL(parseOutput(exeRes, diagnostics));

    // Add the artifact
    artifact->addAssociated(diagnostics);

    ComPtr<IArtifactRepresentation> rep(new CommandLineDownstreamArtifactRepresentation(moduleFilePath.getUnownedSlice(), artifactList));
    artifact->addRepresentation(rep);
    artifact->addAssociated(artifactList);

    *outArtifact = artifact.detach();
    
    return SLANG_OK;
}

}
