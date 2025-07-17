// slang-serialize-container.cpp
#include "slang-serialize-container.h"

#include "../core/slang-byte-encode-util.h"
#include "../core/slang-math.h"
#include "../core/slang-stream.h"
#include "../core/slang-text-io.h"
#include "slang-check-impl.h"
#include "slang-compiler.h"
#include "slang-mangled-lexer.h"
#include "slang-parser.h"
#include "slang-serialize-ast.h"
#include "slang-serialize-ir.h"
#include "slang-serialize-source-loc.h"
#include "slang-serialize-types.h"

namespace Slang
{
struct ModuleEncodingContext
{
private:
    Stream* _stream = nullptr;

    // The string pool used across the whole of the container
    StringSlicePool _containerStringPool;
    RefPtr<SerialSourceLocWriter> _sourceLocWriter;

    RIFF::Builder _riff;
    RIFF::BuildCursor _cursor;

public:
    ModuleEncodingContext(SerialContainerUtil::WriteOptions const& options, Stream* stream)
        : _stream(stream), _containerStringPool(StringSlicePool::Style::Default)
    {
        if (options.sourceManagerToUseWhenSerializingSourceLocs)
        {
            _sourceLocWriter =
                new SerialSourceLocWriter(options.sourceManagerToUseWhenSerializingSourceLocs);
        }

        _cursor = RIFF::BuildCursor(_riff);
    }

    ~ModuleEncodingContext()
    {
        _cursor = RIFF::BuildCursor(_riff.getRootChunk());
        encodeFinalPieces();
        _riff.writeTo(_stream);
    }

    SlangResult encodeModuleList(FrontEndCompileRequest* frontEndReq)
    {
        // Encoding a front-end compile request into a RIFF
        // is simply a matter of encoding the module for each
        // of the translation units that got compiled.
        //
        SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(_cursor, SerialBinary::kModuleListFourCc);
        for (TranslationUnitRequest* translationUnit : frontEndReq->translationUnits)
        {
            SLANG_RETURN_ON_FAIL(encode(translationUnit->module));
        }
        return SLANG_OK;
    }

    SlangResult encode(FrontEndCompileRequest* frontEndReq)
    {
        SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(_cursor, SerialBinary::kContainerFourCc);
        SLANG_RETURN_ON_FAIL(encodeModuleList(frontEndReq));
        return SLANG_OK;
    }

    SlangResult encode(EndToEndCompileRequest* request)
    {
        SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(_cursor, SerialBinary::kContainerFourCc);

        // Encoding an end-to-end compile request starts with the same
        // work as for a front-end request: we encode each of
        // the modules for the translation units.
        //
        SLANG_RETURN_ON_FAIL(encodeModuleList(request->getFrontEndReq()));
        //
        // If code generation is disabled, then we can skip all further
        // steps, and the encoding process is no different
        // than for a front-end request.
        //
        if (request->getOptionSet().getBoolOption(CompilerOptionName::SkipCodeGen))
        {
            return SLANG_OK;
        }

        // If code generation is enabled, then we need to encode
        // information on each of the code generation targets, as well
        // as the entry points.
        //
        // We start with the targets, each of which will have a Slang IR
        // representation of the layout information for the program
        // on that target.
        //
        auto linkage = request->getLinkage();
        auto sink = request->getSink();
        auto program = request->getSpecializedGlobalAndEntryPointsComponentType();
        {
            SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(_cursor, SerialBinary::kArrayFourCC);

            for (auto target : linkage->targets)
            {
                auto targetProgram = program->getTargetProgram(target);
                encode(targetProgram, sink);
            }
        }

        // The compiled `program` may also have zero or more entry points,
        // and we need to encode information about each of them.
        //
        {
            SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(_cursor, SerialBinary::kEntryPointListFourCc);

            auto entryPointCount = program->getEntryPointCount();
            for (Index ii = 0; ii < entryPointCount; ++ii)
            {
                auto entryPoint = program->getEntryPoint(ii);
                auto entryPointMangledName = program->getEntryPointMangledName(ii);
                encode(entryPoint, entryPointMangledName);
            }
        }

        return SLANG_OK;
    }

