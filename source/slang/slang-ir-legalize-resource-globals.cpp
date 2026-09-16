// slang-ir-legalize-resource-globals.cpp
#include "slang-ir-legalize-resource-globals.h"

#include "slang-diagnostics.h"
#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir-use-uninitialized-values.h"
#include "slang-ir-util.h"

namespace Slang
{

namespace
{

enum class ResourceGlobalAccess : UInt
{
    None = 0,
    Read = 1 << 0,
    Write = 1 << 1,
};

static ResourceGlobalAccess mergeAccess(ResourceGlobalAccess left, ResourceGlobalAccess right)
{
    return ResourceGlobalAccess(UInt(left) | UInt(right));
}

static bool hasAccess(ResourceGlobalAccess value, ResourceGlobalAccess test)
{
    return (UInt(value) & UInt(test)) != 0;
}

static bool affectsResourceGlobalAccess(IRInst* user)
{
    if (as<IRDecoration>(user) || as<IRAnnotation>(user) || as<IRAttr>(user) || as<IRType>(user))
        return false;

    if (isTypeOnlyInst(user))
        return false;

    switch (user->getOp())
    {
    case kIROp_DebugValue:
    case kIROp_DebugVar:
    case kIROp_DebugLine:
    case kIROp_DebugInlinedAt:
    case kIROp_DebugScope:
    case kIROp_DebugNoScope:
    case kIROp_DebugInlinedVariable:
    case kIROp_DebugFunction:
    case kIROp_DebugFuncDecoration:
    case kIROp_DebugSource:
    case kIROp_DebugBuildIdentifier:
    case kIROp_DebugCompilationUnit:
        return false;

    default:
        return true;
    }
}

static bool isAddressDerivation(IRUse* use)
{
    auto user = use->getUser();
    if (user->getOperandCount() == 0 || user->getOperandUse(0) != use)
        return false;

    if (isAddressInst(user))
        return true;

    switch (user->getOp())
    {
    case kIROp_BitCast:
    case kIROp_Reinterpret:
    case kIROp_PtrCast:
    case kIROp_InOutImplicitCast:
        return as<IRPtrTypeBase>(user->getDataType()) != nullptr;
    default:
        return false;
    }
}

static ResourceGlobalAccess getCallArgumentAccess(IRCall* call, IRUse* use)
{
    if (use == call->getCalleeUse())
        return mergeAccess(ResourceGlobalAccess::Read, ResourceGlobalAccess::Write);

    Index argIndex = -1;
    for (UInt i = 0; i < call->getArgCount(); ++i)
    {
        if (call->getOperandUse(i + 1) == use)
        {
            argIndex = Index(i);
            break;
        }
    }
    if (argIndex < 0)
        return mergeAccess(ResourceGlobalAccess::Read, ResourceGlobalAccess::Write);

    auto funcType = as<IRFuncType>(call->getCallee()->getDataType());
    if (!funcType || UInt(argIndex) >= funcType->getParamCount())
        return mergeAccess(ResourceGlobalAccess::Read, ResourceGlobalAccess::Write);

    auto paramType = unwrapAttributedType(funcType->getParamType(UInt(argIndex)));
    if (as<IROutParamType>(paramType))
        return ResourceGlobalAccess::Write;
    if (as<IRBorrowInParamType>(paramType))
        return ResourceGlobalAccess::Read;
    if (as<IRBorrowInOutParamType>(paramType) || as<IRRefParamType>(paramType))
        return mergeAccess(ResourceGlobalAccess::Read, ResourceGlobalAccess::Write);
    if (as<IRPtrTypeBase>(paramType))
    {
        // A raw pointer parameter carries no directional contract. Its callee may observe or
        // replace the pointee, so preserve both directions rather than assuming an input use.
        return mergeAccess(ResourceGlobalAccess::Read, ResourceGlobalAccess::Write);
    }
    return ResourceGlobalAccess::Read;
}

static ResourceGlobalAccess classifyTerminalUse(IRUse* use)
{
    auto user = use->getUser();

    if (as<IRLoad>(user) || as<IRAtomicLoad>(user))
        return ResourceGlobalAccess::Read;

    switch (user->getOp())
    {
    case kIROp_Store:
    case kIROp_AtomicStore:
    case kIROp_SwizzledStore:
    case kIROp_MatrixSwizzleStore:
        return user->getOperandUse(0) == use
                   ? ResourceGlobalAccess::Write
                   : mergeAccess(ResourceGlobalAccess::Read, ResourceGlobalAccess::Write);
    default:
        break;
    }

    if (auto call = as<IRCall>(user))
        return getCallArgumentAccess(call, use);

    // A pointer-producing instruction that is not a recognized address derivation may let the
    // address escape. The same is true when the address is routed through control flow or returned.
    // We cannot safely infer how those users access the variable, so preserve both directions.
    if (as<IRPtrTypeBase>(user->getDataType()))
        return mergeAccess(ResourceGlobalAccess::Read, ResourceGlobalAccess::Write);

    switch (user->getOp())
    {
    case kIROp_Return:
    case kIROp_UnconditionalBranch:
    case kIROp_Loop:
    case kIROp_IfElse:
    case kIROp_Switch:
        return mergeAccess(ResourceGlobalAccess::Read, ResourceGlobalAccess::Write);
    default:
        // Ordinary value consumers observe the resource stored at this address.
        return ResourceGlobalAccess::Read;
    }
}

static bool isAddressEscape(IRUse* use)
{
    auto user = use->getUser();
    if (auto store = as<IRStore>(user))
        return store->getOperandUse(0) != use;
    if (as<IRReturn>(user))
        return true;

    switch (user->getOp())
    {
    case kIROp_UnconditionalBranch:
    case kIROp_Loop:
    case kIROp_IfElse:
    case kIROp_Switch:
        return true;
    default:
        return as<IRPtrTypeBase>(user->getDataType()) != nullptr;
    }
}

static bool hasIndependentFunctionReference(IRFunc* func)
{
    for (auto use = func->firstUse; use; use = use->nextUse)
    {
        auto user = use->getUser();
        if (auto call = as<IRCall>(user))
        {
            if (call->getCalleeUse() == use)
                continue;
        }

        // A decoration that names a function can represent a runtime-managed invocation, such as
        // a hull shader's patch-constant function. Other non-call value uses likewise provide no
        // call site where this pass can append a hidden argument. Treat both as independent roots;
        // type and debug metadata do not establish a call boundary.
        if (as<IRDecoration>(user))
            return true;
        if (!affectsResourceGlobalAccess(user))
            continue;
        return true;
    }
    return false;
}

static bool hasPreservedGlobalStorage(IRGlobalVar* globalVar)
{
    // An actual-global (or any other rated global) denotes a distinct externally meaningful
    // location and therefore cannot receive resource-dependent initialization inside each entry
    // point. Lack of linkage alone does not preserve storage: function-scope statics are lowered
    // to unlinked, rate-less globals and still have ordinary per-invocation semantics.
    if (globalVar->getRate())
        return true;

    // `IRExportDecoration` is intentionally absent. Ordinary linked source definitions receive
    // that decoration so other IR modules can resolve them; it does not promise externally
    // addressable storage. The decorations below represent call/storage boundaries that must
    // survive final linking and therefore cannot be localized.
    static const IROp kPreservedStorageDecorations[] = {
        kIROp_ImportDecoration,
        kIROp_UserExternDecoration,
        kIROp_PublicDecoration,
        kIROp_KeepAliveDecoration,
        kIROp_DllImportDecoration,
        kIROp_CudaDeviceExportDecoration,
        kIROp_DllExportDecoration,
        kIROp_HLSLExportDecoration,
        kIROp_ExternCDecoration,
        kIROp_ExternCppDecoration,
        kIROp_DownstreamModuleExportDecoration,
        kIROp_DownstreamModuleImportDecoration,
    };
    for (auto op : kPreservedStorageDecorations)
    {
        if (globalVar->findDecorationImpl(op))
            return true;
    }
    return false;
}

struct FunctionInfo
{
    IRFunc* func = nullptr;
    IRInst* originalFirstOrdinaryInst = nullptr;
    RefPtr<IRDominatorTree> dominatorTree;
    bool isEntryPoint = false;
    bool isIndependentRoot = false;
};

struct FunctionGlobalInfo
{
    ResourceGlobalAccess access = ResourceGlobalAccess::None;
    bool replacesWholeValueOnEveryReturn = false;
    bool semanticallyReadsIncomingValue = false;
    bool preservesIncomingValue = false;
    bool hasDirectUse = false;
    IRInst* replacementAddress = nullptr;
    IRParam* parameter = nullptr;
    List<UninitializedVariableUseEffect> uninitializedUseEffects;

