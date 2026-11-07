#include "slang-ir-specialize-resource-selections.h"

#include "slang-code-gen.h"
#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-resource-selection-diagnostics.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-target-program.h"
#include "slang-target.h"

namespace Slang
{
// An instruction that conditionally computes a resource value, such as a `select` instruction,
// is valid in Slang IR. Several compilation targets, particularly Vulkan SPIR-V, cannot directly
// represent all such instructions. For example, eliminating a resource-valued block parameter
// can require a local texture or sampler variable, which Khronos targets cannot represent.
//
// This file implements `specializeResourceSelections`, a pass that attempts to eliminate
// instructions that conditionally compute resources by moving the conditional computation to:
//
// - An index into a resource array or descriptor heap, allowing one resource access afterward.
// - A branch around each of several resource-using operations, each using a concrete resource,
//   allowing only their non-resource results to be merged afterward.
//
// Consider this example:
//
//     Texture2D<float4> textures[8];
//     Texture2D<float4> other;
//     SamplerState sampler;
//     float4 f(bool useArray, uint i, float2 uv)
//     {
//         Texture2D<float4> t = useArray ? textures[i] : other;
//         return t.SampleLevel(sampler, uv, 0);
//     }
//
// We introduce the term *reconstruction recipe* for a description of how to materialize one of
// the possible resource values. A recipe records fixed inputs, such as the array `textures`,
// and how to use them. Inputs computed during shader execution, such as `i`, are supplied
// separately as *payload values*. The recipe records the types of those inputs so that the
// compiler can carry a payload for each possible recipe through the program's control flow.
// Payloads contain ordinary values, such as integer indices or vector coordinates, whose types
// the target can store without needing resource storage.
//
// For `t` above, one recipe indexes `textures` and needs the index `i` as its payload. A second
// recipe refers directly to `other` and needs no payload. Instead of computing a selected
// texture, we compute a *selector* that identifies the chosen recipe and carry the corresponding
// payload values. At the `SampleLevel` operation, we branch on that selector, materialize the
// resource for the chosen recipe, and perform the sample. Only the ordinary `float4` result
// crosses the merge. We neither access an unselected resource nor move the sample before its
// coordinates have been computed.
//
// As another example, consider this alternative input function:
//
//     Texture2D<float4> textures[8];
//     SamplerState sampler;
//     float4 f(bool useFirst, uint i, uint j, float2 uv)
//     {
//         Texture2D<float4> t = useFirst ? textures[i] : textures[j];
//         return t.SampleLevel(sampler, uv, 0);
//     }
//
// Both possible values of `t` can use one recipe: index `textures` using the supplied payload.
// We select `i` or `j` as that payload, then access `textures[useFirst ? i : j]` and sample it
// without introducing a branch around the sample. We call this case the *same-array optimization*.
// A descriptor-heap load can use the same approach: the heap declaration is fixed, and we select
// the descriptor index to supply as payload.
//
// We call an instruction that constructs a resource value or an address into its contents a
// *resource constructor*. Examples include array access, descriptor-heap loading, and combining
// a texture with a sampler. The recipe for such a combination contains separate child recipes
// for the texture and sampler. A resource or array that is already available at the eventual
// operation, such as a global shader parameter or a resource-valued function parameter, is a
// *root*: its recipe refers to that existing IR value directly. A precombined texture/sampler
// binding is a root too; we do not invent separately addressable bindings for its components.
//
// A *terminal consumer* is an instruction that uses a resource or resource-backed address and
// produces an ordinary result, or only a side effect. Samples, memory operations, and calls
// returning ordinary values are examples. A resource constructor is not a terminal consumer:
// we follow its result to the operation that will use the constructed value. In particular,
// combined texture/sampler construction must be followed through to the sample operation.
//
// SSA construction represents conditional resource computations as `IRSelect` instructions or
// non-entry block parameters whose incoming branch arguments supply the possible resource values.
// We call the connected set of those instructions, their resource constructors, and roots a
// *selection graph*. We follow incoming dependencies and the uses of values that will be replaced,
// so shared resource computations are rewritten together. Roots remain available and can be inputs
// to separate graphs. A loop can introduce cycles through block parameters in a graph.
//
// The implementation has four phases: discover a selection graph, determine its finite recipe
// alternatives and validate the constructions needed by its consumers, replace resource-valued
// state with ordinary selectors and payloads, and rewrite terminal consumers. Selects and block
// parameters belong to the selection graph, not the recipes. We can therefore represent a loop
// with ordinary state instead of recursively expanding a resource expression. Replacement block
// parameters exist before we construct any back-edge argument. Two recipe alternatives can use
// Boolean state and an `if`/`else`; larger sets use integer state and a `switch`.

namespace
{
/// An operand in a resource reconstruction is a child recipe, an ordinary payload, or a fixed
/// value.
///
/// A child recipe is identified by its index in the analysis's recipe list. That reference is
/// distinct from the runtime selector number assigned after recipe discovery. A payload operand
/// records its required type; a fixed operand records an existing IR value, such as a binding or
/// a field key, that can be referenced at every reconstruction.
///
/// The factories create initialized operands. A default-constructed instance has `Flavor::None`
/// and must be assigned an initialized operand before being compared, hashed, or used in a recipe.
class ResourceRecipeOperand
{
public:
    enum class Flavor
    {
        None,
        Resource,
        Payload,
        Fixed,
    };

    /// Create uninitialized storage that must be assigned an operand before use in a recipe.
    ResourceRecipeOperand()
        : m_flavor(Flavor::None)
    {
        // We want usable operands to be created by the named factories, which supply the value
        // required by each flavor. However, `List` default-constructs its allocated slots before
        // assigning elements into them. We therefore need a default constructor whose result is
        // explicitly uninitialized, rather than pretending that it is a usable fixed operand.
    }

    /// Create an operand referring to the recipe at a nonnegative analysis-list index.
    static ResourceRecipeOperand resource(Index recipe)
    {
        SLANG_RELEASE_ASSERT(recipe >= 0);
        ResourceRecipeOperand result(Flavor::Resource);
        result.m_recipe = recipe;
        return result;
    }

    /// Create an operand whose dynamic value must have the non-null payload `type`.
    static ResourceRecipeOperand payload(IRType* type)
    {
        SLANG_RELEASE_ASSERT(type);
        ResourceRecipeOperand result(Flavor::Payload);
        result.m_value = type;
        return result;
    }

    /// Create an operand referring directly to the non-null fixed IR `value`.
    static ResourceRecipeOperand fixed(IRInst* value)
    {
        SLANG_RELEASE_ASSERT(value);
        ResourceRecipeOperand result(Flavor::Fixed);
        result.m_value = value;
        return result;
    }

    Flavor getFlavor() const { return m_flavor; }

    Index getRecipe() const
    {
        SLANG_RELEASE_ASSERT(m_flavor == Flavor::Resource);
        return m_recipe;
    }

    IRType* getPayloadType() const
    {
        SLANG_RELEASE_ASSERT(m_flavor == Flavor::Payload);
        return cast<IRType>(m_value);
    }

    IRInst* getFixedValue() const
    {
        SLANG_RELEASE_ASSERT(m_flavor == Flavor::Fixed);
        SLANG_RELEASE_ASSERT(m_value);
        return m_value;
    }

    bool operator==(const ResourceRecipeOperand& other) const
    {
        SLANG_RELEASE_ASSERT(m_flavor != Flavor::None);
        SLANG_RELEASE_ASSERT(other.m_flavor != Flavor::None);
        if (m_flavor != other.m_flavor)
            return false;
        if (m_flavor == Flavor::Resource)
            return m_recipe == other.m_recipe;
        return m_value == other.m_value;
    }

    HashCode getHashCode() const
    {
        SLANG_RELEASE_ASSERT(m_flavor != Flavor::None);
        if (m_flavor == Flavor::Resource)
            return combineHash(Slang::getHashCode(Int(m_flavor)), Slang::getHashCode(m_recipe));
        return combineHash(Slang::getHashCode(Int(m_flavor)), Slang::getHashCode(m_value));
    }

private:
    explicit ResourceRecipeOperand(Flavor flavor)
        : m_flavor(flavor)
    {
    }

