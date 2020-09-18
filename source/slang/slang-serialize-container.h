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
        SlangTargetFlags    flags = 0;
        Profile             profile;
        FloatingPointMode   floatingPointMode = FloatingPointMode::Default;
    };
    
    struct TargetModule
    {
        // IR module for a specific compilation target
        Target target;
        RefPtr<IRModule> irModule;
    };

    struct TranslationUnit
    {
        RefPtr<IRModule> irModule;
        RefPtr<ASTBuilder> astBuilder;
        NodeBase* astRootNode = nullptr;
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
        translationUnits.clear();
        targetModules.clear();
    }

    List<TranslationUnit> translationUnits;
    List<TargetModule> targetModules;
    List<EntryPoint> entryPoints;
};

struct SerialContainerUtil
{
    struct WriteOptions
    {
        SerialCompressionType compressionType = SerialCompressionType::VariableByteLite;
        SerialOptionFlags optionFlags = SerialOptionFlag::ASTModule | SerialOptionFlag::IRModule;
        SourceManager* sourceManager = nullptr;
    };

    struct ReadOptions
    {
        Session* session = nullptr;
        SourceManager* sourceManager = nullptr;
        NamePool* namePool = nullptr;
        SharedASTBuilder* sharedASTBuilder = nullptr;
    };

        /// Get the serializable contents of the request as data
    static SlangResult requestToData(EndToEndCompileRequest* request, const WriteOptions& options, SerialContainerData& outData);

    static SlangResult write(const SerialContainerData& data, const WriteOptions& options, RiffContainer* container);

    static SlangResult read(RiffContainer* container, const ReadOptions& options, SerialContainerData& out);

        /// Verify IR serialization
    static SlangResult verifyIRSerialize(IRModule* module, Session* session, const WriteOptions& options);
};

} // namespace Slang

#endif