    bool needsIncomingValue() const
    {
        return semanticallyReadsIncomingValue || preservesIncomingValue;
    }
};

struct RootUseInfo
{
    IRUse* use = nullptr;
    Index functionIndex = -1;
};

struct TerminalUseInfo
{
    IRUse* use = nullptr;
    Index functionIndex = -1;
    ResourceGlobalAccess access = ResourceGlobalAccess::None;
    bool replacesWholeValue = false;
};

struct ResourceGlobalInfo
{
    IRGlobalVar* globalVar = nullptr;
    IRType* valueType = nullptr;
    List<FunctionGlobalInfo> functions;
    List<RootUseInfo> rootUses;
    List<TerminalUseInfo> terminalUses;
    List<IRUse*> addressPassingUses;
    List<IRUse*> addressEscapes;
};

struct ResourceDependentStateDiagnosticGroup
{
    SourceLoc sourceLoc;
    IRGlobalVar* representative = nullptr;
    List<IRGlobalVar*> globals;
};

struct CallEdge
{
    IRCall* call = nullptr;
    Index callerIndex = -1;
    Index calleeIndex = -1;
};

// Consider this example:
//
//      Texture2D textures[1];
//      static Texture2D texture;
//      float4 loadTexture() { return texture.Load(int3(0)); }
//      float4 main() { texture = textures[0]; return loadTexture(); }
//
// The global models storage private to one entry-point invocation, but many targets cannot store
// a resource handle in a global variable. This pass replaces it with a local in `main` and threads
// the value through `loadTexture`. A read-only helper receives a value parameter. A helper that
// definitely replaces the whole value receives an `out` parameter, while any writer that can
// preserve or observe the incoming value receives an `inout` parameter. A fixed-point walk over
// direct calls carries those requirements up to each entry point, where the original global storage
// can finally be removed.
//
// Initializer extraction also reports ordinary globals initialized or mutated by the moved
// resource-dependent initializer call graph. For example,
// `static float4 color = texture.Load(int3(0))` is part of the same per-invocation state. If an
// exported or otherwise independently invoked function observes that state, this pass diagnoses
// the boundary: there is no internal caller from which to obtain a hidden parameter, and changing
// the preserved signature would be incorrect.
struct LegalizeResourceGlobalVarsPass
{
    IRModule* module = nullptr;
    List<FunctionInfo> functions;
    Dictionary<IRFunc*, Index> functionIndices;
    List<ResourceGlobalInfo> globals;
    List<CallEdge> callEdges;

