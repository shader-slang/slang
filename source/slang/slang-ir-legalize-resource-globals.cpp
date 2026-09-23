// slang-ir-legalize-resource-globals.cpp
//
// `legalizeResourceGlobalVars` removes mutable resource variables declared `static` at file or
// namespace scope from global IR storage. Consider this example:
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
// Such a mutable `static` variable has a separate value for each entry-point invocation; it does
// not denote persistent program-wide storage. Targets that cannot represent the resource value at
// global scope need the equivalent program to use an entry-point local and explicit arguments:
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
// Immediately before this pass,
// `moveGlobalVarInitializationToEntryPointsForResourceGlobalLegalization` moves each selected
// initializer body into a new zero-argument function, then inserts a store of its result at the
// start of every defined entry point. The pipeline has already completed any resource- or
// empty-type legalization selected for the target. Each selected `IRGlobalVar` therefore still
// represents one IR value and has no initializer blocks when this pass begins. We replace that
// storage in five phases:
//
// 1. We record every function in module preorder, select resource globals in module order, and
//    record direct calls between those functions.
// 2. We reject a variable if its linkage or retention requirements keep it in module-scope
//    storage, or if an instruction outside a function body references it. We reject a
//    non-entry-point function that accesses the variable when some possible invocation has no
//    direct `IRCall` whose arguments we can rewrite.
//    An entry point needs no call for its runtime invocation. We reject an entry point if the
//    module also calls it directly, names it in a decoration that invokes it, or uses it as a
//    function value. We also reject access from a function when generic assembly or an applicable
//    target-intrinsic definition supplies the emitted implementation instead of the IR blocks that
//    this pass would rewrite.
// 3. For each variable, we determine which functions may read or write its value. We follow each
//    use through projections, pointer casts, and l-value casts that transfer a read or write to the
//    original storage. We propagate each non-entry-point function's effects to its callers. We also
//    determine whether every reachable normal return from a writer follows a complete assignment.
//    Finally, we reject uses that may retain the address or observe its storage identity, and calls
//    that pass the same writable value through both an existing argument and a generated argument.
// 4. For each selected variable, we give every entry point whose execution may read or write it a
//    fresh local. We give each non-entry-point function whose execution may read or write the
//    variable, directly or through a callee, a generated parameter and local copy. A function whose
//    only need for replacement storage is a direct non-runtime reference inside its body, such as
//    debug metadata, needs a local but no parameter. We replace direct uses of the globals and
//    append the corresponding arguments to direct calls.
// 5. We check the now-explicit entry-point locals for reads before initialization, then remove the
//    obsolete globals and their attached children.
//
// In this file, "entry point" means either a shader entry point or a CUDA kernel. Both kinds of
// function create their own replacement locals for the selected globals that they use.
//
// The pass treats every remaining direct call between functions in the module as a possible
// invocation. The emission pipeline removes unreachable control flow before invoking the pass, so
// dead calls cannot add parameters or cause unsupported functions to be diagnosed.
//
// This pass runs after any resource- or empty-type legalization selected for the target and before
// `specializeResourceUsage`. The front end admits only declarations represented at that stage by
// one resource-typed global or one global containing a homogeneous array of resource values. The
// accepted types are non-combined textures and typed buffers, sampler states, and the read-only,
// read-write, and rasterizer-ordered forms of structured buffers and byte-address buffers,
// including homogeneous arrays of those types. The analysis recognizes whole-array assignment, but
// it does not prove that separate element writes initialize an entire array.
//
// An acceleration structure also remains one IR value, but this transformation would place that
// value in function-local variables. Khronos and WGSL targets reject those variables.
// `__DynamicResource` remains one value as well, but Khronos legalization requires each cast from
// that value to resolve to one module-scope dynamic-resource parameter or one indexed element. A
// mutable local can merge several such sources, so a later cast may no longer identify which one
// supplied its value. The front end therefore excludes both types on every target so that source
// legality does not depend on the selected backend.
//
// The pass does not recursively replace the fields of a struct. Such a rule would need to preserve
// non-resource fields while mapping every resource field to the values produced by type
// legalization. Parameter groups, combined texture-sampler types, and append or consume buffers can
// likewise map one source variable to several IR values, with details that depend on the target.
// The analysis assumes that each selected global is represented by one IR value. Within each
// affected function, the pass can copy that value through one local and, in a non-entry-point
// function, one parameter. The pass cannot coordinate the several target-dependent values produced
// when these categories are legalized, so the front end rejects them for now.
//
// We choose each generated parameter type from the answers to two questions: can the function read
// the value supplied by its caller before assigning it completely, and does every reachable normal
// return follow a complete assignment? A read-only non-entry-point function gets a value parameter.
// A writer gets an `out` parameter only when it does not need the caller's value and every
// reachable normal return follows a complete assignment. Every other writer gets `inout`.
// On targets that require resource-output specialization, `specializeResourceOutputs` recognizes
// the generated copy-in, local, and copy-out pattern.
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

/// A `ResourceAccess` value records whether an instruction or function may read or write one
/// resource value.
///
/// The function summaries propagate these possible effects transitively from callees to callers.
/// Definite whole-value assignment is tracked separately because `Write` alone does not say whether
/// every path to a reachable normal return performs a complete assignment.
enum class ResourceAccess : UInt
{
    None = 0,
    Read = 1 << 0,
    Write = 1 << 1,
};

/// A `StorageAccessRelation` describes how reads or writes through an instruction's result apply
/// to the source value supplied through operand zero.
enum class StorageAccessRelation : UInt
{
    /// The propagated access applies to the complete source value.
    CompleteValue,

    /// The propagated access applies to a field, element, or other subobject.
    Subobject,

    /// We cannot prove which part of the source value the propagated access applies to.
    Unknown,
};

/// An `AddressUseAnalysis` records the effects reached from one address and whether every use is
/// compatible with function-local replacement storage.
struct AddressUseAnalysis
{
    /// Whether a use may retain the address or otherwise observe its storage identity.
    bool hasUnsupportedUse = false;

    /// Whether a use may read the value stored at the address.
    bool mayRead = false;

    /// Whether a use may write the value stored at the address.
    bool mayWrite = false;
};

/// A `FunctionContext` records facts about one function and caches its dominator tree.
///
/// The module-preorder position gives each function a deterministic index. Resource globals retain
/// module order separately, which determines the order of generated parameters. The original body
/// position lets phase 4 insert locals after parameters but before pre-existing code.
struct FunctionContext
{
    /// The function represented by this record.
    IRFunc* func = nullptr;

    /// The first ordinary instruction in the original function body.
    IRInst* firstPreRewriteOrdinaryInst = nullptr;

    /// The lazily computed CFG dominator tree shared by every per-resource proof in this function.
    RefPtr<IRDominatorTree> dominatorTree;

    /// Whether this function is an entry point, so any required replacement storage is local
    /// rather than passed as a parameter.
    bool isEntryPoint = false;

    /// For a non-entry-point function, whether some possible invocation lacks an in-module direct
    /// call that this pass can rewrite.
    bool lacksRewritableCallForSomeInvocation = false;
};

/// A `FunctionResourceInfo` records the analysis and rewrite decisions for one function and
/// one resource global.
struct FunctionResourceInfo
{
    /// The possible runtime read/write effects, including effects propagated from callees.
    ResourceAccess access = ResourceAccess::None;

    /// For a function that may write, whether every reachable normal return follows an assignment
    /// of the complete value.
    bool assignsWholeValueOnEveryReturn = false;

    /// Whether some execution path reads the value present on entry before assigning it completely.
    bool mayReadValueBeforeWholeValueAssignment = false;

    /// For a function that may write, whether a reachable normal return can occur without an
    /// assignment of the complete value.
    ///
    /// For a non-entry-point function, such a path requires the generated local to begin with the
    /// caller's value because the function can leave some or all of that value unchanged.
    bool mayReturnWithoutWholeValueAssignment = false;

    /// Whether this function or one of its callees may write through an address that selects a
    /// subobject.
    bool mayWriteSubobject = false;

    /// Whether this function contains any direct use of the global, including non-runtime uses.
    bool hasDirectGlobalUse = false;

