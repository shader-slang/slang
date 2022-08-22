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

#include "slang-artifact-associated-impl.h"
#include "slang-artifact-util.h"

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
        auto temporarySharedLibrary = new TemporarySharedLibrary(handle, m_moduleFilePath);
        ComPtr<ISlangSharedLibrary> lib(temporarySharedLibrary);

        // Set any additional info on the non COM pointer
        temporarySharedLibrary->m_temporaryFileSet = m_temporaryFiles;

        *outCastable = lib.detach();
        return SLANG_OK;
    }
    else if (typeGuid == ISlangBlob::getTypeGuid())
    {
        List<uint8_t> contents;
        // Read the binary
            // Read the contents of the binary
        SLANG_RETURN_ON_FAIL(File::readAllBytes(m_moduleFilePath, contents));

        auto blob = ScopeRefObjectBlob::create(ListBlob::moveCreate(contents), m_temporaryFiles);

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

    // Find all the files that will be produced
    RefPtr<TemporaryFileSet> productFileSet(new TemporaryFileSet);
    
    if (options.modulePath.getLength() == 0 || options.sourceContents.getLength() != 0)
    {
        String modulePath = options.modulePath;

        // If there is no module path, generate one.
        if (modulePath.getLength() == 0)
        {
            // Holds the temporary lock path, if a temporary path is used
            String temporaryLockPath;

            // Generate a unique module path name
            SLANG_RETURN_ON_FAIL(File::generateTemporary(UnownedStringSlice::fromLiteral("slang-generated"), temporaryLockPath));
            productFileSet->add(temporaryLockPath);

            modulePath = temporaryLockPath;

            options.modulePath = modulePath;
        }

        if (_isContentsInFile(options))
        {
            options.sourceFiles.add(options.sourceContentsPath);
        }
        else
        {
            String compileSourcePath = modulePath;

            // NOTE: Strictly speaking producing filenames by modifying the generateTemporary path that may introduce a temp filename clash, but in practice is extraordinary unlikely
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
            productFileSet->add(compileSourcePath);
            SLANG_RETURN_ON_FAIL(File::writeAllText(compileSourcePath, options.sourceContents));
            
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
        List<String> paths;
        SLANG_RETURN_ON_FAIL(calcCompileProducts(options, DownstreamProductFlag::All, paths));
        productFileSet->add(paths);
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

    ComPtr<IArtifactRepresentation> rep(new CommandLineDownstreamArtifactRepresentation(moduleFilePath, productFileSet));
    artifact->addRepresentation(rep);

    *outArtifact = artifact.detach();
    
    return SLANG_OK;
}

}