    explicit LegalizeResourceGlobalVarsPass(IRModule* inModule)
        : module(inModule)
    {
    }

    void processModule(List<IRGlobalVar*> const& resourceDependentState, DiagnosticSink* sink)
    {
        collectModuleEntities();
        collectCallEdges();
        if (diagnoseUnsupportedCallBoundaries(resourceDependentState, sink))
            return;

        HashSet<IRGlobalVar*> resourceDependentStateSet;
        for (auto stateGlobal : resourceDependentState)
            resourceDependentStateSet.add(stateGlobal);
        for (auto const& global : globals)
            SLANG_RELEASE_ASSERT(resourceDependentStateSet.contains(global.globalVar));

        if (globals.getCount() == 0)
            return;

        analyzeGlobals();
        recordNonDefiniteWriteEffects();
        if (diagnoseAddressEscapes(sink))
            return;
        if (diagnoseAliasingCallBoundaries(sink))
            return;
        validateDirectCallGraph();
        introduceReplacements();
        replaceGlobalUses();
        rewriteCalls();
        diagnoseUninitializedEntryPointReads(sink);

        for (auto& global : globals)
        {
            SLANG_ASSERT(!global.globalVar->hasUses());
            global.globalVar->removeAndDeallocate();
        }
    }

    bool diagnoseAliasingCallBoundaries(DiagnosticSink* sink)
    {
        bool diagnosed = false;
        for (auto const& global : globals)
        {
            HashSet<IRCall*> diagnosedCalls;
            for (auto use : global.addressPassingUses)
            {
                auto call = cast<IRCall>(use->getUser());
                auto callee = as<IRFunc>(call->getCallee());
                if (!callee)
                    continue;
                auto calleeIndex = functionIndices.tryGetValue(callee);
                if (!calleeIndex)
                {
                    continue;
                }
                auto implicitAccess = global.functions[*calleeIndex].access;
                if (implicitAccess == ResourceGlobalAccess::None)
                    continue;
                if (!diagnosedCalls.add(call))
                    continue;

                // Passing the caller's storage explicitly while also threading the same global
                // by name would give the callee two aliases backed by different local copies. Two
                // writes can diverge, and even read-only references can observe their address
                // identity on targets that support resource pointers. Reject that boundary until
                // the pass can preserve one shared storage identity.
                sink->diagnose(Diagnostics::ResourceStaticAliasesThreadedState{
                    .variable = global.globalVar,
                    .function = callee,
                    .location = call->sourceLoc});
                diagnosed = true;
            }
        }
        return diagnosed;
    }

    bool diagnoseAddressEscapes(DiagnosticSink* sink)
    {
        bool diagnosed = false;
        for (auto const& global : globals)
        {
            for (auto use : global.addressEscapes)
            {
                sink->diagnose(Diagnostics::ResourceStaticAddressEscapes{
                    .variable = global.globalVar,
                    .location = use->getUser()->sourceLoc});
                diagnosed = true;
            }
        }
        return diagnosed;
    }

    void collectModuleEntities()
    {
        // These lists define the transformation's order. In particular, every function receives
        // its new parameters in the same order as the corresponding globals appeared in the
        // module, rather than in hash-table or use-list order.
        for (auto inst : module->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
            {
                FunctionInfo info;
                info.func = func;
                info.isEntryPoint = func->findDecoration<IREntryPointDecoration>() != nullptr;
                // A non-entry keep-alive function has no guaranteed in-module caller even when it
                // has no explicit ABI decoration. Treat it as an independent root as well: there
                // is nowhere to obtain a hidden argument when the retained function is invoked.
                info.isIndependentRoot =
                    !info.isEntryPoint &&
                    (func->findDecoration<IRKeepAliveDecoration>() ||
                     func->findDecoration<IRPublicDecoration>() ||
                     func->findDecoration<IRHLSLExportDecoration>() ||
                     func->findDecoration<IRDllExportDecoration>() ||
                     func->findDecoration<IRExternCDecoration>() ||
                     func->findDecoration<IRExternCppDecoration>() ||
                     func->findDecoration<IRCudaDeviceExportDecoration>() ||
                     func->findDecoration<IRDownstreamModuleExportDecoration>() ||
                     func->findDecoration<IRDownstreamModuleImportDecoration>() ||
                     hasIndependentFunctionReference(func));
                if (auto firstBlock = func->getFirstBlock())
                    info.originalFirstOrdinaryInst = firstBlock->getFirstOrdinaryInst();

                auto index = functions.getCount();
                functions.add(info);
                functionIndices.add(func, index);
            }
        }

        for (auto inst : module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar)
                continue;

            if (!isPerInvocationResourceStateGlobalVar(globalVar))
                continue;

            auto ptrType = cast<IRPtrTypeBase>(globalVar->getDataType());

            // The initializer-moving pass must have turned every initializer into stores in the
            // entry points before this pass chooses the entry-point-local storage.
            SLANG_RELEASE_ASSERT(!globalVar->getFirstBlock());

            ResourceGlobalInfo info;
            info.globalVar = globalVar;
            info.valueType = ptrType->getValueType();
            for (Index i = 0; i < functions.getCount(); ++i)
                info.functions.add(FunctionGlobalInfo());
            globals.add(_Move(info));
        }
    }