    /// The local address that replaces uses of the global in this function; null until phase 4
    /// creates it.
    IRInst* replacementAddress = nullptr;

    /// The generated parameter for a non-entry-point function that accesses the resource.
    ///
    /// This field remains null for entry points, before phase 4, and when a local exists only to
    /// replace a non-runtime use such as debug metadata.
    IRParam* parameter = nullptr;

    /// Per-use effect overrides retained for the generated local's uninitialized-value check.
    List<UninitializedVariableUseEffect> uninitializedUseEffects;

    /// Return whether a non-entry-point function must initialize its local from the generated
    /// parameter.
    bool mustSeedLocalFromCaller() const
    {
        return mayReadValueBeforeWholeValueAssignment || mayReturnWithoutWholeValueAssignment;
    }
};

/// A `DirectGlobalUse` identifies a direct use and the function body that contains it, if any.
struct DirectGlobalUse
{
    IRUse* use = nullptr;

    /// The containing function's index in `functions`, or -1 for a use outside a function body.
    Index functionIndex = -1;
};

/// A `ResourceAddressUse` describes a runtime access reached from the global's address.
struct ResourceAddressUse
{
    /// The operand through which its containing instruction accesses the address.
    IRUse* use = nullptr;

    /// The containing function's index in `functions`.
    Index functionIndex = -1;

    /// The possible read/write effect of the instruction that contains `use`.
    ResourceAccess access = ResourceAccess::None;

    /// Whether every normally completing execution of this instruction assigns the complete value
    /// stored by the original global.
    bool assignsWholeValue = false;
};

/// A `ResourceGlobalToRewrite` holds the analysis results and generated IR for one resource global.
///
/// Phases 1 through 3 populate the original uses and effect records. Phase 4 records a replacement
/// local for each affected function and a generated parameter for each non-entry-point function
/// with runtime access. It retains the analysis that phase 5 needs to check the entry-point locals
/// for uninitialized reads.
struct ResourceGlobalToRewrite
{
    /// The original storage whose uses phase 4 will replace and whose declaration phase 5 removes.
    IRGlobalVar* globalVar = nullptr;

    /// The resource value type stored by each generated local and carried by each generated
    /// parameter.
    IRType* valueType = nullptr;

    /// One analysis and rewrite record per entry in `functions`.
    List<FunctionResourceInfo> perFunctionInfo;

    /// Every direct use-list edge from the global, saved before phase 4 mutates the use list.
    List<DirectGlobalUse> rootUses;

    /// Runtime reads and writes reached by following operations that may transfer storage access
    /// from each root.
    List<ResourceAddressUse> terminalUses;

    /// Call-argument address uses retained for alias validation before rewriting.
    List<IRUse*> addressPassingUses;

    /// Uses that may retain the address or otherwise observe its storage identity.
    List<IRUse*> unsupportedAddressUses;
};

/// A `DirectCallEdge` records one direct call between functions in `functions`.
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
static bool doesInstUseOperandAtRuntime(IRInst* user);
static AddressUseAnalysis analyzeAddressUses(IRInst* rootAddress, CapabilitySet const& targetCaps);
static IRParam* findDirectCalleeParameter(IRCall* call, IRUse* argumentUse);
static ResourceAccess classifyCallArgumentAccess(
    IRCall* call,
    IRUse* use,
    CapabilitySet const& targetCaps);
static ResourceAccess classifyResourceAddressUse(IRUse* use, CapabilitySet const& targetCaps);
static StorageAccessRelation classifyStorageAccessRelation(IRUse* use);
static bool isAddressUseUnsupportedByPerFunctionStorage(
    IRUse* use,
    CapabilitySet const& targetCaps);
static bool doesDecorationInvokeReferencedFunction(IRInst* user);
static bool hasFunctionUseThatCannotBeRewritten(IRFunc* func);
static bool mayBeInvokedWithoutRewritableCall(IRFunc* func);
static bool requiresModuleScopeStorage(IRGlobalVar* globalVar);
static bool isInstInsideFunctionBody(IRInst* inst);

/// A `LegalizeResourceGlobalVarsPass` implements the five-phase transformation described at the
/// top of this file.
///
/// The preceding initializer-movement pass has already extracted every selected initializer body.
/// Phases 2 and 3 reject unsupported IR before this pass changes any use or function signature.
/// Phase 5 diagnoses an uninitialized read in the rewritten IR because that analysis needs the
/// generated entry-point locals.
struct LegalizeResourceGlobalVarsPass
{
    IRModule* module = nullptr;
    CapabilitySet const& targetCaps;
    List<FunctionContext> functions;
    Dictionary<IRFunc*, Index> functionIndices;
    List<ResourceGlobalToRewrite> resourceGlobals;
    List<DirectCallEdge> directCallEdges;

    explicit LegalizeResourceGlobalVarsPass(IRModule* inModule, CapabilitySet const& inTargetCaps)
        : module(inModule), targetCaps(inTargetCaps)
    {
    }

    /// Run the five phases described at the top of this file.
    void processModule(DiagnosticSink* sink)
    {
        // In phase 1, we assign deterministic indices to functions, record resource globals in the
        // order that will determine generated parameter order, and record direct calls between
        // those functions. Later phases use these records instead of hash-table or use-list order.
        collectFunctions();
        collectResourceGlobals();
        collectDirectCalls();

        if (resourceGlobals.getCount() == 0)
            return;

        // In phase 2, we reject cases that per-function locals cannot represent. First, linkage or
        // retention requirements may keep the variable in module-scope storage. Second, an
        // instruction outside a function body has no function-local replacement address. Third, a
        // non-entry-point function that accesses a selected variable may have an invocation whose
        // argument list we cannot extend; a generic body, for example, has calls that do not exist
        // until instantiation. Runtime invocation of an entry point is supported because the entry
        // point creates its own locals. We reject an entry point if the module also calls it
        // directly, names it in a decoration that invokes it, or uses it as a function value.
        // Finally, we reject a function when generic assembly or an applicable target-intrinsic
        // definition supplies the emitted implementation instead of the IR blocks that this pass
        // would rewrite.
        //
        // TODO: Move the target-independent storage, invocation, and generic-assembly checks to an
        // IR validation pass after linking. Front-end checking sees one module at a time and cannot
        // know all callers. The check for an applicable target-intrinsic implementation depends on
        // the current target capabilities, so it must remain target-specific unless the earlier
        // validation stage receives the same target capabilities.
        bool diagnosedUnsupportedProgram = diagnoseGlobalsRequiringModuleScopeStorage(sink);
        diagnosedUnsupportedProgram |= diagnoseResourceUsesOutsideFunctionBodies(sink);
        auto functionsAccessingResourceGlobals = computeFunctionsAccessingAnyResourceGlobal();
        diagnosedUnsupportedProgram |=
            diagnoseFunctionsWithUnrewritableInvocations(sink, functionsAccessingResourceGlobals);
        diagnosedUnsupportedProgram |=
            diagnoseFunctionsWithAlternateImplementations(sink, functionsAccessingResourceGlobals);
        if (diagnosedUnsupportedProgram)
            return;

        // In phase 3, we determine whether each function reads or writes each resource value. These
        // results select value, `out`, or `inout` parameters in phase 4. They also tell the
        // uninitialized-value analysis in phase 5 which writes definitely assign the complete
        // value. Before rewriting, we reject arrays with subobject writes and a possible read
        // before a whole-array assignment. We also reject uses that may retain an address or
        // observe its storage identity, and calls that would receive two writable aliases for the
        // same original global. Phase 2 has already diagnosed every resource-using non-entry-point
        // function with an unrewritable invocation, and every resource-using entry point that may
        // also be invoked as a callable function.
        analyzeResourceAccess();
        if (diagnoseArrayInitializationRequiringSubobjectAnalysis(sink))
            return;
        recordTerminalWriteEffects();
        if (diagnoseUnsupportedAddressUses(sink))
            return;
        if (diagnoseConflictingCallAliases(sink))
            return;
        assertFunctionsThatNeedParametersHaveOnlyRewritableInvocations();

        // In phase 4, we create one local in every function whose execution may access the value,
        // plus any function body with a direct non-runtime reference. Each non-entry-point function
        // that may read or write the value also receives a generated parameter. We redirect the old
        // global uses and append arguments to each direct call.
        introduceReplacements();
        replaceGlobalUses();
        rewriteCalls();

        // In phase 5, we apply the shared uninitialized-value analysis to each generated
        // entry-point local. That analysis reports both reads with no reaching write and reads that
        // lack a definite write on every path. Successful rewriting leaves only direct-child uses
        // that are deleted with the original global.
        diagnoseUninitializedEntryPointReads(sink);
        removeReplacedResourceGlobals();
    }

