// slang-ir-legalize-resource-globals.cpp
//
// `legalizeResourceGlobalVars` replaces mutable resource state that has one value per entry-point
// invocation but is represented in IR by an `IRGlobalVar`. AST-to-IR lowering adds
// `IRFileOrNamespaceScopeMutableVarDecoration` to programmer-declared file- or namespace-scope
// `static` variables and to mutable shadows of HLSL uniform parameters synthesized for `-Gec`.
// The marker records those declaration categories, including a `static` variable with an explicit
// storage rate. In this file, a "selected global" is a marked global for which
// `isResourceGlobalCandidateForPerInvocationReplacement` also accepts the rate and stored type.
//
// Consider a file-scope `static` variable:
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
// not denote persistent program-wide storage. We make those semantics explicit on every target by
// using an entry-point local and passing its value to helper functions:
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
//    storage, or if an instruction outside a function body references it. Attached metadata that
//    has no users outside the global's complete metadata subtree is the only exception; phase 5
//    removes that metadata with the global.
// 3. For each variable, we determine which functions may read or write its value. We follow each
//    use through projections, pointer casts, and l-value casts that transfer a read or write to the
//    original storage. We propagate each non-entry-point function's effects to its callers. We also
//    determine whether every reachable normal return from a writer follows a whole-value
//    assignment. From that result and direct whole-value assignments, we identify instructions
//    after which every continuing execution has a fully assigned value. We call this instruction-
//    level fact a whole-value continuation guarantee. We use these results to reject a non-entry-
//    point function that accesses the variable when some possible invocation has no direct
//    `IRCall` whose arguments we can rewrite. A launch of an entry point needs no rewritable call,
//    because phase 4 gives the entry point its own replacement local. `fixEntryPointCallsites`
//    redirects ordinary calls to shader entry points and CUDA kernels to non-entry-point clones.
//    Invocation decorations name compiler or runtime calls that have no `IRCall` argument list.
//    Passing, storing, or returning a function value can lead to an indirect call that this pass
//    cannot find and rewrite. We reject both cases for a function that needs resource state.
//    Generic assembly and applicable target intrinsics are rejected because their emitted
//    implementations can differ from the IR blocks that this pass would rewrite. Finally, we reject
//    uses that may retain the address or observe its storage identity, and calls that pass the same
//    writable value through both an existing argument and a generated argument.
// 4. For each selected variable, we give every entry point whose execution may read or write it a
//    fresh local. We give each non-entry-point function whose execution may read or write the
//    variable, directly or through a callee, a generated parameter and replacement local. A
//    function whose only need for replacement storage is a direct non-runtime reference inside its
//    body, such as debug metadata, needs a local but no parameter. We replace direct uses of the
//    globals and append the corresponding arguments to direct calls.
// 5. When uninitialized-value diagnostics are enabled, we apply the generated-local control-flow
//    check to every entry-point replacement local for an original global for which the pre-link
//    module-wide check found a possible write. We also check those locals when the linked program
//    still contains a possible write that the earlier check could not see. The check reports a read
//    when uninitialized state can reach it. In every case, we then remove the obsolete globals and
//    the attached metadata accepted in phase 2.
//
// In this file, "entry point" means either a shader entry point or a CUDA kernel. Both kinds of
// entry point create their own replacement locals for the selected globals that they use.
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
// supplied its value. The front end therefore excludes both types on every target so that language
// legality does not depend on the selected backend.
//
// The pass does not recursively replace the fields of a struct. Such a rule would need to preserve
// non-resource fields while mapping every resource field to the values produced by type
// legalization. Parameter groups, combined texture-sampler types, and append or consume buffers can
// likewise map one declaration to several IR values, with details that depend on the target.
// The analysis assumes that each selected global is represented by one IR value. Within each
// affected function, the pass can copy that value through one local and, in a non-entry-point
// function, one parameter. The pass cannot coordinate the several target-dependent values produced
// when these categories are legalized, so the front end rejects them for now.
//
// We choose each generated parameter type from the answers to two questions: can the function read
// the value supplied by its caller before a whole-value assignment, and does every reachable normal
// return follow a whole-value assignment? A read-only non-entry-point function gets a value
// parameter. A writer gets an `out` parameter only when it does not need the caller's value and
// every reachable normal return follows a whole-value assignment. Every other writer gets `inout`.
// On targets that require resource-output specialization, `specializeResourceOutputs` recognizes
// the generated copy-in, local, and copy-out pattern.
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

/// A `ResourceAccess` value records whether an instruction or function may read or write one
/// resource value.
///
/// The per-function summaries propagate these possible effects transitively from callees to
/// callers. Definite whole-value assignment is tracked separately because `Write` alone does not
/// say whether every path to a reachable normal return performs a whole-value assignment.
enum class ResourceAccess : UInt
{
    None = 0,
    Read = 1 << 0,
    Write = 1 << 1,
};

/// Return whether `value` includes `direction`, which must name one access direction.
static bool hasAccessDirection(ResourceAccess value, ResourceAccess direction);

/// A `StorageAccessRelation` describes how reads or writes through an instruction's result apply
/// to storage supplied through operand zero.
enum class StorageAccessRelation : UInt
{
    /// The propagated access applies to the whole value in the operand-zero storage.
    WholeValue,

    /// The propagated access applies to a field, element, or other subobject.
    Subobject,

    /// We cannot prove which part of the operand-zero storage the propagated access applies to.
    Unknown,

    /// The result may refer to storage other than the storage supplied through operand zero.
    Unsupported,
};

/// A `StorageAccessPathState` records the facts carried while following results through which
/// storage access transfers to or from a selected global.
struct StorageAccessPathState
{
    /// Whether a write through the current address assigns the whole original value.
    bool coversWholeValue = false;

    /// Whether the path explicitly selected a field, element, or other subobject.
    bool selectsSubobject = false;

    /// The read and write directions that the path can transfer to the original storage.
    ResourceAccess transferredAccess = ResourceAccess::None;
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

    /// The first ordinary instruction present when this pass begins.
    ///
    /// The preceding initializer-movement pass may already have inserted stores at this point. We
    /// place each replacement local before this anchor so those stores refer to declared storage.
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

    /// For a function that may write, whether every reachable normal return follows a whole-value
    /// assignment.
    bool assignsWholeValueOnEveryReturn = false;

    /// Whether some path reads the value present on entry before a whole-value assignment.
    bool mayReadValueBeforeWholeValueAssignment = false;

    /// Whether this function or one of its callees may write through an address that selects a
    /// subobject.
    bool mayWriteSubobject = false;

    /// Whether this function contains any direct use of the global, including non-runtime uses.
    bool hasDirectGlobalUse = false;

    /// The local address that replaces uses of the global in this function.
    ///
    /// This field remains null before phase 4 and when the function has neither runtime access nor
    /// a direct non-runtime reference to the global.
    IRVar* replacementAddress = nullptr;

    /// The generated parameter for a non-entry-point function that accesses the resource.
    ///
    /// This field remains null for entry points, before phase 4, and when a local exists only to
    /// replace a non-runtime use such as debug metadata.
    IRParam* parameter = nullptr;

    /// Exhaustive runtime effects retained for an entry-point replacement local's uninitialized-
    /// value check. This list remains empty for a non-entry-point function.
    List<GeneratedResourceLocalUseEffect> entryPointLocalUseEffects;

    /// Return whether a writer can return normally without assigning the whole value.
    bool mayReturnWithoutWholeValueAssignment() const
    {
        if (!hasAccessDirection(access, ResourceAccess::Write))
            return false;
        return !assignsWholeValueOnEveryReturn;
    }

    /// Return whether a non-entry-point function must initialize its replacement local from its
    /// generated parameter.
    bool mustInitializeLocalFromGeneratedParameter() const
    {
        if (mayReadValueBeforeWholeValueAssignment)
            return true;
        return mayReturnWithoutWholeValueAssignment();
    }
};

/// A `DirectGlobalUse` identifies a direct use and the function body that contains it, if any.
struct DirectGlobalUse
{
    /// The direct operand use of the original global.
    IRUse* use = nullptr;

    /// The containing function's index in `functions`, or -1 for a use outside a function body.
    Index functionIndex = -1;
};

/// A `TerminalAddressUse` describes a terminal runtime use reached from the global's address.
///
/// Most entries are loads, stores, atomic operations, or call arguments. An unsupported terminal
/// consumer receives conservative provisional effects until phase 3 diagnoses it.
struct TerminalAddressUse
{
    /// The operand through which its containing instruction accesses the address.
    IRUse* use = nullptr;

    /// The containing function's index in `functions`.
    Index functionIndex = -1;

    /// The possible read/write effect of the instruction that contains `use`.
    ResourceAccess access = ResourceAccess::None;

    /// Whether every normally completing execution of this instruction assigns the whole value
    /// stored by the original global.
    bool assignsWholeValue = false;
};

/// A `ResourceGlobalToRewrite` holds the analysis results and generated IR for one resource global.
///
/// Phases 1 through 3 populate the original uses, the per-function effect summaries, and the
/// existing terminal effects in entry-point bodies. Phase 4 records a replacement local for each
/// affected function, a generated parameter for each non-entry-point function with runtime access,
/// and the effects of generated resource-argument loads and writable generated call operands in
/// entry-point bodies. `ResourceGlobalToRewrite` retains the analysis that phase 5 needs to check
/// the entry-point locals for uninitialized reads.
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
    List<TerminalAddressUse> terminalAddressUses;

    /// Call-argument address uses retained for alias validation before rewriting.
    List<IRUse*> addressPassingUses;

    /// Uses that may retain the address or otherwise observe its storage identity.
    List<IRUse*> unsupportedAddressUses;
};

/// A `DirectCallEdge` records one direct call between functions in `functions`.
struct DirectCallEdge
{
    /// The call instruction represented by this edge.
    IRCall* call = nullptr;

    /// The caller's index in `functions`.
    Index callerIndex = -1;

    /// The callee's index in `functions`.
    Index calleeIndex = -1;
};

