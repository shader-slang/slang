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

#include "../core/slang-castable-list-impl.h"

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

    auto artifactList = CastableList::create();

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

    // The 'mainArtifact' is the main product produced from the compilation - the executable/sharedlibrary/object etc
    ComPtr<IArtifact> mainArtifact;
    {
        const auto desc = ArtifactDescUtil::makeDescForCompileTarget(options.targetType);

        List<ComPtr<IArtifact>> artifacts;
        SLANG_RETURN_ON_FAIL(calcCompileProducts(options, DownstreamProductFlag::All, lockFile, artifacts));

        for (IArtifact* artifact : artifacts)
        {
            // The main artifact must be in the list, so add it if we find it
            if (artifact->getDesc() == desc)
            {
                SLANG_ASSERT(mainArtifact == nullptr);
                mainArtifact = artifact;
            }

            artifactList->add(artifact);
        }
    }
    
    SLANG_ASSERT(mainArtifact);
    // Somethings gone wrong if we don't find the main artifact
    if (!mainArtifact)
    {
        return SLANG_FAIL;
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

    // Go through the list of artifacts in the artifactList and check if they exist. 
    // 
    // This is useful because `calcCompileProducts` is conservative and may produce artifacts for products that aren't actually 
    // produced, by the compilation.
    {
        Count count = artifactList->getCount();
        for (Index i = 0; i < count; ++i)
        {
            auto artifact = as<IArtifact>(artifactList->getAt(i));

            if (!artifact->exists())
            {
                // We should find a file rep and if we do we can disown it. Disowning will mean
                // when scope is lost the rep won't try and delete the (apparently non existing) backing file.
                if (auto fileRep = findRepresentation<IFileArtifactRepresentation>(artifact))
                {
                    fileRep->disown();
                }

                // If the main artifact doesn't exist, we don't have a main artifact
                if (artifact == mainArtifact)
                {
                    mainArtifact.setNull();
                }

                // Remove from the list
                artifactList->removeAt(i);
                --count;
                --i;
            }
        }
    }

    auto artifact = ArtifactUtil::createArtifactForCompileTarget(options.targetType);

    auto diagnostics = ArtifactDiagnostics::create();

    SLANG_RETURN_ON_FAIL(parseOutput(exeRes, diagnostics));

    // Add the artifact
    artifact->addAssociated(diagnostics);
    
    // Find the rep from the 'main' artifact, we'll just use the same representation on the output 
    // artifact. Sharing is needed, because the rep owns the file.
    if (auto fileRep = mainArtifact ? findRepresentation<IFileArtifactRepresentation>(mainArtifact) : nullptr)
    {
        artifact->addRepresentation(fileRep);
    }

    // Add the artifact list if there is anything in it
    if (artifactList->getCount())
    {
        artifact->addAssociated(artifactList);
    }

    *outArtifact = artifact.detach();
    
    return SLANG_OK;
}

}
