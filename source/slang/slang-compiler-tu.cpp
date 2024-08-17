// slang-compiler-tu.cpp: Compiles translation units to target language
// and emit precompiled blobs into IR

#include "../core/slang-basic.h"
#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-capability.h"

namespace Slang
{
    SLANG_NO_THROW SlangResult SLANG_MCALL Module::precompileForTarget(
        SlangCompileTarget target,
        slang::IBlob** outDiagnostics)
    {
        if (target != SLANG_DXIL)
        {
            return SLANG_FAIL;
        }
        CodeGenTarget targetEnum = CodeGenTarget(target);

        auto module = getIRModule();
        auto linkage = getLinkage();

        DiagnosticSink sink(linkage->getSourceManager(), Lexer::sourceLocationLexer);
        applySettingsToDiagnosticSink(&sink, &sink, linkage->m_optionSet);
        applySettingsToDiagnosticSink(&sink, &sink, m_optionSet);

        TargetRequest* targetReq = new TargetRequest(linkage, targetEnum);

        List<RefPtr<ComponentType>> allComponentTypes;
        allComponentTypes.add(this); // Add Module as a component type

        for (auto entryPoint : this->getEntryPoints())
        {
            allComponentTypes.add(entryPoint); // Add the entry point as a component type
        }

        auto composite = CompositeComponentType::create(
            linkage,
            allComponentTypes);

        TargetProgram tp(composite, targetReq);
        tp.getOrCreateLayout(&sink);
        Slang::Index const entryPointCount = m_entryPoints.getCount();
        tp.getOptionSet().add(CompilerOptionName::GenerateWholeProgram, true);

        switch (targetReq->getTarget())
        {
        case CodeGenTarget::DXIL:
            tp.getOptionSet().add(CompilerOptionName::Profile, Profile::RawEnum::DX_Lib_6_6);
            break;
        }

        CodeGenContext::EntryPointIndices entryPointIndices;

        entryPointIndices.setCount(entryPointCount);
        for (Index i = 0; i < entryPointCount; i++)
            entryPointIndices[i] = i;
        CodeGenContext::Shared sharedCodeGenContext(&tp, entryPointIndices, &sink, nullptr);
        CodeGenContext codeGenContext(&sharedCodeGenContext);

        ComPtr<IArtifact> outArtifact;
        SlangResult res = codeGenContext.emitTranslationUnit(outArtifact);

        sink.getBlobIfNeeded(outDiagnostics);

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
        }

        return SLANG_OK;
    }
}