    Flavor m_flavor;
    Index m_recipe = -1;
    IRInst* m_value = nullptr;
};

/// A recipe key describes a resource construction's structure while leaving payload values
/// abstract.
///
/// Two constructions share a key only when they agree on:
///
/// - The IR opcode and result type.
/// - The number, order, and flavors of the operands.
/// - The interned child recipe for each resource operand.
/// - The exact IR value for each fixed operand.
/// - The type of each payload operand.
///
/// For example, `a[i]` and `a[j]` share a key when their index payloads have the same type, but
/// `a[i]` and `b[i]` do not. Key equality does not imply equality of the constructed resource
/// values: the payloads can select different elements of the same array.
///
/// `op` is the constructor opcode, except that `kIROp_Invalid` denotes a root key whose sole
/// operand is the fixed IR root value. A default key is incomplete; its type and operands must be
/// assigned before interning.
struct ResourceRecipeKey
{
    IROp op = kIROp_Invalid;
    IRType* type = nullptr;
    List<ResourceRecipeOperand> operands;

    bool operator==(const ResourceRecipeKey& other) const
    {
        if (op != other.op)
            return false;
        if (type != other.type)
            return false;
        if (operands.getCount() != other.operands.getCount())
            return false;
        for (Index i = 0; i < operands.getCount(); i++)
            if (!(operands[i] == other.operands[i]))
                return false;
        return true;
    }

    HashCode getHashCode() const
    {
        Hasher hasher;
        hasher.hashValue(Int(op));
        hasher.hashValue(type);
        for (const auto& operand : operands)
            hasher.hashValue(operand);
        return hasher.getResult();
    }
};

/// A recipe describes one resource construction and the types of its flattened ordinary payload.
//
// `payloadTypes` follows operand order, recursively expanding each child recipe. It records the
// order of constructor arguments, not an ordering imposed on bindings. `selectorNumber` identifies
// this recipe among the recipes with the same `key.type` during shader execution. It is assigned
// after recipe discovery and is distinct from this recipe's index in the analysis's `recipes` list.
struct ResourceRecipe
{
    ResourceRecipeKey key;
    List<IRType*> payloadTypes;
    Index selectorNumber = -1;
};

/// An IR value in a selection graph and the ordinary state used to choose its reconstruction.
//
// For each possible recipe, `payloads` contains values meaningful when `selector` names that
// recipe. Payloads for other recipes need not describe a valid access: they are never reconstructed
// on that execution path. A value with one recipe has a constant selector.
struct ResourceSelectionValue : RefObject
{
    IRInst* inst = nullptr;
    List<ResourceSelectionValue*> dependencies;
    OrderedHashSet<Index> recipes;
    IRInst* selector = nullptr;
    Dictionary<Index, List<IRInst*>> payloads;
    bool isRoot = false;
};

/// A terminal consumer and the selection-graph values referenced by its operands or assembly.
struct ResourceSelectionConsumer
{
    IRInst* inst = nullptr;
    OrderedHashSet<ResourceSelectionValue*> arguments;
};

/// The analysis and replacement mappings for rewriting one selection graph within a function.
struct ResourceSelectionContext
{
    CodeGenContext* codeGenContext;
    IRFunc* func;
    Dictionary<IRInst*, ResourceSelectionValue*> valueMap;
    List<RefPtr<ResourceSelectionValue>> values;
    Dictionary<ResourceRecipeKey, Index> recipeMap;
    List<ResourceRecipe> recipes;
    Dictionary<IRType*, List<Index>> recipesByType;
    Dictionary<IRBlock*, List<IRParam*>> originalParams;
    Dictionary<IRInst*, Index> consumerMap;
    List<ResourceSelectionConsumer> consumers;
    // These payload values are computed by selects or parameters that replace conditional
    // resource computations. When used as descriptor indices, they need `NonUniformResourceIndex`
    // even if their incoming values were constants.
    HashSet<IRInst*> selectedPayloads;
    IRType* commonIndexType = nullptr;

    /// Rewrite the graph containing `candidate`, leaving unsupported graphs unchanged.
    ///
    /// Returns whether the graph was rewritten. On failure, no executable IR is changed.
    bool tryRewrite(IRInst* candidate)
    {
        // We finish analysis before changing executable IR. Roots and constructor operands must
        // be reconstructible, and every use of a replaced resource value must end in a supported
        // ordinary-valued operation. The caller, `specializeResourceUsage`, also runs
        // `specializeResourceOutputs` and tries inlining calls whose resource outputs could not be
        // reconstructed. Its next iteration can retry a graph that could not be rewritten before
        // that inlining.
        if (!discoverGraph(candidate))
            return false;
        if (!discoverRecipes())
            return false;
        if (!canMaterializeConsumerRecipes())
            return false;

        // The recipe alternatives determine the ordinary values carried for each resource
        // parameter. We allocate all parameters first because incoming loop state can be cyclic.
        assignSelectorNumbers();
        createReplacementParams();
        emitReplacementValues();
        transferSelectedStateNames();
        rewriteBranchArguments();

        // Every terminal operation still occupies its original execution point. Its ordinary
        // arguments dominate the new dispatch, including arguments computed after a resource
        // selection. We can therefore clone the operation without moving those calculations.
        for (auto& consumer : consumers)
            rewriteConsumer(consumer);
        removeResourceGraph();
        return true;
    }

    /// Return whether `type` is an image, sampler, or another unstorable resource, including
    /// arrays.
    static bool isOpaqueResourceType(IRType* type)
    {
        // Storage buffers have an existing variable-pointer lowering. We leave their ordinary
        // selections to that lowering; descriptor-heap loads remain accepted separately because
        // replacing those loads with selected heap indices is required for storage buffers.
        type = unwrapArray(type);
        if (as<IRResourceTypeBase>(type))
            return true;
        if (as<IRSamplerStateTypeBase>(type))
            return true;
        if (as<IRSubpassInputType>(type))
            return true;
        return type->getOp() == kIROp_RaytracingAccelerationStructureType;
    }

    /// Return whether an instruction conditionally computes a resource through a select or phi.
    static bool isSelection(IRInst* inst)
    {
        if (!isResourceType(inst->getDataType()))
            return false;
        // Pointer arithmetic can create unbounded paths around a loop. Native pointer choices
        // already have their own legalization and are outside this resource-construction model.
        if (as<IRPtrTypeBase>(unwrapArray(inst->getDataType())))
            return false;
        if (as<IRSelect>(inst))
            return true;
        auto param = as<IRParam>(inst);
        if (!param)
            return false;
        auto block = cast<IRBlock>(param->getParent());
        return block != cast<IRFunc>(block->getParent())->getFirstBlock();
    }

    /// Return whether `inst` has a resource-constructor form recognized by graph discovery.
    ///
    /// This classification does not validate resource dependencies or target support.
    static bool isConstructor(IRInst* inst)
    {
        // These instructions describe resource values or addresses without performing the
        // eventual resource access. `discoverGraph` also checks their dependencies: a field or
        // element address is usable only when its base can be reconstructed from a resource
        // root. Actual reads, writes, and samples are terminal consumers and execute only in
        // the chosen branch.
        switch (inst->getOp())
        {
        case kIROp_GetElement:
            return isResourceType(inst->getDataType());
        case kIROp_SPIRVLoadDescriptorFromHeap:
        case kIROp_MakeCombinedTextureSampler:
        case kIROp_CombinedTextureSamplerGetTexture:
        case kIROp_CombinedTextureSamplerGetSampler:
        case kIROp_ImageSubscript:
        case kIROp_ImageTexelPointer:
        case kIROp_GetElementPtr:
        case kIROp_FieldAddress:
            return true;
        default:
            return false;
        }
    }

