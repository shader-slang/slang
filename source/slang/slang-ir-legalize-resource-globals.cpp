// slang-ir-legalize-resource-globals.cpp
//
// `legalizeResourceGlobalVars` removes mutable file-scope `static` resource variables from global
// IR storage.
// Consider this example:
//
//      Texture2D textures[1];
//      static Texture2D texture;
//
//      float4 loadTexture()
//      {
//          return texture.Load(int3(0));
//      }
//
//      float4 main()
//      {
//          texture = textures[0];
//          return loadTexture();
//      }
//
// A mutable file-scope `static` variable in shader code has a separate value for each shader
// invocation; it does not denote persistent program-wide storage. Targets that cannot represent
// such a resource value at global scope need the equivalent program to use an entry-point local and
// explicit arguments:
//
//      float4 loadTexture(Texture2D texture)
//      {
//          return texture.Load(int3(0));
//      }
//
//      float4 main()
//      {
//          Texture2D texture;
//          texture = textures[0];
//          return loadTexture(texture);
//      }
//
// Before analyzing uses, we move each selected initializer body into a function and insert the
// corresponding initialization at the start of every defined entry point. The selected globals
// then contain storage declarations without initializer code. We replace that storage in five
// phases:
//
// 1. We build a stable structural inventory of functions, resource globals, and direct calls.
// 2. We reject a variable if its address must remain externally visible or if executable code
//    outside a function uses it. We also reject any function that accesses the variable and can be
//    invoked without a direct `IRCall` whose arguments `legalizeResourceGlobalVars` can rewrite.
// 3. For each variable, we determine which functions may read or write its value. We follow
//    addresses derived from the global, and we propagate each non-entry-point function's effects to
//    its callers. We also determine whether every reachable normal return from a writer follows a
//    complete assignment. Finally, we reject uses that retain or compare an address, and calls that
//    pass the same writable value through both an existing argument and a generated argument.
// 4. We give affected entry points fresh locals. We give affected non-entry-point functions
//    generated parameters and local copies. We replace direct uses of the globals and append the
//    corresponding arguments to direct calls.
// 5. We check the now-explicit entry-point locals for reads before initialization, then remove the
//    unused global storage.
//
// This pass runs immediately before resource-type legalization. The front end admits only types that
// are represented by one IR value at this point: textures, samplers, ordinary structured buffers,
// byte-address buffers, and arrays of those types. The analysis recognizes whole-array assignment,
// but diagnoses code that would require combining separate element writes into a complete
// initialization proof. Parameter groups and append or consume buffers are represented by several
// IR values during target lowering. This pass cannot yet relate those values back to one source
// variable and replace them as a unit, so the front end rejects those types on every target for now.
// Running before resource-type legalization also lets the established legalization and
// simplification passes process the resource-typed locals and parameters that this pass creates.
//
// We choose each generated parameter type from the answers to two questions: can the function read
// the incoming value, and does every reachable normal return follow a complete assignment? A
// read-only non-entry-point function gets a value parameter. A writer gets an `out` parameter only
// when it does not read the incoming value and every reachable normal return follows a complete
// assignment. Every other writer gets `inout`, because some path can preserve or observe the
// incoming value. `specializeResourceOutputs` consumes the generated copy-in, local, and copy-out
// pattern.
#include "slang-ir-legalize-resource-globals.h"

#include "slang-diagnostics.h"
#include "slang-ir-dominators.h"
#include "slang-ir-explicit-global-init.h"
#include "slang-ir-insts.h"
#include "slang-ir-use-uninitialized-values.h"
#include "slang-ir-util.h"

namespace Slang
{

namespace
{

/// A `ResourceAccess` value records whether a function may read or write one resource value.
///
/// These flags describe possible effects and are propagated transitively from callees to callers.
/// Definite whole-value replacement is tracked separately because `Write` alone does not say
/// whether every path to a reachable normal return performs a complete assignment.
enum class ResourceAccess : UInt
{
    None = 0,
    Read = 1 << 0,
    Write = 1 << 1,
};

/// A `FunctionContext` records structural facts and cached CFG analysis for one function.
///
/// Functions retain their structural preorder so that generated parameters are deterministic. The
/// original body position lets phase 4 insert locals after parameters but before pre-existing code.
struct FunctionContext
{
    /// The function represented by this record.
    IRFunc* func = nullptr;

    /// The first ordinary instruction present before this pass rewrites resource uses.
    IRInst* firstPreRewriteOrdinaryInst = nullptr;

    /// The lazily computed CFG dominator tree shared by every per-resource proof in this function.
    RefPtr<IRDominatorTree> dominatorTree;

    /// Whether this function is an entry point and therefore creates its own replacement locals.
    bool isEntryPoint = false;

    /// Whether this non-entry-point function may be invoked without an in-module direct call.
    bool mayBeInvokedWithoutDirectCall = false;
};

/// A `FunctionResourceInfo` records the analysis and rewrite decisions for one function and
/// one resource global.
struct FunctionResourceInfo
{
    /// The possible semantic read/write effects, including effects propagated from callees.
    ResourceAccess access = ResourceAccess::None;

    /// Whether every reachable normal return follows a complete assignment of this value.
    bool replacesWholeValueOnEveryReturn = false;

    /// Whether some execution path observes the value supplied by the caller before replacing it.
    bool semanticallyReadsIncomingValue = false;

    /// Whether a reachable return path can preserve the value supplied by the caller.
    ///
    /// We copy the caller's value into the generated local when such a path exists.
    bool mustPreserveIncomingValue = false;

    /// Whether this function or one of its callees may write an element of the value separately.
    bool mayWriteSubobject = false;

    /// Whether this function contains a direct use of the global, including debug metadata.
    bool hasDirectGlobalUse = false;

    /// The generated local address, once phase 4 gives this function a representation of the value.
    IRInst* replacementAddress = nullptr;

    /// The generated parameter for a non-entry-point function that accesses the resource.
    ///
    /// This field remains null for entry points, before phase 4, and when a local exists only to
    /// replace debug metadata.
    IRParam* parameter = nullptr;

    /// Per-use effect overrides retained for the generated local's uninitialized-value check.
    List<UninitializedVariableUseEffect> uninitializedUseEffects;

    /// Return whether the function must copy its caller's value into the generated local.
    bool mustSeedLocalFromCaller() const
    {
        return semanticallyReadsIncomingValue || mustPreserveIncomingValue;
    }
};

/// A `DirectGlobalUse` identifies a direct use and the function body that contains it, if any.
struct DirectGlobalUse
{
    IRUse* use = nullptr;

    /// The containing function's structural-inventory index, or -1 for module-scope metadata.
    Index functionIndex = -1;
};

/// A `ResourceAddressUse` describes a semantic use of the global or an address derived from it.
struct ResourceAddressUse
{
    /// The operand use through which the terminal instruction accesses the address.
    IRUse* use = nullptr;

    /// The containing function's stable structural-inventory index.
    Index functionIndex = -1;

    /// The possible read/write effect of this terminal instruction.
    ResourceAccess access = ResourceAccess::None;

    /// Whether this instruction completely replaces the root value, not merely a subobject.
    bool replacesWholeValue = false;
};

/// A `ResourceGlobalToRewrite` holds the analysis results and generated IR for one resource global.
///
/// Phases 1 through 3 populate the original uses and effect records. Phase 4 then records the local
/// and parameter that replace the global in each affected function, without discarding the analysis
/// that phase 5 needs for uninitialized-value checking.
struct ResourceGlobalToRewrite
{
    /// The original storage whose uses phase 4 will replace and whose declaration phase 5 removes.
    IRGlobalVar* globalVar = nullptr;

    /// The value type used for every generated local and parameter.
    IRType* valueType = nullptr;

    /// One analysis/rewrite record per function in the module inventory.
    List<FunctionResourceInfo> perFunctionInfo;

    /// Every direct use-list edge from the global, saved before phase 4 mutates the use list.
    List<DirectGlobalUse> rootUses;

    /// Semantic reads and writes reached by following each root through derived addresses.
    List<ResourceAddressUse> terminalUses;

    /// Call-argument address uses retained for alias validation before rewriting.
    List<IRUse*> addressPassingUses;