// Forward declarations let the pass appear before its low-level classifiers, so readers encounter
// the algorithm before its implementation details. The definitions follow the transformation.
/// Combine two sets of resource-access directions.
static ResourceAccess mergeAccess(ResourceAccess left, ResourceAccess right);
/// Retain the resource-access directions common to both sets.
static ResourceAccess intersectAccess(ResourceAccess left, ResourceAccess right);
/// Return which accesses through the user's result also access storage supplied by operand zero.
static ResourceAccess getTransferredStorageAccess(IRUse* use);
/// Return whether an instruction observes or changes an operand's runtime value.
static bool doesInstUseOperandAtRuntime(IRInst* user);
/// Analyze reads, writes, and unsupported uses reached from `rootAddress`.
static AddressUseAnalysis analyzeAddressUses(
    IRInst* rootAddress,
    CapabilitySet const& targetCaps,
    ResourceAccess transferredAccess);
/// Return which accesses through a callee parameter also access the caller-provided storage.
static ResourceAccess getCallArgumentBodyTransferAccess(IRInst* parameterType);
/// Return the defined direct callee parameter corresponding to `argumentUse`, if one exists.
static IRParam* findDirectCalleeParameter(IRCall* call, IRUse* argumentUse);
/// Return whether `parameterType` gives an address argument a direction contract.
static bool isDirectionalAddressParameterType(IRInst* parameterType);
/// Return how a call may access the resource address passed by `use`.
static ResourceAccess classifyCallArgumentAccess(
    IRCall* call,
    IRUse* use,
    CapabilitySet const& targetCaps);
/// Return how a runtime use reads or writes the resource reached through `use`.
static ResourceAccess classifyTerminalAddressUse(IRUse* use, CapabilitySet const& targetCaps);
/// Classify how storage access through the user's result applies to operand-zero storage.
static StorageAccessRelation classifyStorageAccessRelation(IRUse* use);
/// Return whether a use may retain an address or observe its storage identity.
static bool isAddressUseUnsupportedByPerFunctionStorage(
    IRUse* use,
    CapabilitySet const& targetCaps);
/// Return whether `user` is a decoration that causes the referenced function to be invoked.
static bool doesDecorationInvokeReferencedFunction(IRInst* user);
/// Return whether `func` has an invocation or value use that this pass cannot rewrite.
static bool hasFunctionUseThatCannotBeRewritten(IRFunc* func);
/// Return whether `func` may be invoked without a direct call that this pass can rewrite.
static bool mayBeInvokedWithoutRewritableCall(IRFunc* func);
/// Return whether `globalVar` must remain in module-scope storage.
static bool requiresModuleScopeStorage(IRGlobalVar* globalVar);
/// Return whether `inst` is nested inside a function block.
static bool isInstInsideFunctionBody(IRInst* inst);
/// Return whether `use` occurs in metadata attached to `globalVar`.
static bool isUseInsideAttachedMetadata(IRUse* use, IRGlobalVar* globalVar);
/// Return a use whose user is outside `globalVar` and whose operand belongs to metadata that will
/// be deleted with the global.
static IRUse* findUseOfAttachedMetadataOutsideGlobal(IRGlobalVar* globalVar);

/// A `LegalizeResourceGlobalVarsPass` implements the five-phase transformation described at the
/// top of this file.
///
/// The preceding initializer-movement pass has already extracted every selected initializer body.
/// Phases 2 and 3 reject unsupported IR before this pass changes any use or function signature.
/// Phase 5 performs any required uninitialized-read analysis in the rewritten IR because that
/// analysis needs the generated entry-point locals.
struct LegalizeResourceGlobalVarsPass
{
    /// The module whose selected globals and function signatures this pass rewrites.
    IRModule* module = nullptr;

    /// The target capabilities used to select applicable target-intrinsic implementations.
    CapabilitySet const& targetCaps;

    /// Whether phase 5 should diagnose reads before initialization in generated entry-point locals.
    bool shouldDiagnoseUninitializedValues = false;

    /// Every function in module preorder. A function's position is its stable index in this pass.
    List<FunctionContext> functions;

    /// The reverse lookup for indices in `functions`.
    Dictionary<IRFunc*, Index> functionIndices;

    /// Selected globals in module order, which also defines generated parameter order.
    List<ResourceGlobalToRewrite> resourceGlobals;

    /// Direct calls whose caller and callee are both indexed by `functions`.
    List<DirectCallEdge> directCallEdges;

    explicit LegalizeResourceGlobalVarsPass(
        IRModule* inModule,
        CapabilitySet const& inTargetCaps,
        bool inShouldDiagnoseUninitializedValues)
        : module(inModule)
        , targetCaps(inTargetCaps)
        , shouldDiagnoseUninitializedValues(inShouldDiagnoseUninitializedValues)
    {
    }

    /// Run the five phases described at the top of this file.
    void processModule(DiagnosticSink* sink)
    {
        // In phase 1, we assign deterministic indices to functions, record resource globals in the
        // order that will determine generated parameter order, and record direct calls between
        // those functions. Later phases use these records instead of hash-table or use-list order.
        collectFunctionContexts();
        collectResourceGlobals();
        collectDirectCalls();

        if (resourceGlobals.getCount() == 0)
            return;

        // In phase 2, we reject cases in which replacing the global with per-function storage
        // cannot preserve behavior. Linkage or retention requirements may require the variable to
        // remain in module-scope storage. An instruction outside a function body has no
        // function-local replacement address. We allow attached metadata only when nothing outside
        // the selected global uses any instruction in that metadata subtree, because phase 5
        // removes the complete subtree with the global.
        //
        // TODO: Move these target-independent storage checks to an IR validation pass after
        // linking. Front-end checking sees one module at a time and cannot know every reference.
        bool diagnosedUnsupportedProgram = diagnoseGlobalsRequiringModuleScopeStorage(sink);
        diagnosedUnsupportedProgram |= diagnoseResourceUsesOutsideFunctionBodies(sink);
        diagnosedUnsupportedProgram |= diagnoseUsesOfAttachedMetadataOutsideGlobal(sink);
        if (diagnosedUnsupportedProgram)
            return;

        // In phase 3, we determine whether each function reads or writes each resource value. These
        // results select value, `out`, or `inout` parameters in phase 4. For each existing terminal
        // operand use in an entry-point body, we also record its runtime effect and whether it has
        // a whole-value continuation guarantee. Phase 4 adds corresponding records for generated
        // resource-argument loads and writable generated call operands. Phase 5 uses the combined
        // records to decide where uninitialized state stops propagating. We first use the completed
        // access summaries to reject a function whose invocations cannot all receive the value, or
        // whose emitted implementation can differ from the IR body. We then reject arrays with
        // subobject writes and a possible read before a whole-array assignment, uses that may
        // retain an address or observe its storage identity, and calls that would receive two
        // writable aliases for the same original global.
        //
        // TODO: Move the target-independent invocation and generic-assembly checks to an IR
        // validation pass after linking. Front-end checking cannot know callers from other modules.
        // The target-intrinsic check requires the current target capabilities, so it must remain in
        // a target-specific stage unless the proposed post-link validator receives those
        // capabilities.
        analyzeResourceAccess();
        auto functionsAccessingResourceGlobals = computeFunctionsAccessingAnyResourceGlobal();
        diagnosedUnsupportedProgram =
            diagnoseFunctionsWithUnrewritableInvocations(sink, functionsAccessingResourceGlobals);
        diagnosedUnsupportedProgram |=
            diagnoseFunctionsWithAlternateImplementations(sink, functionsAccessingResourceGlobals);
        if (diagnosedUnsupportedProgram)
            return;
        if (diagnoseArrayInitializationRequiringSubobjectAnalysis(sink))
            return;
        recordEntryPointTerminalUseEffects();
        if (diagnoseUnsupportedAddressUses(sink))
            return;
        if (diagnoseConflictingCallAliases(sink))
            return;
        assertFunctionsThatNeedParametersHaveOnlyRewritableInvocations();

        // In phase 4, we create one local in every function whose execution may access the value,
        // plus any function body with a direct non-runtime reference. Each non-entry-point function
        // that may read or write the value also receives a generated parameter. We redirect the old
        // global uses and append arguments to each direct call.
        createReplacementLocalsAndParameters();
        replaceGlobalUses();
        rewriteCalls();

        // In phase 5, when uninitialized-value diagnostics are enabled, we run the control-flow
        // check on each generated entry-point replacement local whose original global still has a
        // possible write or carries the marker recorded by the pre-link module-wide check when it
        // found a possible write. The check reports both reads with no preceding possible write and
        // reads reached while uninitialized state can still propagate. Successful rewriting leaves
        // only the attached metadata uses that phase 2 proved safe to delete with the original
        // global.
        if (shouldDiagnoseUninitializedValues)
            diagnoseUninitializedEntryPointReads(sink);
        removeReplacedResourceGlobals();
    }

    // ## Phase 1: Functions, resource globals, and direct calls

    /// Collect the context that later phases need for every function in module preorder.
    void collectFunctionContexts()
    {
        // For each function, we record its stable index, entry-point status, insertion anchor, and
        // whether a non-entry-point invocation may lack a direct call that phase 4 can rewrite.
        // Most functions are direct children of the module, but an unspecialized generic contains
        // its function body inside an `IRGeneric`. We include those nested bodies so phase 3 can
        // reject an access that this pass cannot rewrite safely.
        collectFunctionContextsUnder(module->getModuleInst());
    }

    /// Collect context for every function contained by `parent` in module preorder.
    void collectFunctionContextsUnder(IRInst* parent)
    {
        // A function nested under an `IRGeneric` can be instantiated after this pass. Its eventual
        // calls do not exist yet, so phase 4 cannot add a resource argument to them. We record that
        // limitation on every nested function. Entry-point specialization has already placed every
        // entry point directly under the module. If a limited nested function accesses a selected
        // resource, phase 3 diagnoses it before changing any signature.
        for (auto inst : parent->getChildren())
        {
            auto func = as<IRFunc>(inst);
            if (!func)
            {
                collectFunctionContextsUnder(inst);
                continue;
            }

            FunctionContext context;
            context.func = func;
            context.isEntryPoint = isShaderOrCudaKernelEntryPoint(func);
            if (context.isEntryPoint)
                SLANG_RELEASE_ASSERT(func->getParent() == module->getModuleInst());
            if (!context.isEntryPoint)
            {
                if (func->getParent() != module->getModuleInst())
                    context.lacksRewritableCallForSomeInvocation = true;
                else if (mayBeInvokedWithoutRewritableCall(func))
                    context.lacksRewritableCallForSomeInvocation = true;
            }
            if (auto firstBlock = func->getFirstBlock())
                context.firstPreRewriteOrdinaryInst = firstBlock->getFirstOrdinaryInst();

            auto index = functions.getCount();
            functions.add(context);
            functionIndices.add(func, index);
        }
    }