    /// Return which operands of a supported constructor are themselves resource descriptions.
    static bool isResourceOperand(IRInst* inst, UInt operandIndex)
    {
        if (inst->getOp() == kIROp_SPIRVLoadDescriptorFromHeap)
            return false;
        if (inst->getOp() == kIROp_MakeCombinedTextureSampler)
            return true;
        return operandIndex == 0;
    }

    /// Return whether a constructor operand is an index requiring common-width conversion.
    static bool isDescriptorIndex(IRInst* inst, UInt operandIndex)
    {
        if (operandIndex != 1)
            return false;
        if (inst->getOp() == kIROp_GetElement)
            return true;
        return inst->getOp() == kIROp_SPIRVLoadDescriptorFromHeap;
    }

    /// Return whether an operand names static construction metadata rather than ordinary state.
    ///
    /// Requires a constructor recognized by `isConstructor`.
    static bool isFixedOperand(IRInst* inst, UInt operandIndex)
    {
        // For the supported constructors, types and typeless field keys identify static IR
        // metadata, not values calculated during shader execution. A heap declaration is fixed
        // metadata too; `discoverGraph` separately requires it to have module scope. Those
        // operands can therefore be reused wherever `materializeRecipe` reconstructs a resource
        // or resource-backed address.
        if (inst->getOp() == kIROp_SPIRVLoadDescriptorFromHeap)
            if (operandIndex == 0)
                return true;
        auto operand = inst->getOperand(operandIndex);
        if (as<IRType>(operand))
            return true;
        return !operand->getDataType();
    }

    /// Add a graph value once, so shared subexpressions and back edges terminate discovery.
    ResourceSelectionValue* addValue(IRInst* inst)
    {
        if (auto existing = valueMap.tryGetValue(inst))
            return *existing;
        RefPtr<ResourceSelectionValue> value = new ResourceSelectionValue;
        value->inst = inst;
        valueMap.add(inst, value);
        values.add(value);
        return value;
    }

    /// Find `inst` or an ancestor instruction directly owned by an `IRBlock` in `func`.
    ///
    /// Returns null for globals and instructions in other functions.
    IRInst* findEnclosingInstructionInBlockOfCurrentFunc(IRInst* inst)
    {
        // An `IRSPIRVAsmOperand` is nested inside an `IRSPIRVAsm` operation. Conditional execution
        // must include that whole operation, so consumer discovery needs its enclosing block
        // child, not the operand wrapper.
        while (inst)
        {
            auto parent = inst->getParent();
            if (as<IRBlock>(parent))
            {
                if (parent->getParent() == func)
                    return inst;
                return nullptr;
            }
            inst = parent;
        }
        return nullptr;
    }

    /// Include all opaque arguments of one terminal operation in the same component.
    void includeConsumerArguments(ResourceSelectionConsumer& consumer)
    {
        // A GLSL sample is an intrinsic call with separate texture and sampler arguments. Their
        // choices are related through that call even without a `MakeCombinedTextureSampler`
        // node. Discovering both arguments together lets us clone and diagnose the original call
        // once. Assembly operands can be nested children, so we inspect the operation's owned
        // subtree. We never traverse ordinary argument definitions, which must retain their
        // evaluation point.
        List<IRInst*> pending;
        pending.add(consumer.inst);
        for (Index i = 0; i < pending.getCount(); i++)
        {
            auto inst = pending[i];
            for (UInt operandIndex = 0; operandIndex < inst->getOperandCount(); operandIndex++)
            {
                auto value = inst->getOperand(operandIndex);
                auto type = value->getDataType();
                if (!type)
                    continue;
                // Assembly operand wrappers have the wrapped value's type, but belong to the
                // assembly representation. We clone internal values with the operation. Only
                // references outside its owned subtree are resource arguments of the graph.
                if (findEnclosingInstructionInBlockOfCurrentFunc(value) == consumer.inst)
                    continue;
                if (isOpaqueResourceType(type))
                    consumer.arguments.add(addValue(value));
            }
            for (auto child : inst->getChildren())
                pending.add(child);
        }
    }

    /// Include the destination parameter for one exact resource argument use on a branch.
    ///
    /// Requires `use` to refer to an argument of `branch`.
    void includeBranchArgumentDestinationParam(IRUnconditionalBranch* branch, IRUse* use)
    {
        auto param = branch->getTargetBlock()->getFirstParam();
        for (UInt i = 0; i < branch->getArgCount(); i++, param = param->getNextParam())
        {
            SLANG_RELEASE_ASSERT(param);
            if (branch->getArgs() + i == use)
            {
                addValue(param);
                return;
            }
        }
        SLANG_UNEXPECTED("A resource branch use must name an argument.");
    }

    /// Record the broadest descriptor index type without narrowing existing 64-bit indices.
    void includeIndexType(IRType* type)
    {
        // A recipe has one payload type per indexing step, but incoming descriptor indices can
        // have different integer types. We choose a common type for the graph so accesses to the
        // same array can share one recipe and select their index payload. Valid indices are
        // nonnegative: the widest incoming type can represent all of them when unsigned wins an
        // equal-width tie. This preserves wider indices without introducing 64-bit state for
        // entirely narrower input.
        if (!commonIndexType)
        {
            commonIndexType = type;
            return;
        }
        auto oldInfo = getIntTypeInfo(codeGenContext->getTargetReq(), commonIndexType);
        auto newInfo = getIntTypeInfo(codeGenContext->getTargetReq(), type);
        if (newInfo.width > oldInfo.width)
            commonIndexType = type;
        else if (newInfo.width == oldInfo.width)
            if (!newInfo.isSigned)
                commonIndexType = type;
    }

    /// Discover the resource graph containing `candidate`, rejecting unsupported producers before
    /// executable IR is changed.
    bool discoverGraph(IRInst* candidate)
    {
        addValue(candidate);
        for (Index i = 0; i < values.getCount(); i++)
        {
            auto value = values[i];
            auto inst = value->inst;
            if (auto select = as<IRSelect>(inst))
            {
                value->dependencies.add(addValue(select->getTrueResult()));
                value->dependencies.add(addValue(select->getFalseResult()));
            }
            else if (auto param = as<IRParam>(inst))
            {
                auto block = cast<IRBlock>(param->getParent());
                if (block == func->getFirstBlock())
                {
                    // Entry parameters dominate the entire body and can be named directly.
                    if (!isOpaqueResourceType(param->getDataType()))
                        return false;
                    value->isRoot = true;
                }
                else
                {
                    if (block->getPredecessors().isEmpty())
                        return false;
                    if (!isSelection(param))
                        return false;
                    auto paramIndex = getParamIndexInBlock(param);
                    for (auto predecessor : block->getPredecessors())
                    {
                        auto branch = cast<IRUnconditionalBranch>(predecessor->getTerminator());
                        value->dependencies.add(addValue(branch->getArg((UInt)paramIndex)));
                    }
                    if (!originalParams.containsKey(block))
                        for (auto originalParam : block->getParams())
                            originalParams[block].add(originalParam);
                }
            }
            else if (as<IRGlobalParam>(inst))
            {
                if (!isOpaqueResourceType(inst->getDataType()))
                    return false;
                value->isRoot = true;
            }
            else if (isConstructor(inst))
            {
                if (inst->getOp() == kIROp_SPIRVLoadDescriptorFromHeap)
                {
                    // A heap is fixed recipe metadata, so its declaration must be available at
                    // every reconstruction. Built-in heap declarations have module scope. A
                    // computed heap would need another selected payload and is not handled here.
                    auto heap = inst->getOperand(0);
                    if (!as<IRSPIRVResourceHeap>(heap))
                        if (!as<IRSPIRVSamplerHeap>(heap))
                            return false;
                }
                for (UInt operandIndex = 0; operandIndex < inst->getOperandCount(); operandIndex++)
                {
                    auto operand = inst->getOperand(operandIndex);
                    if (isResourceOperand(inst, operandIndex))
                        value->dependencies.add(addValue(operand));
                    else if (isDescriptorIndex(inst, operandIndex))
                        includeIndexType(operand->getDataType());
                }
            }
            else
            {
                // A resource-valued call or load can require inlining or SSA simplification.
                // `specializeResourceUsage` retries this graph after those transformations, so
                // failure to recognize this producer does not discard its resource computation.
                return false;
            }

            // Roots remain available after the graph is rewritten, so their other uses need no
            // replacement. We stop at a root rather than pulling an independent resource-valued
            // operation into this graph, where it could prevent a supported selection rewrite.
            if (value->isRoot)
                continue;

            // For values that we replace, we follow uses as well as dependencies. Otherwise a
            // shared selection could be deleted while a second selection still expected its
            // resource-valued representation.
            for (auto use = inst->firstUse; use; use = use->nextUse)
            {
                auto user = use->getUser();
                auto operation = findEnclosingInstructionInBlockOfCurrentFunc(user);
                if (!operation)
                    continue;
                if (isSelection(operation))
                    addValue(operation);
                else if (isConstructor(operation))
                    addValue(operation);
                else if (auto branch = as<IRUnconditionalBranch>(operation))
                    includeBranchArgumentDestinationParam(branch, use);
                else
                {
                    // `specializeResourceOutputs` and the output-call inlining performed by
                    // `specializeResourceUsage` handle resource returns. Address-valued results
                    // also need reconstruction; they cannot cross an ordinary result merge.
                    if (operation->getOp() == kIROp_Return)
                        return false;
                    if (isResourceType(operation->getDataType()))
                        return false;
                    Index consumerIndex;
                    if (auto existing = consumerMap.tryGetValue(operation))
                        consumerIndex = *existing;
                    else
                    {
                        consumerIndex = consumers.getCount();
                        consumerMap.add(operation, consumerIndex);
                        ResourceSelectionConsumer consumer;
                        consumer.inst = operation;
                        includeConsumerArguments(consumer);
                        consumers.add(consumer);
                    }
                    consumers[consumerIndex].arguments.add(value);
                }
            }
        }
        return true;
    }