    /// Uses that may retain the address or observe that separate functions now use separate locals.
    List<IRUse*> unsupportedAddressUses;
};

/// A `DirectCallEdge` records one direct call between functions in the structural inventory.
struct DirectCallEdge
{
    IRCall* call = nullptr;
    Index callerIndex = -1;
    Index calleeIndex = -1;
};

// We present the pass before these low-level classifiers so that the file reads in algorithm order.
// Their definitions follow the transformation that uses them.
static ResourceAccess mergeAccess(ResourceAccess left, ResourceAccess right);
static bool hasAccess(ResourceAccess value, ResourceAccess test);
static bool doesInstSemanticallyUseOperandValue(IRInst* user);
static ResourceAccess classifyCallArgumentAccess(IRCall* call, IRUse* use);
static ResourceAccess classifyResourceAddressUse(IRUse* use);
static bool doesUsePreserveWholeAddress(IRUse* use);
static bool isAddressUseUnsupportedByPerFunctionStorage(IRUse* use);
static bool doesDecorationInvokeReferencedFunction(IRInst* user);
static bool hasFunctionUseThatCannotBeRewritten(IRFunc* func);
static bool mayBeInvokedWithoutDirectCall(IRFunc* func);
static bool requiresExternallyVisibleStorage(IRGlobalVar* globalVar);

/// A `LegalizeResourceGlobalVarsPass` implements the five-phase transformation described at the
/// top of this file.
///
/// The pass records its call-graph and address-use analysis before it changes any resource-global
/// uses. The initializer bodies have already moved by then, but an unsupported program cannot be
/// left with only part of its resource uses rewritten.
struct LegalizeResourceGlobalVarsPass
{
    IRModule* module = nullptr;
    List<FunctionContext> functions;
    Dictionary<IRFunc*, Index> functionIndices;
    List<ResourceGlobalToRewrite> resourceGlobals;
    List<DirectCallEdge> directCallEdges;

    explicit LegalizeResourceGlobalVarsPass(IRModule* inModule)
        : module(inModule)
    {
    }

    /// Run the five phases described at the top of this file.
    void processModule(DiagnosticSink* sink)
    {
        // In phase 1, we record every function in a stable structural order. Later phases use the
        // recorded indices, so their output does not depend on hash-table or use-list order.
        collectFunctions();
        collectResourceGlobals();
        collectDirectCalls();

        if (resourceGlobals.getCount() == 0)
            return;

        // In phase 2, we reject three cases that per-function locals cannot represent. First, an ABI
        // or storage decoration may require the global address to remain externally visible.
        // Second, executable code outside a function has no function-local replacement address.
        // Third, a function may be invoked without a direct call whose argument list we can extend;
        // this includes a generic body whose eventual calls do not exist until it is instantiated.
        //
        // TODO: Add a target-independent validation stage after IR modules are linked, and move the
        // phase-2 validations into that stage. Semantic checking currently runs on each module
        // separately, before the linker reveals every possible caller of a function.
        bool diagnosedUnsupportedProgram = diagnoseGlobalsRequiringExternallyVisibleStorage(sink);
        diagnosedUnsupportedProgram |= diagnoseResourceUsesOutsideFunctions(sink);
        diagnosedUnsupportedProgram |= diagnoseFunctionsWithUnrewritableInvocations(sink);
        if (diagnosedUnsupportedProgram)
            return;

        // In phase 3, we determine whether each function reads or writes each resource value. These
        // results select value, `out`, or `inout` parameters in phase 4. They also tell the
        // uninitialized-value analysis in phase 5 which writes definitely assign the complete value.
        // Before rewriting, we reject arrays with element writes and a possible read before a
        // whole-array assignment. We also reject uses that retain or compare an address, and calls
        // that would receive two writable aliases for the same original global. Phase 2 has already
        // diagnosed every affected function that may be invoked without a direct call.
        analyzeResourceAccess();
        if (diagnoseElementWiseArrayInitialization(sink))
            return;
        recordPossibleWriteEffectsForUninitializedValueCheck();
        if (diagnoseUnsupportedAddressUses(sink))
            return;
        if (diagnoseConflictingCallAliases(sink))
            return;
        assertFunctionsThatNeedParametersHaveOnlyDirectCalls();

        // In phase 4, we create one local in every affected function. Each affected non-entry-point
        // function also receives a generated parameter. We redirect the old global uses and append
        // arguments to each direct call.
        introduceReplacements();
        replaceGlobalUses();
        rewriteCalls();

        // In phase 5, we apply the shared uninitialized-value analysis to each generated
        // entry-point local. That analysis reports both reads with no reaching write and reads that
        // lack a definite write on every path. Successful rewriting leaves the original globals
        // unused and safe to delete.
        diagnoseUninitializedEntryPointReads(sink);
        removeReplacedResourceGlobals();
    }

    // ## Phase 1: Stable structural inventory

    /// Collect functions and determine whether each one may be invoked without a direct call.
    void collectFunctions()
    {
        // We traverse the module's structural tree in child order. Most functions are direct
        // children of the module, but an unspecialized generic contains its function body inside an
        // `IRGeneric`. We include those nested bodies in the call graph so phase 2 can reject a
        // resource access that this pass cannot rewrite safely.
        collectFunctionsUnder(module->getModuleInst());
    }

    /// Collect every function structurally contained by `parent` in a stable preorder.
    void collectFunctionsUnder(IRInst* parent)
    {
        // A function nested under an `IRGeneric` can be instantiated after this pass. Because the
        // eventual calls do not exist yet, phase 4 cannot add an argument to them. We therefore
        // record a nested function as independently invoked. If it accesses a selected resource,
        // phase 2 diagnoses it before any signature changes are made.
        for (auto inst : parent->getChildren())
        {
            auto func = as<IRFunc>(inst);
            if (!func)
            {
                collectFunctionsUnder(inst);
                continue;
            }

            FunctionContext context;
            context.func = func;
            context.isEntryPoint = func->findDecoration<IREntryPointDecoration>() != nullptr;
            context.mayBeInvokedWithoutDirectCall =
                !context.isEntryPoint &&
                (func->getParent() != module->getModuleInst() || mayBeInvokedWithoutDirectCall(func));
            if (auto firstBlock = func->getFirstBlock())
                context.firstPreRewriteOrdinaryInst = firstBlock->getFirstOrdinaryInst();

            auto index = functions.getCount();
            functions.add(context);
            functionIndices.add(func, index);
        }
    }

    /// Collect the file-scope `static` resource globals that this pass must replace.
    ///
    /// The initializer mover has already removed each selected initializer body. Keeping these
    /// records in module order makes the order of generated parameters deterministic.
    void collectResourceGlobals()
    {
        // The shared predicate requires the marker emitted for a file-scope `static` variable and a
        // resource type that remains one IR value through this pipeline point. The front-end check
        // is responsible for rejecting source types outside the supported subset.
        for (auto inst : module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar || !isFileScopeStaticResourceGlobalToReplace(globalVar))
                continue;

            auto ptrType = cast<IRPtrTypeBase>(globalVar->getDataType());
            SLANG_RELEASE_ASSERT(!globalVar->getFirstBlock());

            ResourceGlobalToRewrite resourceGlobal;
            resourceGlobal.globalVar = globalVar;
            resourceGlobal.valueType = ptrType->getValueType();
            for (Index i = 0; i < functions.getCount(); ++i)
                resourceGlobal.perFunctionInfo.add(FunctionResourceInfo());
            resourceGlobals.add(_Move(resourceGlobal));
        }
    }