    /// Collect each mutable resource global that is a candidate for per-invocation replacement.
    ///
    /// The initializer mover has already removed each selected initializer body. Keeping these
    /// records in module order makes the order of generated parameters deterministic.
    void collectResourceGlobals()
    {
        // `visitVarDecl` and `visitUniformParameterShadowVarDecl` attach
        // `IRFileOrNamespaceScopeMutableVarDecoration` to the two relevant source-language
        // declaration categories. The shared candidate predicate then rejects explicit rates and
        // value types that the pass cannot represent as one whole value in a local for each
        // affected function and, for a non-entry-point function with runtime access, in one
        // generated parameter.
        for (auto inst : module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar)
                continue;
            if (!isResourceGlobalCandidateForPerInvocationReplacement(globalVar))
                continue;

            // The shared accessor enforces the same concrete post-link pointer contract as the
            // candidate predicate, including pointer types wrapped in attributes.
            auto valueType = getGlobalVarValueType(globalVar);
            SLANG_RELEASE_ASSERT(!globalVar->getFirstBlock());

            ResourceGlobalToRewrite resourceGlobal;
            resourceGlobal.globalVar = globalVar;
            resourceGlobal.valueType = valueType;
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
                    if (!callee)
                        continue;
                    auto calleeIndex = functionIndices.tryGetValue(callee);
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
    /// Linkage and retention decorations can require the module-scope declaration to remain
    /// externally accessible or explicitly retained. Replacing such a declaration with separate
    /// function-local variables would not preserve that contract. Return whether any diagnostic
    /// was emitted.
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
            sink->diagnose(Diagnostics::MutableResourceRequiresModuleScopeStorage{
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
        // its body, even though `getParentFunc` reports that function as its owner. We allow a
        // reference in metadata attached to the selected global because phase 5 removes that
        // metadata with the global. Every other use outside a function body lacks a local to which
        // we can redirect it. For example, a runtime use can belong to another global's
        // initializer. A non-runtime query or metadata instruction may describe another object, so
        // removing the selected global would not preserve that reference.
        bool diagnosed = false;
        for (auto const& resourceGlobal : resourceGlobals)
        {
            for (auto use = resourceGlobal.globalVar->firstUse; use; use = use->nextUse)
            {
                auto user = use->getUser();
                if (isInstInsideFunctionBody(user))
                    continue;
                if (isUseInsideAttachedMetadata(use, resourceGlobal.globalVar))
                    continue;

                if (doesInstUseOperandAtRuntime(user))
                {
                    sink->diagnose(Diagnostics::MutableResourceUsedOutsideFunction{
                        .variable = resourceGlobal.globalVar,
                        .location = user->sourceLoc});
                }
                else
                {
                    sink->diagnose(Diagnostics::MutableResourceHasReferenceOutsideFunctionBody{
                        .variable = resourceGlobal.globalVar,
                        .location = user->sourceLoc});
                }
                diagnosed = true;
            }
        }
        return diagnosed;
    }

    /// Diagnose uses outside a selected global of metadata that phase 5 would remove with it, and
    /// return whether any diagnostic was emitted.
    bool diagnoseUsesOfAttachedMetadataOutsideGlobal(DiagnosticSink* sink)
    {
        // Removing an `IRGlobalVar` recursively removes its decorations and annotations. An
        // instruction in that metadata subtree need not refer back to the global, and an
        // instruction anywhere else in the module can refer to it. We reject such a use because
        // removing the metadata would leave the surviving instruction with a dangling operand.
        // This rule is independent of whether that instruction is inside a function.
        bool diagnosed = false;
        for (auto const& resourceGlobal : resourceGlobals)
        {
            auto externalUse = findUseOfAttachedMetadataOutsideGlobal(resourceGlobal.globalVar);
            if (!externalUse)
                continue;

            sink->diagnose(Diagnostics::MutableResourceAttachedMetadataHasExternalReference{
                .variable = resourceGlobal.globalVar,
                .location = externalUse->getUser()->sourceLoc});
            diagnosed = true;
        }
        return diagnosed;
    }

    // ## Phase 3: Resource reads, writes, and address validation

    /// Analyze how every function reads and writes every resource global.
    ///
    /// The resulting facts determine the generated parameter type for each function and identify
    /// reads that require the value supplied by the caller.
    void analyzeResourceAccess()
    {
        // We analyze one global at a time. We first record its direct uses and follow every
        // supported storage-access transfer. We then propagate callee effects to callers. Finally,
        // we prove which writers assign the whole value on every reachable normal return and which
        // functions may read the value present on entry.
        for (auto& resourceGlobal : resourceGlobals)
        {
            collectRootUses(resourceGlobal);

            HashSet<IRInst*> visitedAddresses;
            StorageAccessPathState rootPath;
            rootPath.coversWholeValue = true;
            rootPath.transferredAccess = mergeAccess(ResourceAccess::Read, ResourceAccess::Write);
            analyzeStorageAccesses(
                resourceGlobal,
                resourceGlobal.globalVar,
                visitedAddresses,
                rootPath);

            propagateEffectsToCallers(resourceGlobal);
            determineWholeValueAssignmentsOnEveryReturn(resourceGlobal);
            determineEntryValueRequirements(resourceGlobal);
        }
    }

    /// Return which functions may read or write at least one selected resource global.
    List<bool> computeFunctionsAccessingAnyResourceGlobal()
    {
        // `analyzeResourceAccess` has already followed every supported storage transfer and
        // propagated callee effects to callers. We combine those per-global summaries so that the
        // invocation checks use the same definition of runtime access as parameter generation.
        List<bool> accessesResource;
        for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
        {
            bool functionAccessesResource = false;
            for (auto const& resourceGlobal : resourceGlobals)
            {
                if (resourceGlobal.perFunctionInfo[functionIndex].access != ResourceAccess::None)
                {
                    functionAccessesResource = true;
                    break;
                }
            }
            accessesResource.add(functionAccessesResource);
        }
        return accessesResource;
    }

    /// Diagnose functions whose execution may access a selected global but whose invocation cannot
    /// be rewritten.
    ///
    /// The pipeline has already redirected every ordinary call to a shader entry point or CUDA
    /// kernel to a non-entry-point clone. Explicit launch instructions may still refer to the
    /// original entry point, because phase 4 inserts one replacement-local instruction into that
    /// function's body. Each launch executes the body with a distinct dynamic instance of the
    /// local. An entry point fails this check only when an invocation decoration or function-value
    /// use can cause an additional invocation without a direct call whose arguments can be
    /// extended.
    bool diagnoseFunctionsWithUnrewritableInvocations(
        DiagnosticSink* sink,
        List<bool> const& accessesResource)
    {
        // Phase 4 can extend a direct call inside a function because the caller has replacement
        // storage whose value it can pass. A call outside a function body has no caller-local
        // value. An external invocation or an invocation decoration such as
        // `IRPatchConstantFuncDecoration` has no direct `IRCall` argument list to extend.
        //
        // `fixEntryPointCallsites` is required to redirect every ordinary call to a shader entry
        // point or CUDA kernel before this pass runs. A remaining direct-call edge to an entry
        // point is therefore a compiler pipeline defect, not an unsupported source program.
        bool diagnosed = false;
        for (auto const& edge : directCallEdges)
            SLANG_RELEASE_ASSERT(!functions[edge.calleeIndex].isEntryPoint);

        for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
        {
            if (!accessesResource[functionIndex])
                continue;

            auto const& function = functions[functionIndex];
            if (function.isEntryPoint)
            {
                if (hasFunctionUseThatCannotBeRewritten(function.func))
                {
                    sink->diagnose(
                        Diagnostics::MutableResourceUsedByEntryPointWithUnrewritableInvocation{
                            .function = function.func,
                            .location = function.func->sourceLoc});
                    diagnosed = true;
                }
            }
            else if (function.lacksRewritableCallForSomeInvocation)
            {
                sink->diagnose(Diagnostics::MutableResourceUsedByFunctionWithoutRewritableCall{
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
        // Phase 3 analyzes the function's IR blocks, and phase 4 inserts any required copy-in and
        // copy-out operations into those blocks. Generic assembly or an applicable target intrinsic
        // replaces the blocks during emission. The emitted implementation would therefore omit the
        // operations introduced by this pass, so we reject the function before changing its
        // signature.
        bool diagnosed = false;
        for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
        {
            if (!accessesResource[functionIndex])
                continue;

            auto func = functions[functionIndex].func;

            if (findBestTargetIntrinsicDecoration(func, targetCaps))
            {
                sink->diagnose(Diagnostics::MutableResourceUsedByTargetIntrinsicFunction{
                    .function = func,
                    .location = func->sourceLoc});
                diagnosed = true;
            }
            else if (hasGenericAssemblyImplementation(func))
            {
                sink->diagnose(Diagnostics::MutableResourceUsedByGenericAssemblyFunction{
                    .function = func,
                    .location = func->sourceLoc});
                diagnosed = true;
            }
        }
        return diagnosed;
    }

    /// Save every direct use of the global before phase 4 mutates any use list.
    ///
    /// Function-body uses establish where replacement storage is required. After phase 2, every
    /// remaining use outside a function body belongs to attached metadata that phase 2 proved safe
    /// to remove with the global.
    void collectRootUses(ResourceGlobalToRewrite& global)
    {
        // We record function membership now because phase 4 will replace the use-list links. Phase
        // 2 has already established that a use outside a function body belongs to a discardable
        // metadata subtree attached to the global.
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
    /// storage identity. We also distinguish a whole-value assignment from a write to only part of
    /// the value.
    void analyzeTerminalAddressUse(
        ResourceGlobalToRewrite& global,
        IRUse* use,
        StorageAccessPathState const& path)
    {
        // We first identify the containing function and classify the effects transferred through
        // this storage-access path. When the use may escape the address or is a call argument, we
        // retain it for the corresponding validation. We then update the function summary and
        // record whether the use assigns the whole value.
        auto user = use->getUser();
        auto parentFunc = getParentFunc(user);
        SLANG_RELEASE_ASSERT(parentFunc);
        auto functionIndex = functionIndices.tryGetValue(parentFunc);
        SLANG_RELEASE_ASSERT(functionIndex);

        auto terminalAccess =
            intersectAccess(classifyTerminalAddressUse(use, targetCaps), path.transferredAccess);
        if (terminalAccess == ResourceAccess::None)
            return;

        if (as<IRCall>(user))
            global.addressPassingUses.add(use);
        if (isAddressUseUnsupportedByPerFunctionStorage(use, targetCaps))
            global.unsupportedAddressUses.add(use);

        auto& functionInfo = global.perFunctionInfo[*functionIndex];
        functionInfo.access = mergeAccess(functionInfo.access, terminalAccess);
        if (path.selectsSubobject)
        {
            if (hasAccessDirection(terminalAccess, ResourceAccess::Write))
                functionInfo.mayWriteSubobject = true;
        }

        // A store through an access path that covers the whole value assigns that value. Passing
        // the same address to an `out` parameter also establishes a whole-value assignment when the
        // call returns normally, because that is the parameter's language-level contract.
        bool assignsWholeValue = false;
        if (path.coversWholeValue)
        {
            if (auto store = as<IRStore>(user))
                assignsWholeValue = store->ptr.get() == use->get();
            else if (auto call = as<IRCall>(user))
            {
                // The `out` contract and the callee's actual reads are independent facts. A callee
                // may read through a cast before it assigns the parameter, but a normal return
                // still guarantees that it assigned the whole value.
                assignsWholeValue = as<IROutParamType>(findCallArgumentParameterType(call, use));
            }
        }
        global.terminalAddressUses.add(
            TerminalAddressUse{use, *functionIndex, terminalAccess, assignsWholeValue});
    }

    /// Follow operations that may transfer storage access from one resource global, and classify
    /// the runtime uses reached through them.
    ///
    /// For each use, we either follow a result that may transfer a later access to or from the
    /// source, or hand the terminal use to `analyzeTerminalAddressUse`. We carry three facts
    /// through the recursion.
    /// Whether the access still covers the whole original value determines whether a terminal
    /// write is a whole-value assignment. Whether an operation explicitly selected a subobject
    /// determines whether an array may require subobject-sensitive initialization analysis. We
    /// also carry the read and write directions that the path can transfer. In particular, an
    /// `OutImplicitCast` can transfer writes from its temporary back to the source, but it cannot
    /// transfer a read of that temporary to the source.
    void analyzeStorageAccesses(
        ResourceGlobalToRewrite& global,
        IRInst* address,
        HashSet<IRInst*>& visitedAddresses,
        StorageAccessPathState const& path)
    {
        // We follow only operand zero of an operation that may transfer storage access, so each
        // resulting address has one traversable predecessor. Its path state is therefore unique,
        // and the visited set needs to identify only the address instruction.
        if (!visitedAddresses.add(address))
            return;

        for (auto use = address->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (!doesInstUseOperandAtRuntime(user))
                continue;

            if (doesUserResultTransferStorageAccessFromUse(use))
            {
                auto relation = classifyStorageAccessRelation(use);
                if (relation == StorageAccessRelation::Unsupported)
                {
                    global.unsupportedAddressUses.add(use);
                    continue;
                }

                StorageAccessPathState resultPath = path;
                resultPath.transferredAccess =
                    intersectAccess(path.transferredAccess, getTransferredStorageAccess(use));
                if (resultPath.transferredAccess == ResourceAccess::None)
                    continue;

                resultPath.coversWholeValue = false;
                if (path.coversWholeValue)
                    resultPath.coversWholeValue = relation == StorageAccessRelation::WholeValue;
                if (relation == StorageAccessRelation::Subobject)
                    resultPath.selectsSubobject = true;

                analyzeStorageAccesses(global, user, visitedAddresses, resultPath);
                continue;
            }
            analyzeTerminalAddressUse(global, use, path);
        }
    }

    /// Propagate possible read, write, and subobject-write effects from callees to callers until no
    /// call edge changes its caller's summary.
    void propagateEffectsToCallers(ResourceGlobalToRewrite& global)
    {
        // Each fact can only change from absent to present. Repeating the call edges therefore
        // handles recursive components as well as ordinary call chains and must terminate.
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
                if (mergedAccess != callerInfo.access)
                {
                    callerInfo.access = mergedAccess;
                    changed = true;
                }
                if (calleeMayWriteSubobject)
                {
                    if (!callerInfo.mayWriteSubobject)
                    {
                        callerInfo.mayWriteSubobject = true;
                        changed = true;
                    }
                }
            }
        } while (changed);
    }

    /// Diagnose every resource array whose initialization may require subobject-sensitive analysis,
    /// and return whether any diagnostic was emitted.
    bool diagnoseArrayInitializationRequiringSubobjectAnalysis(DiagnosticSink* sink)
    {
        // The phase-3 control-flow proof recognizes a whole-array assignment as one instruction.
        // Proving that writes to selected subobjects initialize the whole array would also
        // require tracking which parts are written on each path. An array that has both a subobject
        // write and a read before a whole-array assignment may depend on that unsupported proof, so
        // we reject it. The invocation checks earlier in phase 3 rejected every non-entry-point
        // function whose execution may access the selected global and can run without a recorded
        // call. The preceding fixed points propagated both facts through those calls. We can
        // therefore diagnose the unsupported array from each entry point's summary.
        bool diagnosed = false;
        for (auto const& global : resourceGlobals)
        {
            if (!as<IRArrayTypeBase>(unwrapAttributedType(global.valueType)))
                continue;

            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                auto const& functionInfo = global.perFunctionInfo[functionIndex];
                if (!functions[functionIndex].isEntryPoint)
                    continue;
                if (!functionInfo.mayWriteSubobject)
                    continue;
                if (!functionInfo.mayReadValueBeforeWholeValueAssignment)
                    continue;

                sink->diagnose(Diagnostics::MutableResourceArrayCompleteInitializationNotProven{
                    .variable = global.globalVar,
                    .location = global.globalVar->sourceLoc});
                diagnosed = true;
                break;
            }
        }
        return diagnosed;
    }

    /// Determine which writers assign the whole resource value before every reachable normal
    /// return.
    ///
    /// An argument that denotes the whole value and is passed to an explicit `out` parameter
    /// counts as an assignment by the parameter's contract. A call to a function that may write the
    /// global establishes the value for its caller after we prove that every reachable normal
    /// return from the callee follows a whole-value assignment. A writer with no reachable normal
    /// return also satisfies that condition because execution cannot continue after its call.
    /// Repeating the proof propagates this guarantee through the call graph.
    void determineWholeValueAssignmentsOnEveryReturn(ResourceGlobalToRewrite& global)
    {
        // The property is conditional: if a function returns normally, every returning path must
        // first assign the value. We begin by treating every writer as a candidate. We then remove
        // a candidate when a reachable return path reaches neither a direct whole-value assignment
        // nor a call to a function that remains a candidate. The iteration computes a greatest
        // fixed point: each flag can change only from true to false. Starting with every candidate
        // lets recursive functions support one another, while iteration still removes a cycle that
        // can return without assigning the value.
        for (auto& resourceInfo : global.perFunctionInfo)
        {
            resourceInfo.assignsWholeValueOnEveryReturn =
                hasAccessDirection(resourceInfo.access, ResourceAccess::Write);
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

    /// Return whether every reachable normal return follows a whole-value assignment.
    ///
    /// A writer with no reachable normal return also satisfies this condition because no execution
    /// continues past the function with a value to copy out.
    bool doesFunctionAssignWholeValueOnEveryReturn(
        ResourceGlobalToRewrite& global,
        Index functionIndex)
    {
        // We collect each instruction after which every continuing execution has a whole value. A
        // reachable return disproves the contract if execution can reach it without first reaching
        // one of those instructions. When there is no reachable return, there is no value to copy
        // out.
        HashSet<IRInst*> wholeValueContinuationGuarantees;
        collectWholeValueContinuationGuarantees(
            global,
            functionIndex,
            wholeValueContinuationGuarantees);

        auto dominatorTree = getDominatorTree(functionIndex);
        for (auto block : functions[functionIndex].func->getBlocks())
        {
            if (dominatorTree->isUnreachable(block))
                continue;

            auto returnInst = as<IRReturn>(block->getTerminator());
            if (!returnInst)
                continue;

            if (canReachInstructionWithoutWholeValueContinuationGuarantee(
                    functionIndex,
                    returnInst,
                    wholeValueContinuationGuarantees))
                return false;
        }

        return true;
    }

    /// Collect instructions after which every continuing execution has a whole resource value.
    ///
    /// A direct whole-value store establishes the value immediately. A normally returning call
    /// establishes it at completion when the whole-value address is passed to an explicit `out`
    /// parameter. A call to a function that may write the global establishes a whole value when
    /// every reachable normal return from the callee follows a whole-value assignment. A callee
    /// that may write but cannot return normally also satisfies that condition because execution
    /// never continues past the call.
    void collectWholeValueContinuationGuarantees(
        ResourceGlobalToRewrite& global,
        Index functionIndex,
        HashSet<IRInst*>& continuationGuarantees)
    {
        // Terminal uses contain direct stores and calls that pass an address known to denote the
        // whole value to an explicit `out` parameter. We then add calls after which every
        // continuing path has a whole value.
        for (auto const& terminalUse : global.terminalAddressUses)
        {
            if (terminalUse.functionIndex != functionIndex)
                continue;
            if (terminalUse.assignsWholeValue)
                continuationGuarantees.add(terminalUse.use->getUser());
        }

        for (auto const& edge : directCallEdges)
        {
            if (edge.callerIndex != functionIndex)
                continue;
            if (global.perFunctionInfo[edge.calleeIndex].assignsWholeValueOnEveryReturn)
                continuationGuarantees.add(edge.call);
        }
    }

    /// Return the dominator tree used to exclude unreachable returns from all-path proofs.
    IRDominatorTree* getDominatorTree(Index functionIndex)
    {
        // We ask the same CFG questions for every resource value. We compute the tree lazily on the
        // first query and retain it in the function record so later resources reuse the analysis.
        auto& function = functions[functionIndex];
        if (!function.dominatorTree)
            function.dominatorTree = computeDominatorTree(function.func);
        return function.dominatorTree;
    }

    /// Return whether control can reach `target` without a prior whole-value continuation
    /// guarantee.
    ///
    /// After an instruction with that guarantee, every continuing execution has a whole value.
    /// Reaching `target` without crossing such an instruction proves that some part of the entry
    /// value can reach it.
    bool canReachInstructionWithoutWholeValueContinuationGuarantee(
        Index functionIndex,
        IRInst* target,
        HashSet<IRInst*> const& continuationGuarantees)
    {
        // We search forward from the entry block and stop each path at its first instruction with a
        // whole-value continuation guarantee. Each block needs at most one visit because every path
        // in the worklist still carries some part of the entry value.
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

            bool pathReachedWholeValueGuarantee = false;
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
            {
                // A call can read the entry value before its normally returning executions
                // establish a whole value. We therefore test the target before stopping at that
                // call.
                if (inst == target)
                    return true;
                if (continuationGuarantees.contains(inst))
                {
                    pathReachedWholeValueGuarantee = true;
                    break;
                }
            }
            if (pathReachedWholeValueGuarantee)
                continue;

            for (auto successor : block->getSuccessors())
            {
                if (visited.add(successor))
                    workList.add(successor);
            }
        }

        return false;
    }

    /// Determine which functions may read the resource value present on entry.
    ///
    /// We classify direct reads by whether every path to them crosses an instruction with a whole-
    /// value continuation guarantee. We then propagate callee read requirements to callers,
    /// stopping when such a guarantee makes the entry value unavailable. Whether a writer may
    /// preserve part of the entry value is derived separately from its whole-value-on-return proof.
    void determineEntryValueRequirements(ResourceGlobalToRewrite& global)
    {
        // The fixed point in `determineWholeValueAssignmentsOnEveryReturn` has finalized every
        // `assignsWholeValueOnEveryReturn` fact, so a function's whole-value continuation
        // guarantees remain stable during this analysis. The direct-read scan and the caller-
        // propagation loop can both query the same function, so we collect each set on its first
        // query and reuse it for later queries. Phase 4 rebuilds and removes calls stored in these
        // sets, so we keep the raw instruction pointers local to this phase-3 routine.
        List<HashSet<IRInst*>> wholeValueContinuationGuaranteesByFunction;
        wholeValueContinuationGuaranteesByFunction.setCount(functions.getCount());
        auto wholeValueContinuationGuaranteesHaveBeenCollected =
            List<bool>::makeRepeated(false, functions.getCount());
        auto getWholeValueContinuationGuarantees =
            [&](Index functionIndex) -> HashSet<IRInst*> const&
        {
            if (!wholeValueContinuationGuaranteesHaveBeenCollected[functionIndex])
            {
                collectWholeValueContinuationGuarantees(
                    global,
                    functionIndex,
                    wholeValueContinuationGuaranteesByFunction[functionIndex]);
                wholeValueContinuationGuaranteesHaveBeenCollected[functionIndex] = true;
            }
            return wholeValueContinuationGuaranteesByFunction[functionIndex];
        };

        // We first find direct reads reachable without a prior whole-value continuation guarantee.
        for (auto const& terminalUse : global.terminalAddressUses)
        {
            if (!hasAccessDirection(terminalUse.access, ResourceAccess::Read))
                continue;
            auto const& wholeValueContinuationGuarantees =
                getWholeValueContinuationGuarantees(terminalUse.functionIndex);
            if (canInstructionExecuteWhileEntryValueRemains(
                    terminalUse.functionIndex,
                    terminalUse.use->getUser(),
                    wholeValueContinuationGuarantees))
            {
                global.perFunctionInfo[terminalUse.functionIndex]
                    .mayReadValueBeforeWholeValueAssignment = true;
            }
        }

        // We then propagate read requirements from callees to callers. This is a least fixed point:
        // each requirement can change only from false to true.
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
                if (caller.mayReadValueBeforeWholeValueAssignment)
                    continue;
                auto const& wholeValueContinuationGuarantees =
                    getWholeValueContinuationGuarantees(edge.callerIndex);
                if (!canInstructionExecuteWhileEntryValueRemains(
                        edge.callerIndex,
                        edge.call,
                        wholeValueContinuationGuarantees))
                    continue;
                caller.mayReadValueBeforeWholeValueAssignment = true;
                changed = true;
            }
        } while (changed);
    }

    /// Return whether `instruction` can execute while some part of the entry value remains.
    ///
    /// An unreachable instruction cannot execute. A reachable instruction can execute while some
    /// part of the entry value remains when an entry-to-instruction path crosses no instruction
    /// with a whole-value continuation guarantee. `wholeValueContinuationGuarantees` must contain
    /// every such instruction for the current resource global in the function identified by
    /// `functionIndex`.
    bool canInstructionExecuteWhileEntryValueRemains(
        Index functionIndex,
        IRInst* instruction,
        HashSet<IRInst*> const& wholeValueContinuationGuarantees)
    {
        // The dominator tree identifies blocks that the entry cannot reach. When the function has
        // no whole-value continuation guarantee, every reachable instruction can execute while
        // some part of the entry value remains. Otherwise, the forward search answers whether at
        // least one path avoids every such guarantee.
        auto instructionBlock = getBlock(instruction);
        SLANG_RELEASE_ASSERT(instructionBlock);
        if (getDominatorTree(functionIndex)->isUnreachable(instructionBlock))
            return false;

        if (wholeValueContinuationGuarantees.getCount() == 0)
            return true;
        return canReachInstructionWithoutWholeValueContinuationGuarantee(
            functionIndex,
            instruction,
            wholeValueContinuationGuarantees);
    }

    /// Record every terminal runtime effect on an entry-point replacement local.
    ///
    /// Phase 3 has already followed supported storage transfers and classified each terminal use.
    /// We retain that complete list so the later checker needs only control-flow analysis; it does
    /// not repeat the address or interprocedural analysis after rewriting.
    void recordEntryPointTerminalUseEffects()
    {
        // Uninitialized state originates in entry-point replacement locals. Phase 3 summarizes a
        // helper's behavior, and phase 4 records that summary on each rewritten entry-point call.
        // We therefore retain direct terminal effects only for entry points. We associate each
        // effect with its exact address operand because a call can pass one value to several
        // parameters with different direction contracts.
        for (auto& global : resourceGlobals)
        {
            for (auto const& terminalUse : global.terminalAddressUses)
            {
                if (!functions[terminalUse.functionIndex].isEntryPoint)
                    continue;
                auto& resourceInfo = global.perFunctionInfo[terminalUse.functionIndex];

                // A direct store or a whole-address `out` argument establishes that every
                // continuation from this use has a fully assigned value. We record that guarantee
                // so the generated-local check stops propagating uninitialized state at this use.
                resourceInfo.entryPointLocalUseEffects.add(GeneratedResourceLocalUseEffect{
                    .use = terminalUse.use,
                    .readsValue = hasAccessDirection(terminalUse.access, ResourceAccess::Read),
                    .mayWriteValue = hasAccessDirection(terminalUse.access, ResourceAccess::Write),
                    .stopsUninitializedStatePropagation = terminalUse.assignsWholeValue,
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
                sink->diagnose(Diagnostics::MutableResourceAddressHasUnsupportedUse{
                    .variable = resourceGlobal.globalVar,
                    .location = use->getUser()->sourceLoc});
                diagnosed = true;
            }
        }
        return diagnosed;
    }

    /// Diagnose calls in which the selected global aliases two callee access paths and either path
    /// may write.
    ///
    /// In the original call, the callee's implicit global accesses and its explicit parameter both
    /// refer to the selected global. After rewriting, the former implicit accesses use the callee's
    /// replacement local, while the explicit parameter still accesses the caller-provided argument.
    /// Separating the two paths preserves behavior only when both are read-only. Return whether any
    /// diagnostic was emitted.
    bool diagnoseConflictingCallAliases(DiagnosticSink* sink)
    {
        // We visit each existing argument that passes the selected global's address. When the
        // callee also accesses that global implicitly, we compare the two paths and report the call
        // once unless both paths are read-only.
        bool diagnosed = false;
        for (auto const& resourceGlobal : resourceGlobals)
        {
            HashSet<IRCall*> diagnosedCalls;
            for (auto use : resourceGlobal.addressPassingUses)
            {
                auto call = cast<IRCall>(use->getUser());
                auto callee = as<IRFunc>(call->getCallee());
                if (!callee)
                    continue;
                auto calleeIndex = functionIndices.tryGetValue(callee);
                if (!calleeIndex)
                    continue;

                auto implicitAccess = resourceGlobal.perFunctionInfo[*calleeIndex].access;
                if (implicitAccess == ResourceAccess::None)
                    continue;

                auto explicitAccess = classifyCallArgumentAccess(call, use, targetCaps);
                bool bothPathsAreReadOnly = implicitAccess == ResourceAccess::Read;
                if (explicitAccess != ResourceAccess::Read)
                    bothPathsAreReadOnly = false;
                if (bothPathsAreReadOnly)
                {
                    continue;
                }
                if (!diagnosedCalls.add(call))
                    continue;

                sink->diagnose(Diagnostics::MutableResourceHasConflictingCallAliases{
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
        // Phase 3 has already diagnosed functions with an invocation that phase 4 cannot rewrite
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
                SLANG_RELEASE_ASSERT(call);
                SLANG_RELEASE_ASSERT(call->getCalleeUse() == use);
                SLANG_RELEASE_ASSERT(getParentFunc(call));
            }
        }
    }

    // ## Phase 4: Locals, parameters, and call arguments

    /// Create a local for each function whose execution may access the value, plus each function
    /// body with a direct non-runtime reference. Create a parameter for each affected
    /// non-entry-point function with runtime access.
    void createReplacementLocalsAndParameters()
    {
        // Each entry point that may read or write the value creates a fresh local. Each
        // non-entry-point function whose execution may access the value receives a parameter and
        // operates on a replacement local. A function with a direct non-runtime reference and no
        // direct or transitive runtime access needs a local so that phase 4 can redirect that
        // reference, but it needs no runtime parameter. After all parameters exist, we rebuild each
        // changed function type and its debug type once.
        IRBuilder builder(module);

        for (auto& global : resourceGlobals)
        {
            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                auto const& function = functions[functionIndex];
                auto& resourceInfo = global.perFunctionInfo[functionIndex];
                bool hasRuntimeAccess = resourceInfo.access != ResourceAccess::None;
                if (!hasRuntimeAccess)
                {
                    if (!resourceInfo.hasDirectGlobalUse)
                        continue;
                }

                if (function.isEntryPoint)
                    createReplacementLocal(builder, functionIndex, global, resourceInfo);
                else if (!hasRuntimeAccess)
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
        // A read-only function receives the resource by value. Any writer that needs the entry
        // value requires `inout`. A writer that can return without a whole-value assignment also
        // requires `inout`, because copy-out may preserve part of the caller's value. Every other
        // writer can use `out`.
        bool writesValue = hasAccessDirection(resourceInfo.access, ResourceAccess::Write);
        if (!writesValue)
            return global.valueType;
        if (resourceInfo.mustInitializeLocalFromGeneratedParameter())
            return builder.getBorrowInOutParamType(global.valueType);
        return builder.getOutParamType(global.valueType);
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

    /// Initialize the local when the function may read the entry value or return without a
    /// whole-value assignment.
    void initializeLocalFromGeneratedParameterIfNeeded(
        IRBuilder& builder,
        ResourceGlobalToRewrite const& global,
        FunctionResourceInfo& resourceInfo)
    {
        // A read-only parameter is already a value. An `inout` parameter is an address, so we load
        // it before storing into the local. A function with an `out` parameter deliberately starts
        // with an uninitialized local.
        bool writesValue = hasAccessDirection(resourceInfo.access, ResourceAccess::Write);
        if (writesValue)
        {
            if (!resourceInfo.mustInitializeLocalFromGeneratedParameter())
                return;
        }

        IRInst* inputValue = resourceInfo.parameter;
        if (writesValue)
            inputValue = builder.emitLoad(global.valueType, resourceInfo.parameter);
        builder.emitStore(resourceInfo.replacementAddress, inputValue);
    }

    /// Copy a writer's local value to its generated parameter at every normal return.
    void copyLocalToGeneratedParameterAtReturnsIfNeeded(
        IRBuilder& builder,
        FunctionContext const& function,
        ResourceGlobalToRewrite const& global,
        FunctionResourceInfo& resourceInfo)
    {
        // These stores implement the caller-visible `out` or `inout` update. On targets that
        // require resource-output specialization, `specializeResourceOutputs` recognizes and
        // rewrites this copy-out pattern. A read-only function needs no copy-out.
        if (!hasAccessDirection(resourceInfo.access, ResourceAccess::Write))
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

    /// Add a generated resource parameter and replacement local to one non-entry-point function.
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
        initializeLocalFromGeneratedParameterIfNeeded(builder, global, resourceInfo);
        copyLocalToGeneratedParameterAtReturnsIfNeeded(builder, function, global, resourceInfo);
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
        // Phase 2 rejected every use outside a function body except metadata that can be discarded
        // with the selected global. Every function-body use can be redirected to the local chosen
        // for that function.
        for (auto& global : resourceGlobals)
        {
            for (auto const& rootUse : global.rootUses)
            {
                if (rootUse.functionIndex < 0)
                {
                    SLANG_RELEASE_ASSERT(
                        isUseInsideAttachedMetadata(rootUse.use, global.globalVar));
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

            if (functions[edge.callerIndex].isEntryPoint)
            {
                remapRecordedEntryPointEffectsForRebuiltCall(oldCall, newCall, edge.callerIndex);
                recordEntryPointWritableArgumentEffects(oldCall, newCall, edge);
            }

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
        // A read-only function receives a loaded value. For an entry-point caller, we record that
        // new load because phase 3 ran before phase 4 created it. A helper caller needs no record:
        // phase 3 computes the helper's transitive effects, and phase 4 records that summary on the
        // rewritten entry-point call. A writer receives the caller's local address for its
        // generated `out` or `inout` parameter; `recordEntryPointWritableArgumentEffects` records
        // that use after the new call exists.
        for (auto& global : resourceGlobals)
        {
            auto const& calleeInfo = global.perFunctionInfo[edge.calleeIndex];
            if (!calleeInfo.parameter)
                continue;

            auto& callerInfo = global.perFunctionInfo[edge.callerIndex];
            SLANG_RELEASE_ASSERT(callerInfo.replacementAddress);
            if (calleeInfo.access == ResourceAccess::Read)
            {
                auto argument = builder.emitLoad(global.valueType, callerInfo.replacementAddress);
                argument->sourceLoc = edge.call->sourceLoc;
                arguments.add(argument);
                if (functions[edge.callerIndex].isEntryPoint)
                {
                    callerInfo.entryPointLocalUseEffects.add(GeneratedResourceLocalUseEffect{
                        .use = argument->getOperandUse(0),
                        .readsValue = true,
                    });
                }
            }
            else
                arguments.add(callerInfo.replacementAddress);
        }
    }

    /// Retarget saved entry-point effects when phase 4 replaces a call instruction.
    void remapRecordedEntryPointEffectsForRebuiltCall(
        IRCall* oldCall,
        IRCall* newCall,
        Index callerIndex)
    {
        // Effects belong to exact `IRUse` objects. We match pre-existing arguments by index because
        // the rebuilt call preserves their order.
        for (auto& global : resourceGlobals)
        {
            auto& effects = global.perFunctionInfo[callerIndex].entryPointLocalUseEffects;
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

    /// Record each generated `out` or `inout` argument's effect on an entry-point caller's local.
    void recordEntryPointWritableArgumentEffects(
        IRCall* oldCall,
        IRCall* newCall,
        DirectCallEdge const& edge)
    {
        // The generated parameter direction cannot express all of phase 3's facts. An `inout`
        // parameter carries the caller's value into the callee when a reachable normal return may
        // occur without a whole-value assignment, even when the callee never reads the incoming
        // value.
        //
        // Conversely, the caller's local is known to be fully assigned after a normally returning
        // call only when every reachable normal return from the callee follows a whole-value
        // assignment. A writer that cannot return normally satisfies the same continuation
        // guarantee because execution never continues past the call. We record the callee's entry-
        // value read requirement and whole-value continuation guarantee on the exact generated
        // argument use.
        //
        // Copying the input into a generated local does not itself represent a read written in the
        // program. If a function returns without a whole-value assignment, copy-out may preserve an
        // uninitialized value. That is not an error unless the caller later reads the value.
        UInt resourceArgIndex = oldCall->getArgCount();
        for (auto& global : resourceGlobals)
        {
            auto const& calleeInfo = global.perFunctionInfo[edge.calleeIndex];
            if (!calleeInfo.parameter)
                continue;

            auto& callerInfo = global.perFunctionInfo[edge.callerIndex];
            if (hasAccessDirection(calleeInfo.access, ResourceAccess::Write))
            {
                callerInfo.entryPointLocalUseEffects.add(GeneratedResourceLocalUseEffect{
                    .use = newCall->getOperandUse(resourceArgIndex + 1),
                    .readsValue = calleeInfo.mayReadValueBeforeWholeValueAssignment,
                    .mayWriteValue = true,
                    .stopsUninitializedStatePropagation = calleeInfo.assignsWholeValueOnEveryReturn,
                });
            }
            resourceArgIndex++;
        }
    }

    // ## Phase 5: Generated-local diagnostics and obsolete-storage removal

    /// Diagnose entry-point paths that read a generated local before a whole-value assignment.
    void diagnoseUninitializedEntryPointReads(DiagnosticSink* sink)
    {
        // `checkForUsingUninitializedValues` ran before this pass created these locals. We
        // therefore apply its control-flow analysis to each generated entry-point local and supply
        // the exhaustive runtime effects recorded during analysis and call rewriting.
        //
        // The pre-link module-wide uninitialized-global check emitted a diagnostic when the
        // original module had no possible write. When it found a possible write instead, it marked
        // the global to record that the check was inconclusive. Entry-point linking may remove that
        // writer. We therefore analyze each entry point when either the linked module still has a
        // possible write or the marker is present. Otherwise the earlier diagnostic already covers
        // every read, and a second check would duplicate it.
        for (auto const& global : resourceGlobals)
        {
            bool hasPossibleWrite = false;
            for (auto const& functionInfo : global.perFunctionInfo)
            {
                if (hasAccessDirection(functionInfo.access, ResourceAccess::Write))
                {
                    hasPossibleWrite = true;
                    break;
                }
            }
            auto inconclusiveMarker =
                global.globalVar
                    ->findDecoration<IRUninitializedGlobalCheckFoundPossibleWriteDecoration>();
            bool preLinkUninitializedGlobalCheckWasInconclusive = inconclusiveMarker != nullptr;
            if (!hasPossibleWrite && !preLinkUninitializedGlobalCheckWasInconclusive)
                continue;

            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                if (!functions[functionIndex].isEntryPoint)
                    continue;
                auto replacement = global.perFunctionInfo[functionIndex].replacementAddress;
                if (!replacement)
                    continue;
                checkForUsingUninitializedGeneratedResourceLocal(
                    functions[functionIndex].func,
                    replacement,
                    global.perFunctionInfo[functionIndex].entryPointLocalUseEffects.getArrayView(),
                    sink);
            }
        }
    }

    /// Remove each obsolete global and its discardable metadata after redirecting function-body
    /// uses.
    void removeReplacedResourceGlobals()
    {
        // Phase 4 has redirected every function-body use. Phase 2 proved that every remaining use
        // belongs to attached metadata. It also proved that no instruction outside the global uses
        // anything in that metadata subtree. Deleting the global can therefore delete the subtree
        // recursively without leaving a dangling reference.
        for (auto& global : resourceGlobals)
        {
            for (auto use = global.globalVar->firstUse; use; use = use->nextUse)
                SLANG_RELEASE_ASSERT(isUseInsideAttachedMetadata(use, global.globalVar));
            SLANG_RELEASE_ASSERT(!findUseOfAttachedMetadataOutsideGlobal(global.globalVar));
            global.globalVar->removeAndDeallocate();
        }
    }
};

// ## IR-use classification rules

static ResourceAccess mergeAccess(ResourceAccess left, ResourceAccess right)
{
    return ResourceAccess(UInt(left) | UInt(right));
}

static ResourceAccess intersectAccess(ResourceAccess left, ResourceAccess right)
{
    return ResourceAccess(UInt(left) & UInt(right));
}

static bool hasAccessDirection(ResourceAccess value, ResourceAccess direction)
{
    SLANG_RELEASE_ASSERT(direction == ResourceAccess::Read || direction == ResourceAccess::Write);
    return (UInt(value) & UInt(direction)) != 0;
}

/// Return which accesses through the user's result also access storage supplied by operand zero.
static ResourceAccess getTransferredStorageAccess(IRUse* use)
{
    // Address projections and pointer casts preserve both directions. An `inout` l-value cast also
    // copies the source into its temporary and copies the temporary back. An `out` l-value cast
    // performs only the copy back, so a read through its temporary does not read the source.
    SLANG_RELEASE_ASSERT(doesUserResultTransferStorageAccessFromUse(use));
    if (use->getUser()->getOp() == kIROp_OutImplicitCast)
        return ResourceAccess::Write;
    return mergeAccess(ResourceAccess::Read, ResourceAccess::Write);
}

/// Return which accesses through a callee parameter also access the caller-provided storage.
static ResourceAccess getCallArgumentBodyTransferAccess(IRInst* parameterType)
{
    // An `out` argument uses callee-local temporary storage and copies only its final value back,
    // so a read in the callee does not read the caller's value. Other address parameters can alias
    // the caller's storage. We retain both directions for them because an explicit pointer cast can
    // perform an effect that their declared direction would otherwise forbid.
    if (as<IROutParamType>(parameterType))
        return ResourceAccess::Write;
    return mergeAccess(ResourceAccess::Read, ResourceAccess::Write);
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

/// Return whether `use` occurs in metadata attached to `globalVar`.
static bool isUseInsideAttachedMetadata(IRUse* use, IRGlobalVar* globalVar)
{
    // We walk from the user to the direct child of `globalVar` that contains it. Only a decoration
    // or annotation belongs to the discardable metadata subtree. A separate check proves that no
    // instruction outside `globalVar` refers to anything in that subtree.
    auto subtreeRoot = use->getUser();
    while (subtreeRoot && subtreeRoot->getParent() != globalVar)
        subtreeRoot = subtreeRoot->getParent();
    if (!subtreeRoot)
        return false;
    if (as<IRDecoration>(subtreeRoot))
        return true;
    return as<IRAnnotation>(subtreeRoot) != nullptr;
}

/// Return a use whose user is outside `globalVar` and whose operand belongs to metadata that will
/// be deleted with the global.
static IRUse* findUseOfAttachedMetadataOutsideGlobal(IRGlobalVar* globalVar)
{
    // Initializer movement has removed the global's code blocks, so every remaining child must be
    // metadata. We visit that entire subtree because a decoration need not refer to its owner. For
    // each instruction, we require every user to belong to the same global subtree that phase 5
    // removes.
    List<IRInst*> workList;
    for (auto child : globalVar->getDecorationsAndChildren())
    {
        bool isMetadata = as<IRDecoration>(child) != nullptr;
        if (as<IRAnnotation>(child))
            isMetadata = true;
        SLANG_RELEASE_ASSERT(isMetadata);
        workList.add(child);
    }
    while (workList.getCount())
    {
        auto inst = workList.getLast();
        workList.removeLast();
        for (auto subtreeUse = inst->firstUse; subtreeUse; subtreeUse = subtreeUse->nextUse)
        {
            if (!isChildInstOf(subtreeUse->getUser(), globalVar))
                return subtreeUse;
        }
        for (auto child : inst->getDecorationsAndChildren())
            workList.add(child);
    }
    return nullptr;
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
    if (as<IRDecoration>(user))
        return false;
    if (as<IRAnnotation>(user))
        return false;
    if (as<IRAttr>(user))
        return false;
    if (as<IRType>(user))
        return false;
    if (isDebugInfoInst(user))
        return false;

    if (doesInstOnlyDependOnOperandTypes(user))
        return false;

    return true;
}

/// Return how a call may access the resource address passed by `use`.
///
/// A parameter's direction provides its declared read/write contract. When a defined direct callee
/// receives an address, we also inspect the parameter's IR uses because an explicit pointer cast
/// can perform an effect that the declared parameter direction does not express.
static ResourceAccess classifyCallArgumentAccess(
    IRCall* call,
    IRUse* use,
    CapabilitySet const& targetCaps)
{
    // We first apply the declared direction. A raw pointer or a use with no corresponding parameter
    // provides no directional guarantee, so we retain both effects. For a directional parameter,
    // we then merge additional effects found by following the callee's address uses. A read through
    // an `out` temporary does not read caller storage, while a write updates caller storage. Other
    // pointer-like parameters may alias caller storage, so an explicit cast can add either effect.
    auto paramType = findCallArgumentParameterType(call, use);
    if (!paramType)
        return mergeAccess(ResourceAccess::Read, ResourceAccess::Write);

    ResourceAccess access = ResourceAccess::Read;
    if (as<IROutParamType>(paramType))
        access = ResourceAccess::Write;
    else if (as<IRBorrowInOutParamType>(paramType))
        access = mergeAccess(ResourceAccess::Read, ResourceAccess::Write);
    else if (as<IRRefParamType>(paramType))
        access = mergeAccess(ResourceAccess::Read, ResourceAccess::Write);
    else if (as<IRPtrTypeBase>(paramType))
    {
        if (!as<IRBorrowInParamType>(paramType))
        {
            // A raw pointer parameter carries no directional contract. Its callee may read or write
            // the pointee, so we preserve both directions rather than assuming an input use.
            return mergeAccess(ResourceAccess::Read, ResourceAccess::Write);
        }
    }

    if (auto parameter = findDirectCalleeParameter(call, use))
    {
        auto addressUse =
            analyzeAddressUses(parameter, targetCaps, getCallArgumentBodyTransferAccess(paramType));
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
/// caller decides whether a write assigns the whole value, because that also depends on whether the
/// storage-access path covers the whole value or only a subobject.
static ResourceAccess classifyTerminalAddressUse(IRUse* use, CapabilitySet const& targetCaps)
{
    // We classify loads, atomic operations, stores, and calls from their operand contracts. An
    // unfamiliar pointer consumer may both read and write. Any other instruction consumes the
    // resource value reached through the address, so we provisionally classify it as a read before
    // the address-escape validation below accepts or rejects it.
    auto user = use->getUser();

    if (as<IRLoad>(user))
        return ResourceAccess::Read;
    if (as<IRAtomicLoad>(user))
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
        if (user->getOperandUse(0) == use)
            return ResourceAccess::Write;
        return mergeAccess(ResourceAccess::Read, ResourceAccess::Write);
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

/// Classify how storage access through the user's result applies to operand-zero storage.
///
/// Field and element operations select a subobject. `GetAddress` and `AssumeAddress` preserve the
/// whole value. The lowering of an implicit l-value cast either reuses the supplied address or
/// copies the whole value between the source and a temporary. Any access transferred by that cast
/// therefore preserves whether the source denotes the whole value or a selected subobject. Other
/// pointer casts preserve the whole value only when their source and result have equal
/// pointee types.
static StorageAccessRelation classifyStorageAccessRelation(IRUse* use)
{
    // We first handle operations whose semantics state the relation directly. For the remaining
    // pointer casts, equal pointee types prove that both addresses cover the same whole value;
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
            // a subobject of that type. An offset of zero therefore preserves the whole value. Any
            // other offset may name a neighboring object rather than the operand-zero storage.
            // Replacing the source with an independently allocated local cannot preserve that
            // relation, so phase 3 rejects the transfer instead of following it.
            auto offset = as<IRIntLit>(user->getOperand(1));
            if (!offset)
                return StorageAccessRelation::Unsupported;
            if (offset->getValue() != 0)
                return StorageAccessRelation::Unsupported;
            return StorageAccessRelation::WholeValue;
        }

    case kIROp_GetAddress:
    case kIROp_AssumeAddress:
    case kIROp_OutImplicitCast:
    case kIROp_InOutImplicitCast:
        return StorageAccessRelation::WholeValue;

    case kIROp_BitCast:
    case kIROp_Reinterpret:
    case kIROp_PtrCast:
        break;
    default:
        SLANG_UNEXPECTED("unrecognized storage-access transfer");
    }

    auto sourcePointerType = as<IRPtrTypeBase>(unwrapAttributedType(use->get()->getDataType()));
    auto resultPointerType = as<IRPtrTypeBase>(unwrapAttributedType(user->getDataType()));
    SLANG_RELEASE_ASSERT(sourcePointerType);
    SLANG_RELEASE_ASSERT(resultPointerType);

    // IR linking replaces every `IRSymbolAlias` with its target before this pass runs. The pointee
    // operands must therefore be IR types. We assert that pipeline invariant instead of silently
    // accepting an unlinked representation.
    auto sourceValueType = as<IRType>(unwrapAttributedType(sourcePointerType->getOperand(0)));
    auto resultValueType = as<IRType>(unwrapAttributedType(resultPointerType->getOperand(0)));
    SLANG_RELEASE_ASSERT(sourceValueType);
    SLANG_RELEASE_ASSERT(resultValueType);
    if (isTypeEqual(sourceValueType, resultValueType))
        return StorageAccessRelation::WholeValue;
    return StorageAccessRelation::Unknown;
}

/// Return the defined direct callee's parameter corresponding to `argumentUse`, or null when the
/// call or use has no such parameter.
static IRParam* findDirectCalleeParameter(IRCall* call, IRUse* argumentUse)
{
    // Operand zero names the callee. We match the remaining operand use by identity because one
    // value may be passed to several parameters. We require an IR body because a declaration does
    // not expose how the callee uses the address.
    auto callee = as<IRFunc>(call->getCallee());
    if (!callee)
        return nullptr;
    if (!callee->getFirstBlock())
        return nullptr;
    if (argumentUse == call->getCalleeUse())
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

/// Return whether `parameterType` gives an address argument a direction contract.
static bool isDirectionalAddressParameterType(IRInst* parameterType)
{
    // These four parameter types describe how the callee may access an address supplied by its
    // caller. A raw pointer has no such contract, and a value parameter does not receive an
    // address.
    if (as<IRBorrowInParamType>(parameterType))
        return true;
    if (as<IROutParamType>(parameterType))
        return true;
    if (as<IRBorrowInOutParamType>(parameterType))
        return true;
    if (as<IRRefParamType>(parameterType))
        return true;
    return false;
}

/// Analyze reads, writes, and unsupported uses reached from `rootAddress`.
///
/// A supported runtime use either transfers storage access through another result, reads or writes
/// through the address, or passes it to a directional parameter of a directly called function
/// whose IR body is available. We follow that callee parameter under the same rules. Generic
/// assembly and an applicable target intrinsic are unsupported because the emitted implementation
/// may use a parameter differently from the function's IR body.
static AddressUseAnalysis analyzeAddressUses(
    IRInst* rootAddress,
    CapabilitySet const& targetCaps,
    ResourceAccess transferredAccess)
{
    // We visit each reachable address separately for the read and write directions that can flow
    // back to the root. A pending item therefore pairs one address with those directions. The
    // finite set of such pairs makes the traversal terminate even when recursive functions forward
    // a parameter around a call cycle.
    struct PendingAddressUse
    {
        /// The address whose users remain to be examined.
        IRInst* address = nullptr;

        /// The accesses through this address that also access `rootAddress`.
        ResourceAccess transferredAccess = ResourceAccess::None;
    };

    AddressUseAnalysis result;
    List<PendingAddressUse> workList;
    HashSet<KeyValuePair<IRInst*, UInt>> visitedStates;
    workList.add({rootAddress, transferredAccess});
    for (Index workItemIndex = 0; workItemIndex < workList.getCount(); ++workItemIndex)
    {
        auto workItem = workList[workItemIndex];
        KeyValuePair<IRInst*, UInt> stateKey(workItem.address, UInt(workItem.transferredAccess));
        if (!visitedStates.add(stateKey))
            continue;
        auto address = workItem.address;

        // A function parameter exposes only the uses in its IR body. Generic assembly and an
        // applicable target intrinsic replace that body during emission, so neither body can prove
        // how the emitted implementation uses the parameter.
        if (as<IRParam>(address))
        {
            auto function = getParentFunc(address);
            SLANG_RELEASE_ASSERT(function);
            if (hasGenericAssemblyImplementation(function))
            {
                result.hasUnsupportedUse = true;
                return result;
            }
            if (findBestTargetIntrinsicDecoration(function, targetCaps))
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
            // or from the source. We carry the transferred read/write directions so an `out`
            // temporary cannot turn a read of the temporary into a read of the source.
            if (doesUserResultTransferStorageAccessFromUse(use))
            {
                if (classifyStorageAccessRelation(use) == StorageAccessRelation::Unsupported)
                {
                    result.hasUnsupportedUse = true;
                    return result;
                }

                auto resultAccess =
                    intersectAccess(workItem.transferredAccess, getTransferredStorageAccess(use));
                if (resultAccess != ResourceAccess::None)
                    workList.add({user, resultAccess});
                continue;
            }

            // A load uses the address only to read its pointee. Atomic load has the same contract.
            if (as<IRLoad>(user))
            {
                if (hasAccessDirection(workItem.transferredAccess, ResourceAccess::Read))
                    result.mayRead = true;
                continue;
            }
            if (as<IRAtomicLoad>(user))
            {
                if (hasAccessDirection(workItem.transferredAccess, ResourceAccess::Read))
                    result.mayRead = true;
                continue;
            }

            if (auto store = as<IRStore>(user))
            {
                // Operand zero is the destination address. A use in that operand writes the
                // pointee. A use in the stored value instead stores the address itself and lets it
                // escape its replacement local.
                if (store->getOperandUse(0) != use)
                {
                    result.hasUnsupportedUse = true;
                    return result;
                }
                if (hasAccessDirection(workItem.transferredAccess, ResourceAccess::Write))
                    result.mayWrite = true;
                continue;
            }

            // Atomic operations and swizzled stores also take their destination address in operand
            // zero. Atomic store and the two swizzled-store operations only write the pointee.
            // Every other atomic operation is read-modify-write.
            bool usesAddressInOperandZero = false;
            bool readsBeforeWriting = false;
            if (as<IRAtomicOperation>(user))
            {
                usesAddressInOperandZero = true;
                readsBeforeWriting = !as<IRAtomicStore>(user);
            }
            else if (as<IRSwizzledStore>(user))
            {
                usesAddressInOperandZero = true;
            }
            else if (as<IRMatrixSwizzleStore>(user))
            {
                usesAddressInOperandZero = true;
            }
            if (usesAddressInOperandZero)
            {
                if (user->getOperandUse(0) != use)
                {
                    result.hasUnsupportedUse = true;
                    return result;
                }
                if (hasAccessDirection(workItem.transferredAccess, ResourceAccess::Write))
                    result.mayWrite = true;
                if (readsBeforeWriting)
                {
                    if (hasAccessDirection(workItem.transferredAccess, ResourceAccess::Read))
                        result.mayRead = true;
                }
                continue;
            }

            if (auto call = as<IRCall>(user))
            {
                // We can follow an address into a callee only when the call names a defined
                // function and the corresponding parameter states how the address is transferred.
                // The next work item applies the same rules to every use of that parameter.
                auto parameter = findDirectCalleeParameter(call, use);
                if (!parameter)
                {
                    result.hasUnsupportedUse = true;
                    return result;
                }
                auto parameterType = as<IRType>(unwrapAttributedType(parameter->getDataType()));
                if (!parameterType)
                {
                    result.hasUnsupportedUse = true;
                    return result;
                }
                if (!isDirectionalAddressParameterType(parameterType))
                {
                    result.hasUnsupportedUse = true;
                    return result;
                }
                auto resultAccess = intersectAccess(
                    workItem.transferredAccess,
                    getCallArgumentBodyTransferAccess(parameterType));
                if (resultAccess != ResourceAccess::None)
                    workList.add({parameter, resultAccess});
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
    if (as<IRLoad>(user))
        return false;
    if (as<IRAtomicLoad>(user))
        return false;
    if (auto store = as<IRStore>(user))
        return store->getOperandUse(0) != use;
    if (as<IRAtomicOperation>(user))
        return user->getOperandUse(0) != use;
    if (as<IRSwizzledStore>(user))
        return user->getOperandUse(0) != use;
    if (as<IRMatrixSwizzleStore>(user))
        return user->getOperandUse(0) != use;
    if (auto call = as<IRCall>(user))
    {
        // A directional parameter does not prevent the callee from storing its argument's address.
        // We accept the call only when the available callee body proves that every runtime use of
        // the matching parameter is non-escaping. `analyzeAddressUses` documents the accepted uses.
        auto parameter = findDirectCalleeParameter(call, use);
        auto parameterType = findCallArgumentParameterType(call, use);
        if (!parameter)
            return true;
        if (!parameterType)
            return true;
        auto analysis = analyzeAddressUses(
            parameter,
            targetCaps,
            getCallArgumentBodyTransferAccess(parameterType));
        return analysis.hasUnsupportedUse;
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

/// Return whether `use` names the kernel operand of an instruction that launches an entry point.
static bool isEntryPointLaunchUse(IRUse* use)
{
    // Both launch instructions keep the kernel function in operand zero. We compare the exact use
    // so that another operand which happens to reference the same function is not accepted as a
    // launch.
    auto user = use->getUser();
    if (user->getOperandCount() == 0 || user->getOperandUse(0) != use)
        return false;
    return user->getOp() == kIROp_DispatchKernel || user->getOp() == kIROp_CudaKernelLaunch;
}

/// Return whether `func` has an invocation or value use that phase 4 cannot rewrite.
static bool hasFunctionUseThatCannotBeRewritten(IRFunc* func)
{
    // We can append resource arguments to a direct call inside a function. A call in a global
    // initializer has no function-local resource value to pass. A decoration such as
    // `IRPatchConstantFuncDecoration` can cause another invocation but provides no argument list.
    // Passing, storing, or returning the function as a value also prevents phase 4 from extending
    // every eventual invocation. A launch instruction is different: phase 4 inserts the replacement
    // local into the original entry-point body, and each launch executes that body with a distinct
    // dynamic instance of the local. No generated argument is needed. We accept only the kernel
    // operand of the two launch instructions and continue to reject every other runtime use.
    const bool isEntryPoint = isShaderOrCudaKernelEntryPoint(func);
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

        if (isEntryPoint && isEntryPointLaunchUse(use))
            continue;
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
    // invocation appears as a direct call whose argument list we can extend.
    static const IROp kInvocationExposureDecorations[] = {
        kIROp_KeepAliveDecoration,
        kIROp_PublicDecoration,
        kIROp_HLSLExportDecoration,
        kIROp_DllExportDecoration,
        kIROp_ExternCDecoration,
        kIROp_ExternCppDecoration,
        kIROp_CudaDeviceExportDecoration,
        kIROp_DownstreamModuleExportDecoration,
        kIROp_DownstreamModuleImportDecoration,
    };
    for (auto op : kInvocationExposureDecorations)
    {
        if (func->findDecorationImpl(op))
            return true;
    }

    // A non-call use can pass the function as a value or name it in an invocation decoration such
    // as `IRPatchConstantFuncDecoration`.
    return hasFunctionUseThatCannotBeRewritten(func);
}

/// Return whether `globalVar` must remain in module-scope storage.
///
/// Retention decorations and linkage that exposes storage outside this module establish this
/// requirement. `IRExportDecoration`, which only matches definitions during IR linking, does not.
static bool requiresModuleScopeStorage(IRGlobalVar* globalVar)
{
    // `IRExportDecoration` is intentionally absent. Definitions merged by IR linking use it for
    // matching, but `IRExportDecoration` does not require the final program to expose their
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

bool doesModuleContainResourceGlobalCandidate(IRModule* module)
{
    // We use the same predicate as phase 1 of the legalizer so that pass ordering and candidate
    // collection cannot disagree about whether the module needs resource-global replacement.
    for (auto inst : module->getGlobalInsts())
    {
        auto globalVar = as<IRGlobalVar>(inst);
        if (globalVar && isResourceGlobalCandidateForPerInvocationReplacement(globalVar))
            return true;
    }
    return false;
}

void legalizeResourceGlobalVars(
    IRModule* module,
    CapabilitySet const& targetCaps,
    bool shouldDiagnoseUninitializedValues,
    DiagnosticSink* sink)
{
    // The caller has already extracted every selected initializer body. We can therefore replace
    // each selected storage declaration without losing initializer code.
    LegalizeResourceGlobalVarsPass pass(module, targetCaps, shouldDiagnoseUninitializedValues);
    pass.processModule(sink);
}

} // namespace Slang