    SlangResult encode(TargetProgram* targetProgram, DiagnosticSink* sink)
    {
        // TODO:
        // Serialization of target component IR is causing the embedded precompiled binary
        // feature to fail. The resulting data modules contain both TU IR and TC IR, with only
        // one module header. Yong suggested to ignore the TC IR for now, though also that
        // OV was using the feature, so disabling this might cause problems.

        IRModule* irModule = targetProgram->getOrCreateIRModuleForLayout(sink);

        // Okay, we need to serialize this target program and its IR too...
        {
            SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(_cursor, PropertyKeys<IRModule>::IRModule);
            writeSerializedModuleIR(_cursor, irModule, _sourceLocWriter);
        }

        return SLANG_OK;
    }

    void encodeData(void const* data, size_t size, FourCC type)
    {
        _cursor.addDataChunk(type, data, size);
    }

    void encode(String const& value, FourCC type = SerialBinary::kStringFourCC)
    {
        encodeData(value.getBuffer(), value.getLength(), type);
    }

    void encode(Name* name, FourCC type = SerialBinary::kNameFourCC) { encode(name->text, type); }


    void encode(uint32_t value, FourCC type = SerialBinary::kUInt32FourCC)
    {
        encodeData(&value, sizeof(value), type);
    }

    SlangResult encode(EntryPoint* entryPoint, String const& entryPointMangledName)
    {
        SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(_cursor, SerialBinary::kEntryPointFourCc);

        {
            encode(entryPoint->getName(), SerialBinary::kNameFourCC);
        }
        {
            encode(entryPoint->getProfile().raw, SerialBinary::kProfileFourCC);
        }
        {
            encode(entryPointMangledName, SerialBinary::kMangledNameFourCC);
        }

        return SLANG_OK;
    }


    SlangResult encode(Module* module)
    {
        SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(_cursor, SerialBinary::kModuleFourCC);

        // The first piece that we write for a module is its header.
        // The header is intended to provide information that can be
        // used to determine if a precompiled module is up-to-date.
        //
        // Update(tfoley): Okay, let's skip the whole header idea and just
        // serialize these things as properties of the module itself...
        {
            // So many things need the module name, that it makes
            // sense to serialize it separately from all the rest.
            //
            {
                encode(module->getNameObj(), SerialBinary::kNameFourCC);
            }

            // The header includes a digest of all the compile options and
            // the files that the compiled result depended on.
            //
            auto digest = module->computeDigest();
            _cursor.addDataChunk(PropertyKeys<Module>::Digest, digest.data, sizeof(digest.data));

            // The header includes an array of the paths of all of the
            // files that the compiled result depended on.
            //
            encodeModuleDependencyPaths(module);
        }

        // If there is IR available for this module, then we we encode it.
        //
        if (auto irModule = module->getIRModule())
        {
            SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(_cursor, PropertyKeys<IRModule>::IRModule);
            writeSerializedModuleIR(_cursor, irModule, _sourceLocWriter);
        }

        // If we have AST information available, then we serialize it here.
        //
        if (auto moduleDecl = module->getModuleDecl())
        {
            SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(_cursor, PropertyKeys<Module>::ASTModule);
            writeSerializedModuleAST(_cursor, moduleDecl, _sourceLocWriter);
        }

        return SLANG_OK;
    }

