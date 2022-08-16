#include "slang-artifact-output-util.h"

#include "../core/slang-platform.h"

#include "../core/slang-io.h"
#include "../core/slang-string-util.h"
#include "../core/slang-hex-dump-util.h"

#include "../core/slang-type-text-util.h"

// Artifact
#include "../compiler-core/slang-artifact-desc-util.h"
#include "../compiler-core/slang-artifact-util.h"

#include "slang-compiler.h"

namespace Slang
{

// Given a desc returns a codegen target if it can be used for disassembly
// Returns Unknown if cannot be used for generating disassembly
//
// NOTE! This returns the code gen target for the input binary, *not* the dissassembly output
static CodeGenTarget _getDisassemblyCodeGenTarget(const ArtifactDesc& desc)
{
    switch (desc.payload)
    {
        case ArtifactPayload::DXIL:     return CodeGenTarget::DXIL;
        case ArtifactPayload::DXBC:     return CodeGenTarget::DXBytecode; 
        case ArtifactPayload::SPIRV:    return CodeGenTarget::SPIRV; 
        default: break;
    }
    return CodeGenTarget::Unknown;
}

/* static */SlangResult ArtifactOutputUtil::dissassembleWithDownstream(Session* session, IArtifact* artifact, DiagnosticSink* sink, IArtifact** outArtifact)
{
    auto desc = artifact->getDesc();

    // Get the downstream compiler that can be used for this target
    // TODO(JS):
    // This could perhaps be performed in some other manner if there was more than one way to produce
    // disassembly from a binary.

    const CodeGenTarget target = _getDisassemblyCodeGenTarget(desc);
    if (target == CodeGenTarget::Unknown)
    {
        return SLANG_FAIL;
    }

    auto downstreamCompiler = getDownstreamCompilerRequiredForTarget(target);

    // Get the required downstream compiler
    DownstreamCompiler* compiler = session->getOrLoadDownstreamCompiler(downstreamCompiler, sink);

    if (!compiler)
    {
        if (sink)
        {
            auto compilerName = TypeTextUtil::getPassThroughAsHumanText((SlangPassThrough)downstreamCompiler);
            sink->diagnose(SourceLoc(), Diagnostics::passThroughCompilerNotFound, compilerName);
        }
        return SLANG_FAIL;
    }

    ComPtr<ISlangBlob> blob;
    SLANG_RETURN_ON_FAIL(artifact->loadBlob(ArtifactKeep::No, blob.writeRef()));

    const auto data = blob->getBufferPointer();
    const auto dataSizeInBytes = blob->getBufferSize();

    ComPtr<ISlangBlob> dissassemblyBlob;
    SLANG_RETURN_ON_FAIL(compiler->disassemble(SlangCompileTarget(target), data, dataSizeInBytes, dissassemblyBlob.writeRef()));

    ArtifactDesc disassemblyDesc(desc);
    disassemblyDesc.kind = ArtifactKind::Assembly;

    auto disassemblyArtifact = ArtifactUtil::createArtifact(disassemblyDesc);
    disassemblyArtifact->addRepresentationUnknown(dissassemblyBlob);

    *outArtifact = disassemblyArtifact.detach();
    return SLANG_OK;
}

SlangResult ArtifactOutputUtil::maybeDisassemble(Session* session, IArtifact* artifact, DiagnosticSink* sink, ComPtr<IArtifact>& outArtifact)
{
    const auto desc = artifact->getDesc();
    if (ArtifactDescUtil::isText(artifact->getDesc()))
    {
        // Nothing to convert
        return SLANG_OK;
    }

    if (_getDisassemblyCodeGenTarget(desc) != CodeGenTarget::Unknown)
    {
        // Get the blob
        ComPtr<ISlangBlob> blob;
        SLANG_RETURN_ON_FAIL(artifact->loadBlob(ArtifactKeep::No, blob.writeRef()));

        ComPtr<IArtifact> disassemblyArtifact;

        if (SLANG_SUCCEEDED(dissassembleWithDownstream(session, artifact, sink, disassemblyArtifact.writeRef())))
        {
            // Check it is now text
            SLANG_ASSERT(ArtifactDescUtil::isText(disassemblyArtifact->getDesc()));

            outArtifact.swap(disassemblyArtifact);
            return SLANG_OK;
        }
    }

    return SLANG_OK;
}

/* static */SlangResult ArtifactOutputUtil::write(const ArtifactDesc& desc, ISlangBlob* blob, ISlangWriter* writer)
{
    // If is text, we can just output
    if (ArtifactDescUtil::isText(desc))
    {
        return writer->write((const char*)blob->getBufferPointer(), blob->getBufferSize());
    }
    else
    {
        if (writer->isConsole())
        {
            // Else just dump as text
            return HexDumpUtil::dumpWithMarkers((const uint8_t*)blob->getBufferPointer(), blob->getBufferSize(), 24, writer);
        }
        else
        {
            // Redirecting stdout to a file, so do the usual thing
            writer->setMode(SLANG_WRITER_MODE_BINARY);
            return writer->write((const char*)blob->getBufferPointer(), blob->getBufferSize());
        }
    }
}

/* static */SlangResult ArtifactOutputUtil::write(IArtifact* artifact, ISlangWriter* writer)
{
    ComPtr<ISlangBlob> blob;
    SLANG_RETURN_ON_FAIL(artifact->loadBlob(ArtifactKeep::No, blob.writeRef()));
    return write(artifact->getDesc(), blob, writer);
}

static SlangResult _requireBlob(IArtifact* artifact, DiagnosticSink* sink, ComPtr<ISlangBlob>& outBlob)
{
    const auto res = artifact->loadBlob(ArtifactKeep::No, outBlob.writeRef());
    if (SLANG_FAILED(res))
    {
        sink->diagnose(SourceLoc(), Diagnostics::cannotAccessAsBlob);
        return res;
    }
    return SLANG_OK;
}

/* static */SlangResult ArtifactOutputUtil::write(IArtifact* artifact, DiagnosticSink* sink, const UnownedStringSlice& writerName, ISlangWriter* writer)
{
    if (sink == nullptr)
    {
        return write(artifact, writer);
    }

    // Make sure we can access as a blob
    ComPtr<ISlangBlob> blob;
    SLANG_RETURN_ON_FAIL(_requireBlob(artifact, sink, blob));

    const auto res = write(artifact->getDesc(), blob, writer);
    if (SLANG_FAILED(res))
    {
        sink->diagnose(SourceLoc(), Diagnostics::cannotWriteOutputFile, writerName);
    }
    return res;
}

/* static */SlangResult ArtifactOutputUtil::maybeConvertAndWrite(Session* session, IArtifact* artifact, DiagnosticSink* sink, const UnownedStringSlice& writerName, ISlangWriter* writer)
{
    // If the output is console we will try and turn into disassembly
    if (writer->isConsole())
    {
        ComPtr<IArtifact> disassemblyArtifact;
        maybeDisassemble(session, artifact, sink, disassemblyArtifact);

        if (disassemblyArtifact)
        {
            return write(disassemblyArtifact, sink, writerName, writer);
        }
    }

    return write(artifact, sink, writerName, writer);
}

/* static */SlangResult ArtifactOutputUtil::writeToFile(const ArtifactDesc& desc, const void* data, size_t size, const String& path)
{
    if (ArtifactDescUtil::isText(desc))
    {
        return File::writeNativeText(path, data, size);
    }
    else
    {
        return File::writeAllBytes(path, data, size);
    }
}

/* static */SlangResult ArtifactOutputUtil::writeToFile(const ArtifactDesc& desc, ISlangBlob* blob, const String& path)
{
    SLANG_RETURN_ON_FAIL(writeToFile(desc, blob->getBufferPointer(), blob->getBufferSize(), path));
    return SLANG_OK;
}

/* static */SlangResult ArtifactOutputUtil::writeToFile(IArtifact* artifact, const String& path)
{
    // Get the blob
    ComPtr<ISlangBlob> blob;
    SLANG_RETURN_ON_FAIL(artifact->loadBlob(ArtifactKeep::No, blob.writeRef()));
    return writeToFile(artifact->getDesc(), blob, path);
}
 
/* static */SlangResult ArtifactOutputUtil::writeToFile(IArtifact* artifact, DiagnosticSink* sink, const String& path)
{
    if (!sink)
    {
        return writeToFile(artifact, path);
    }

    ComPtr<ISlangBlob> blob;
    SLANG_RETURN_ON_FAIL(_requireBlob(artifact, sink, blob));

    const auto res = writeToFile(artifact, path);
    if (SLANG_FAILED(res) && sink)
    {
        sink->diagnose(SourceLoc(), Diagnostics::cannotWriteOutputFile, path);
    }

    return res;
}

}