    /// Intern a completed construction key and compute its flattened payload layout.
    ///
    /// Requires a completed key with a non-null result type and initialized operands. A root key
    /// must contain exactly one fixed operand identifying the existing root value.
    Index internRecipe(const ResourceRecipeKey& key)
    {
        // Both producers build keys incrementally. Before interning, we assert the result type
        // and the root's single-fixed-operand form. Operand hashing rejects uninitialized slots.
        SLANG_RELEASE_ASSERT(key.type);
        if (key.op == kIROp_Invalid)
        {
            SLANG_RELEASE_ASSERT(key.operands.getCount() == 1);
            SLANG_RELEASE_ASSERT(
                key.operands[0].getFlavor() == ResourceRecipeOperand::Flavor::Fixed);
        }
        if (auto existing = recipeMap.tryGetValue(key))
            return *existing;
        ResourceRecipe recipe;
        recipe.key = key;
        for (const auto& operand : key.operands)
        {
            if (operand.getFlavor() == ResourceRecipeOperand::Flavor::Resource)
                recipe.payloadTypes.addRange(recipes[operand.getRecipe()].payloadTypes);
            else if (operand.getFlavor() == ResourceRecipeOperand::Flavor::Payload)
                recipe.payloadTypes.add(operand.getPayloadType());
        }
        auto index = recipes.getCount();
        recipes.add(recipe);
        recipeMap.add(key, index);
        recipesByType[key.type].add(index);
        return index;
    }

    /// Describe a constructor with a concrete recipe for each resource operand.
    ///
    /// Requires `childRecipes` to identify one recipe per resource dependency, in operand order.
    /// Returns an interned recipe index, reusing the child recipe for a projection of a known pair.
    Index getConstructorRecipe(ResourceSelectionValue* value, const List<Index>& childRecipes)
    {
        // Texture/sampler projections of a known pair use the corresponding child recipe. In
        // addition to avoiding needless reconstruction, this prevents a combine/project loop from
        // generating an endlessly deeper description of the same resource.
        auto inst = value->inst;
        if (inst->getOp() == kIROp_CombinedTextureSamplerGetTexture)
            if (recipes[childRecipes[0]].key.op == kIROp_MakeCombinedTextureSampler)
                return recipes[childRecipes[0]].key.operands[0].getRecipe();
        if (inst->getOp() == kIROp_CombinedTextureSamplerGetSampler)
            if (recipes[childRecipes[0]].key.op == kIROp_MakeCombinedTextureSampler)
                return recipes[childRecipes[0]].key.operands[1].getRecipe();

        ResourceRecipeKey key;
        key.op = inst->getOp();
        key.type = inst->getDataType();
        Index childIndex = 0;
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            auto operand = inst->getOperand(i);
            if (isResourceOperand(inst, i))
                key.operands.add(ResourceRecipeOperand::resource(childRecipes[childIndex++]));
            else if (isDescriptorIndex(inst, i))
                key.operands.add(ResourceRecipeOperand::payload(commonIndexType));
            else if (isFixedOperand(inst, i))
                key.operands.add(ResourceRecipeOperand::fixed(operand));
            else
                key.operands.add(ResourceRecipeOperand::payload(operand->getDataType()));
        }
        return internRecipe(key);
    }

    /// Add recipes for every combination of a constructor's resource-operand alternatives.
    ///
    /// Requires `childRecipes` to contain one chosen recipe per dependency before
    /// `dependencyIndex`. Restores that prefix on return. Returns whether `value->recipes` gained
    /// an alternative.
    bool addConstructorRecipes(
        ResourceSelectionValue* value,
        Index dependencyIndex,
        List<Index>& childRecipes)
    {
        // We extend the chosen prefix with each recipe of the next dependency. Once every
        // dependency has a chosen recipe, `getConstructorRecipe` describes the resulting
        // constructor. Removing each appended choice after recursion restores the caller's
        // prefix, so trying one alternative cannot affect the next.
        if (dependencyIndex == value->dependencies.getCount())
            return value->recipes.add(getConstructorRecipe(value, childRecipes));
        bool changed = false;
        for (auto recipe : value->dependencies[dependencyIndex]->recipes)
        {
            childRecipes.add(recipe);
            changed |= addConstructorRecipes(value, dependencyIndex + 1, childRecipes);
            childRecipes.removeLast();
        }
        return changed;
    }

    /// Compute finite recipe alternatives to a fixed point, including cycles through block params.
    ///
    /// Requires completed graph discovery. Returns false if any graph value has no recipe after
    /// convergence; otherwise, every graph value has at least one reconstruction alternative.
    bool discoverRecipes()
    {
        // A select or block parameter only unions the recipes of its incoming resource values.
        // Indexing descends through finite array types. Address constructors, such as
        // `GetElementPtr` and `FieldAddress`, can extend a recipe only along acyclic dependencies:
        // `isSelection` and `discoverGraph` exclude pointer-valued selection state, so an address
        // cannot be carried back around a loop to extend itself. Combination constructs one
        // sampled-image recipe from texture and sampler recipes, and projection normalization
        // removes the inverse construction. Thus loops can propagate alternatives but cannot keep
        // adding access steps, so discovery terminates.
        bool changed = true;
        while (changed)
        {
            changed = false;
            for (auto value : values)
            {
                if (value->isRoot)
                {
                    // A root recipe records the available IR value itself. The default
                    // `kIROp_Invalid` in its key distinguishes that reference from a constructor
                    // opcode; `materializeRecipe` will reuse the value without emitting an op.
                    ResourceRecipeKey key;
                    key.type = value->inst->getDataType();
                    key.operands.add(ResourceRecipeOperand::fixed(value->inst));
                    changed |= value->recipes.add(internRecipe(key));
                }
                else if (isSelection(value->inst))
                {
                    for (auto dependency : value->dependencies)
                        for (auto recipe : dependency->recipes)
                            changed |= value->recipes.add(recipe);
                }
                else
                {
                    List<Index> childRecipes;
                    changed |= addConstructorRecipes(value, 0, childRecipes);
                }
            }
        }
        // An unreachable loop can have only mutually dependent parameters and no resource root.
        // No reconstruction is possible, so we leave that graph to ordinary unreachable-code DCE.
        for (auto value : values)
            if (value->recipes.getCount() == 0)
                return false;

        return true;
    }

