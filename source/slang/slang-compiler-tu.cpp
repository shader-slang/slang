// slang-compiler-tu.cpp: Compiles translation units to target language
//

#include "../core/slang-basic.h"
#include "../core/slang-platform.h"
#include "../core/slang-io.h"
#include "../core/slang-string-util.h"
//#include "../core/slang-hex-dump-util.h"
#include "../core/slang-castable.h"

#include "slang-check.h"
#include "slang-compiler.h"

#include "../compiler-core/slang-lexer.h"

// Artifact
#include "../compiler-core/slang-artifact-desc-util.h"
#include "../compiler-core/slang-artifact-representation-impl.h"
#include "../compiler-core/slang-artifact-impl.h"
#include "../compiler-core/slang-artifact-util.h"
#include "../compiler-core/slang-artifact-associated.h"
#include "../compiler-core/slang-artifact-diagnostic-util.h"
#include "../compiler-core/slang-artifact-container-util.h"

// Artifact output
#include "slang-artifact-output-util.h"

#include "slang-lower-to-ir.h"
#include "slang-mangle.h"
#include "slang-parameter-binding.h"
#include "slang-parser.h"
#include "slang-preprocessor.h"
#include "slang-type-layout.h"

#include "slang-serialize-ast.h"
#include "slang-serialize-container.h"

namespace Slang
{
    SLANG_NO_THROW SlangResult SLANG_MCALL Module::precompileForTargets(
        DiagnosticSink* sink,
        EndToEndCompileRequest* endToEndReq,
        TargetRequest* targetReq)
    {
        SLANG_UNUSED(sink);
        assert(this->getIRModule());
        auto module = getIRModule();
        Slang::Session* session = endToEndReq->getSession();
        Slang::ASTBuilder* astBuilder = session->getGlobalASTBuilder();
        Slang::Linkage* builtinLinkage = session->getBuiltinLinkage();
        Slang::Linkage linkage(session, astBuilder, builtinLinkage);
        linkage.addTarget(Slang::CodeGenTarget::DXIL);
        
        TargetProgram tp(this, targetReq);
        
        CodeGenContext::EntryPointIndices entryPointIndices;
        CodeGenContext::Shared sharedCodeGenContext(&tp, entryPointIndices, sink, endToEndReq);
        CodeGenContext codeGenContext(&sharedCodeGenContext);
        
        ComPtr<IArtifact> outArtifact;
        codeGenContext.emitTranslationUnit(outArtifact);
        
        ISlangBlob* blob;
        outArtifact->loadBlob(ArtifactKeep::Yes, &blob);

        auto builder = IRBuilder(module);
        builder.setInsertInto(module);
        printf("Emit embedded DXIL. Blob size %d\n", blob->getBufferSize());
        builder.emitEmbeddedDXIL(blob);
               
        return SLANG_OK;
    }
}