    bool diagnoseUnsupportedCallBoundaries(
        List<IRGlobalVar*> const& resourceDependentState,
        DiagnosticSink* sink)
    {
        bool diagnosed = false;
        List<bool> hasOrdinaryCallSite;
        for (Index i = 0; i < functions.getCount(); ++i)
            hasOrdinaryCallSite.add(false);
        for (auto const& edge : callEdges)
            hasOrdinaryCallSite[edge.calleeIndex] = true;

        // A function-scope static with an initializer lowers to two globals: the user-named
        // storage and a compiler-generated initialization guard. Both carry the declaration's
        // source location, while source metadata such as the declaration and name identifies the
        // storage when it survives this far. Treat globals at that exact source location as one
        // source-level state item so that a boundary involving both does not produce duplicate
        // diagnostics. An invalid location cannot establish that identity, so such a global
        // deliberately remains in its own group.
        List<ResourceDependentStateDiagnosticGroup> stateGroups;
        Dictionary<SourceLoc::RawValue, Index> sourceLocToGroupIndex;
        for (auto stateGlobal : resourceDependentState)
        {
            SourceLoc sourceLoc = stateGlobal->sourceLoc;
            if (auto declDecoration = stateGlobal->findDecoration<IRHighLevelDeclDecoration>())
            {
                auto declLoc = declDecoration->getDecl()->loc;
                if (declLoc.isValid())
                    sourceLoc = declLoc;
            }

            Index groupIndex = -1;
            if (sourceLoc.isValid())
            {
                if (auto existingIndex = sourceLocToGroupIndex.tryGetValue(sourceLoc.getRaw()))
                    groupIndex = *existingIndex;
            }

            if (groupIndex < 0)
            {
                groupIndex = stateGroups.getCount();
                ResourceDependentStateDiagnosticGroup group;
                group.sourceLoc = sourceLoc;
                group.representative = stateGlobal;
                stateGroups.add(_Move(group));
                if (sourceLoc.isValid())
                    sourceLocToGroupIndex.add(sourceLoc.getRaw(), groupIndex);
            }

            auto& group = stateGroups[groupIndex];
            group.globals.add(stateGlobal);

            // The declaration-backed global is the source variable. Some pipelines have already
            // stripped its high-level declaration by this point, but preserve its name hint, so
            // use either piece of source information to prefer it over a synthesized companion.
            auto representativeHasDecl =
                group.representative->findDecoration<IRHighLevelDeclDecoration>() != nullptr;
            auto stateGlobalHasDecl =
                stateGlobal->findDecoration<IRHighLevelDeclDecoration>() != nullptr;
            auto representativeHasName =
                group.representative->findDecoration<IRNameHintDecoration>() != nullptr;
            auto stateGlobalHasName =
                stateGlobal->findDecoration<IRNameHintDecoration>() != nullptr;
            if ((!representativeHasDecl && stateGlobalHasDecl) ||
                (representativeHasDecl == stateGlobalHasDecl && !representativeHasName &&
                 stateGlobalHasName))
            {
                group.representative = stateGlobal;
            }
        }

        for (auto const& stateGroup : stateGroups)
        {
            IRGlobalVar* preservedGlobal = nullptr;
            for (auto stateGlobal : stateGroup.globals)
            {
                if (!hasPreservedGlobalStorage(stateGlobal))
                    continue;
                preservedGlobal = stateGlobal;
                if (stateGlobal == stateGroup.representative)
                    break;
            }
            if (preservedGlobal)
            {
                sink->diagnose(Diagnostics::ResourceDependentStateHasPreservedStorage{
                    .variable = preservedGlobal,
                    .location = stateGroup.sourceLoc});
                diagnosed = true;
                continue;
            }

            List<bool> functionAccessesState;
            for (Index i = 0; i < functions.getCount(); ++i)
                functionAccessesState.add(false);

            for (auto stateGlobal : stateGroup.globals)
            {
                for (auto use = stateGlobal->firstUse; use; use = use->nextUse)
                {
                    auto parentFunc = getParentFunc(use->getUser());
                    if (!parentFunc)
                        continue;
                    if (auto functionIndex = functionIndices.tryGetValue(parentFunc))
                        functionAccessesState[*functionIndex] = true;
                }
            }

            // An independent root may reach the state through any number of internal helpers. Use
            // the same caller propagation as the resource access analysis so indirect users are
            // subject to the call-boundary check as well.
            bool changed = false;
            do
            {
                changed = false;
                for (auto const& edge : callEdges)
                {
                    if (!functionAccessesState[edge.calleeIndex] ||
                        functionAccessesState[edge.callerIndex])
                    {
                        continue;
                    }
                    functionAccessesState[edge.callerIndex] = true;
                    changed = true;
                }
            } while (changed);

            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                auto const& function = functions[functionIndex];
                if (function.isEntryPoint && hasOrdinaryCallSite[functionIndex] &&
                    functionAccessesState[functionIndex])
                {
                    // A true entry-point invocation needs fresh per-invocation state, while an
                    // ordinary call must share its caller's current state. The existing late
                    // callable-entry split cannot represent both: cloning after initialization
                    // would give the ordinary call a second local and run the initializer again.
                    // Require an explicit ordinary helper until the pipeline has a distinct
                    // entry-wrapper representation to carry the caller's state.
                    sink->diagnose(Diagnostics::ResourceDependentStaticUsedByCallableEntryPoint{
                        .function = function.func,
                        .variable = stateGroup.representative,
                        .location = stateGroup.sourceLoc});
                    diagnosed = true;
                    break;
                }
                if (!function.isIndependentRoot || !functionAccessesState[functionIndex])
                {
                    continue;
                }

                // An independent root has no internal caller from which to receive hidden state.
                // Rewriting its signature would silently change a promised call boundary, while
                // leaving the global in place reaches an unsupported emitter shape. Diagnose this
                // boundary until it is represented by a wrapper with per-invocation state.
                sink->diagnose(Diagnostics::ResourceDependentStaticUsedByPreservedFunction{
                    .variable = stateGroup.representative,
                    .function = function.func,
                    .location = stateGroup.sourceLoc});
                diagnosed = true;
                break;
            }
        }
        return diagnosed;
    }