    /// Return whether every construction needed by a consumer can be emitted on this target path.
    ///
    /// Requires completed recipe discovery. Checks only recipes reachable from terminal consumers.
    bool canMaterializeConsumerRecipes()
    {
        // Normalization can remove an intermediate constructor that the target cannot emit. For
        // example, projecting a known pair's sampler retains the actual sampler root. We therefore
        // validate the recipes reachable from consumers, rather than rejecting every intermediate
        // recipe discovered while analyzing the graph.
        const bool emitsSPIRVDirectly =
            codeGenContext->getTargetProgram()->shouldEmitSPIRVDirectly();
        List<Index> pending;
        HashSet<Index> visited;
        for (const auto& consumer : consumers)
            for (auto argument : consumer.arguments)
                for (auto recipe : argument->recipes)
                    pending.add(recipe);
        for (Index i = 0; i < pending.getCount(); i++)
        {
            auto recipe = pending[i];
            if (!visited.add(recipe))
                continue;
            const auto& key = recipes[recipe].key;
            switch (key.op)
            {
            case kIROp_CombinedTextureSamplerGetSampler:
                // Neither Khronos path can extract a separately addressable sampler from a
                // precombined binding. Only projection normalization can support this operation.
                return false;
            case kIROp_MakeCombinedTextureSampler:
            case kIROp_CombinedTextureSamplerGetTexture:
            case kIROp_SPIRVLoadDescriptorFromHeap:
            case kIROp_ImageTexelPointer:
                // These are direct SPIR-V constructions. GLSL's standard library represents
                // its combined sampling and heap fallbacks with target intrinsics and arrays.
                // A direct call to an internal constructor can still produce this IR, so we
                // reject that graph before mutation instead of reaching an unsupported emitter.
                if (!emitsSPIRVDirectly)
                    return false;
                break;
            default:
                break;
            }
            for (const auto& operand : key.operands)
                if (operand.getFlavor() == ResourceRecipeOperand::Flavor::Resource)
                    pending.add(operand.getRecipe());
        }
        return true;
    }

    /// Assign recipe numbers within each resource type before constructing replacement selectors.
    void assignSelectorNumbers()
    {
        for (auto& entry : recipesByType)
            for (Index i = 0; i < entry.second.getCount(); i++)
                recipes[entry.second[i]].selectorNumber = i;
    }

    /// Return the Boolean or integer selector type for recipes of `resourceType` in this graph.
    IRType* getSelectorType(IRBuilder& builder, IRType* resourceType)
    {
        if (recipesByType.getValue(resourceType).getCount() <= 2)
            return builder.getBoolType();
        return builder.getUIntType();
    }

    /// Return the selector constant for `recipeIndex`, with the first Boolean alternative true.
    IRInst* getSelectorConstant(IRBuilder& builder, Index recipeIndex)
    {
        const auto& recipe = recipes[recipeIndex];
        auto type = getSelectorType(builder, recipe.key.type);
        if (type == builder.getBoolType())
            return builder.getBoolValue(recipe.selectorNumber == 0);
        return builder.getIntValue(type, recipe.selectorNumber);
    }

    /// Negate a selector condition while eliminating a generated double negation.
    IRInst* emitConditionNot(IRBuilder& builder, IRInst* condition)
    {
        // Recipe numbering can reverse a Boolean choice, and a consumer can reverse it again.
        // Folding that pair during construction keeps the original condition even when
        // optimization is disabled.
        if (condition->getOp() == kIROp_Not)
            return condition->getOperand(0);
        return builder.emitNot(builder.getBoolType(), condition);
    }

    /// Build an ordinary-value select, recovering the original condition for Boolean choices.
    ///
    /// Requires a Boolean condition and equally typed non-resource arms.
    IRInst* emitValueSelect(IRBuilder& builder, IRInst* condition, IRInst* onTrue, IRInst* onFalse)
    {
        // We intentionally perform these folds during construction. Existing SCCP cannot recover
        // `c` from a nonconstant `select(c, true, false)`. A nested use of the same condition, such
        // as `select(c, select(c, a, b), d)`, cannot choose `b` and reduces to `select(c, a, d)`.
        // Such expressions arise when a texture and sampler are both chosen using `c`. Eliminating
        // their unreachable combinations keeps the generated selector dependent on `c` directly.
        if (auto literal = as<IRBoolLit>(condition))
            return literal->getValue() ? onTrue : onFalse;
        if (auto select = as<IRSelect>(onTrue))
            if (select->getCondition() == condition)
                onTrue = select->getTrueResult();
        if (auto select = as<IRSelect>(onFalse))
            if (select->getCondition() == condition)
                onFalse = select->getFalseResult();
        if (onTrue == onFalse)
            return onTrue;
        if (auto trueLiteral = as<IRBoolLit>(onTrue))
            if (auto falseLiteral = as<IRBoolLit>(onFalse))
            {
                if (trueLiteral->getValue())
                    return condition;
                return emitConditionNot(builder, condition);
            }
        IRInst* operands[] = {condition, onTrue, onFalse};
        return builder.emitIntrinsicInst(onTrue->getDataType(), kIROp_Select, 3, operands);
    }

    /// Compare a generated selector to a recipe number without hiding an existing Boolean choice.
    IRInst* emitSelectorEquals(IRBuilder& builder, IRInst* selector, IRInst* constant)
    {
        if (as<IRConstant>(selector))
            return builder.getBoolValue(selector == constant);
        if (selector->getDataType() == builder.getBoolType())
        {
            if (cast<IRBoolLit>(constant)->getValue())
                return selector;
            return emitConditionNot(builder, selector);
        }
        if (auto select = as<IRSelect>(selector))
        {
            auto onTrue = emitSelectorEquals(builder, select->getTrueResult(), constant);
            auto onFalse = emitSelectorEquals(builder, select->getFalseResult(), constant);
            return emitValueSelect(builder, select->getCondition(), onTrue, onFalse);
        }
        return builder.emitEql(selector, constant);
    }

    /// Return one alternative's payload, using harmless ordinary defaults when that choice is
    /// absent.
    ///
    /// The caller must guard reconstruction with a selector naming `recipe`; a default for an
    /// absent alternative is valid as ordinary SSA state, not as a resource access.
    IRInst* getPayloadOrDefault(
        IRBuilder& builder,
        ResourceSelectionValue* value,
        Index recipe,
        Index i)
    {
        if (auto payload = value->payloads.tryGetValue(recipe))
            return (*payload)[i];
        return builder.emitDefaultConstruct(recipes[recipe].payloadTypes[i]);
    }

    /// Allocate the complete replacement parameter layout before any incoming argument is
    /// rewritten.
    void createReplacementParams()
    {
        for (auto value : values)
        {
            if (value->isRoot)
                continue;
            auto param = as<IRParam>(value->inst);
            if (!param)
                continue;
            IRBuilder builder(param);
            // One recipe needs no runtime choice. We keep a constant selector for the common
            // replacement-state interface; later DCE can discard it after reconstruction.
            if (value->recipes.getCount() == 1)
                value->selector = getSelectorConstant(builder, *value->recipes.begin());
            else
            {
                auto replacement =
                    builder.createParam(getSelectorType(builder, param->getDataType()));
                replacement->insertBefore(param);
                replacement->sourceLoc = param->sourceLoc;
                value->selector = replacement;
            }
            for (auto recipe : value->recipes)
            {
                auto& payload = value->payloads[recipe];
                for (auto type : recipes[recipe].payloadTypes)
                {
                    auto replacement = builder.createParam(type);
                    replacement->insertBefore(param);
                    replacement->sourceLoc = param->sourceLoc;
                    selectedPayloads.add(replacement);
                    payload.add(replacement);
                }
            }
        }
    }