    /// Collect direct calls whose caller and callee both appear in the structural inventory.
    void collectDirectCalls()
    {
        // We record these calls so phase 3 can propagate resource access from callees to callers.
        // Phase 4 uses the same records to append the generated resource arguments.
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
                    auto calleeIndex = callee ? functionIndices.tryGetValue(callee) : nullptr;
                    if (!calleeIndex)
                        continue;

                    directCallEdges.add(DirectCallEdge{call, callerIndex, *calleeIndex});
                }
            }
        }
    }

    // ## Phase 2: Cases that cannot use per-function storage

    /// Diagnose resource globals whose storage must remain externally visible.
    ///
    /// Replacing such a variable with a distinct local in every entry point would change its
    /// address or lifetime.
    bool diagnoseGlobalsRequiringExternallyVisibleStorage(DiagnosticSink* sink)
    {
        // We cannot replace one externally visible address with a distinct local in each entry
        // point. We diagnose this case before rewriting any resource-global use or function
        // signature.
        bool diagnosed = false;
        for (auto const& resourceGlobal : resourceGlobals)
        {
            auto globalVar = resourceGlobal.globalVar;
            if (!requiresExternallyVisibleStorage(globalVar))
                continue;
            sink->diagnose(Diagnostics::ResourceStaticRequiresExternallyVisibleStorage{
                .variable = globalVar,
                .location = globalVar->sourceLoc});
            diagnosed = true;
        }
        return diagnosed;
    }

    /// Diagnose executable uses of a resource global that do not belong to a function.
    bool diagnoseResourceUsesOutsideFunctions(DiagnosticSink* sink)
    {
        // The rewrite assigns one local to each function. A semantic use in another global's
        // initializer has no such local, so we reject it explicitly instead of treating it as
        // metadata that can disappear with the old global.
        bool diagnosed = false;
        for (auto const& resourceGlobal : resourceGlobals)
        {
            for (auto use = resourceGlobal.globalVar->firstUse; use; use = use->nextUse)
            {
                auto user = use->getUser();
                if (!doesInstSemanticallyUseOperandValue(user) || getParentFunc(user))
                    continue;

                sink->diagnose(Diagnostics::ResourceStaticUsedOutsideFunction{
                    .variable = resourceGlobal.globalVar,
                    .location = user->sourceLoc});
                diagnosed = true;
            }
        }
        return diagnosed;
    }

    /// Return which functions directly or transitively access a selected resource global.
    List<bool> computeFunctionsAccessingAnyResourceGlobal()
    {
        // We first mark functions with a direct semantic use. We then repeatedly mark their callers
        // until the reverse-call closure stops changing. The fixed point includes recursive call
        // cycles without depending on function order.
        List<bool> accessesResource;
        for (Index i = 0; i < functions.getCount(); ++i)
            accessesResource.add(false);

        for (auto const& resourceGlobal : resourceGlobals)
        {
            for (auto use = resourceGlobal.globalVar->firstUse; use; use = use->nextUse)
            {
                if (!doesInstSemanticallyUseOperandValue(use->getUser()))
                    continue;
                auto parentFunc = getParentFunc(use->getUser());
                auto functionIndex = parentFunc ? functionIndices.tryGetValue(parentFunc) : nullptr;
                if (functionIndex)
                    accessesResource[*functionIndex] = true;
            }
        }

        bool changed = false;
        do
        {
            changed = false;
            for (auto const& edge : directCallEdges)
            {
                if (!accessesResource[edge.calleeIndex] || accessesResource[edge.callerIndex])
                    continue;
                accessesResource[edge.callerIndex] = true;
                changed = true;
            }
        } while (changed);
        return accessesResource;
    }

    /// Diagnose functions that access a resource global but may run without a rewritable call.
    bool diagnoseFunctionsWithUnrewritableInvocations(DiagnosticSink* sink)
    {
        // We can append a resource argument to a direct call. We cannot change the arguments used
        // by an external caller or by a decoration such as `IRPatchConstantFuncDecoration`. An
        // entry point that is also used as an ordinary function needs two implementations: one that
        // creates the entry-point local, and one that receives the caller's value. This pass does
        // not split an entry point into those two implementations.
        bool diagnosed = false;
        auto accessesResource = computeFunctionsAccessingAnyResourceGlobal();

        List<bool> hasDirectCaller;
        for (Index i = 0; i < functions.getCount(); ++i)
            hasDirectCaller.add(false);
        for (auto const& edge : directCallEdges)
            hasDirectCaller[edge.calleeIndex] = true;

        for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
        {
            if (!accessesResource[functionIndex])
                continue;

            auto const& function = functions[functionIndex];
            if (function.isEntryPoint &&
                (hasDirectCaller[functionIndex] ||
                 hasFunctionUseThatCannotBeRewritten(function.func)))
            {
                sink->diagnose(Diagnostics::ResourceStaticUsedByCallableEntryPoint{
                    .function = function.func,
                    .location = function.func->sourceLoc});
                diagnosed = true;
            }
            else if (function.mayBeInvokedWithoutDirectCall)
            {
                sink->diagnose(Diagnostics::ResourceStaticUsedByFunctionWithoutRewritableCall{
                    .function = function.func,
                    .location = function.func->sourceLoc});
                diagnosed = true;
            }
        }
        return diagnosed;
    }

    // ## Phase 3: Resource reads, writes, and address validation

    /// Analyze how every function reads, writes, and replaces every resource global.
    ///
    /// For each global, we first classify direct and derived-address uses and then propagate
    /// possible effects through callers. Finally, we prove which writers replace the whole value
    /// on all returns and which reads or partial writers therefore need the caller's incoming
    /// value.
    void analyzeResourceAccess()
    {
        // We analyze one global at a time. We first record its direct and derived-address uses,
        // then propagate callee effects to callers. With those possible effects known, we determine
        // whether each writer assigns the complete value on every path and whether it needs the
        // value supplied by its caller.
        for (auto& resourceGlobal : resourceGlobals)
        {
            collectRootUses(resourceGlobal);

            HashSet<IRInst*> visitedAddresses;
            analyzeDerivedAddressUses(
                resourceGlobal,
                resourceGlobal.globalVar,
                visitedAddresses,
                true);

            propagateEffectsToCallers(resourceGlobal);
            determineWholeValueReplacementsOnEveryReturn(resourceGlobal);
            determineIncomingValueRequirements(resourceGlobal);
        }
    }

    /// Save every direct use of the global before phase 4 mutates any use list.
    ///
    /// Function-local uses establish where replacement storage is required. Module-scope metadata
    /// is retained only so `replaceGlobalUses` can remove it safely with the obsolete global.
    void collectRootUses(ResourceGlobalToRewrite& global)
    {
        // We record function membership now because phase 4 will replace the use-list links. A use
        // outside a function must be non-semantic metadata; phase 2 has already diagnosed any
        // executable use outside a function.
        for (auto use = global.globalVar->firstUse; use; use = use->nextUse)
        {
            DirectGlobalUse rootUse;
            rootUse.use = use;

            if (auto parentFunc = getParentFunc(use->getUser()))
            {
                if (auto index = functionIndices.tryGetValue(parentFunc))
                {
                    rootUse.functionIndex = *index;
                    global.perFunctionInfo[*index].hasDirectGlobalUse = true;
                }
            }

            global.rootUses.add(rootUse);
        }
    }

    /// Record one semantic use reached at the end of a derived-address chain.
    ///
    /// A terminal use contributes read/write effects to its containing function. We also retain
    /// calls and escaping addresses for validation later in this phase. We also distinguish a
    /// complete assignment from a write to only part of the value.
    void analyzeTerminalAddressUse(
        ResourceGlobalToRewrite& global,
        IRUse* use,
        bool representsWholeValue)
    {
        // We retain call arguments and escaping uses for the later validity checks. We then add the
        // use's read/write effects to its function and record whether a write assigns the complete
        // resource value.
        auto user = use->getUser();
        if (as<IRCall>(user))
            global.addressPassingUses.add(use);
        if (isAddressUseUnsupportedByPerFunctionStorage(use))
            global.unsupportedAddressUses.add(use);

        auto parentFunc = getParentFunc(user);
        SLANG_RELEASE_ASSERT(parentFunc);
        auto functionIndex = functionIndices.tryGetValue(parentFunc);
        SLANG_RELEASE_ASSERT(functionIndex);

        auto terminalAccess = classifyResourceAddressUse(use);
        auto& functionInfo = global.perFunctionInfo[*functionIndex];
        functionInfo.access = mergeAccess(functionInfo.access, terminalAccess);
        if (!representsWholeValue && hasAccess(terminalAccess, ResourceAccess::Write))
            functionInfo.mayWriteSubobject = true;

        bool replacesWholeValue = false;
        if (representsWholeValue)
        {
            if (auto store = as<IRStore>(user))
                replacesWholeValue = store->ptr.get() == use->get();
            else if (as<IRCall>(user))
                replacesWholeValue = terminalAccess == ResourceAccess::Write;
        }
        global.terminalUses.add(
            ResourceAddressUse{use, *functionIndex, terminalAccess, replacesWholeValue});
    }

    /// Walk the address graph rooted at one resource global and classify its semantic uses.
    ///
    /// For each use, we either recurse through another derived address or hand the terminal use to
    /// `analyzeTerminalAddressUse`. We carry whether the current address still denotes the whole
    /// root value so terminal writes can distinguish replacement from partial mutation.
    void analyzeDerivedAddressUses(
        ResourceGlobalToRewrite& global,
        IRInst* address,
        HashSet<IRInst*>& visitedAddresses,
        bool representsWholeValue)
    {
        // We visit each derived address once. Field and element addresses denote only a subobject.
        // A pointer cast still denotes the complete value only when it preserves the pointee type,
        // so a store through any other derived address cannot satisfy an `out` contract.
        if (!visitedAddresses.add(address))
            return;

        for (auto use = address->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (!doesInstSemanticallyUseOperandValue(user))
                continue;

            if (isUseBaseOfDerivedAddress(use))
            {
                auto derivedAddressRepresentsWholeValue =
                    representsWholeValue && doesUsePreserveWholeAddress(use);
                analyzeDerivedAddressUses(
                    global,
                    user,
                    visitedAddresses,
                    derivedAddressRepresentsWholeValue);
                continue;
            }
            analyzeTerminalAddressUse(global, use, representsWholeValue);
        }
    }

    /// Propagate possible read, write, and subobject-write effects from callees to callers.
    ///
    /// Iterating direct calls to a fixed point handles ordinary chains and recursive strongly
    /// connected components without relying on a particular function order.
    void propagateEffectsToCallers(ResourceGlobalToRewrite& global)
    {
        // We repeatedly merge each callee's effects into its caller. We also propagate whether a
        // callee may write only a subobject, because the array limitation checked below applies to
        // indirect writes as well as direct ones. Iteration stops when no edge changes, which
        // handles recursive calls as well as ordinary call chains.
        bool changed = false;
        do
        {
            changed = false;
            for (auto const& edge : directCallEdges)
            {
                auto calleeAccess = global.perFunctionInfo[edge.calleeIndex].access;
                auto calleeMayWriteSubobject =
                    global.perFunctionInfo[edge.calleeIndex].mayWriteSubobject;
                auto& callerInfo = global.perFunctionInfo[edge.callerIndex];
                auto mergedAccess = mergeAccess(callerInfo.access, calleeAccess);
                if (mergedAccess != callerInfo.access ||
                    (calleeMayWriteSubobject && !callerInfo.mayWriteSubobject))
                {
                    callerInfo.access = mergedAccess;
                    callerInfo.mayWriteSubobject |= calleeMayWriteSubobject;
                    changed = true;
                }
            }
        } while (changed);
    }

    /// Diagnose a resource array whose initialization would require element-sensitive analysis.
    bool diagnoseElementWiseArrayInitialization(DiagnosticSink* sink)
    {
        // A whole-array assignment is one instruction, so the phase-3 CFG proof can recognize it.
        // Proving that separate element stores cover the array would require tracking which indices
        // are written on each path. Until that analysis exists, we reject only an array that both
        // has subobject writes and may be read before a whole-array assignment. Phase 2 rejected
        // every affected non-entry-point function that can run without a recorded call, and the
        // preceding fixed points propagated both facts through those calls. We can therefore make
        // this decision from the summary at each entry point.
        bool diagnosed = false;
        for (auto const& global : resourceGlobals)
        {
            if (!as<IRArrayTypeBase>(unwrapAttributedType(global.valueType)))
                continue;

            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                auto const& functionInfo = global.perFunctionInfo[functionIndex];
                if (!functions[functionIndex].isEntryPoint || !functionInfo.mayWriteSubobject ||
                    !functionInfo.semanticallyReadsIncomingValue)
                {
                    continue;
                }

                sink->diagnose(Diagnostics::ResourceStaticArrayElementInitializationNotProven{
                    .variable = global.globalVar,
                    .location = global.globalVar->sourceLoc});
                diagnosed = true;
                break;
            }
        }
        return diagnosed;
    }

    /// Determine which writers assign the complete resource value on every normal return path.
    ///
    /// A call counts as a complete assignment only after its callee has been proved to assign the
    /// complete value. Repeating the proof therefore propagates facts through the call graph.
    void determineWholeValueReplacementsOnEveryReturn(ResourceGlobalToRewrite& global)
    {
        // The property is conditional: if a function returns normally, every returning path must
        // first replace the value. We therefore begin by treating every writer as a candidate and
        // repeatedly remove candidates that fail the CFG proof. Starting from this greatest
        // candidate set lets a terminating recursive call establish the value, while a reachable
        // return without a replacement still removes every caller that depends on it.
        for (auto& resourceInfo : global.perFunctionInfo)
        {
            resourceInfo.replacesWholeValueOnEveryReturn =
                hasAccess(resourceInfo.access, ResourceAccess::Write);
        }

        bool changed = false;
        do
        {
            changed = false;
            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                auto& resourceInfo = global.perFunctionInfo[functionIndex];
                if (!resourceInfo.replacesWholeValueOnEveryReturn)
                    continue;

                if (!doesFunctionReplaceWholeValueOnEveryReturn(global, functionIndex))
                {
                    resourceInfo.replacesWholeValueOnEveryReturn = false;
                    changed = true;
                }
            }
        } while (changed);
    }

    /// Return whether every reachable normal return follows a complete assignment.
    ///
    /// A function with no reachable normal return also satisfies this condition because its caller
    /// cannot observe a returned value.
    bool doesFunctionReplaceWholeValueOnEveryReturn(
        ResourceGlobalToRewrite& global,
        Index functionIndex)
    {
        // We collect the direct assignments and calls that are already known to assign the complete
        // value. A reachable return disproves the contract if it can execute before one of those
        // instructions. When there is no reachable return, no caller needs a value copied back.
        HashSet<IRInst*> replacements;
        collectWholeValueReplacementInstructions(global, functionIndex, replacements);

        auto dominatorTree = getDominatorTree(functionIndex);
        for (auto block : functions[functionIndex].func->getBlocks())
        {
            if (dominatorTree->isUnreachable(block))
                continue;

            auto returnInst = as<IRReturn>(block->getTerminator());
            if (!returnInst)
                continue;

            if (canReachInstructionBeforeWholeValueReplacement(
                    functionIndex,
                    returnInst,
                    replacements))
                return false;
        }

        return true;
    }

    /// Collect instructions that establish a complete resource value in this function.
    ///
    /// A direct whole-value store establishes the value immediately. A call does so only after the
    /// fixed-point analysis proves the same property for its callee.
    void collectWholeValueReplacementInstructions(
        ResourceGlobalToRewrite& global,
        Index functionIndex,
        HashSet<IRInst*>& replacements)
    {
        // We collect direct assignments first, then calls whose callees remain in the current
        // fixed-point candidate set.
        for (auto const& terminalUse : global.terminalUses)
        {
            if (terminalUse.functionIndex == functionIndex && terminalUse.replacesWholeValue)
                replacements.add(terminalUse.use->getUser());
        }

        for (auto const& edge : directCallEdges)
        {
            if (edge.callerIndex == functionIndex &&
                global.perFunctionInfo[edge.calleeIndex].replacesWholeValueOnEveryReturn)
            {
                replacements.add(edge.call);
            }
        }
    }

    /// Return the dominator tree used to exclude unreachable returns from all-path proofs.
    ///
    /// We ask the same CFG questions for every resource value. We compute the tree lazily on the
    /// first query and retain it in the function record so later resources reuse the same analysis.
    IRDominatorTree* getDominatorTree(Index functionIndex)
    {
        // We compute this function-level structure lazily and reuse it for every resource global.
        auto& function = functions[functionIndex];
        if (!function.dominatorTree)
            function.dominatorTree = computeDominatorTree(function.func);
        return function.dominatorTree;
    }

    /// Return whether control can reach `target` before a complete assignment.
    ///
    /// The worklist contains paths on which the resource still has its incoming value. A complete
    /// assignment ends such a path. Reaching `target` first proves that the incoming value can reach
    /// it.
    bool canReachInstructionBeforeWholeValueReplacement(
        Index functionIndex,
        IRInst* target,
        HashSet<IRInst*> const& replacements)
    {
        // We search forward from the entry block and stop each path at its first complete
        // assignment. Each block needs at most one visit because the worklist represents only the
        // single state in which the incoming value is still present.
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
                // A call can read the incoming value before it returns a replacement. We therefore
                // test the target before treating that same instruction as an assignment.
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

    /// Determine which functions need the resource value supplied by their caller.
    ///
    /// We first mark partial or conditional writers that must preserve an incoming value even if
    /// they never read it semantically. We next classify direct reads by whether every path to them
    /// crosses a replacement. Finally, we propagate callee read requirements to callers, stopping
    /// when an earlier replacement supplies the value instead.
    void determineIncomingValueRequirements(ResourceGlobalToRewrite& global)
    {
        // A writer that can return without replacing the complete value must preserve its input,
        // even when it never reads that input directly. We then find direct reads reachable before
        // a complete assignment and propagate those read requirements from callees to callers.
        for (auto& resourceInfo : global.perFunctionInfo)
        {
            if (hasAccess(resourceInfo.access, ResourceAccess::Write) &&
                !resourceInfo.replacesWholeValueOnEveryReturn)
            {
                resourceInfo.mustPreserveIncomingValue = true;
            }
        }

        for (auto const& terminalUse : global.terminalUses)
        {
            if (!hasAccess(terminalUse.access, ResourceAccess::Read))
                continue;
            if (!hasPriorWholeValueReplacement(
                    global,
                    terminalUse.functionIndex,
                    terminalUse.use->getUser()))
            {
                global.perFunctionInfo[terminalUse.functionIndex].semanticallyReadsIncomingValue =
                    true;
            }
        }

        bool changed = false;
        do
        {
            changed = false;
            for (auto const& edge : directCallEdges)
            {
                if (!global.perFunctionInfo[edge.calleeIndex].semanticallyReadsIncomingValue)
                    continue;
                auto& caller = global.perFunctionInfo[edge.callerIndex];
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

    /// Return whether every path to `read` first assigns the complete resource value.
    ///
    /// Direct whole-value stores and calls to proved callees are the instructions that establish
    /// the value.
    bool hasPriorWholeValueReplacement(
        ResourceGlobalToRewrite& global,
        Index functionIndex,
        IRInst* read)
    {
        // Every path has assigned the value exactly when no path can reach the read before one of
        // the collected assignment instructions.
        HashSet<IRInst*> replacements;
        collectWholeValueReplacementInstructions(global, functionIndex, replacements);
        return replacements.getCount() != 0 &&
               !canReachInstructionBeforeWholeValueReplacement(functionIndex, read, replacements);
    }

    /// Record writes that ordinary IR-use classification would mistake for complete assignments.
    ///
    /// A subobject store or an `inout` call may write the value but does not initialize the whole
    /// value on every path. We preserve that semantic distinction for the phase-5 CFG solver.
    void recordPossibleWriteEffectsForUninitializedValueCheck()
    {
        // A subobject or conditional write is possible, but it does not definitely initialize the
        // complete value. We attach that exact effect to the original use so the later
        // uninitialized-value check does not infer a stronger effect from the opcode or formal
        // parameter type.
        for (auto& global : resourceGlobals)
        {
            for (auto const& terminalUse : global.terminalUses)
            {
                if (!hasAccess(terminalUse.access, ResourceAccess::Write) ||
                    terminalUse.replacesWholeValue)
                {
                    continue;
                }

                auto& resourceInfo = global.perFunctionInfo[terminalUse.functionIndex];
                resourceInfo.uninitializedUseEffects.add(UninitializedVariableUseEffect{
                    .use = terminalUse.use,
                    .readsValue = hasAccess(terminalUse.access, ResourceAccess::Read),
                    .mayWriteValue = true,
                    .definitelyWritesValue = false,
                });
            }
        }
    }

    /// Diagnose address uses that separate per-function storage cannot preserve.
    ///
    /// Phase 4 gives each entry point and ordinary function its own storage. Code that retains an
    /// address or compares it with another address could observe that change in storage identity.
    bool diagnoseUnsupportedAddressUses(DiagnosticSink* sink)
    {
        // We reject each use for which replacing one global address with several local addresses
        // could change the program's observable behavior.
        bool diagnosed = false;
        for (auto const& resourceGlobal : resourceGlobals)
        {
            for (auto use : resourceGlobal.unsupportedAddressUses)
            {
                sink->diagnose(Diagnostics::ResourceStaticAddressHasUnsupportedUse{
                    .variable = resourceGlobal.globalVar,
                    .location = use->getUser()->sourceLoc});
                diagnosed = true;
            }
        }
        return diagnosed;
    }

    /// Diagnose calls that can modify one resource through both an explicit and generated argument.
    ///
    /// When either path can write, separate local copies would no longer behave like the original
    /// shared global address.
    bool diagnoseConflictingCallAliases(DiagnosticSink* sink)
    {
        // Two read-only arguments may safely contain the same resource value. If either path can
        // write, however, phase 4 would give the callee separate local copies. A write through one
        // copy would not be visible through the other, unlike the original shared global address.
        bool diagnosed = false;
        for (auto const& resourceGlobal : resourceGlobals)
        {
            HashSet<IRCall*> diagnosedCalls;
            for (auto use : resourceGlobal.addressPassingUses)
            {
                auto call = cast<IRCall>(use->getUser());
                auto callee = as<IRFunc>(call->getCallee());
                auto calleeIndex = callee ? functionIndices.tryGetValue(callee) : nullptr;
                if (!calleeIndex)
                    continue;

                auto implicitAccess = resourceGlobal.perFunctionInfo[*calleeIndex].access;
                if (implicitAccess == ResourceAccess::None)
                    continue;

                auto explicitAccess = classifyCallArgumentAccess(call, use);
                if (implicitAccess == ResourceAccess::Read &&
                    explicitAccess == ResourceAccess::Read)
                {
                    continue;
                }
                if (!diagnosedCalls.add(call))
                    continue;

                sink->diagnose(Diagnostics::ResourceStaticHasConflictingCallAliases{
                    .variable = resourceGlobal.globalVar,
                    .function = callee,
                    .location = call->sourceLoc});
                diagnosed = true;
            }
        }
        return diagnosed;
    }

    /// Assert that every function needing a resource parameter has only direct-call uses.
    void assertFunctionsThatNeedParametersHaveOnlyDirectCalls()
    {
        // Phase 2 has already diagnosed exported functions and non-call references. This assertion
        // ensures that phase 4 cannot change a signature while leaving an unrewritten invocation.
        for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
        {
            auto func = functions[functionIndex].func;
            if (functions[functionIndex].isEntryPoint)
                continue;

            bool needsParameter = false;
            for (auto const& global : resourceGlobals)
            {
                needsParameter |=
                    global.perFunctionInfo[functionIndex].access != ResourceAccess::None;
            }
            if (!needsParameter)
                continue;

            for (auto use = func->firstUse; use; use = use->nextUse)
            {
                auto user = use->getUser();
                if (!doesInstSemanticallyUseOperandValue(user))
                    continue;

                auto call = as<IRCall>(user);
                SLANG_RELEASE_ASSERT(call && call->getCalleeUse() == use);
                SLANG_RELEASE_ASSERT(getParentFunc(call));
            }
        }
    }

    // ## Phase 4: Locals, parameters, and call arguments

    /// Create a local for each affected function and a parameter for each non-entry-point function.
    void introduceReplacements()
    {
        // Each affected entry point creates a fresh local. Each affected non-entry-point function
        // receives a parameter and operates on a local copy. After all parameters exist, we rebuild
        // each affected function type and its debug type once.
        IRBuilder builder(module);

        for (auto& global : resourceGlobals)
        {
            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                auto const& function = functions[functionIndex];
                auto& resourceInfo = global.perFunctionInfo[functionIndex];
                if (resourceInfo.access == ResourceAccess::None &&
                    !resourceInfo.hasDirectGlobalUse)
                    continue;

                if (function.isEntryPoint || resourceInfo.access == ResourceAccess::None)
                    createReplacementLocal(builder, functionIndex, global, resourceInfo);
                else
                    createParameterAndLocalForNonEntryPointFunction(
                        builder,
                        functionIndex,
                        global,
                        resourceInfo);
            }
        }

        for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
        {
            bool hasGeneratedParameter = false;
            for (auto const& global : resourceGlobals)
                hasGeneratedParameter |=
                    global.perFunctionInfo[functionIndex].parameter != nullptr;
            if (hasGeneratedParameter)
            {
                fixUpFuncType(functions[functionIndex].func);
                fixUpDebugFuncType(functions[functionIndex].func);
            }
        }
    }

    /// Return the parameter type required by a function's read and write effects.
    IRType* getGeneratedParameterType(
        IRBuilder& builder,
        ResourceGlobalToRewrite const& global,
        FunctionResourceInfo const& resourceInfo)
    {
        // A read-only function receives the resource by value. A writer may use `out` only when
        // every normal return follows a complete assignment and no path reads the incoming value.
        // Every other writer requires `inout`.
        bool writesValue = hasAccess(resourceInfo.access, ResourceAccess::Write);
        if (writesValue && !resourceInfo.semanticallyReadsIncomingValue &&
            resourceInfo.replacesWholeValueOnEveryReturn)
        {
            return builder.getOutParamType(global.valueType);
        }
        if (writesValue)
            return builder.getBorrowInOutParamType(global.valueType);
        return global.valueType;
    }

    /// Insert replacement storage at the start of a function's original body.
    void createReplacementLocal(
        IRBuilder& builder,
        Index functionIndex,
        ResourceGlobalToRewrite const& global,
        FunctionResourceInfo& resourceInfo)
    {
        // We use the original insertion point saved in phase 1 so that later generated locals and
        // copy-in operations all precede the pre-existing body.
        setInsertAtOriginalBodyStart(builder, functionIndex);
        resourceInfo.replacementAddress = builder.emitVar(global.valueType);
        resourceInfo.replacementAddress->sourceLoc = global.globalVar->sourceLoc;
        copyNameHint(builder, global.globalVar, resourceInfo.replacementAddress);
    }

    /// Initialize a function's local from its generated parameter when the body needs the input.
    void initializeLocalFromGeneratedParameter(
        IRBuilder& builder,
        ResourceGlobalToRewrite const& global,
        FunctionResourceInfo& resourceInfo)
    {
        // A read-only parameter is already a value. An `inout` parameter is an address, so we load
        // it before storing into the local. A function with a true `out` parameter deliberately
        // starts with an uninitialized local.
        bool writesValue = hasAccess(resourceInfo.access, ResourceAccess::Write);
        if (writesValue && !resourceInfo.mustSeedLocalFromCaller())
            return;

        auto inputValue = !writesValue ? static_cast<IRInst*>(resourceInfo.parameter)
                                       : builder.emitLoad(global.valueType, resourceInfo.parameter);
        builder.emitStore(resourceInfo.replacementAddress, inputValue);
    }

    /// Copy a writer's local value to its generated parameter at every normal return.
    void copyLocalToGeneratedParameterAtReturns(
        IRBuilder& builder,
        FunctionContext const& function,
        ResourceGlobalToRewrite const& global,
        FunctionResourceInfo& resourceInfo)
    {
        // Resource-output specialization recognizes these stores and implements the caller-visible
        // `out` or `inout` update. A read-only function needs no copy-out.
        if (!hasAccess(resourceInfo.access, ResourceAccess::Write))
            return;

        for (auto block : function.func->getBlocks())
        {
            auto returnInst = as<IRReturn>(block->getTerminator());
            if (!returnInst)
                continue;

            builder.setInsertBefore(returnInst);
            auto result = builder.emitLoad(global.valueType, resourceInfo.replacementAddress);
            builder.emitStore(resourceInfo.parameter, result);
        }
    }

    /// Add a generated resource parameter and local copy to one non-entry-point function.
    void createParameterAndLocalForNonEntryPointFunction(
        IRBuilder& builder,
        Index functionIndex,
        ResourceGlobalToRewrite const& global,
        FunctionResourceInfo& resourceInfo)
    {
        // We add the generated parameter after the pre-existing parameters, create the local before
        // the pre-existing body, copy the input when required, and copy a writer's result back on
        // each normal return.
        auto const& function = functions[functionIndex];
        auto firstBlock = function.func->getFirstBlock();
        SLANG_RELEASE_ASSERT(firstBlock);

        auto paramType = getGeneratedParameterType(builder, global, resourceInfo);
        resourceInfo.parameter = builder.createParam(paramType);
        resourceInfo.parameter->sourceLoc = global.globalVar->sourceLoc;
        firstBlock->addParam(resourceInfo.parameter);
        copyNameHint(builder, global.globalVar, resourceInfo.parameter);

        createReplacementLocal(builder, functionIndex, global, resourceInfo);
        initializeLocalFromGeneratedParameter(builder, global, resourceInfo);
        copyLocalToGeneratedParameterAtReturns(builder, function, global, resourceInfo);
    }

    /// Set the insertion point before the first ordinary instruction captured during phase 1.
    void setInsertAtOriginalBodyStart(IRBuilder& builder, Index functionIndex)
    {
        // Reusing the pre-mutation anchor keeps every generated local and copy-in before the
        // original body, even when several resources add instructions to the same block.
        auto const& function = functions[functionIndex];
        if (function.firstPreRewriteOrdinaryInst)
            builder.setInsertBefore(function.firstPreRewriteOrdinaryInst);
        else
            builder.setInsertInto(function.func->getFirstBlock());
    }

    /// Copy an instruction's name hint to a generated local or parameter.
    void copyNameHint(IRBuilder& builder, IRInst* source, IRInst* target)
    {
        if (auto nameHint = source->findDecoration<IRNameHintDecoration>())
            builder.addNameHintDecoration(target, nameHint->getName());
    }

    /// Redirect saved function-local uses and remove metadata attached to the deleted global.
    void replaceGlobalUses()
    {
        // We replace function uses immediately. We defer deletion of module metadata because one
        // metadata instruction may refer to several globals whose saved `IRUse` links are still
        // needed.
        List<IRInst*> moduleMetadataUsers;
        HashSet<IRInst*> seenModuleMetadataUsers;

        for (auto& global : resourceGlobals)
        {
            for (auto const& rootUse : global.rootUses)
            {
                if (rootUse.functionIndex < 0)
                {
                    // Decorations and other module metadata disappear with the storage they name.
                    auto user = rootUse.use->getUser();
                    SLANG_RELEASE_ASSERT(!doesInstSemanticallyUseOperandValue(user));
                    if (user->getParent() != global.globalVar && seenModuleMetadataUsers.add(user))
                    {
                        moduleMetadataUsers.add(user);
                    }
                    continue;
                }

                auto replacement = global.perFunctionInfo[rootUse.functionIndex].replacementAddress;
                SLANG_RELEASE_ASSERT(replacement);
                rootUse.use->set(replacement);
            }
        }

        for (auto user : moduleMetadataUsers)
            user->removeAndDeallocate();
    }

    /// Replace direct calls whose callees gained resource parameters.
    void rewriteCalls()
    {
        // We preserve every pre-existing argument at its original index, then append generated
        // resource arguments in module order. We also preserve source location, decorations, and the
        // use-specific effects needed by the later uninitialized-value check.
        IRBuilder builder(module);

        for (auto const& edge : directCallEdges)
        {
            if (!callNeedsResourceArguments(edge))
                continue;

            auto oldCall = edge.call;
            List<IRInst*> arguments;
            for (UInt i = 0; i < oldCall->getArgCount(); ++i)
                arguments.add(oldCall->getArg(i));

            builder.setInsertBefore(oldCall);
            appendResourceArguments(builder, edge, arguments);

            auto newCall =
                builder.emitCallInst(oldCall->getFullType(), oldCall->getCallee(), arguments);
            newCall->sourceLoc = oldCall->sourceLoc;
            oldCall->transferDecorationsTo(newCall);

            remapExplicitArgumentUseEffects(oldCall, newCall, edge.callerIndex);
            recordResourceArgumentUseEffects(oldCall, newCall, edge);

            oldCall->replaceUsesWith(newCall);
            oldCall->removeAndDeallocate();
        }
    }

    /// Return whether a call's callee gained at least one resource parameter.
    bool callNeedsResourceArguments(DirectCallEdge const& edge)
    {
        // A non-null parameter records that the callee needs an argument for this resource.
        for (auto const& global : resourceGlobals)
        {
            if (global.perFunctionInfo[edge.calleeIndex].parameter)
                return true;
        }
        return false;
    }

    /// Append resource arguments in the same global order used to create callee parameters.
    void appendResourceArguments(
        IRBuilder& builder,
        DirectCallEdge const& edge,
        List<IRInst*>& arguments)
    {
        // A read-only function receives a loaded value. A writer receives the caller's local
        // address for its generated `out` or `inout` parameter.
        for (auto const& global : resourceGlobals)
        {
            auto const& calleeInfo = global.perFunctionInfo[edge.calleeIndex];
            if (!calleeInfo.parameter)
                continue;

            auto const& callerInfo = global.perFunctionInfo[edge.callerIndex];
            SLANG_RELEASE_ASSERT(callerInfo.replacementAddress);
            if (calleeInfo.access == ResourceAccess::Read)
            {
                auto argument =
                    builder.emitLoad(global.valueType, callerInfo.replacementAddress);
                argument->sourceLoc = edge.call->sourceLoc;
                arguments.add(argument);
            }
            else
                arguments.add(callerInfo.replacementAddress);
        }
    }

    /// Retarget saved explicit-argument effects when phase 4 replaces a call instruction.
    void remapExplicitArgumentUseEffects(IRCall* oldCall, IRCall* newCall, Index callerIndex)
    {
        // Effects belong to exact `IRUse` objects. We match pre-existing arguments by index because
        // the rebuilt call preserves their order.
        for (auto& global : resourceGlobals)
        {
            auto& effects = global.perFunctionInfo[callerIndex].uninitializedUseEffects;
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

    /// Record the uninitialized-value effect of each generated resource argument.
    void recordResourceArgumentUseEffects(
        IRCall* oldCall,
        IRCall* newCall,
        DirectCallEdge const& edge)
    {
        // Parameter direction alone cannot distinguish a conditional write from a complete
        // assignment. We attach the phase-3 facts to the exact new argument use. Copying the input
        // into a generated local does not itself count as a source-level read: when a conditional
        // writer leaves an uninitialized value unchanged and no later code reads it, the source
        // program has not observed that value.
        UInt resourceArgIndex = oldCall->getArgCount();
        for (auto& global : resourceGlobals)
        {
            auto const& calleeInfo = global.perFunctionInfo[edge.calleeIndex];
            if (!calleeInfo.parameter)
                continue;

            auto& callerInfo = global.perFunctionInfo[edge.callerIndex];
            if (hasAccess(calleeInfo.access, ResourceAccess::Write))
            {
                callerInfo.uninitializedUseEffects.add(UninitializedVariableUseEffect{
                    .use = newCall->getOperandUse(resourceArgIndex + 1),
                    .readsValue = calleeInfo.semanticallyReadsIncomingValue,
                    .mayWriteValue = true,
                    .definitelyWritesValue = calleeInfo.replacesWholeValueOnEveryReturn,
                });
            }
            resourceArgIndex++;
        }
    }

    // ## Phase 5: Generated-local diagnostics and obsolete-storage removal

    /// Diagnose entry-point paths that read a generated local before a complete assignment.
    void diagnoseUninitializedEntryPointReads(DiagnosticSink* sink)
    {
        // `checkForUsingUninitializedValues` ran before this pass created these locals. We apply its
        // intraprocedural analysis to each generated entry-point local and supply the exact effects
        // recorded while rewriting calls.
        for (auto const& global : resourceGlobals)
        {
            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                if (!functions[functionIndex].isEntryPoint)
                    continue;
                auto replacement = global.perFunctionInfo[functionIndex].replacementAddress;
                if (!replacement)
                    continue;
                checkForUsingUninitializedVariable(
                    functions[functionIndex].func,
                    replacement,
                    global.perFunctionInfo[functionIndex].uninitializedUseEffects.getArrayView(),
                    sink);
            }
        }
    }

    /// Remove each obsolete global after all of its uses have been redirected.
    void removeReplacedResourceGlobals()
    {
        // An unhandled use would become dangling after deletion. We assert that the rewrite removed
        // every use before destroying the global.
        for (auto& global : resourceGlobals)
        {
            SLANG_RELEASE_ASSERT(!global.globalVar->hasUses());
            global.globalVar->removeAndDeallocate();
        }
    }
};

// ## IR-use classification rules

static ResourceAccess mergeAccess(ResourceAccess left, ResourceAccess right)
{
    return ResourceAccess(UInt(left) | UInt(right));
}

static bool hasAccess(ResourceAccess value, ResourceAccess test)
{
    return (UInt(value) & UInt(test)) != 0;
}

/// Return whether an instruction observes or changes an operand's runtime value.
///
/// Address-use analysis must ignore decorations, debug records, types, and queries whose result
/// depends only on an operand's type. Every other use is conservatively treated as a value use so
/// that an unfamiliar instruction cannot silently omit a required resource argument.
static bool doesInstSemanticallyUseOperandValue(IRInst* user)
{
    // Decorations, debug records, types, and type-only queries do not depend on the runtime value.
    // We classify every other instruction as a value use so that a new opcode cannot silently lose
    // a required resource argument.
    if (as<IRDecoration>(user) || as<IRAnnotation>(user) || as<IRAttr>(user) || as<IRType>(user) ||
        isDebugInfoInst(user))
        return false;

    if (doesInstOnlyDependOnOperandTypes(user))
        return false;

    return true;
}

/// Return how a call may access the resource address passed by `use`.
///
/// Directional parameter types provide the precise read/write contract. If the use cannot be
/// matched to such a parameter, both effects are retained: under-classifying an unknown call would
/// permit the rewrite to omit a required input value or write-back.
static ResourceAccess classifyCallArgumentAccess(IRCall* call, IRUse* use)
{
    // Directional parameter types state whether the callee reads, writes, or does both. A raw
    // pointer or a use that cannot be matched to a parameter provides no such guarantee, so we
    // retain both effects.
    auto paramType = findCallArgumentParameterType(call, use);
    if (!paramType)
        return mergeAccess(ResourceAccess::Read, ResourceAccess::Write);

    if (as<IROutParamType>(paramType))
        return ResourceAccess::Write;
    if (as<IRBorrowInParamType>(paramType))
        return ResourceAccess::Read;
    if (as<IRBorrowInOutParamType>(paramType) || as<IRRefParamType>(paramType))
        return mergeAccess(ResourceAccess::Read, ResourceAccess::Write);
    if (as<IRPtrTypeBase>(paramType))
    {
        // A raw pointer parameter carries no directional contract. Its callee may observe or
        // replace the pointee, so we preserve both directions rather than assuming an input use.
        return mergeAccess(ResourceAccess::Read, ResourceAccess::Write);
    }
    return ResourceAccess::Read;
}

/// Return how a semantic use reads or writes the resource reached through `use`.
///
/// Loads are reads, stores through the address operand are writes, and calls inherit the formal
/// parameter's direction. Uses with no precise contract conservatively preserve every effect they
/// might have. Whether a write replaces the *whole* value is decided by the caller because it also
/// depends on whether this address denotes the root or only a subobject.
static ResourceAccess classifyResourceAddressUse(IRUse* use)
{
    // We classify known memory operations and calls from their operand contracts. An unfamiliar
    // pointer consumer may both read and write. An ordinary value consumer reads the stored
    // resource.
    auto user = use->getUser();

    if (as<IRLoad>(user) || as<IRAtomicLoad>(user))
        return ResourceAccess::Read;

    if (auto atomicOperation = as<IRAtomicOperation>(user))
    {
        // Atomic load was handled above. Atomic store writes the pointee, while every other atomic
        // operation both reads and writes it. A use in any non-address operand is conservatively
        // classified as both.
        if (atomicOperation->getOperandUse(0) != use)
            return mergeAccess(ResourceAccess::Read, ResourceAccess::Write);
        if (as<IRAtomicStore>(atomicOperation))
            return ResourceAccess::Write;
        return mergeAccess(ResourceAccess::Read, ResourceAccess::Write);
    }

    switch (user->getOp())
    {
    case kIROp_Store:
    case kIROp_SwizzledStore:
    case kIROp_MatrixSwizzleStore:
        return user->getOperandUse(0) == use
                   ? ResourceAccess::Write
                   : mergeAccess(ResourceAccess::Read, ResourceAccess::Write);
    default:
        break;
    }

    if (auto call = as<IRCall>(user))
        return classifyCallArgumentAccess(call, use);

    // A pointer-producing instruction that is not a recognized address derivation may let the
    // address escape. The same is true when the address is routed through control flow or returned.
    // We cannot safely infer how those users access the variable, so we preserve both directions.
    if (as<IRPtrTypeBase>(user->getDataType()))
        return mergeAccess(ResourceAccess::Read, ResourceAccess::Write);

    switch (user->getOp())
    {
    case kIROp_Return:
    case kIROp_UnconditionalBranch:
    case kIROp_Loop:
    case kIROp_IfElse:
    case kIROp_Switch:
        return mergeAccess(ResourceAccess::Read, ResourceAccess::Write);
    default:
        // Ordinary value consumers observe the resource stored at this address.
        return ResourceAccess::Read;
    }
}

/// Return whether an address derivation preserves the complete pointee type.
///
/// Field, element, and offset operations select a subobject. A pointer cast may also narrow the
/// pointee, so we preserve the whole-value fact only for casts whose source and result have equal
/// pointee types. `IRInOutImplicitCast` is deliberately excluded: its pointer form may introduce a
/// temporary rather than another representation of the same storage.
static bool doesUsePreserveWholeAddress(IRUse* use)
{
    // Field, element, and offset operations cannot preserve the whole-value fact. For a pointer
    // cast, we compare the source and result pointee types because equal types preserve the extent
    // of the original value.
    auto user = use->getUser();
    switch (user->getOp())
    {
    case kIROp_BitCast:
    case kIROp_Reinterpret:
    case kIROp_PtrCast:
        break;
    default:
        return false;
    }

    auto sourcePointerType = as<IRPtrTypeBase>(use->get()->getDataType());
    auto resultPointerType = as<IRPtrTypeBase>(user->getDataType());
    SLANG_RELEASE_ASSERT(sourcePointerType && resultPointerType);

    auto sourceValueType = cast<IRType>(unwrapAttributedType(sourcePointerType->getValueType()));
    auto resultValueType = cast<IRType>(unwrapAttributedType(resultPointerType->getValueType()));
    return isTypeEqual(sourceValueType, resultValueType);
}

/// Return whether a use may retain the address or compare it with another address.
///
/// Loads, stores through the address, atomic operations, and calls through directional parameters
/// use the address only for the current operation. Other uses cannot safely receive a replacement
/// address that refers to a different local in each function.
static bool isAddressUseUnsupportedByPerFunctionStorage(IRUse* use)
{
    // Known memory operations use the address only while they execute. Directional call parameters
    // likewise borrow the address for the call. Every other terminal consumer may retain the
    // address or compare it with another pointer.
    auto user = use->getUser();
    if (as<IRLoad>(user) || as<IRAtomicLoad>(user))
        return false;
    if (auto store = as<IRStore>(user))
        return store->getOperandUse(0) != use;
    if (as<IRAtomicOperation>(user) || as<IRSwizzledStore>(user) || as<IRMatrixSwizzleStore>(user))
        return user->getOperandUse(0) != use;
    if (auto call = as<IRCall>(user))
    {
        // `BorrowIn`, `Out`, `BorrowInOut`, and `Ref` parameters borrow their argument storage only
        // for the duration of the call. A raw pointer or unknown parameter has no such contract, so
        // passing the replacement local's address through it could let the callee retain storage
        // owned by this invocation.
        auto paramType = findCallArgumentParameterType(call, use);
        return !paramType ||
               !(as<IRBorrowInParamType>(paramType) || as<IROutParamType>(paramType) ||
                 as<IRBorrowInOutParamType>(paramType) || as<IRRefParamType>(paramType));
    }

    // Every other terminal consumer may retain the address or compare it with another value. For
    // example, pointer-to-integer conversion and pointer comparison would observe that different
    // functions now use different locals.
    return true;
}

/// Return whether `user` is a decoration that causes the referenced function to be invoked.
static bool doesDecorationInvokeReferencedFunction(IRInst* user)
{
    // These decorations name functions that the compiler or runtime calls without an `IRCall` in
    // this module. We enumerate them so that metadata decorations which also point to a function,
    // such as `IREntryPointParamDecoration`, do not make that function appear independently invoked.
    switch (user->getOp())
    {
    case kIROp_PatchConstantFuncDecoration:
    case kIROp_DispatchFuncDecoration:
    case kIROp_CudaKernelForwardDerivativeDecoration:
    case kIROp_CudaKernelBackwardDerivativeDecoration:
        return true;
    default:
        return false;
    }
}

/// Return whether `func` has an invocation or first-class use that phase 4 cannot rewrite.
static bool hasFunctionUseThatCannotBeRewritten(IRFunc* func)
{
    // We can append resource arguments to a direct call inside a function. A call in a global
    // initializer has no function-local resource value to pass. An invocation-bearing decoration
    // or another first-class use likewise provides no argument list that phase 4 can extend.
    for (auto use = func->firstUse; use; use = use->nextUse)
    {
        auto user = use->getUser();
        if (auto call = as<IRCall>(user))
        {
            if (call->getCalleeUse() == use)
            {
                if (getParentFunc(call))
                    continue;
                return true;
            }
        }

        if (doesDecorationInvokeReferencedFunction(user))
            return true;
        if (as<IRDecoration>(user))
            continue;
        if (!doesInstSemanticallyUseOperandValue(user))
            continue;
        return true;
    }
    return false;
}

/// Return whether `func` may be invoked without an in-module direct call.
static bool mayBeInvokedWithoutDirectCall(IRFunc* func)
{
    // Export and keep-alive decorations promise an external caller. A non-call use can pass the
    // function as a value or name it in a decoration such as `IRPatchConstantFuncDecoration`.
    // Those invocation paths contain no `IRCall` whose argument list we can extend.
    return func->findDecoration<IRKeepAliveDecoration>() ||
           func->findDecoration<IRPublicDecoration>() ||
           func->findDecoration<IRHLSLExportDecoration>() ||
           func->findDecoration<IRDllExportDecoration>() ||
           func->findDecoration<IRExternCDecoration>() ||
           func->findDecoration<IRExternCppDecoration>() ||
           func->findDecoration<IRCudaDeviceExportDecoration>() ||
           func->findDecoration<IRDownstreamModuleExportDecoration>() ||
           func->findDecoration<IRDownstreamModuleImportDecoration>() ||
           hasFunctionUseThatCannotBeRewritten(func);
}

/// Return whether `globalVar` must retain one externally visible storage location.
///
/// Such storage cannot be replaced by a distinct local in every entry point. An explicit ABI or
/// storage decoration establishes this requirement; ordinary IR linkage decorations do not.
static bool requiresExternallyVisibleStorage(IRGlobalVar* globalVar)
{
    // `IRExportDecoration` is intentionally absent. Linked source definitions use it for IR
    // linking, but `IRExportDecoration` does not require the final program to expose their
    // addresses. The decorations below do require the storage to remain externally visible.
    static const IROp kExternallyVisibleStorageDecorations[] = {
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
    for (auto op : kExternallyVisibleStorageDecorations)
    {
        if (globalVar->findDecorationImpl(op))
            return true;
    }
    return false;
}

} // namespace

void legalizeResourceGlobalVars(IRModule* module, DiagnosticSink* sink)
{
    // The earlier initializer pass has removed every selected initializer body. We can therefore
    // replace each selected storage declaration without losing initializer code.
    LegalizeResourceGlobalVarsPass pass(module);
    pass.processModule(sink);
}

} // namespace Slang