    SlangResult encodeModuleDependencyPaths(Module* module)
    {
        SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(_cursor, PropertyKeys<Module>::FileDependencies);

        // TODO(tfoley): This is some of the most complicated logic
        // in the encoding system, because it tries to translate
        // the file dependency paths into something that isn't
        // specific to the machine on which a module was built.
        //
        // The comments that follow are from the original implementation
        // of this logic, because I cannot state with confidence
        // that I know what's happening in all of this.


        // Here we assume that the first file in the file dependencies is the module's file path.
        // We store the module's file path as a relative path with respect to the first search
        // directory that contains the module, and store the paths of dependent files as relative
        // paths with respect to the module's path.

        // First, we try to extract the module's main file path from the file dependency list.
        auto fileDependencies = module->getFileDependencies();
        String canonicalModulePath, moduleDir;
        if (fileDependencies.getCount() != 0)
        {
            IncludeSystem includeSystem(
                &module->getLinkage()->getSearchDirectories(),
                module->getLinkage()->getFileSystemExt(),
                module->getLinkage()->getSourceManager());
            PathInfo outFoundSourcePath;
            // If we can find the first file, use that as the module's path.
            if (SLANG_SUCCEEDED(includeSystem.findFile(
                    fileDependencies[0]->getPathInfo().foundPath,
                    "",
                    outFoundSourcePath)))
            {
                canonicalModulePath = outFoundSourcePath.foundPath;
                Path::getCanonical(canonicalModulePath, canonicalModulePath);
                moduleDir = Path::getParentDirectory(canonicalModulePath);
            }
        }

        // If we can't find the module's path from the file dependencies list above,
        // use the file path stored on the module as a fallback.
        // Note that if the module is loaded from a binary precompiled module,
        // this path will be pointing to that binary file instead of the original source file.
        if (!canonicalModulePath.getLength())
        {
            if (auto modulePath = module->getFilePath())
            {
                canonicalModulePath = modulePath;
                Path::getCanonical(canonicalModulePath, canonicalModulePath);
                moduleDir = Path::getParentDirectory(canonicalModulePath);
            }
        }

        // Find the first search directory that contains the module's main file path,
        // so we can store the module's path (the first entry in the dependent files list)
        // as a relative path with respect to that directory.

        String linkageRoot = ".";
        if (auto linkage = module->getLinkage())
        {
            for (const auto& searchDir : linkage->getSearchDirectories().searchDirectories)
            {
                String fullSearchDir;
                Path::getCanonical(searchDir.path, fullSearchDir);
                String relativePath = Path::getRelativePath(fullSearchDir, canonicalModulePath);
                if (!Path::hasRelativeElement(relativePath))
                {
                    linkageRoot = searchDir.path;
                    break;
                }
            }
        }
        Path::getCanonical(linkageRoot, linkageRoot);

        for (auto file : fileDependencies)
        {
            if (file->getPathInfo().hasFoundPath())
            {
                String canonicalFilePath = file->getPathInfo().foundPath;
                Path::getCanonical(file->getPathInfo().foundPath, canonicalFilePath);

                // If the dependnet file is the same as the module's file path, store it as a
                // relative path with respect to the search directory discovered above.
                if (file->getPathInfo().hasFileFoundPath())
                {
                    if (canonicalFilePath == canonicalModulePath)
                    {
                        auto relativeModulePath =
                            Path::getRelativePath(linkageRoot, canonicalModulePath);

                        encode(relativeModulePath);
                    }
                    else
                    {
                        // For all other dependnet files, store them as relative paths with respect
                        // to the module's path.
                        canonicalFilePath = Path::getRelativePath(moduleDir, canonicalFilePath);
                        encode(canonicalFilePath);
                    }
                }
                else
                {
                    // If the module is coming from string instead of an actual file, store it as
                    // is.
                    encode(canonicalModulePath);
                }
            }
            else
            {
                encode(file->getPathInfo().getMostUniqueIdentity());
            }
        }

        return SLANG_OK;
    }