    /// Build the selector decision tree for one constructor from its children's scalar selectors.
    ///
    /// Requires completed recipe discovery and dependency replacement state. `childRecipes` must
    /// contain one chosen recipe per dependency before `dependencyIndex` and is restored on return.
    /// Emits only ordinary selects, not resource accesses.
    IRInst* emitConstructorSelector(
        IRBuilder& builder,
        ResourceSelectionValue* value,
        Index dependencyIndex,
        List<Index>& childRecipes)
    {
        // We extend the chosen prefix to obtain each possible constructor recipe, then use the
        // dependencies' selectors to choose its number. Each dependency selects one of its
        // analyzed alternatives, so its first alternative can be the default while later matches
        // override that choice.
        if (dependencyIndex == value->dependencies.getCount())
            return getSelectorConstant(builder, getConstructorRecipe(value, childRecipes));
        auto dependency = value->dependencies[dependencyIndex];
        IRInst* result = nullptr;
        for (auto recipe : dependency->recipes)
        {
            childRecipes.add(recipe);
            auto alternative =
                emitConstructorSelector(builder, value, dependencyIndex + 1, childRecipes);
            childRecipes.removeLast();
            if (!result)
                result = alternative;
            else
            {
                auto condition = emitSelectorEquals(
                    builder,
                    dependency->selector,
                    getSelectorConstant(builder, recipe));
                result = emitValueSelect(builder, condition, alternative, result);
            }
        }
        SLANG_RELEASE_ASSERT(result);
        return result;
    }

    /// Build payloads for every recipe produced by a constructor's resource-operand alternatives.
    ///
    /// Requires completed dependency replacement state. `childRecipes` must contain one chosen
    /// recipe per dependency before `dependencyIndex` and is restored on return. Populates
    /// `value->payloads` using the flattened layouts from `internRecipe`.
    void addConstructorPayloads(
        IRBuilder& builder,
        ResourceSelectionValue* value,
        Index dependencyIndex,
        List<Index>& childRecipes)
    {
        // As in recipe discovery, recursion enumerates the remaining dependency alternatives and
        // restores the caller's chosen prefix after each alternative.
        if (dependencyIndex != value->dependencies.getCount())
        {
            for (auto recipe : value->dependencies[dependencyIndex]->recipes)
            {
                childRecipes.add(recipe);
                addConstructorPayloads(builder, value, dependencyIndex + 1, childRecipes);
                childRecipes.removeLast();
            }
            return;
        }
        // For a complete combination, we concatenate child payloads in resource-operand order,
        // then append the constructor's own ordinary operands. This agrees with the layout from
        // `internRecipe`; fixed metadata occupies no payload slot. A normalized projection may
        // retain only one child's payload, which we handle before appending ordinary operands.
        auto recipeIndex = getConstructorRecipe(value, childRecipes);
        List<IRInst*> payload;
        for (Index i = 0; i < value->dependencies.getCount(); i++)
            payload.addRange(value->dependencies[i]->payloads.getValue(childRecipes[i]));

        auto inst = value->inst;
        // Projection normalization drops the unused half of a known combined resource. Its
        // payload must follow the projected recipe too, rather than retaining both halves.
        bool isProjection = inst->getOp() == kIROp_CombinedTextureSamplerGetTexture;
        if (inst->getOp() == kIROp_CombinedTextureSamplerGetSampler)
            isProjection = true;
        if (isProjection)
        {
            const auto& combined = recipes[childRecipes[0]];
            if (combined.key.op == kIROp_MakeCombinedTextureSampler)
            {
                Index offset = 0;
                if (inst->getOp() == kIROp_CombinedTextureSamplerGetSampler)
                    offset = recipes[combined.key.operands[0].getRecipe()].payloadTypes.getCount();
                List<IRInst*> projected;
                for (Index i = 0; i < recipes[recipeIndex].payloadTypes.getCount(); i++)
                    projected.add(payload[offset + i]);
                payload = _Move(projected);
            }
        }
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            if (isResourceOperand(inst, i))
                continue;
            auto operand = inst->getOperand(i);
            if (isDescriptorIndex(inst, i))
                payload.add(builder.emitCast(commonIndexType, operand));
            else if (!isFixedOperand(inst, i))
                payload.add(operand);
        }
        SLANG_RELEASE_ASSERT(payload.getCount() == recipes[recipeIndex].payloadTypes.getCount());
        if (auto existing = value->payloads.tryGetValue(recipeIndex))
        {
            // Projecting two combined choices can produce one array recipe with different
            // indices. Consider this example:
            //
            //     Texture2D<float4> textures[8];
            //     SamplerState s0, s1;
            //     float4 loadPair(bool c, uint i, uint j)
            //     {
            //         Sampler2D<float4> p = select(c,
            //             __makeCombinedTextureSampler(textures[i], s0),
            //             __makeCombinedTextureSampler(textures[j], s1));
            //         return p.Load(int3(0));
            //     }
            //
            // The SPIR-V implementation of combined `Load` extracts the image with
            // `CombinedTextureSamplerGetTexture`. Normalization finds the same texture-array
            // recipe in both alternatives, but their index payloads still differ. We merge those
            // payloads with the combined selector so the fetch uses the selected `i` or `j`.
            SLANG_RELEASE_ASSERT(isProjection);
            auto condition = emitSelectorEquals(
                builder,
                value->dependencies[0]->selector,
                getSelectorConstant(builder, childRecipes[0]));
            for (Index i = 0; i < payload.getCount(); i++)
            {
                auto replacement = emitValueSelect(builder, condition, payload[i], (*existing)[i]);
                selectedPayloads.add(replacement);
                (*existing)[i] = replacement;
            }
        }
        else
            value->payloads.add(recipeIndex, payload);
    }

    /// Emit acyclic replacement calculations after all parameter replacements have been allocated.
    ///
    /// Requires completed recipe discovery, selector numbering, and replacement-parameter
    /// allocation. Completes the selector and payload state for every graph value.
    void emitReplacementValues()
    {
        // Discovery order need not place every dependency before its use, so we repeatedly scan
        // the graph values and complete whichever ones have ready dependencies. Between visits,
        // a non-null `selector` means both the selector and every recipe payload have been
        // created. Within one visit, we may assign the selector first, but we fill all payloads
        // before advancing; no dependent visit observes that partially built state.
        //
        // `createReplacementParams` has allocated the complete ordinary state for the graph's
        // non-entry resource block parameters. Those replacements break loop dependencies. Every
        // other value can be completed once its dependencies' states exist, justifying the
        // progress check below.
        Index remaining = 0;
        for (auto value : values)
            if (!value->selector)
                remaining++;
        while (remaining)
        {
            Index previousRemaining = remaining;
            for (auto value : values)
            {
                if (value->selector)
                    continue;
                bool dependenciesReady = true;
                for (auto dependency : value->dependencies)
                    if (!dependency->selector)
                        dependenciesReady = false;
                if (!dependenciesReady)
                    continue;

                IRBuilder builder(value->inst);
                if (!value->isRoot)
                    builder.setInsertBefore(value->inst);
                if (value->isRoot)
                {
                    auto recipe = *value->recipes.begin();
                    value->selector = getSelectorConstant(builder, recipe);
                    value->payloads[recipe] = List<IRInst*>();
                }
                else if (auto select = as<IRSelect>(value->inst))
                {
                    auto onTrue = value->dependencies[0];
                    auto onFalse = value->dependencies[1];
                    value->selector = emitValueSelect(
                        builder,
                        select->getCondition(),
                        onTrue->selector,
                        onFalse->selector);
                    for (auto recipe : value->recipes)
                    {
                        auto& payload = value->payloads[recipe];
                        for (Index i = 0; i < recipes[recipe].payloadTypes.getCount(); i++)
                        {
                            auto truePayload = getPayloadOrDefault(builder, onTrue, recipe, i);
                            auto falsePayload = getPayloadOrDefault(builder, onFalse, recipe, i);
                            auto replacement = emitValueSelect(
                                builder,
                                select->getCondition(),
                                truePayload,
                                falsePayload);
                            if (replacement != truePayload)
                                if (replacement != falsePayload)
                                    selectedPayloads.add(replacement);
                            payload.add(replacement);
                        }
                    }
                }
                else
                {
                    List<Index> childRecipes;
                    value->selector = emitConstructorSelector(builder, value, 0, childRecipes);
                    addConstructorPayloads(builder, value, 0, childRecipes);
                }
                remaining--;
            }
            // Only parameter edges can close a resource-value cycle. Their replacement values were
            // allocated before this loop, so every remaining acyclic dependency must progress.
            SLANG_RELEASE_ASSERT(remaining < previousRemaining);
        }
    }

