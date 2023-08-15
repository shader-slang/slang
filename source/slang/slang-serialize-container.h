// slang-serialize-container.h
#ifndef SLANG_SERIALIZE_CONTAINER_H
#define SLANG_SERIALIZE_CONTAINER_H

#include "../core/slang-riff.h"
#include "slang-serialize-types.h"
#include "slang-ir-insts.h"
#include "slang-profile.h"

namespace Slang {

class EndToEndCompileRequest;

/* The binary representation actually held in riff/file format*/
struct SerialContainerBinary
{
    struct Target
    {
        uint32_t target;
        uint32_t flags;
        uint32_t profile;
        uint32_t floatingPointMode;
    };

    struct EntryPoint
    {
        uint32_t name;
        uint32_t profile;
        uint32_t mangledName;
    };
};

/* Struct that holds all the data that can be held in a 'container' */
struct SerialContainerData
{
    struct Target
    {
        CodeGenTarget       codeGenTarget = CodeGenTarget::Unknown;
        SlangTargetFlags    flags = kDefaultTargetFlags;
        Profile             profile;
        FloatingPointMode   floatingPointMode = FloatingPointMode::Default;
    };
    
    struct TargetComponent
    {
        // IR module for a specific compilation target
        Target target;
        RefPtr<IRModule> irModule;
    };

    struct Module
    {
        RefPtr<IRModule> irModule;              ///< The IR for the module
        RefPtr<ASTBuilder> astBuilder;          ///< The astBuilder that owns the astRootNode
        NodeBase* astRootNode = nullptr;        ///< The module decl
    };

    struct EntryPoint
    {
        Name* name = nullptr;
        Profile profile;
        String mangledName;
    };

    void clear()
    {
        entryPoints.clear();
        modules.clear();
        targetComponents.clear();
    }

    List<Module> modules;
    List<TargetComponent> targetComponents;
    List<EntryPoint> entryPoints;
};

struct SerialContainerUtil
{
    struct WriteOptions
    {
        SerialCompressionType compressionType = SerialCompressionType::VariableByteLite;                ///< If compression is used what type to use (only some parts can be compressed)
        SerialOptionFlags optionFlags = SerialOptionFlag::ASTModule | SerialOptionFlag::IRModule;       ///< Flags controlling what is written
        SourceManager* sourceManager = nullptr;                                                         ///< The source manager used for the SourceLoc in the input
    };

    struct ReadOptions
    {
        Session* session = nullptr;
        SourceManager* sourceManager = nullptr;
        NamePool* namePool = nullptr;
        SharedASTBuilder* sharedASTBuilder = nullptr;
        ASTBuilder* astBuilder = nullptr; // Optional. If not provided will create one in SerialContainerData.
        Linkage* linkage = nullptr;
        DiagnosticSink* sink = nullptr;
    };

        /// Add module to outData
    static SlangResult addModuleToData(Module* module, const WriteOptions& options, SerialContainerData& outData);

        /// Get the serializable contents of the request as data
    static SlangResult addEndToEndRequestToData(EndToEndCompileRequest* request, const WriteOptions& options, SerialContainerData& outData);

        /// Convert front end request into something serializable
    static SlangResult addFrontEndRequestToData(FrontEndCompileRequest* request, const WriteOptions& options, SerialContainerData& outData);

        /// Write the data into the container
    static SlangResult write(const SerialContainerData& data, const WriteOptions& options, RiffContainer* container);

        /// Read the container into outData
    static SlangResult read(RiffContainer* container, const ReadOptions& options, SerialContainerData& outData);

        /// Verify IR serialization
    static SlangResult verifyIRSerialize(IRModule* module, Session* session, const WriteOptions& options);

        /// Write the request to the stream
    static SlangResult write(FrontEndCompileRequest* frontEndReq, const WriteOptions& options, Stream* stream);
    static SlangResult write(EndToEndCompileRequest* request, const WriteOptions& options, Stream* stream);
    static SlangResult write(Module* module, const WriteOptions& options, Stream* stream);

};

} // namespace Slang

#endif