    void collectCallEdges()
    {
        for (Index callerIndex = 0; callerIndex < functions.getCount(); ++callerIndex)
        {
            auto func = functions[callerIndex].func;
            if (!func->getFirstBlock())
                continue;

            for (auto block : func->getBlocks())
            {
                for (auto inst : block->getChildren())
                {
                    auto call = as<IRCall>(inst);
                    if (!call)
                        continue;

                    auto callee = as<IRFunc>(call->getCallee());
                    if (!callee)
                        continue;

                    auto calleeIndex = functionIndices.tryGetValue(callee);
                    if (!calleeIndex)
                        continue;

                    callEdges.add(CallEdge{call, callerIndex, *calleeIndex});
                }
            }
        }
    }

    void analyzeGlobals()
    {
        for (auto& global : globals)
        {
            collectRootUses(global);

            HashSet<IRInst*> visitedAddresses;
            classifyAddressUses(global, global.globalVar, visitedAddresses, true);

            propagateAccessToCallers(global);
            findWholeValueReplacementsOnEveryReturn(global);
            findIncomingValueRequirements(global);
        }
    }

    void recordNonDefiniteWriteEffects()
    {
        // The IR type of a write does not always say whether it initializes the whole value. A
        // store through a derived address changes only one subobject, while passing the whole
        // address to an `inout` helper may write only on some control-flow paths. Record both as
        // possible, rather than definite, initializations. Treating a subobject write this way is
        // conservative until the shared checker can track individual subobject access paths.
        for (auto& global : globals)
        {
            for (auto const& terminalUse : global.terminalUses)
            {
                if (!hasAccess(terminalUse.access, ResourceGlobalAccess::Write) ||
                    terminalUse.replacesWholeValue)
                {
                    continue;
                }

                auto& functionUse = global.functions[terminalUse.functionIndex];
                functionUse.uninitializedUseEffects.add(UninitializedVariableUseEffect{
                    .use = terminalUse.use,
                    .readsValue = hasAccess(terminalUse.access, ResourceGlobalAccess::Read),
                    .mayWriteValue = true,
                    .definitelyWritesValue = false,
                });
            }
        }
    }

    void remapExplicitArgumentUseEffects(IRCall* oldCall, IRCall* newCall, Index callerIndex)
    {
        // Rewriting a call replaces every explicit argument use along with the instruction. Keep
        // the source-level effects collected for `inout`/`ref` arguments attached to the
        // corresponding use on the new call; otherwise the checker falls back to the generated IR
        // type and can mistake a conditional write for a definite initialization.
        for (auto& global : globals)
        {
            auto& effects = global.functions[callerIndex].uninitializedUseEffects;
            for (auto& effect : effects)
            {
                for (UInt argIndex = 0; argIndex < oldCall->getArgCount(); ++argIndex)
                {
                    if (effect.use == oldCall->getOperandUse(argIndex + 1))
                    {
                        effect.use = newCall->getOperandUse(argIndex + 1);
                        break;
                    }
                }
            }
        }
    }

    void propagateAccessToCallers(ResourceGlobalInfo& global)
    {
        // A caller needs every access required by its callees. Iterating the direct-call edges to a
        // fixed point handles ordinary chains and recursive strongly-connected components without
        // relying on a particular module order.
        bool changed = false;
        do
        {
            changed = false;
            for (auto const& edge : callEdges)
            {
                auto calleeAccess = global.functions[edge.calleeIndex].access;
                auto& callerAccess = global.functions[edge.callerIndex].access;
                auto merged = mergeAccess(callerAccess, calleeAccess);
                if (merged != callerAccess)
                {
                    callerAccess = merged;
                    changed = true;
                }
            }
        } while (changed);
    }

    IRDominatorTree* getDominatorTree(Index functionIndex)
    {
        auto& function = functions[functionIndex];
        if (!function.dominatorTree)
            function.dominatorTree = computeDominatorTree(function.func);
        return function.dominatorTree;
    }

    void collectWholeValueReplacements(
        ResourceGlobalInfo& global,
        Index functionIndex,
        HashSet<IRInst*>& replacements)
    {
        // A direct store or an argument passed to an existing `out` parameter replaces the whole
        // resource. Derived-address stores are deliberately excluded because assigning one array
        // element does not define the rest of the value.
        for (auto const& terminalUse : global.terminalUses)
        {
            if (terminalUse.functionIndex == functionIndex && terminalUse.replacesWholeValue)
                replacements.add(terminalUse.use->getUser());
        }

        // A call to a helper already proven to replace the value has the same effect in its
        // caller. The fixed-point analysis below adds these barriers as callee proofs become
        // available.
        for (auto const& edge : callEdges)
        {
            if (edge.callerIndex == functionIndex &&
                global.functions[edge.calleeIndex].replacesWholeValueOnEveryReturn)
            {
                replacements.add(edge.call);
            }
        }
    }