    /// Move a selected resource's name to its ordinary state without naming constants or roots.
    void transferSelectedStateNames()
    {
        // For the same-array optimization, the selected index is in the first payload. For
        // choices between different recipes, the selected recipe is identified by the selector.
        // We move the original name hint to that ordinary value so an IR dump still identifies
        // the state corresponding to the programmer's resource variable, including across loop
        // iterations.
        for (auto value : values)
        {
            if (value->isRoot)
                continue;
            auto nameHint = value->inst->findDecoration<IRNameHintDecoration>();
            if (!nameHint)
                continue;
            auto replacement = value->selector;
            if (value->recipes.getCount() == 1)
            {
                auto recipe = *value->recipes.begin();
                const auto& payload = value->payloads.getValue(recipe);
                if (payload.getCount())
                    replacement = payload[0];
            }
            if (as<IRConstant>(replacement))
                continue;
            if (replacement->findDecoration<IRNameHintDecoration>())
                continue;
            nameHint->removeFromParent();
            nameHint->insertAtStart(replacement);
        }
    }

    /// Replace each resource parameter's argument slot with its selector and payload arguments.
    ///
    /// Requires completed replacement state. Preserves ordinary arguments and structural loop
    /// operands.
    void rewriteBranchArguments()
    {
        // We rebuild complete branch argument lists from the saved parameter lists. That expands
        // each resource slot exactly once, even when two parameters received the same incoming
        // resource value, while preserving ordinary arguments and structural loop operands.
        for (auto& entry : originalParams)
        {
            List<IRBlock*> predecessors;
            for (auto predecessor : entry.first->getPredecessors())
                predecessors.add(predecessor);
            for (auto predecessor : predecessors)
            {
                auto branch = cast<IRUnconditionalBranch>(predecessor->getTerminator());
                IRBuilder builder(branch);
                builder.setInsertBefore(branch);
                List<IRInst*> arguments;
                for (Index i = 0; i < entry.second.getCount(); i++)
                {
                    auto mapped = valueMap.tryGetValue(entry.second[i]);
                    if (!mapped)
                    {
                        arguments.add(branch->getArg((UInt)i));
                        continue;
                    }
                    auto destination = *mapped;
                    auto incoming = valueMap.getValue(branch->getArg((UInt)i));
                    if (destination->recipes.getCount() > 1)
                        arguments.add(incoming->selector);
                    for (auto recipe : destination->recipes)
                        for (Index j = 0; j < recipes[recipe].payloadTypes.getCount(); j++)
                            arguments.add(getPayloadOrDefault(builder, incoming, recipe, j));
                }
                List<IRInst*> operands;
                auto fixedCount = branch->getOperandCount() - branch->getArgCount();
                for (UInt i = 0; i < fixedCount; i++)
                    operands.add(branch->getOperand(i));
                operands.addRange(arguments);
                auto replacement = builder.emitIntrinsicInst(
                    branch->getFullType(),
                    branch->getOp(),
                    (UInt)operands.getCount(),
                    operands.getBuffer());
                replacement->sourceLoc = branch->sourceLoc;
                branch->transferDecorationsTo(replacement);
                branch->removeAndDeallocate();
            }
        }
    }

    /// Emit the construction for one recipe using its ordinary payload values.
    ///
    /// Requires target validation by `canMaterializeConsumerRecipes` and payload values that
    /// dominate the builder's insertion point. `payloadIndex` identifies the first unread slot
    /// and advances by the number of payload values consumed by this recipe.
    IRInst* materializeRecipe(
        IRBuilder& builder,
        Index recipeIndex,
        const List<IRInst*>& payload,
        Index& payloadIndex)
    {
        // Each recursive child consumes its flattened payload slots before the following
        // operands, so we pass one shared payload and cursor through the recipe tree. Replacement
        // calculations and block parameters establish payload availability before consumer
        // rewriting.
        // Recipe discovery has finished, so recursive materialization cannot invalidate the key.
        const auto& key = recipes[recipeIndex].key;
        if (key.op == kIROp_Invalid)
            // `discoverGraph` accepts only module parameters and entry parameters as roots.
            // Their values are available at every reconstruction, so no instruction is needed.
            return key.operands[0].getFixedValue();
        List<IRInst*> operands;
        for (const auto& operand : key.operands)
        {
            switch (operand.getFlavor())
            {
            case ResourceRecipeOperand::Flavor::None:
                SLANG_UNEXPECTED("A recipe operand must be initialized before materialization.");
            case ResourceRecipeOperand::Flavor::Resource:
                operands.add(
                    materializeRecipe(builder, operand.getRecipe(), payload, payloadIndex));
                break;
            case ResourceRecipeOperand::Flavor::Payload:
                operands.add(payload[payloadIndex++]);
                break;
            case ResourceRecipeOperand::Flavor::Fixed:
                operands.add(operand.getFixedValue());
                break;
            }
        }
        if (key.op == kIROp_GetElement)
        {
            // Selecting between array indices can introduce nonuniform indexing even when
            // each original index was constant. Unselected ordinary accesses keep their marker
            // policy; visiting a shared array root does not add capabilities to every access.
            if (selectedPayloads.contains(operands[1]))
                operands[1] = builder.emitNonUniformResourceIndexInst(operands[1]);
        }
        return builder
            .emitIntrinsicInst(key.type, key.op, (UInt)operands.getCount(), operands.getBuffer());
    }

    /// Return the recipe identified by one constant selector in a graph value's alternative set.
    Index findRecipeForConstant(IRBuilder& builder, ResourceSelectionValue* value, IRInst* constant)
    {
        for (auto recipe : value->recipes)
            if (getSelectorConstant(builder, recipe) == constant)
                return recipe;
        SLANG_UNEXPECTED("A resource selector must name one of its analyzed alternatives.");
    }