    // ## Phase 1: Functions, resource globals, and direct calls

    /// Collect functions and determine whether phase 4 can rewrite every possible invocation.
    void collectFunctions()
    {
        // We traverse the module's instruction tree in child order. Most functions are direct
        // children of the module, but an unspecialized generic contains its function body inside an
        // `IRGeneric`. We include those nested bodies in the call graph so phase 2 can reject a
        // resource access that this pass cannot rewrite safely.
        collectFunctionsUnder(module->getModuleInst());
    }

    /// Collect every function contained by `parent` in module preorder.
    void collectFunctionsUnder(IRInst* parent)
    {
        // A function nested under an `IRGeneric` can be instantiated after this pass. Its eventual
        // calls do not exist yet, so phase 4 cannot add a resource argument to them. We record that
        // limitation on every nested function. Entry-point specialization has already placed every
        // entry point directly under the module. If a limited nested function accesses a selected
        // resource, phase 2 diagnoses it before changing any signature.
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
            context.isEntryPoint = isShaderOrCudaKernelEntryPoint(func);
            SLANG_RELEASE_ASSERT(
                !context.isEntryPoint || func->getParent() == module->getModuleInst());
            context.lacksRewritableCallForSomeInvocation =
                !context.isEntryPoint && (func->getParent() != module->getModuleInst() ||
                                          mayBeInvokedWithoutRewritableCall(func));
            if (auto firstBlock = func->getFirstBlock())
                context.firstPreRewriteOrdinaryInst = firstBlock->getFirstOrdinaryInst();

            auto index = functions.getCount();
            functions.add(context);
            functionIndices.add(func, index);
        }
    }

    /// Collect resource globals declared `static` at file or namespace scope that this pass must
    /// replace.
    ///
    /// The initializer mover has already removed each selected initializer body. Keeping these
    /// records in module order makes the order of generated parameters deterministic.
    void collectResourceGlobals()
    {
        // The shared predicate checks the file-or-namespace-scope `static` marker, the absence of
        // an explicit rate, and the IR type categories produced for the front end's allow-list.
        // Source checking is the authority that restricts those categories to values this
        // transformation can replace.
        for (auto inst : module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar || !isFileOrNamespaceScopeStaticResourceGlobalToReplace(globalVar))
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

    /// Collect direct calls whose caller and callee both appear in `functions`.
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

    /// Diagnose resource globals that must remain in module-scope storage.
    ///
    /// Linkage and retention decorations can require storage that outlives any one entry-point
    /// invocation. Replacing such a variable with function-local storage would not preserve that
    /// requirement. Return whether any diagnostic was emitted.
    bool diagnoseGlobalsRequiringModuleScopeStorage(DiagnosticSink* sink)
    {
        // We report each incompatible storage requirement before rewriting any resource-global use
        // or function signature.
        bool diagnosed = false;
        for (auto const& resourceGlobal : resourceGlobals)
        {
            auto globalVar = resourceGlobal.globalVar;
            if (!requiresModuleScopeStorage(globalVar))
                continue;
            sink->diagnose(Diagnostics::ResourceStaticRequiresModuleScopeStorage{
                .variable = globalVar,
                .location = globalVar->sourceLoc});
            diagnosed = true;
        }
        return diagnosed;
    }

    /// Diagnose uses outside a function body that cannot be redirected to per-function storage,
    /// and return whether any diagnostic was emitted.
    bool diagnoseResourceUsesOutsideFunctionBodies(DiagnosticSink* sink)
    {
        // A replacement local exists inside one function body and therefore cannot replace a
        // reference outside that body. A decoration attached directly to a function is not inside
        // its body, even though `getParentFunc` reports that function as its owner. The global's
        // own decorations disappear with the global and need no replacement. Every other use
        // outside a function body lacks a local to which we can redirect it. A runtime use outside
        // a function body normally belongs to another global's initializer. A non-runtime query or
        // metadata instruction may still have its own users or describe another object, so deleting
        // that instruction would not preserve the module.
        bool diagnosed = false;
        for (auto const& resourceGlobal : resourceGlobals)
        {
            for (auto use = resourceGlobal.globalVar->firstUse; use; use = use->nextUse)
            {
                auto user = use->getUser();
                if (isInstInsideFunctionBody(user) || user->getParent() == resourceGlobal.globalVar)
                    continue;

                if (doesInstUseOperandAtRuntime(user))
                {
                    sink->diagnose(Diagnostics::ResourceStaticUsedOutsideFunction{
                        .variable = resourceGlobal.globalVar,
                        .location = user->sourceLoc});
                }
                else
                {
                    sink->diagnose(Diagnostics::ResourceStaticHasReferenceOutsideFunctionBody{
                        .variable = resourceGlobal.globalVar,
                        .location = user->sourceLoc});
                }
                diagnosed = true;
            }
        }
        return diagnosed;
    }

    /// Return which functions directly or transitively read or write a selected resource global at
    /// runtime.
    List<bool> computeFunctionsAccessingAnyResourceGlobal()
    {
        // We first mark functions with a direct runtime use. We then propagate each mark from a
        // callee to its callers until no mark changes. The fixed point includes recursive call
        // cycles without depending on function order.
        List<bool> accessesResource;
        for (Index i = 0; i < functions.getCount(); ++i)
            accessesResource.add(false);

        for (auto const& resourceGlobal : resourceGlobals)
        {
            for (auto use = resourceGlobal.globalVar->firstUse; use; use = use->nextUse)
            {
                if (!doesInstUseOperandAtRuntime(use->getUser()))
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

    /// Diagnose resource-using non-entry-point functions that may be invoked without a rewritable
    /// direct call. Diagnose resource-using entry points that are also called directly, invoked
    /// through a decoration, or used as function values. Return whether any diagnostic was emitted.
    bool diagnoseFunctionsWithUnrewritableInvocations(
        DiagnosticSink* sink,
        List<bool> const& accessesResource)
    {
        // We can append a resource argument to a direct call inside a function. We cannot change
        // arguments supplied by an external caller, a call outside a function body, or a decoration
        // such as `IRPatchConstantFuncDecoration`. An entry point that is also called directly,
        // invoked by a decoration, or used as a function value needs two implementations: one that
        // creates the entry-point local and one that receives the caller's value. This pass does
        // not split an entry point into those two implementations.
        bool diagnosed = false;
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
            if (function.isEntryPoint && (hasDirectCaller[functionIndex] ||
                                          hasFunctionUseThatCannotBeRewritten(function.func)))
            {
                sink->diagnose(Diagnostics::ResourceStaticUsedByCallableEntryPoint{
                    .function = function.func,
                    .location = function.func->sourceLoc});
                diagnosed = true;
            }
            else if (function.lacksRewritableCallForSomeInvocation)
            {
                sink->diagnose(Diagnostics::ResourceStaticUsedByFunctionWithoutRewritableCall{
                    .function = function.func,
                    .location = function.func->sourceLoc});
                diagnosed = true;
            }
        }
        return diagnosed;
    }

    /// Diagnose functions whose emitted implementation omits the IR body that this pass would
    /// rewrite, and return whether any diagnostic was emitted.
    bool diagnoseFunctionsWithAlternateImplementations(
        DiagnosticSink* sink,
        List<bool> const& accessesResource)
    {
        // Phase 3 analyzes the function's IR blocks, and phase 4 inserts copy-in and copy-out
        // operations into those blocks. Generic assembly or an applicable target intrinsic replaces
        // the blocks during emission. The emitted implementation would therefore omit the
        // operations introduced by this pass, so we reject the function before changing its
        // signature.
        bool diagnosed = false;
        for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
        {
            if (!accessesResource[functionIndex])
                continue;

            auto func = functions[functionIndex].func;
            bool hasGenericAssembly = false;
            for (auto block : func->getBlocks())
            {
                if (as<IRGenericAsm>(block->getTerminator()))
                {
                    hasGenericAssembly = true;
                    break;
                }
            }

            if (findBestTargetIntrinsicDecoration(func, targetCaps))
            {
                sink->diagnose(Diagnostics::ResourceStaticUsedByTargetIntrinsicFunction{
                    .function = func,
                    .location = func->sourceLoc});
                diagnosed = true;
            }
            else if (hasGenericAssembly)
            {
                sink->diagnose(Diagnostics::ResourceStaticUsedByGenericAssemblyFunction{
                    .function = func,
                    .location = func->sourceLoc});
                diagnosed = true;
            }
        }
        return diagnosed;
    }

    // ## Phase 3: Resource reads, writes, and address validation

    /// Analyze how every function reads and writes every resource global.
    ///
    /// For each global, we first follow storage-access transfers from its direct uses and then
    /// propagate possible effects through callers. Finally, we determine which writers assign the
    /// complete value on every reachable normal return and which functions may read the value
    /// present on entry. A non-entry-point function needs its caller's value if it may read that
    /// entry value or return without replacing it completely.
    void analyzeResourceAccess()
    {
        // We analyze one global at a time. We first record its direct uses and the storage accesses
        // propagated through them, then propagate callee effects to callers. With those possible
        // effects known, we determine which writers assign the complete value before every
        // reachable normal return and which functions may read the value present on entry.
        for (auto& resourceGlobal : resourceGlobals)
        {
            collectRootUses(resourceGlobal);

            HashSet<IRInst*> visitedAddresses;
            analyzeStorageAccesses(
                resourceGlobal,
                resourceGlobal.globalVar,
                visitedAddresses,
                true,
                false);

            propagateEffectsToCallers(resourceGlobal);
            determineWholeValueAssignmentsOnEveryReturn(resourceGlobal);
            determineEntryValueRequirements(resourceGlobal);
        }
    }

    /// Save every direct use of the global before phase 4 mutates any use list.
    ///
    /// Function-body uses establish where replacement storage is required. After phase 2, every
    /// remaining use outside a function body belongs to a child that will be removed with the
    /// global.
    void collectRootUses(ResourceGlobalToRewrite& global)
    {
        // We record function membership now because phase 4 will replace the use-list links. Phase
        // 2 has already established that a use outside a function body belongs to the global
        // itself.
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

    /// Record one runtime use reached at the end of a storage-access path.
    ///
    /// A terminal use contributes read/write effects to its containing function. We retain
    /// call-argument uses for alias validation and uses that may retain an address or observe its
    /// storage identity. We also distinguish a complete assignment from a write to only part of the
    /// value.
    void analyzeTerminalAddressUse(
        ResourceGlobalToRewrite& global,
        IRUse* use,
        bool accessCoversCompleteValue,
        bool accessSelectsSubobject)
    {
        // We retain address uses needed by the later validation and record the instruction's
        // read/write effects in its containing function.
        auto user = use->getUser();
        auto parentFunc = getParentFunc(user);
        SLANG_RELEASE_ASSERT(parentFunc);
        auto functionIndex = functionIndices.tryGetValue(parentFunc);
        SLANG_RELEASE_ASSERT(functionIndex);

        if (as<IRCall>(user))
            global.addressPassingUses.add(use);
        if (isAddressUseUnsupportedByPerFunctionStorage(use, targetCaps))
            global.unsupportedAddressUses.add(use);

        auto terminalAccess = classifyResourceAddressUse(use, targetCaps);
        auto& functionInfo = global.perFunctionInfo[*functionIndex];
        functionInfo.access = mergeAccess(functionInfo.access, terminalAccess);
        if (accessSelectsSubobject && hasAccess(terminalAccess, ResourceAccess::Write))
            functionInfo.mayWriteSubobject = true;

        // A store through an access path that covers the complete value assigns that value. Passing
        // the same address to an `out` parameter also establishes a complete assignment when the
        // call returns normally, because that is the parameter's language-level contract.
        bool assignsWholeValue = false;
        if (accessCoversCompleteValue)
        {
            if (auto store = as<IRStore>(user))
                assignsWholeValue = store->ptr.get() == use->get();
            else if (auto call = as<IRCall>(user))
            {
                // The `out` contract and the callee's actual reads are independent facts. A callee
                // may read through a cast before it assigns the parameter, but a normal return
                // still guarantees that it assigned the complete value.
                assignsWholeValue = as<IROutParamType>(findCallArgumentParameterType(call, use));
            }
        }
        global.terminalUses.add(
            ResourceAddressUse{use, *functionIndex, terminalAccess, assignsWholeValue});
    }

    /// Follow operations that may transfer storage access from one resource global, and classify
    /// the runtime uses reached through them.
    ///
    /// For each use, we either follow a result that may transfer a later access to or from the
    /// source, or hand the terminal use to `analyzeTerminalAddressUse`. We carry two facts through
    /// the recursion.
    /// Whether the access still covers the complete original value determines whether a terminal
    /// write is a complete assignment. Whether an operation explicitly selected a subobject
    /// determines whether an array may require subobject-sensitive initialization analysis. A
    /// pointee-changing cast or an unknown pointer offset makes the first fact unknown, but neither
    /// operation necessarily selects a subobject.
    void analyzeStorageAccesses(
        ResourceGlobalToRewrite& global,
        IRInst* address,
        HashSet<IRInst*>& visitedAddresses,
        bool accessCoversCompleteValue,
        bool accessSelectsSubobject)
    {
        // We follow only operand zero of an operation that may transfer storage access, so each
        // visited instruction has one traversable predecessor. Along that path, we record two
        // independent facts. The first says whether a write through the current address assigns the
        // complete original value. The second says whether the path contains an operation that
        // explicitly selects a subobject. A pointee-changing cast or an unknown pointer offset
        // invalidates the first fact without implying the second.
        if (!visitedAddresses.add(address))
            return;

        for (auto use = address->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (!doesInstUseOperandAtRuntime(user))
                continue;

            if (mayUseTransferStorageAccess(use))
            {
                auto relation = classifyStorageAccessRelation(use);
                auto resultAccessCoversCompleteValue =
                    accessCoversCompleteValue && relation == StorageAccessRelation::CompleteValue;
                auto resultAccessSelectsSubobject =
                    accessSelectsSubobject || relation == StorageAccessRelation::Subobject;
                analyzeStorageAccesses(
                    global,
                    user,
                    visitedAddresses,
                    resultAccessCoversCompleteValue,
                    resultAccessSelectsSubobject);
                continue;
            }
            analyzeTerminalAddressUse(
                global,
                use,
                accessCoversCompleteValue,
                accessSelectsSubobject);
        }
    }

    /// Propagate possible read, write, and subobject-write effects from callees to callers.
    ///
    /// Iterating direct calls to a fixed point handles non-recursive call chains and recursive
    /// strongly connected components without relying on a particular function order.
    void propagateEffectsToCallers(ResourceGlobalToRewrite& global)
    {
        // We repeatedly merge each callee's effects into its caller. We also propagate whether a
        // callee may write a subobject separately, because the array limitation checked below
        // applies to indirect writes as well as direct ones. Iteration stops when no edge changes,
        // which handles recursive calls as well as non-recursive call chains.
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

    /// Diagnose every resource array whose initialization may require subobject-sensitive analysis,
    /// and return whether any diagnostic was emitted.
    bool diagnoseArrayInitializationRequiringSubobjectAnalysis(DiagnosticSink* sink)
    {
        // The phase-3 control-flow proof recognizes a whole-array assignment as one instruction.
        // Proving that writes to selected subobjects initialize the complete array would also
        // require tracking which parts are written on each path. An array that has both a subobject
        // write and a read before a whole-array assignment may depend on that unsupported proof, so
        // we reject it. Phase 2 rejected every resource-using non-entry-point function that can run
        // without a recorded call, and the preceding fixed points propagated both facts through
        // those calls. We can therefore diagnose the unsupported array from each entry point's
        // summary.
        bool diagnosed = false;
        for (auto const& global : resourceGlobals)
        {
            if (!as<IRArrayTypeBase>(unwrapAttributedType(global.valueType)))
                continue;

            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                auto const& functionInfo = global.perFunctionInfo[functionIndex];
                if (!functions[functionIndex].isEntryPoint || !functionInfo.mayWriteSubobject ||
                    !functionInfo.mayReadValueBeforeWholeValueAssignment)
                {
                    continue;
                }

                sink->diagnose(Diagnostics::ResourceStaticArrayCompleteInitializationNotProven{
                    .variable = global.globalVar,
                    .location = global.globalVar->sourceLoc});
                diagnosed = true;
                break;
            }
        }
        return diagnosed;
    }

    /// Determine which writers assign the complete resource value before every reachable normal
    /// return.
    ///
    /// An argument that denotes the complete value and is passed to an explicit `out` parameter
    /// counts as an assignment by the parameter's contract. A call to a function that may write the
    /// global establishes the value for its caller after we prove that every reachable normal
    /// return from the callee follows a complete assignment. A writer with no reachable normal
    /// return also satisfies that condition because execution cannot continue after its call.
    /// Repeating the proof propagates this guarantee through the call graph.
    void determineWholeValueAssignmentsOnEveryReturn(ResourceGlobalToRewrite& global)
    {
        // The property is conditional: if a function returns normally, every returning path must
        // first assign the value. We begin by treating every writer as a candidate. We then remove
        // a candidate when a reachable return path reaches neither a direct complete assignment nor
        // a call to a function that remains a candidate. Starting with every candidate lets
        // recursive functions support one another, while iteration still removes a cycle that can
        // return without assigning the value.
        for (auto& resourceInfo : global.perFunctionInfo)
        {
            resourceInfo.assignsWholeValueOnEveryReturn =
                hasAccess(resourceInfo.access, ResourceAccess::Write);
        }

        bool changed = false;
        do
        {
            changed = false;
            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                auto& resourceInfo = global.perFunctionInfo[functionIndex];
                if (!resourceInfo.assignsWholeValueOnEveryReturn)
                    continue;

                if (!doesFunctionAssignWholeValueOnEveryReturn(global, functionIndex))
                {
                    resourceInfo.assignsWholeValueOnEveryReturn = false;
                    changed = true;
                }
            }
        } while (changed);
    }

    /// Return whether every reachable normal return follows a complete assignment.
    ///
    /// A writer with no reachable normal return also satisfies this condition because no execution
    /// continues past the function with a value to copy out.
    bool doesFunctionAssignWholeValueOnEveryReturn(
        ResourceGlobalToRewrite& global,
        Index functionIndex)
    {
        // We collect direct assignments and calls after which every continuing execution has a
        // complete value. A reachable return disproves the contract if execution can reach it
        // without first crossing one of those instructions. When there is no reachable return,
        // there is no value to copy out.
        HashSet<IRInst*> instructionsEnsuringCompleteValueBeforeContinuation;
        collectInstructionsEnsuringCompleteValueBeforeContinuation(
            global,
            functionIndex,
            instructionsEnsuringCompleteValueBeforeContinuation);

        auto dominatorTree = getDominatorTree(functionIndex);
        for (auto block : functions[functionIndex].func->getBlocks())
        {
            if (dominatorTree->isUnreachable(block))
                continue;

            auto returnInst = as<IRReturn>(block->getTerminator());
            if (!returnInst)
                continue;

            if (canReachInstructionWithoutPriorCompleteValue(
                    functionIndex,
                    returnInst,
                    instructionsEnsuringCompleteValueBeforeContinuation))
                return false;
        }

        return true;
    }

    /// Collect instructions after which every continuing execution has a complete resource value.
    ///
    /// A direct whole-value store establishes the value immediately. A normally returning call
    /// establishes it at completion when the complete-value address is passed to an explicit `out`
    /// parameter. A call to a function that may write the global establishes a complete value when
    /// every reachable normal return from the callee follows a complete assignment. A callee that
    /// may write but cannot return normally also satisfies that condition because execution never
    /// continues past the call.
    void collectInstructionsEnsuringCompleteValueBeforeContinuation(
        ResourceGlobalToRewrite& global,
        Index functionIndex,
        HashSet<IRInst*>& instructionsEnsuringCompleteValueBeforeContinuation)
    {
        // Terminal uses contain direct stores and calls that pass an address known to denote the
        // complete value to an explicit `out` parameter. We then add calls after which every
        // continuing path has a complete value.
        for (auto const& terminalUse : global.terminalUses)
        {
            if (terminalUse.functionIndex == functionIndex && terminalUse.assignsWholeValue)
                instructionsEnsuringCompleteValueBeforeContinuation.add(terminalUse.use->getUser());
        }

        for (auto const& edge : directCallEdges)
        {
            if (edge.callerIndex == functionIndex &&
                global.perFunctionInfo[edge.calleeIndex].assignsWholeValueOnEveryReturn)
            {
                instructionsEnsuringCompleteValueBeforeContinuation.add(edge.call);
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

    /// Return whether control can reach `target` without first establishing a complete value.
    ///
    /// Each instruction in `instructionsEnsuringCompleteValueBeforeContinuation` either assigns
    /// the complete value directly or calls a writer whose every normal return follows such an
    /// assignment. A writer with no normal return also satisfies that condition because execution
    /// cannot continue past the call. Reaching `target` first proves that some part of the entry
    /// value can reach it.
    bool canReachInstructionWithoutPriorCompleteValue(
        Index functionIndex,
        IRInst* target,
        HashSet<IRInst*> const& instructionsEnsuringCompleteValueBeforeContinuation)
    {
        // We search forward from the entry block and stop each path at the first instruction after
        // which any continuation has a complete value. Each block needs at most one visit because
        // the worklist represents only the state in which no complete assignment has occurred.
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

            bool pathEstablishedCompleteValue = false;
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
            {
                // A call can read the entry value before its normally returning executions
                // establish a complete value. We therefore test the target before stopping at that
                // call.
                if (inst == target)
                    return true;
                if (instructionsEnsuringCompleteValueBeforeContinuation.contains(inst))
                {
                    pathEstablishedCompleteValue = true;
                    break;
                }
            }
            if (pathEstablishedCompleteValue)
                continue;

            for (auto successor : block->getSuccessors())
            {
                if (visited.add(successor))
                    workList.add(successor);
            }
        }

        return false;
    }

    /// Determine which functions may read or preserve the resource value present on entry.
    ///
    /// We first mark writers that may return without assigning the complete value. In a
    /// non-entry-point function, its local must then start with the caller's value even when the
    /// function never reads it. We next classify direct reads by whether every path to them crosses
    /// a complete assignment. Finally, we propagate callee read requirements to callers, stopping
    /// when an earlier assignment supplies the value instead.
    void determineEntryValueRequirements(ResourceGlobalToRewrite& global)
    {
        // When a non-entry-point writer can return without assigning the complete value, its local
        // must begin with the caller's value even when the function never reads that value
        // directly. We record the same analysis fact for entry points because phase 3 treats every
        // function uniformly. We then find direct reads reachable before a complete assignment and
        // propagate those read requirements from callees to callers.
        for (auto& resourceInfo : global.perFunctionInfo)
        {
            if (hasAccess(resourceInfo.access, ResourceAccess::Write) &&
                !resourceInfo.assignsWholeValueOnEveryReturn)
            {
                resourceInfo.mayReturnWithoutWholeValueAssignment = true;
            }
        }

        for (auto const& terminalUse : global.terminalUses)
        {
            if (!hasAccess(terminalUse.access, ResourceAccess::Read))
                continue;
            if (!isReadBlockedByCompleteValue(
                    global,
                    terminalUse.functionIndex,
                    terminalUse.use->getUser()))
            {
                global.perFunctionInfo[terminalUse.functionIndex]
                    .mayReadValueBeforeWholeValueAssignment = true;
            }
        }

        bool changed = false;
        do
        {
            changed = false;
            for (auto const& edge : directCallEdges)
            {
                if (!global.perFunctionInfo[edge.calleeIndex]
                         .mayReadValueBeforeWholeValueAssignment)
                    continue;
                auto& caller = global.perFunctionInfo[edge.callerIndex];
                if (caller.mayReadValueBeforeWholeValueAssignment ||
                    isReadBlockedByCompleteValue(global, edge.callerIndex, edge.call))
                {
                    continue;
                }
                caller.mayReadValueBeforeWholeValueAssignment = true;
                changed = true;
            }
        } while (changed);
    }

    /// Return whether the function contains an instruction after which any continuing execution
    /// has a complete value, and no path reaches `read` before such an instruction.
    ///
    /// Direct whole-value stores establish the value. A call to a proved writer either establishes
    /// it before returning normally or prevents execution from continuing.
    bool isReadBlockedByCompleteValue(
        ResourceGlobalToRewrite& global,
        Index functionIndex,
        IRInst* read)
    {
        // We first require at least one such instruction in this function. We then search for an
        // entry-to-read path that reaches none of those instructions. An unreachable read has no
        // such path. Requiring a nonempty set prevents us from treating a function with no
        // complete assignment as initialized.
        HashSet<IRInst*> instructionsEnsuringCompleteValueBeforeContinuation;
        collectInstructionsEnsuringCompleteValueBeforeContinuation(
            global,
            functionIndex,
            instructionsEnsuringCompleteValueBeforeContinuation);
        return instructionsEnsuringCompleteValueBeforeContinuation.getCount() != 0 &&
               !canReachInstructionWithoutPriorCompleteValue(
                   functionIndex,
                   read,
                   instructionsEnsuringCompleteValueBeforeContinuation);
    }

    /// Record the phase-3 effect of every terminal write for the later uninitialized-value check.
    ///
    /// The shared checker can infer ordinary store and parameter-direction effects, but phase 3 may
    /// know more. For example, a callee can read through a cast from an `out` parameter before it
    /// assigns that parameter. We retain both the possible read and the complete-assignment fact so
    /// the checker does not have to reconstruct this interprocedural analysis from rewritten IR.
    void recordTerminalWriteEffects()
    {
        // We associate the effect with the exact address operand because a call can pass one value
        // to several parameters with different direction contracts.
        for (auto& global : resourceGlobals)
        {
            for (auto const& terminalUse : global.terminalUses)
            {
                if (!hasAccess(terminalUse.access, ResourceAccess::Write))
                    continue;

                auto& resourceInfo = global.perFunctionInfo[terminalUse.functionIndex];
                resourceInfo.uninitializedUseEffects.add(UninitializedVariableUseEffect{
                    .use = terminalUse.use,
                    .readsValue = hasAccess(terminalUse.access, ResourceAccess::Read),
                    .mayWriteValue = true,
                    .definitelyWritesValue = terminalUse.assignsWholeValue,
                });
            }
        }
    }

    /// Diagnose address uses that separate per-function storage cannot preserve.
    ///
    /// Phase 4 gives each affected function its own storage. Code that retains an address or
    /// otherwise observes its storage identity could detect that change. Return whether any
    /// diagnostic was emitted.
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
    /// shared global address. Return whether any diagnostic was emitted.
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

                auto explicitAccess = classifyCallArgumentAccess(call, use, targetCaps);
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

    /// Assert that every runtime invocation of a function needing a resource parameter is a direct
    /// call that phase 4 can rewrite.
    void assertFunctionsThatNeedParametersHaveOnlyRewritableInvocations()
    {
        // Phase 2 has already diagnosed functions with an invocation that phase 4 cannot rewrite
        // and runtime uses other than direct calls inside functions. Direct non-runtime references
        // inside the function body may remain. This assertion ensures that phase 4 cannot change a
        // signature while leaving an invocation unchanged.
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
                if (!doesInstUseOperandAtRuntime(user))
                    continue;

                auto call = as<IRCall>(user);
                SLANG_RELEASE_ASSERT(call && call->getCalleeUse() == use);
                SLANG_RELEASE_ASSERT(getParentFunc(call));
            }
        }
    }

    // ## Phase 4: Locals, parameters, and call arguments

    /// Create a local for each function whose execution may access the value, plus each function
    /// body with a direct non-runtime reference. Create a parameter for each affected
    /// non-entry-point function with runtime access.
    void introduceReplacements()
    {
        // Each entry point that may read or write the value creates a fresh local. Each
        // resource-using non-entry-point function receives a parameter and operates on a local
        // copy. A function with a direct non-runtime reference and no direct or transitive runtime
        // access needs a local so that phase 4 can redirect that reference, but it needs no runtime
        // parameter. After all parameters exist, we rebuild each changed function type and its
        // debug type once.
        IRBuilder builder(module);

        for (auto& global : resourceGlobals)
        {
            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                auto const& function = functions[functionIndex];
                auto& resourceInfo = global.perFunctionInfo[functionIndex];
                if (resourceInfo.access == ResourceAccess::None && !resourceInfo.hasDirectGlobalUse)
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
                hasGeneratedParameter |= global.perFunctionInfo[functionIndex].parameter != nullptr;
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
        // every reachable normal return follows a complete assignment and no path reads the entry
        // value. Every other writer requires `inout`.
        bool writesValue = hasAccess(resourceInfo.access, ResourceAccess::Write);
        if (writesValue && !resourceInfo.mayReadValueBeforeWholeValueAssignment &&
            resourceInfo.assignsWholeValueOnEveryReturn)
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

    /// Initialize the local when the function may read the entry value or return without completely
    /// replacing it.
    void initializeLocalFromGeneratedParameter(
        IRBuilder& builder,
        ResourceGlobalToRewrite const& global,
        FunctionResourceInfo& resourceInfo)
    {
        // A read-only parameter is already a value. An `inout` parameter is an address, so we load
        // it before storing into the local. A function with an `out` parameter deliberately starts
        // with an uninitialized local.
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
        // These stores implement the caller-visible `out` or `inout` update. On targets that
        // require resource-output specialization, `specializeResourceOutputs` recognizes and
        // rewrites this copy-out pattern. A read-only function needs no copy-out.
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

    /// Redirect each saved function-local use to that function's generated local.
    void replaceGlobalUses()
    {
        // Phase 2 rejected every use outside a function body except a direct child of the selected
        // global. Such a child disappears with the global in phase 5. Every function-body use can
        // be redirected to the local chosen for that function.
        for (auto& global : resourceGlobals)
        {
            for (auto const& rootUse : global.rootUses)
            {
                if (rootUse.functionIndex < 0)
                {
                    auto user = rootUse.use->getUser();
                    SLANG_RELEASE_ASSERT(user->getParent() == global.globalVar);
                    continue;
                }

                auto replacement = global.perFunctionInfo[rootUse.functionIndex].replacementAddress;
                SLANG_RELEASE_ASSERT(replacement);
                rootUse.use->set(replacement);
            }
        }
    }

    /// Replace direct calls whose callees gained resource parameters.
    void rewriteCalls()
    {
        // We preserve every pre-existing argument at its original index, then append generated
        // resource arguments in module order. We also preserve source location, decorations, and
        // the use-specific effects needed by the later uninitialized-value check.
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
                auto argument = builder.emitLoad(global.valueType, callerInfo.replacementAddress);
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

    /// Record the uninitialized-value write effect of each generated `out` or `inout` argument.
    void recordResourceArgumentUseEffects(
        IRCall* oldCall,
        IRCall* newCall,
        DirectCallEdge const& edge)
    {
        // The generated parameter direction cannot express all of phase 3's facts. An `inout`
        // parameter carries the caller's value into the callee when a reachable normal return may
        // occur without a complete assignment, even when the callee never reads the incoming value.
        // Conversely, a call is a definite write only when every normally completing callee
        // execution assigns the complete value. A writer that cannot return normally also satisfies
        // that condition because no successor can observe a value. We attach those answers to the
        // exact generated argument use.
        //
        // Copying the input into a generated local does not itself count as a source-level read. If
        // a function returns without completely assigning the value, copy-out may preserve an
        // uninitialized value. That is not a source-level error unless the caller later reads it.
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
                    .readsValue = calleeInfo.mayReadValueBeforeWholeValueAssignment,
                    .mayWriteValue = true,
                    .definitelyWritesValue = calleeInfo.assignsWholeValueOnEveryReturn,
                });
            }
            resourceArgIndex++;
        }
    }

    // ## Phase 5: Generated-local diagnostics and obsolete-storage removal

    /// Diagnose entry-point paths that read a generated local before a complete assignment.
    void diagnoseUninitializedEntryPointReads(DiagnosticSink* sink)
    {
        // `checkForUsingUninitializedValues` ran before this pass created these locals. We apply
        // its intraprocedural analysis to each generated entry-point local and supply the per-use
        // effects recorded during analysis and call rewriting.
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

    /// Remove each obsolete global and its attached children after redirecting all function-body
    /// uses.
    void removeReplacedResourceGlobals()
    {
        // Phase 4 has redirected every function-body use. A remaining use may come only from a
        // direct child of the global, such as attached metadata. Deleting the global also deletes
        // those children, which clears their operands before the global itself is removed.
        for (auto& global : resourceGlobals)
        {
            for (auto use = global.globalVar->firstUse; use; use = use->nextUse)
                SLANG_RELEASE_ASSERT(use->getUser()->getParent() == global.globalVar);
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

/// Return whether `inst` is nested inside a function block.
static bool isInstInsideFunctionBody(IRInst* inst)
{
    // We walk toward the module until we reach a block or function. A block belongs to executable
    // function code only when `getParentFunc` finds its function. Reaching a function first means
    // that `inst` is a function-level child, such as a decoration, rather than part of the body.
    for (auto parent = inst->getParent(); parent; parent = parent->getParent())
    {
        if (as<IRBlock>(parent))
            return getParentFunc(parent) != nullptr;
        if (as<IRFunc>(parent))
            return false;
    }
    return false;
}

/// Return whether an instruction observes or changes an operand's runtime value.
///
/// Address-use analysis must ignore decorations, debug records, types, and queries whose result
/// depends only on an operand's type. Every other use is conservatively treated as a value use so
/// that an unfamiliar instruction cannot silently omit a required resource argument.
static bool doesInstUseOperandAtRuntime(IRInst* user)
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
/// A parameter's direction provides its declared read/write contract. When a defined direct callee
/// receives an address, we also inspect the parameter's IR uses because an explicit pointer cast
/// can perform an effect that the source-level direction does not express.
static ResourceAccess classifyCallArgumentAccess(
    IRCall* call,
    IRUse* use,
    CapabilitySet const& targetCaps)
{
    // We first apply the declared direction. A raw pointer or a use with no corresponding parameter
    // provides no directional guarantee, so we retain both effects. For a directional parameter,
    // we then merge any additional reads or writes found by following the callee's address uses.
    auto paramType = findCallArgumentParameterType(call, use);
    if (!paramType)
        return mergeAccess(ResourceAccess::Read, ResourceAccess::Write);

    ResourceAccess access = ResourceAccess::Read;
    if (as<IROutParamType>(paramType))
        access = ResourceAccess::Write;
    else if (as<IRBorrowInOutParamType>(paramType) || as<IRRefParamType>(paramType))
        access = mergeAccess(ResourceAccess::Read, ResourceAccess::Write);
    else if (as<IRPtrTypeBase>(paramType) && !as<IRBorrowInParamType>(paramType))
    {
        // A raw pointer parameter carries no directional contract. Its callee may read or write the
        // pointee, so we preserve both directions rather than assuming an input use.
        return mergeAccess(ResourceAccess::Read, ResourceAccess::Write);
    }

    if (auto parameter = findDirectCalleeParameter(call, use))
    {
        auto addressUse = analyzeAddressUses(parameter, targetCaps);
        if (addressUse.mayRead)
            access = mergeAccess(access, ResourceAccess::Read);
        if (addressUse.mayWrite)
            access = mergeAccess(access, ResourceAccess::Write);
    }
    return access;
}

/// Return how a runtime use reads or writes the resource reached through `use`.
///
/// Loads are reads, and stores through the address operand are writes. Calls combine the formal
/// parameter's declared direction with any additional reads or writes found in a defined direct
/// callee. Uses with no precise contract conservatively preserve every effect they might have. The
/// caller decides whether a write assigns the complete value, because that also depends on whether
/// the storage-access path covers the complete value or only a subobject.
static ResourceAccess classifyResourceAddressUse(IRUse* use, CapabilitySet const& targetCaps)
{
    // We classify loads, atomic operations, stores, and calls from their operand contracts. An
    // unfamiliar pointer consumer may both read and write. A non-address value consumer reads the
    // stored resource.
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
        return classifyCallArgumentAccess(call, use, targetCaps);

    // A pointer-producing instruction that does not use a recognized storage-access transfer may
    // let the address escape. The same is true when the address is routed through control flow
    // or returned. We cannot safely infer how those users access the variable, so we preserve both
    // directions.
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
        // We provisionally classify every other terminal use as a read. The validation in phase 3
        // rejects a use that may retain the address or observe its storage identity instead of
        // reading its value.
        return ResourceAccess::Read;
    }
}

/// Classify how storage access through the user's result applies to the source value.
///
/// Field and element operations select a subobject. `GetAddress` and `AssumeAddress` preserve the
/// complete value. The lowering of an implicit l-value cast either reuses the source address or
/// copies a complete value between the source and a temporary. Any access transferred by that cast
/// therefore preserves whether the source denotes the complete value or a selected subobject.
/// Other pointer casts preserve the complete value only when their source and result have equal
/// pointee types.
static StorageAccessRelation classifyStorageAccessRelation(IRUse* use)
{
    // We first handle operations whose semantics state the relation directly. For the remaining
    // pointer casts, equal pointee types prove that both addresses cover the same complete value;
    // unequal pointee types leave the relation unknown.
    auto user = use->getUser();
    switch (user->getOp())
    {
    case kIROp_FieldAddress:
    case kIROp_GetElementPtr:
    case kIROp_RWStructuredBufferGetElementPtr:
    case kIROp_NodeOutputRecordGetElementPtr:
        return StorageAccessRelation::Subobject;

    case kIROp_GetOffsetPtr:
        {
            // `GetOffsetPtr` moves between objects with the same pointee type; it does not select
            // a subobject of that type. An offset of zero therefore preserves the complete value.
            // Any other offset may name a neighboring object rather than the source value, so it
            // establishes neither a complete-value relation nor a subobject relation.
            auto offset = as<IRIntLit>(user->getOperand(1));
            return offset && offset->getValue() == 0 ? StorageAccessRelation::CompleteValue
                                                     : StorageAccessRelation::Unknown;
        }

    case kIROp_GetAddress:
    case kIROp_AssumeAddress:
    case kIROp_OutImplicitCast:
    case kIROp_InOutImplicitCast:
        return StorageAccessRelation::CompleteValue;

    case kIROp_BitCast:
    case kIROp_Reinterpret:
    case kIROp_PtrCast:
        break;
    default:
        SLANG_UNEXPECTED("unrecognized storage-access transfer");
    }

    auto sourcePointerType = as<IRPtrTypeBase>(use->get()->getDataType());
    auto resultPointerType = as<IRPtrTypeBase>(user->getDataType());
    SLANG_RELEASE_ASSERT(sourcePointerType && resultPointerType);

    auto sourceValueType = cast<IRType>(unwrapAttributedType(sourcePointerType->getValueType()));
    auto resultValueType = cast<IRType>(unwrapAttributedType(resultPointerType->getValueType()));
    return isTypeEqual(sourceValueType, resultValueType) ? StorageAccessRelation::CompleteValue
                                                         : StorageAccessRelation::Unknown;
}

/// Return the defined direct callee's parameter corresponding to `argumentUse`, or null when the
/// call or use has no such parameter.
static IRParam* findDirectCalleeParameter(IRCall* call, IRUse* argumentUse)
{
    // Operand zero names the callee. We match the remaining operand use by identity because one
    // value may be passed to several parameters. We require an IR body because a declaration does
    // not expose how the callee uses the address.
    auto callee = as<IRFunc>(call->getCallee());
    if (!callee || !callee->getFirstBlock() || argumentUse == call->getCalleeUse())
        return nullptr;

    UInt argumentIndex = 0;
    for (; argumentIndex < call->getArgCount(); ++argumentIndex)
    {
        if (call->getOperandUse(argumentIndex + 1) == argumentUse)
            break;
    }
    if (argumentIndex == call->getArgCount())
        return nullptr;

    UInt parameterIndex = 0;
    for (auto parameter : callee->getParams())
    {
        if (parameterIndex == argumentIndex)
            return parameter;
        ++parameterIndex;
    }
    return nullptr;
}

/// Analyze reads, writes, and unsupported uses reached from `rootAddress`.
///
/// A supported runtime use either transfers storage access through another result, reads or writes
/// through the address, or passes it to a directional parameter of a directly called function
/// whose IR body is available. We follow that callee parameter under the same rules. An applicable
/// target-intrinsic implementation is unsupported because it may use a parameter differently from
/// the function's IR body.
static AddressUseAnalysis analyzeAddressUses(IRInst* rootAddress, CapabilitySet const& targetCaps)
{
    // We visit each reachable address once. The work list therefore terminates even when recursive
    // functions forward a parameter around a call cycle, while still inspecting every reachable
    // runtime use of that parameter.
    AddressUseAnalysis result;
    List<IRInst*> addressesToInspect;
    HashSet<IRInst*> visitedAddresses;
    addressesToInspect.add(rootAddress);
    for (Index addressIndex = 0; addressIndex < addressesToInspect.getCount(); ++addressIndex)
    {
        auto address = addressesToInspect[addressIndex];
        if (!visitedAddresses.add(address))
            continue;

        // An applicable target intrinsic replaces its function's IR body during emission. The
        // body therefore cannot prove how the emitted implementation uses one of its parameters.
        if (as<IRParam>(address))
        {
            auto function = getParentFunc(address);
            if (function && findBestTargetIntrinsicDecoration(function, targetCaps))
            {
                result.hasUnsupportedUse = true;
                return result;
            }
        }

        for (auto use = address->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (!doesInstUseOperandAtRuntime(user))
                continue;

            // An address projection or pointer-to-pointer cast can make a later access through its
            // result affect storage reached from the source operand. An l-value implicit cast may
            // use a temporary, but its copy-in or copy-out transfers at least one access direction.
            // We inspect the result so its terminal uses reveal the storage effects transferred to
            // or from the source. Because this analysis does not track transfer direction, it
            // conservatively rejects any terminal use that it cannot classify.
            if (mayUseTransferStorageAccess(use))
            {
                addressesToInspect.add(user);
                continue;
            }

            if (as<IRLoad>(user) || as<IRAtomicLoad>(user))
            {
                result.mayRead = true;
                continue;
            }
            if (auto store = as<IRStore>(user))
            {
                if (store->getOperandUse(0) == use)
                {
                    result.mayWrite = true;
                    continue;
                }
                result.hasUnsupportedUse = true;
                return result;
            }
            if (as<IRAtomicOperation>(user) || as<IRSwizzledStore>(user) ||
                as<IRMatrixSwizzleStore>(user))
            {
                if (user->getOperandUse(0) == use)
                {
                    result.mayWrite = true;
                    if (!as<IRAtomicStore>(user) && !as<IRSwizzledStore>(user) &&
                        !as<IRMatrixSwizzleStore>(user))
                    {
                        result.mayRead = true;
                    }
                    continue;
                }
                result.hasUnsupportedUse = true;
                return result;
            }

            if (auto call = as<IRCall>(user))
            {
                auto parameter = findDirectCalleeParameter(call, use);
                auto parameterType =
                    parameter ? as<IRType>(unwrapAttributedType(parameter->getDataType()))
                              : nullptr;
                bool hasDirectionalParameter =
                    as<IRBorrowInParamType>(parameterType) || as<IROutParamType>(parameterType) ||
                    as<IRBorrowInOutParamType>(parameterType) || as<IRRefParamType>(parameterType);
                if (!hasDirectionalParameter)
                {
                    result.hasUnsupportedUse = true;
                    return result;
                }
                addressesToInspect.add(parameter);
                continue;
            }

            // Returning the address, storing it as a value, comparing it, or converting it to an
            // integer can make the identity or lifetime of the replacement local observable.
            result.hasUnsupportedUse = true;
            return result;
        }
    }
    return result;
}

/// Return whether a use may retain the address or otherwise observe its storage identity.
static bool isAddressUseUnsupportedByPerFunctionStorage(IRUse* use, CapabilitySet const& targetCaps)
{
    // The loads and writes handled below cannot retain the address. For a call, the parameter's
    // direction does not establish that the callee leaves the address unretained. We inspect the
    // matching parameter and any direct calls to which it is forwarded. Every other terminal use
    // may retain the address or derive an identity-dependent value from it.
    auto user = use->getUser();
    if (as<IRLoad>(user) || as<IRAtomicLoad>(user))
        return false;
    if (auto store = as<IRStore>(user))
        return store->getOperandUse(0) != use;
    if (as<IRAtomicOperation>(user) || as<IRSwizzledStore>(user) || as<IRMatrixSwizzleStore>(user))
        return user->getOperandUse(0) != use;
    if (auto call = as<IRCall>(user))
    {
        // A directional parameter does not prevent the callee from storing its argument's address.
        // We accept the call only when the available callee body proves that every runtime use of
        // the matching parameter is non-escaping. `analyzeAddressUses` documents the accepted uses.
        auto parameter = findDirectCalleeParameter(call, use);
        return !parameter || analyzeAddressUses(parameter, targetCaps).hasUnsupportedUse;
    }

    // Every other terminal consumer may retain the address or derive an identity-dependent value
    // from it. For example, pointer-to-integer conversion and pointer comparison would observe that
    // different functions now use different locals.
    return true;
}

/// Return whether `user` is a decoration that causes the referenced function to be invoked.
static bool doesDecorationInvokeReferencedFunction(IRInst* user)
{
    // These decorations name functions that the compiler or runtime calls without an `IRCall` in
    // the current module. We enumerate them so that metadata decorations which also point to a
    // function, such as `IREntryPointParamDecoration`, do not imply another invocation.
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
        if (!doesInstUseOperandAtRuntime(user))
            continue;
        return true;
    }
    return false;
}

/// Return whether `func` may be invoked without an in-module direct call that can be rewritten.
static bool mayBeInvokedWithoutRewritableCall(IRFunc* func)
{
    // Each listed linkage or retention decoration prevents the module from proving that every
    // invocation appears as a direct call whose argument list we can extend. A non-call use can
    // likewise pass the function as a value or name it in an invocation decoration such as
    // `IRPatchConstantFuncDecoration`.
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

/// Return whether `globalVar` must remain in module-scope storage.
///
/// Retention decorations and linkage that exposes storage outside this module establish this
/// requirement. `IRExportDecoration`, which only matches definitions during IR linking, does not.
static bool requiresModuleScopeStorage(IRGlobalVar* globalVar)
{
    // `IRExportDecoration` is intentionally absent. Linked source definitions use it for IR
    // linking, but `IRExportDecoration` does not require the final program to expose their
    // addresses or retain their storage. Every decoration below prevents replacement with
    // entry-point-local storage.
    static const IROp kModuleScopeStorageDecorations[] = {
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
    for (auto op : kModuleScopeStorageDecorations)
    {
        if (globalVar->findDecorationImpl(op))
            return true;
    }
    return false;
}

} // namespace

void legalizeResourceGlobalVars(
    IRModule* module,
    CapabilitySet const& targetCaps,
    DiagnosticSink* sink)
{
    // The caller has already extracted every selected initializer body. We can therefore replace
    // each selected storage declaration without losing initializer code.
    LegalizeResourceGlobalVarsPass pass(module, targetCaps);
    pass.processModule(sink);
}

} // namespace Slang