    bool canReachWithoutWholeValueReplacement(
        Index functionIndex,
        IRInst* target,
        HashSet<IRInst*> const& replacements)
    {
        // The worklist represents execution paths that have not replaced the value yet. Reaching
        // a replacement stops that path; reaching `target` first proves that the incoming value is
        // still observable there. Visiting each block once is sufficient because every queued path
        // has the same "not replaced" state.
        HashSet<IRBlock*> visited;
        List<IRBlock*> workList;
        auto entryBlock = functions[functionIndex].func->getFirstBlock();
        if (!entryBlock)
            return false;
        visited.add(entryBlock);
        workList.add(entryBlock);

        while (workList.getCount())
        {
            auto block = workList.getLast();
            workList.removeLast();

            bool pathWasReplaced = false;
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
            {
                // A call can both read the incoming value and replace it before returning. Test
                // the target first so its own replacement does not hide that read.
                if (inst == target)
                    return true;
                if (replacements.contains(inst))
                {
                    pathWasReplaced = true;
                    break;
                }
            }
            if (pathWasReplaced)
                continue;

            for (auto successor : block->getSuccessors())
            {
                if (visited.add(successor))
                    workList.add(successor);
            }
        }

        return false;
    }

    bool hasWholeValueReplacementOnEveryReturn(ResourceGlobalInfo& global, Index functionIndex)
    {
        HashSet<IRInst*> replacements;
        collectWholeValueReplacements(global, functionIndex, replacements);
        if (replacements.getCount() == 0)
            return false;

        bool foundReachableReturn = false;
        auto dominatorTree = getDominatorTree(functionIndex);
        for (auto block : functions[functionIndex].func->getBlocks())
        {
            if (dominatorTree->isUnreachable(block))
                continue;

            auto returnInst = as<IRReturn>(block->getTerminator());
            if (!returnInst)
                continue;

            foundReachableReturn = true;
            if (canReachWithoutWholeValueReplacement(functionIndex, returnInst, replacements))
                return false;
        }

        return foundReachableReturn;
    }