    /// Clone an operation with concrete resource arguments, dispatching each independent choice.
    ///
    /// Requires completed replacement state and validated recipes. `environment` must map the
    /// arguments before `argumentIndex` to their materialized resources. Returns the cloned
    /// operation or the block parameter merging its result; a conditional operation without a
    /// result returns null. Leaves the builder in the final merge block when dispatch is emitted,
    /// or at the insertion point after the cloned operation otherwise.
    IRInst* emitConsumer(
        IRBuilder& builder,
        ResourceSelectionConsumer& consumer,
        const List<ResourceSelectionValue*>& arguments,
        Index argumentIndex,
        IRCloneEnv& environment)
    {
        // Each recursive level handles one resource argument, not another path through the
        // original selection graph. A repeated argument is registered only once. Independently
        // selected arguments can require a Cartesian product, but a selected combined pair
        // remains one argument and therefore preserves its joint choice.
        //
        // Each branch receives a child clone environment so choosing a resource in one arm
        // cannot change the mappings used by another arm.
        if (argumentIndex == arguments.getCount())
            return cloneInst(&environment, &builder, consumer.inst);
        auto value = arguments[argumentIndex];
        if (value->recipes.getCount() == 1)
        {
            auto recipe = *value->recipes.begin();
            Index payloadIndex = 0;
            auto resource =
                materializeRecipe(builder, recipe, value->payloads.getValue(recipe), payloadIndex);
            environment.mapOldValToNew[value->inst] = resource;
            return emitConsumer(builder, consumer, arguments, argumentIndex + 1, environment);
        }

        List<Index> alternatives;
        IRInst* condition = nullptr;
        // A constructed pair can have a larger abstract alternative set but only two reachable
        // choices when both components used the same condition. Scalar-select construction has
        // already removed the impossible combinations; we dispatch its two constant arms directly.
        if (auto select = as<IRSelect>(value->selector))
        {
            if (as<IRConstant>(select->getTrueResult()))
                if (as<IRConstant>(select->getFalseResult()))
                {
                    alternatives.add(
                        findRecipeForConstant(builder, value, select->getTrueResult()));
                    alternatives.add(
                        findRecipeForConstant(builder, value, select->getFalseResult()));
                    condition = select->getCondition();
                }
        }
        if (alternatives.getCount() == 0)
        {
            for (auto recipe : value->recipes)
                alternatives.add(recipe);
            if (alternatives.getCount() == 2)
                condition = emitSelectorEquals(
                    builder,
                    value->selector,
                    getSelectorConstant(builder, alternatives[0]));
        }

        auto after = builder.createBlock();
        func->addBlock(after);
        List<IRBlock*> arms;
        for (Index i = 0; i < alternatives.getCount(); i++)
        {
            auto arm = builder.createBlock();
            func->addBlock(arm);
            arms.add(arm);
        }
        if (condition)
            builder.emitIfElse(condition, arms[0], arms[1], after);
        else
        {
            // The generated selector identifies one of the analyzed alternatives. After the
            // explicit case tests fail, it must identify the last alternative, so that arm can
            // be the default without an additional invalid-selector path.
            List<IRInst*> cases;
            for (Index i = 0; i + 1 < alternatives.getCount(); i++)
            {
                cases.add(getSelectorConstant(builder, alternatives[i]));
                cases.add(arms[i]);
            }
            builder.emitSwitch(
                value->selector,
                after,
                arms.getLast(),
                (UInt)cases.getCount(),
                cases.getBuffer());
        }

        auto resultType = consumer.inst->getDataType();
        // Stores have no result type, while a void-returning call has `VoidType`. Both
        // execute in their selected arm and reach the merge without a result argument.
        bool hasResult = false;
        if (resultType)
            hasResult = resultType->getOp() != kIROp_VoidType;
        for (Index i = 0; i < alternatives.getCount(); i++)
        {
            builder.setInsertInto(arms[i]);
            auto recipe = alternatives[i];
            Index payloadIndex = 0;
            auto resource =
                materializeRecipe(builder, recipe, value->payloads.getValue(recipe), payloadIndex);
            IRCloneEnv armEnvironment;
            armEnvironment.parent = &environment;
            armEnvironment.mapOldValToNew[value->inst] = resource;
            auto result =
                emitConsumer(builder, consumer, arguments, argumentIndex + 1, armEnvironment);
            if (hasResult)
                builder.emitBranch(after, 1, &result);
            else
                builder.emitBranch(after);
        }
        builder.setInsertInto(after);
        if (hasResult)
            return builder.emitParam(resultType);
        return nullptr;
    }

    /// Replace one terminal operation at its execution point and merge only its ordinary result.
    void rewriteConsumer(ResourceSelectionConsumer& consumer)
    {
        bool requiresDispatch = false;
        List<ResourceSelectionValue*> arguments;
        for (auto argument : consumer.arguments)
        {
            arguments.add(argument);
            if (argument->recipes.getCount() > 1)
                requiresDispatch = true;
        }
        if (requiresDispatch)
        {
            // Introducing divergent execution can change implicit derivatives. We still perform
            // resource legalization, including for implicit samples, and issue a targeted warning.
            // Speculatively executing all arms would access unselected descriptors and cannot be
            // used for writes. A future extension can compute gradients before dispatch for
            // recognized sampling operations; bias, comparison, and LOD queries need their own
            // semantic lowering. No purity annotation establishes derivative independence.
            diagnoseResourceSelectionDispatch(codeGenContext, consumer.inst);
        }
        auto original = consumer.inst;
        auto block = cast<IRBlock>(original->getParent());
        IRBuilder builder(original);
        IRBlock* temporaryContinuation = nullptr;
        if (requiresDispatch)
        {
            // We split at the operation, after all ordinary argument calculations. The original
            // continuation remains shared by all arms, so no unrelated work is duplicated.
            auto continuation = builder.createBlock();
            func->addBlock(continuation);
            temporaryContinuation = continuation;
            for (auto inst = original; inst;)
            {
                auto next = inst->getNextInst();
                inst->insertAtEnd(continuation);
                inst = next;
            }
            builder.setInsertInto(block);
            IRCloneEnv environment;
            auto replacement = emitConsumer(builder, consumer, arguments, 0, environment);
            if (replacement)
                original->replaceUsesWith(replacement);

            // The final dispatch merge already dominates the continuation. We append the
            // remaining instructions to that merge rather than adding an empty forwarding
            // block and branch, so the simple case matches a hand-written conditional at -O0.
            auto merge = builder.getBlock();
            for (auto inst = original->getNextInst(); inst;)
            {
                auto next = inst->getNextInst();
                inst->insertAtEnd(merge);
                inst = next;
            }
        }
        else
        {
            builder.setInsertBefore(original);
            IRCloneEnv environment;
            auto replacement = emitConsumer(builder, consumer, arguments, 0, environment);
            if (replacement)
                original->replaceUsesWith(replacement);
        }
        original->removeAndDeallocate();
        if (temporaryContinuation)
            temporaryContinuation->removeAndDeallocate();
    }

    /// Delete replaced instructions after their external uses are rewritten, retaining roots.
    void removeResourceGraph()
    {
        // Selection cycles can still reference each other even after all executable consumers
        // have been replaced. We unlink operands of every replaced instruction before deallocating
        // any of them, so deleting a loop parameter cannot leave a dangling use in another node.
        for (auto value : values)
        {
            if (value->isRoot)
                continue;
            for (UInt i = 0; i < value->inst->getOperandCount(); i++)
                value->inst->setOperand(i, nullptr);
        }
        for (auto value : values)
            if (!value->isRoot)
                value->inst->removeAndDeallocate();
    }
};
} // namespace

bool specializeResourceSelections(CodeGenContext* codeGenContext, IRModule* module)
{
    // Resource-array indexing and the emitted reconstruction operations obey Khronos rules.
    // WGSL requires separate validation of its resource-array and uniformity restrictions.
    if (!isKhronosTarget(codeGenContext->getTargetReq()))
        return false;
    bool changed = false;
    for (auto global : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(global);
        if (!func)
            continue;
        // Generic assembly replaces the emitted function implementation. Ordinary dispatch
        // blocks inserted into such a function would not define the behavior of that assembly.
        // Its conditional resource computations remain intact for the other steps in
        // `specializeResourceUsage` and the target's later unsupported-instruction checks.
        if (hasGenericAssemblyImplementation(func))
            continue;
        OrderedHashSet<IRInst*> candidates;
        for (auto block : func->getBlocks())
            for (auto inst : block->getChildren())
                if (ResourceSelectionContext::isSelection(inst))
                    candidates.add(inst);
        while (candidates.getCount())
        {
            auto candidate = *candidates.begin();
            candidates.remove(candidate);
            ResourceSelectionContext context;
            context.codeGenContext = codeGenContext;
            context.func = func;
            if (!context.tryRewrite(candidate))
                continue;
            // The component rewrite can remove several saved candidates. We erase their
            // addresses before the next iteration; no later candidate dereferences deleted IR.
            for (auto value : context.values)
                candidates.remove(value->inst);
            changed = true;
        }
    }
    return changed;
}
} // namespace Slang
