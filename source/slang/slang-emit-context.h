// slang-emit-context.h
#ifndef SLANG_EMIT_CONTEXT_H_INCLUDED
#define SLANG_EMIT_CONTEXT_H_INCLUDED

#include "../core/slang-basic.h"

#include "slang-compiler.h"
#include "slang-type-layout.h"
#include "slang-emit-source-writer.h"
#include "slang-extension-usage-tracker.h"

namespace Slang
{

// Shared state for an entire emit session
struct EmitContext
{
    DiagnosticSink* getSink() { return compileRequest->getSink(); }
    LineDirectiveMode getLineDirectiveMode() { return compileRequest->getLineDirectiveMode(); }
    SourceManager* getSourceManager() { return compileRequest->getSourceManager(); }
    void noteInternalErrorLoc(SourceLoc loc) { return getSink()->noteInternalErrorLoc(loc); }

    BackEndCompileRequest* compileRequest = nullptr;

    // The entry point we are being asked to compile
    EntryPoint* entryPoint;

    // The layout for the entry point
    EntryPointLayout* entryPointLayout;

    // The target language we want to generate code for
    CodeGenTarget target;

    // Where source is written to
    SourceWriter* writer;

    // We only want to emit each `import`ed module one time, so
    // we maintain a set of already-emitted modules.
    HashSet<ModuleDecl*> modulesAlreadyEmitted;

    // We track the original global-scope layout so that we can
    // find layout information for `import`ed parameters.
    //
    // TODO: This will probably change if we represent imports
    // explicitly in the layout data.
    StructTypeLayout* globalStructLayout;

    ProgramLayout* programLayout;

    ModuleDecl* program;

    ExtensionUsageTracker extensionUsageTracker;

    UInt uniqueIDCounter = 1;
    Dictionary<IRInst*, UInt> mapIRValueToID;
    Dictionary<Decl*, UInt> mapDeclToID;

    HashSet<String> irDeclsVisited;

    HashSet<String> irTupleTypes;

    // The "effective" profile that is being used to emit code,
    // combining information from the target and entry point.
    Profile effectiveProfile;

    // Map a string name to the number of times we have seen this
    // name used so far during code emission.
    Dictionary<String, UInt> uniqueNameCounters;

    // Map an IR instruction to the name that we've decided
    // to use for it when emitting code.
    Dictionary<IRInst*, String> mapInstToName;

    Dictionary<IRInst*, UInt> mapIRValueToRayPayloadLocation;
    Dictionary<IRInst*, UInt> mapIRValueToCallablePayloadLocation;
};

}
#endif