    void findWholeValueReplacementsOnEveryReturn(ResourceGlobalInfo& global)
    {
        // Callee proofs feed caller proofs, so iterate to a fixed point. Starting from false makes
        // recursive cycles conservative unless some function in the cycle has its own whole-value
        // writes covering every return path.
        bool changed = false;
        do
        {
            changed = false;
            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                auto& function = global.functions[functionIndex];
                if (!hasAccess(function.access, ResourceGlobalAccess::Write) ||
                    function.replacesWholeValueOnEveryReturn)
                    continue;

                if (hasWholeValueReplacementOnEveryReturn(global, functionIndex))
                {
                    function.replacesWholeValueOnEveryReturn = true;
                    changed = true;
                }
            }
        } while (changed);
    }

    bool hasPriorWholeValueReplacement(
        ResourceGlobalInfo& global,
        Index functionIndex,
        IRInst* read)
    {
        HashSet<IRInst*> replacements;
        collectWholeValueReplacements(global, functionIndex, replacements);
        return replacements.getCount() != 0 &&
               !canReachWithoutWholeValueReplacement(functionIndex, read, replacements);
    }

    void findIncomingValueRequirements(ResourceGlobalInfo& global)
    {
        // A partial or conditional writer must transport the caller's value through paths that do
        // not replace it. That transport is distinct from a source-level read: copying the value
        // solely to preserve it must not by itself trigger an uninitialized-use diagnostic.
        for (auto& function : global.functions)
        {
            if (hasAccess(function.access, ResourceGlobalAccess::Write) &&
                !function.replacesWholeValueOnEveryReturn)
            {
                function.preservesIncomingValue = true;
            }
        }

        // A read observes the incoming value exactly when some entry-to-read path reaches it
        // without first replacing the whole value. The CFG proof recognizes either one dominating
        // write or several writes that collectively cover all paths.
        for (auto const& terminalUse : global.terminalUses)
        {
            if (!hasAccess(terminalUse.access, ResourceGlobalAccess::Read))
                continue;
            if (!hasPriorWholeValueReplacement(
                    global,
                    terminalUse.functionIndex,
                    terminalUse.use->getUser()))
            {
                global.functions[terminalUse.functionIndex].semanticallyReadsIncomingValue = true;
            }
        }

        // Calls participate in execution order just like direct reads. Iterate to a fixed point so
        // a callee's incoming-value requirement reaches every caller not protected by an earlier
        // whole-value replacement.
        bool changed = false;
        do
        {
            changed = false;
            for (auto const& edge : callEdges)
            {
                if (!global.functions[edge.calleeIndex].semanticallyReadsIncomingValue)
                    continue;
                auto& caller = global.functions[edge.callerIndex];
                if (caller.semanticallyReadsIncomingValue ||
                    hasPriorWholeValueReplacement(global, edge.callerIndex, edge.call))
                {
                    continue;
                }
                caller.semanticallyReadsIncomingValue = true;
                changed = true;
            }
        } while (changed);
    }

    void collectRootUses(ResourceGlobalInfo& global)
    {
        for (auto use = global.globalVar->firstUse; use; use = use->nextUse)
        {
            RootUseInfo rootUse;
            rootUse.use = use;

            if (auto parentFunc = getParentFunc(use->getUser()))
            {
                if (auto index = functionIndices.tryGetValue(parentFunc))
                {
                    rootUse.functionIndex = *index;
                    global.functions[*index].hasDirectUse = true;
                }
            }

            global.rootUses.add(rootUse);
        }
    }

    void validateDirectCallGraph()
    {
        for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
        {
            auto func = functions[functionIndex].func;
            if (functions[functionIndex].isEntryPoint)
                continue;

            bool needsParameter = false;
            for (auto const& global : globals)
            {
                needsParameter |=
                    global.functions[functionIndex].access != ResourceGlobalAccess::None;
            }
            if (!needsParameter)
                continue;

            // Signature rewriting is intentionally limited to the direct-call graph promised by
            // this pass's pipeline position. Assert that contract before changing the signature;
            // silently overlooking a witness-table, specialization, or function-pointer use would
            // leave a mismatched caller behind.
            for (auto use = func->firstUse; use; use = use->nextUse)
            {
                auto user = use->getUser();
                if (!affectsResourceGlobalAccess(user))
                    continue;

                auto call = as<IRCall>(user);
                SLANG_RELEASE_ASSERT(call && call->getCalleeUse() == use);
                SLANG_RELEASE_ASSERT(getParentFunc(call));
            }
        }
    }

    void classifyAddressUses(
        ResourceGlobalInfo& global,
        IRInst* address,
        HashSet<IRInst*>& visitedAddresses,
        bool representsWholeValue)
    {
        if (!visitedAddresses.add(address))
            return;

        for (auto use = address->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (!affectsResourceGlobalAccess(user))
                continue;

            if (isAddressDerivation(use))
            {
                classifyAddressUses(global, user, visitedAddresses, false);
                continue;
            }

            if (as<IRCall>(user))
                global.addressPassingUses.add(use);
            if (isAddressEscape(use))
                global.addressEscapes.add(use);

            auto parentFunc = getParentFunc(user);
            SLANG_RELEASE_ASSERT(parentFunc);
            auto functionIndex = functionIndices.tryGetValue(parentFunc);
            SLANG_RELEASE_ASSERT(functionIndex);

            auto terminalAccess = classifyTerminalUse(use);
            auto& access = global.functions[*functionIndex].access;
            access = mergeAccess(access, terminalAccess);

            bool replacesWholeValue = false;
            if (representsWholeValue)
            {
                if (auto store = as<IRStore>(user))
                    replacesWholeValue = store->ptr.get() == address;
                else if (as<IRCall>(user))
                    replacesWholeValue = terminalAccess == ResourceGlobalAccess::Write;
            }
            global.terminalUses.add(
                TerminalUseInfo{use, *functionIndex, terminalAccess, replacesWholeValue});
        }
    }

    void introduceReplacements()
    {
        IRBuilder builder(module);

        for (auto& global : globals)
        {
            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                auto const& function = functions[functionIndex];
                auto& use = global.functions[functionIndex];
                if (use.access == ResourceGlobalAccess::None && !use.hasDirectUse)
                    continue;

                auto firstBlock = function.func->getFirstBlock();
                SLANG_RELEASE_ASSERT(firstBlock);

                if (function.isEntryPoint || use.access == ResourceGlobalAccess::None)
                {
                    setInsertAtOriginalBodyStart(builder, functionIndex);
                    use.replacementAddress = builder.emitVar(global.valueType);
                    use.replacementAddress->sourceLoc = global.globalVar->sourceLoc;
                    copyNameHint(builder, global.globalVar, use.replacementAddress);
                    continue;
                }

                IRType* paramType = global.valueType;
                bool writesValue = hasAccess(use.access, ResourceGlobalAccess::Write);
                if (writesValue && !use.semanticallyReadsIncomingValue &&
                    use.replacesWholeValueOnEveryReturn)
                    paramType = builder.getOutParamType(global.valueType);
                else if (writesValue)
                    paramType = builder.getBorrowInOutParamType(global.valueType);

                use.parameter = builder.createParam(paramType);
                use.parameter->sourceLoc = global.globalVar->sourceLoc;
                firstBlock->addParam(use.parameter);
                copyNameHint(builder, global.globalVar, use.parameter);

                // Resource-output specialization expects the same canonical representation the
                // front end uses for out/inout parameters: the body accesses a local, and every
                // normal return copies that local to the formal output parameter. Keeping arbitrary
                // body stores away from the formal also preserves an inout value on paths that do
                // not assign it.
                setInsertAtOriginalBodyStart(builder, functionIndex);
                use.replacementAddress = builder.emitVar(global.valueType);
                use.replacementAddress->sourceLoc = global.globalVar->sourceLoc;
                copyNameHint(builder, global.globalVar, use.replacementAddress);

                if (!writesValue || use.needsIncomingValue())
                {
                    auto inputValue = !writesValue
                                          ? static_cast<IRInst*>(use.parameter)
                                          : builder.emitLoad(global.valueType, use.parameter);
                    builder.emitStore(use.replacementAddress, inputValue);
                }

                if (hasAccess(use.access, ResourceGlobalAccess::Write))
                {
                    for (auto block : function.func->getBlocks())
                    {
                        auto returnInst = as<IRReturn>(block->getTerminator());
                        if (!returnInst)
                            continue;

                        builder.setInsertBefore(returnInst);
                        auto result = builder.emitLoad(global.valueType, use.replacementAddress);
                        builder.emitStore(use.parameter, result);
                    }
                }
            }
        }

        for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
        {
            bool changed = false;
            for (auto const& global : globals)
                changed |= global.functions[functionIndex].parameter != nullptr;
            if (changed)
            {
                fixUpFuncType(functions[functionIndex].func);
                fixUpDebugFuncType(functions[functionIndex].func);
            }
        }
    }

    void setInsertAtOriginalBodyStart(IRBuilder& builder, Index functionIndex)
    {
        auto const& function = functions[functionIndex];
        if (function.originalFirstOrdinaryInst)
            builder.setInsertBefore(function.originalFirstOrdinaryInst);
        else
            builder.setInsertInto(function.func->getFirstBlock());
    }

    void copyNameHint(IRBuilder& builder, IRInst* source, IRInst* target)
    {
        if (auto nameHint = source->findDecoration<IRNameHintDecoration>())
            builder.addNameHintDecoration(target, nameHint->getName());
    }

    void replaceGlobalUses()
    {
        List<IRInst*> moduleMetadataUsers;
        HashSet<IRInst*> seenModuleMetadataUsers;

        for (auto& global : globals)
        {
            for (auto const& rootUse : global.rootUses)
            {
                if (rootUse.functionIndex < 0)
                {
                    // Decorations and other module metadata disappear with the storage they name.
                    auto user = rootUse.use->getUser();
                    SLANG_RELEASE_ASSERT(!affectsResourceGlobalAccess(user));
                    if (user->getParent() != global.globalVar && seenModuleMetadataUsers.add(user))
                    {
                        moduleMetadataUsers.add(user);
                    }
                    continue;
                }

                auto replacement = global.functions[rootUse.functionIndex].replacementAddress;
                SLANG_RELEASE_ASSERT(replacement);
                rootUse.use->set(replacement);
            }
        }

        // Defer deletion until every saved use has been processed: one metadata instruction can
        // mention more than one resource global, and deleting it earlier would invalidate the
        // other globals' saved IRUse pointers.
        for (auto user : moduleMetadataUsers)
            user->removeAndDeallocate();
    }

    void rewriteCalls()
    {
        IRBuilder builder(module);

        for (auto const& edge : callEdges)
        {
            bool needsRewrite = false;
            for (auto const& global : globals)
                needsRewrite |= global.functions[edge.calleeIndex].parameter != nullptr;
            if (!needsRewrite)
                continue;

            auto oldCall = edge.call;
            List<IRInst*> args;
            for (UInt i = 0; i < oldCall->getArgCount(); ++i)
                args.add(oldCall->getArg(i));

            builder.setInsertBefore(oldCall);
            for (auto const& global : globals)
            {
                auto const& calleeUse = global.functions[edge.calleeIndex];
                if (!calleeUse.parameter)
                    continue;

                auto const& callerUse = global.functions[edge.callerIndex];
                SLANG_RELEASE_ASSERT(callerUse.replacementAddress);
                if (calleeUse.access == ResourceGlobalAccess::Read)
                    args.add(builder.emitLoad(global.valueType, callerUse.replacementAddress));
                else
                    args.add(callerUse.replacementAddress);
            }

            auto newCall = builder.emitCallInst(oldCall->getFullType(), oldCall->getCallee(), args);
            newCall->sourceLoc = oldCall->sourceLoc;
            oldCall->transferDecorationsTo(newCall);

            remapExplicitArgumentUseEffects(oldCall, newCall, edge.callerIndex);

            // The hidden address arguments use canonical out/inout types for downstream resource
            // specialization, but those types cannot express a conditional write that merely
            // preserves an incoming value without observing it. Record the exact argument use so
            // the source-level uninitialized-value check sees the callee's semantic read and
            // definite-write effects instead of inferring both from the generated ABI type.
            UInt hiddenArgIndex = oldCall->getArgCount();
            for (auto& global : globals)
            {
                auto const& calleeUse = global.functions[edge.calleeIndex];
                if (!calleeUse.parameter)
                    continue;

                auto& callerUse = global.functions[edge.callerIndex];
                if (hasAccess(calleeUse.access, ResourceGlobalAccess::Write))
                {
                    callerUse.uninitializedUseEffects.add(UninitializedVariableUseEffect{
                        .use = newCall->getOperandUse(hiddenArgIndex + 1),
                        .readsValue = calleeUse.semanticallyReadsIncomingValue,
                        .mayWriteValue = true,
                        .definitelyWritesValue = calleeUse.replacesWholeValueOnEveryReturn,
                    });
                }
                hiddenArgIndex++;
            }

            oldCall->replaceUsesWith(newCall);
            oldCall->removeAndDeallocate();
        }
    }

    void diagnoseUninitializedEntryPointReads(DiagnosticSink* sink)
    {
        for (auto const& global : globals)
        {
            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                if (!functions[functionIndex].isEntryPoint)
                    continue;
                auto replacement = global.functions[functionIndex].replacementAddress;
                if (!replacement)
                    continue;
                checkForUsingUninitializedVariable(
                    functions[functionIndex].func,
                    replacement,
                    global.functions[functionIndex].uninitializedUseEffects.getArrayView(),
                    sink);
            }
        }
    }
};

} // namespace

void legalizeResourceGlobalVars(
    IRModule* module,
    List<IRGlobalVar*> const& resourceDependentState,
    DiagnosticSink* sink)
{
    LegalizeResourceGlobalVarsPass pass(module);
    pass.processModule(resourceDependentState, sink);
}

} // namespace Slang
