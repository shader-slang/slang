// slang-compiler-tu.cpp: Compiles translation units to target language
// and emit precompiled blobs into IR

#include "../core/slang-basic.h"
#include "slang-compiler.h"
#include "slang-ir-insts.h"

namespace Slang
{
    SLANG_NO_THROW SlangResult SLANG_MCALL Module::precompileForTargets(
        DiagnosticSink* sink,
        EndToEndCompileRequest* endToEndReq,
        TargetRequest* targetReq)
    {
        auto module = getIRModule();
        Slang::Session* session = endToEndReq->getSession();
        Slang::ASTBuilder* astBuilder = session->getGlobalASTBuilder();
        Slang::Linkage* builtinLinkage = session->getBuiltinLinkage();
        Slang::Linkage linkage(session, astBuilder, builtinLinkage);
        switch (targetReq->getTarget())
        {
        case CodeGenTarget::DXIL:
            linkage.addTarget(Slang::CodeGenTarget::DXIL);
            break;
        default:
            assert(!"Unhandled target");
            break;
        }

        List<RefPtr<ComponentType>> allComponentTypes;
        allComponentTypes.add(this); // Add Module as a component type

        for (auto entryPoint : this->getEntryPoints())
        {
            allComponentTypes.add(entryPoint); // Add the entry point as a component type
        }

        auto composite = CompositeComponentType::create(
            &linkage,
            allComponentTypes);

        TargetProgram tp(composite, targetReq);
        tp.getOrCreateLayout(sink);
        Slang::Index const entryPointCount = m_entryPoints.getCount();

        CodeGenContext::EntryPointIndices entryPointIndices;

        entryPointIndices.setCount(entryPointCount);
        for (Index i = 0; i < entryPointCount; i++)
            entryPointIndices[i] = i;
        CodeGenContext::Shared sharedCodeGenContext(&tp, entryPointIndices, sink, endToEndReq);
        CodeGenContext codeGenContext(&sharedCodeGenContext);

        ComPtr<IArtifact> outArtifact;
        SlangResult res = codeGenContext.emitTranslationUnit(outArtifact);
        if (res != SLANG_OK)
        {
            return res;
        }

        ISlangBlob* blob;
        outArtifact->loadBlob(ArtifactKeep::Yes, &blob);

        auto builder = IRBuilder(module);
        builder.setInsertInto(module);

        switch (targetReq->getTarget())
        {
        case CodeGenTarget::DXIL:
            builder.emitEmbeddedDXIL(blob);
            break;
        default:
            assert(!"Unhandled target");
            break;
        }

        return SLANG_OK;
    }
}