    SlangResult encodeFinalPieces()
    {
        // We can now output the debug information. This is for all IR and AST
        if (_sourceLocWriter)
        {
            // Write out the debug info
            SerialSourceLocData debugData;
            _sourceLocWriter->write(&debugData);

            debugData.writeTo(_cursor);
        }

        // Write the container string table
        if (_containerStringPool.getAdded().getCount() > 0)
        {
            List<char> encodedTable;
            SerialStringTableUtil::encodeStringTable(_containerStringPool, encodedTable);

            _cursor.addDataChunk(
                SerialBinary::kStringTableFourCc,
                encodedTable.getBuffer(),
                encodedTable.getCount());
        }

        return SLANG_OK;
    }
};

//
// To serialize a module (or compile request) to a stream, we first
// construct a RIFF container from it, and then serialize that
// container out to a byte stream.
//

/* static */ SlangResult SerialContainerUtil::write(
    Module* module,
    const WriteOptions& options,
    Stream* stream)
{
    ModuleEncodingContext context(options, stream);
    SLANG_RETURN_ON_FAIL(context.encode(module));
    return SLANG_OK;
}

/* static */ SlangResult SerialContainerUtil::write(
    FrontEndCompileRequest* request,
    const WriteOptions& options,
    Stream* stream)
{
    ModuleEncodingContext context(options, stream);
    SLANG_RETURN_ON_FAIL(context.encode(request));
    return SLANG_OK;
}

/* static */ SlangResult SerialContainerUtil::write(
    EndToEndCompileRequest* request,
    const WriteOptions& options,
    Stream* stream)
{
    ModuleEncodingContext context(options, stream);
    SLANG_RETURN_ON_FAIL(context.encode(request));
    return SLANG_OK;
}

String StringChunk::getValue() const
{
    return String(UnownedStringSlice((char const*)getPayload(), getPayloadSize()));
}

RIFF::ChunkList<StringChunk> ModuleChunk::getFileDependencies() const
{
    auto found = findListChunk(PropertyKeys<Module>::FileDependencies);
    if (!found)
        return RIFF::ChunkList<StringChunk>();
    return found->getChildren().cast<StringChunk>();
}

ModuleChunk const* ModuleChunk::find(RIFF::ListChunk const* baseChunk)
{
    auto found = baseChunk->findListChunkRec(SerialBinary::kModuleFourCC);
    return static_cast<ModuleChunk const*>(found);
}

SHA1::Digest ModuleChunk::getDigest() const
{
    auto foundChunk = findDataChunk(PropertyKeys<Module>::Digest);
    if (!foundChunk)
    {
        SLANG_UNEXPECTED("module chunk had no digest");
    }
    return foundChunk->readPayloadAs<SHA1::Digest>();
}

String ModuleChunk::getName() const
{
    auto found = findDataChunk(SerialBinary::kNameFourCC);
    if (!found)
    {
        SLANG_UNEXPECTED("module chunk had no name");
    }
    return static_cast<StringChunk const*>(found)->getValue();
}


IRModuleChunk const* ModuleChunk::findIR() const
{
    auto foundProperty = findListChunk(PropertyKeys<IRModule>::IRModule);
    if (!foundProperty)
        return nullptr;

    return static_cast<IRModuleChunk const*>(foundProperty->getFirstChild().get());
}

ASTModuleChunk const* ModuleChunk::findAST() const
{
    auto foundProperty = findListChunk(PropertyKeys<Module>::ASTModule);
    if (!foundProperty)
        return nullptr;

    return static_cast<ASTModuleChunk const*>(foundProperty->getFirstChild().get());
}

ContainerChunk const* ContainerChunk::find(RIFF::ListChunk const* baseChunk)
{
    auto found = baseChunk->findListChunkRec(SerialBinary::kContainerFourCc);
    return static_cast<ContainerChunk const*>(found);
}

RIFF::ChunkList<ModuleChunk> ContainerChunk::getModules() const
{
    auto found = findListChunk(SerialBinary::kModuleListFourCc);
    if (!found)
        return RIFF::ChunkList<ModuleChunk>();
    return found->getChildren().cast<ModuleChunk>();
}

RIFF::ChunkList<EntryPointChunk> ContainerChunk::getEntryPoints() const
{
    auto found = findListChunk(SerialBinary::kEntryPointListFourCc);
    if (!found)
        return RIFF::ChunkList<EntryPointChunk>();
    return found->getChildren().cast<EntryPointChunk>();
}

String EntryPointChunk::getMangledName() const
{
    auto found = findDataChunk(SerialBinary::kMangledNameFourCC);
    if (!found)
    {
        SLANG_UNEXPECTED("entry point chunk had no mangled name");
    }
    return static_cast<StringChunk const*>(found)->getValue();
}

String EntryPointChunk::getName() const
{
    auto found = findDataChunk(SerialBinary::kNameFourCC);
    if (!found)
    {
        SLANG_UNEXPECTED("entry point chunk had no name");
    }
    return static_cast<StringChunk const*>(found)->getValue();
}

Profile EntryPointChunk::getProfile() const
{
    auto found = findDataChunk(SerialBinary::kProfileFourCC);
    if (!found)
    {
        SLANG_UNEXPECTED("entry point chunk had no profile");
    }
    auto rawVal = found->readPayloadAs<Profile::RawVal>();
    return Profile(rawVal);
}


DebugChunk const* DebugChunk::find(RIFF::ListChunk const* baseChunk)
{
    auto found = baseChunk->findListChunkRec(SerialSourceLocData::kDebugFourCc);
    return static_cast<DebugChunk const*>(found);
}

DebugChunk const* DebugChunk::find(
    RIFF::ListChunk const* baseChunk,
    RIFF::ListChunk const* containerChunk)
{
    if (auto found = find(baseChunk))
        return found;
    if (containerChunk)
        return find(containerChunk);
    return nullptr;
}

SlangResult readSourceLocationsFromDebugChunk(
    DebugChunk const* debugChunk,
    SourceManager* sourceManager,
    RefPtr<SerialSourceLocReader>& outReader)
{
    if (!debugChunk)
        return SLANG_FAIL;

    // Source location serialization uses the old approach where
    // there is an intermediate in-memory data structure that the
    // raw data from the RIFF gets deserialized into, before that
    // intermediate representation gets transformed into something
    // more directly usable.
    //
    // Thus we start with a first step where we simply read the data
    // from the RIFF into the intermediate structure.
    //
    SerialSourceLocData intermediateData;
    SLANG_RETURN_ON_FAIL(intermediateData.readFrom(debugChunk));

    // After reading the data into the intermediate representation,
    // we turn it into a `SerialSourceLocReader`, which vends source
    // location information to other deserialization tasks (both IR
    // and AST deserialization).
    //
    auto reader = RefPtr(new SerialSourceLocReader());
    SLANG_RETURN_ON_FAIL(reader->read(&intermediateData, sourceManager));

    outReader = reader;
    return SLANG_OK;
}

static void calcModuleInstructionList(IRModule* module, List<IRInst*>& instsOut)
{
    // We reserve 0 for null
    instsOut.setCount(1);
    instsOut[0] = nullptr;

    IRModuleInst* moduleInst = module->getModuleInst();

    traverseInstsInSerializationOrder(moduleInst, [&](IRInst* inst) { instsOut.add(inst); });
}

/* static */ SlangResult SerialContainerUtil::verifyIRSerialize(
    IRModule* module,
    Session* session,
    const WriteOptions& options)
{
    // Verify if we can stream out with raw source locs

    List<IRInst*> originalInsts;
    calcModuleInstructionList(module, originalInsts);

    OwnedMemoryStream memoryStream(FileAccess::ReadWrite);

    {
        RIFF::Builder builder;
        RIFF::BuildCursor cursor(builder);

        // Need to put all of this in a module chunk
        SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(cursor, SerialBinary::kModuleFourCC);
        RefPtr<SerialSourceLocWriter> sourceLocWriter;

        if (options.sourceManagerToUseWhenSerializingSourceLocs)
        {
            sourceLocWriter =
                new SerialSourceLocWriter(options.sourceManagerToUseWhenSerializingSourceLocs);
        }

        {
            SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(cursor, PropertyKeys<IRModule>::IRModule);
            writeSerializedModuleIR(cursor, module, sourceLocWriter);
        }

        // Write the debug info Riff container
        if (sourceLocWriter)
        {
            SerialSourceLocData serialData;
            sourceLocWriter->write(&serialData);

            SLANG_RETURN_ON_FAIL(serialData.writeTo(cursor));
        }

        SLANG_RETURN_ON_FAIL(builder.writeTo(&memoryStream));
    }

    // Reset stream
    memoryStream.seek(SeekOrigin::Start, 0);

    SourceManager workSourceManager;
    workSourceManager.initialize(options.sourceManagerToUseWhenSerializingSourceLocs, nullptr);

    // The read ir module
    RefPtr<IRModule> irReadModule;
    {
        auto streamContents = memoryStream.getContents();

        auto rootChunk =
            RIFF::RootChunk::getFromBlob(streamContents.getBuffer(), streamContents.getCount());
        if (!rootChunk)
        {
            return SLANG_FAIL;
        }

        auto moduleChunk = ModuleChunk::find(rootChunk);
        if (!moduleChunk)
        {
            return SLANG_FAIL;
        }

        RefPtr<SerialSourceLocReader> sourceLocReader;

        // If we have debug info then find and read it
        if (options.sourceManagerToUseWhenSerializingSourceLocs)
        {
            auto debugChunk = DebugChunk::find(moduleChunk);
            if (!debugChunk)
            {
                return SLANG_FAIL;
            }
            SerialSourceLocData sourceLocData;
            SLANG_RETURN_ON_FAIL(sourceLocData.readFrom(debugChunk));

            sourceLocReader = new SerialSourceLocReader;
            SLANG_RETURN_ON_FAIL(sourceLocReader->read(&sourceLocData, &workSourceManager));
        }

        {
            auto irChunk = moduleChunk->findIR();
            if (!irChunk)
            {
                return SLANG_FAIL;
            }

            SLANG_RETURN_ON_FAIL(
                readSerializedModuleIR(irChunk, session, sourceLocReader, irReadModule));
        }
    }

    List<IRInst*> readInsts;
    calcModuleInstructionList(irReadModule, readInsts);

    if (readInsts.getCount() != originalInsts.getCount())
    {
        SLANG_ASSERT(!"Instruction counts don't match");
        return SLANG_FAIL;
    }

    if (options.sourceManagerToUseWhenSerializingSourceLocs)
    {
        // They should be on the same line nos
        for (Index i = 1; i < readInsts.getCount(); ++i)
        {
            IRInst* origInst = originalInsts[i];
            IRInst* readInst = readInsts[i];

            if (origInst->sourceLoc.getRaw() == readInst->sourceLoc.getRaw())
            {
                continue;
            }

            // Work out the
            SourceView* origSourceView =
                options.sourceManagerToUseWhenSerializingSourceLocs->findSourceView(
                    origInst->sourceLoc);
            SourceView* readSourceView = workSourceManager.findSourceView(readInst->sourceLoc);

            // if both are null we are done
            if (origSourceView == nullptr && origSourceView == readSourceView)
            {
                continue;
            }
            SLANG_ASSERT(origSourceView && readSourceView);

            // The offset should be the same
            Index origOffset =
                origInst->sourceLoc.getRaw() - origSourceView->getRange().begin.getRaw();
            Index readOffset =
                readInst->sourceLoc.getRaw() - readSourceView->getRange().begin.getRaw();

            if (origOffset != readOffset)
            {
                SLANG_ASSERT(!"SourceLoc offset debug data didn't match");
                return SLANG_FAIL;
            }

            {
                auto origInfo =
                    origSourceView->getHumaneLoc(origInst->sourceLoc, SourceLocType::Actual);
                auto readInfo =
                    readSourceView->getHumaneLoc(readInst->sourceLoc, SourceLocType::Actual);

                if (!(origInfo.line == readInfo.line && origInfo.column == readInfo.column &&
                      origInfo.pathInfo.foundPath == readInfo.pathInfo.foundPath))
                {
                    SLANG_ASSERT(!"Debug data didn't match");
                    return SLANG_FAIL;
                }
            }

            // We may have adjusted line numbers -> but they may not match, because we only
            // reconstruct one view So for now disable this test

            if (false)
            {
                auto origInfo =
                    origSourceView->getHumaneLoc(origInst->sourceLoc, SourceLocType::Nominal);
                auto readInfo =
                    readSourceView->getHumaneLoc(readInst->sourceLoc, SourceLocType::Nominal);

                if (!(origInfo.line == readInfo.line && origInfo.column == readInfo.column &&
                      origInfo.pathInfo.foundPath == readInfo.pathInfo.foundPath))
                {
                    SLANG_ASSERT(!"Debug data didn't match");
                    return SLANG_FAIL;
                }
            }
        }
    }

    return SLANG_OK;
}

} // namespace Slang
