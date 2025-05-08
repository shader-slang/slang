// slang-serialize-container.h
#ifndef SLANG_SERIALIZE_CONTAINER_H
#define SLANG_SERIALIZE_CONTAINER_H

#include "../core/slang-riff.h"
#include "slang-ir-insts.h"
#include "slang-profile.h"
#include "slang-serialize-types.h"

namespace Slang
{

class EndToEndCompileRequest;


struct SerialContainerUtil
{
    struct WriteOptions
    {
        SerialOptionFlags optionFlags =
            SerialOptionFlag::ASTModule |
            SerialOptionFlag::IRModule; ///< Flags controlling what is written
        SourceManager* sourceManager =
            nullptr; ///< The source manager used for the SourceLoc in the input
    };

    struct ReadOptions
    {
        Session* session = nullptr;
        SourceManager* sourceManager = nullptr;
        NamePool* namePool = nullptr;
        SharedASTBuilder* sharedASTBuilder = nullptr;
        ASTBuilder* astBuilder =
            nullptr; // Optional. If not provided will create one in SerialContainerData.
        Linkage* linkage = nullptr;
        DiagnosticSink* sink = nullptr;
        bool readHeaderOnly = false;
        String modulePath;
    };

    /// Verify IR serialization
    static SlangResult verifyIRSerialize(
        IRModule* module,
        Session* session,
        const WriteOptions& options);

    /// Write the request to the stream
    static SlangResult write(
        FrontEndCompileRequest* frontEndReq,
        const WriteOptions& options,
        Stream* stream);
    static SlangResult write(
        EndToEndCompileRequest* request,
        const WriteOptions& options,
        Stream* stream);
    static SlangResult write(Module* module, const WriteOptions& options, Stream* stream);
};

struct StringChunk : RIFF::DataChunk
{
public:
    String getValue() const;
};

struct IRModuleChunk;

struct ASTModuleChunk : RIFF::ListChunk
{};

struct ModuleChunk : RIFF::ListChunk
{
public:
    static ModuleChunk const* find(RIFF::ListChunk const* baseChunk);

    String getName() const;

    IRModuleChunk const* findIR() const;
    ASTModuleChunk const* findAST() const;

    SHA1::Digest getDigest() const;

    RIFF::ChunkList<StringChunk> getFileDependencies() const;
};

struct EntryPointChunk : RIFF::ListChunk
{
public:
    String getMangledName() const;
    String getName() const;
    Profile getProfile() const;
};

struct ContainerChunk : RIFF::ListChunk
{
public:
    static ContainerChunk const* find(RIFF::ListChunk const* baseChunk);

    RIFF::ChunkList<ModuleChunk> getModules() const;

    RIFF::ChunkList<EntryPointChunk> getEntryPoints() const;
};

struct DebugChunk : RIFF::ListChunk
{
public:
    static DebugChunk const* find(RIFF::ListChunk const* baseChunk);
};

SlangResult readSourceLocationsFromDebugChunk(
    DebugChunk const* debugChunk,
    SourceManager* sourceManager,
    RefPtr<SerialSourceLocReader>& outReader);

SlangResult decodeModuleIR(
    RefPtr<IRModule>& outIRModule,
    IRModuleChunk const* chunk,
    Session* session,
    SerialSourceLocReader* sourceLocReader);

} // namespace Slang

#endif
