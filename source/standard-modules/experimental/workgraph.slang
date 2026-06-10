/**
Experimental work graph APIs for Slang.

@remarks EXPERIMENTAL: This module is under active design and may change
significantly or be removed in future versions. DO NOT USE IN PRODUCTION.

@category workgraph
*/
[ExperimentalModule]
module workgraph;

// Work Graph Node Shader attributes

/// Specifies the launch mode for a work-graph node shader.
/// @param mode Case-insensitive; one of "broadcasting", "thread", or "coalescing".
__attributeTarget(FuncDecl)
attribute_syntax [NodeLaunch(mode: String)] : NodeLaunchAttribute;

/// Specifies the maximum dispatch grid for a broadcasting node.
__attributeTarget(FuncDecl)
attribute_syntax [NodeMaxDispatchGrid(x: int, y: int, z: int)] : NodeMaxDispatchGridAttribute;

/// Specifies a fixed dispatch grid for a broadcasting node.
__attributeTarget(FuncDecl)
attribute_syntax [NodeDispatchGrid(x: int, y: int, z: int)] : NodeDispatchGridAttribute;

/// Specifies the maximum record count for a work-graph node.
/// On a node output parameter, this is the maximum number of output records.
/// On a coalescing node input parameter, this is the maximum number of input records.
/// On a node function, this sets the node-level max-records contract.
__attributeTarget(FuncDecl)
__attributeTarget(ParamDecl)
attribute_syntax [MaxRecords(count: int)] : MaxRecordsAttribute;

/// Overrides the node identifier (name and optional array index) used in the work graph.
/// The second argument (array index) is optional and defaults to 0.
__attributeTarget(FuncDecl)
__attributeTarget(ParamDecl)
attribute_syntax [NodeID(name: String, arrayIndex: int = 0)] : NodeIDAttribute;

/// Marks this node shader as a program entry point in the work graph.
__attributeTarget(FuncDecl)
attribute_syntax [NodeIsProgramEntry] : NodeIsProgramEntryAttribute;

/// Specifies the number of nodes in a node output array.
__attributeTarget(ParamDecl)
attribute_syntax [NodeArraySize(count: int)] : NodeArraySizeAttribute;

/// Allows a node output array to contain unresolved (sparse) entries.
__attributeTarget(ParamDecl)
attribute_syntax [AllowSparseNodes] : AllowSparseNodesAttribute;

/// Read-only input to a broadcasting-launch node shader.
__generic<T>
[require(hlsl)]
__intrinsic_type(DispatchNodeInputRecord)
public struct DispatchNodeInputRecord
{
    /// Returns the input record value.
    public T Get()
    {
        __target_switch
        {
        case hlsl:
            __intrinsic_asm ".Get";
        }
    }
};

/// Read-only input to a thread-launch node shader.
__generic<T>
[require(hlsl)]
__intrinsic_type(ThreadNodeInputRecord)
public struct ThreadNodeInputRecord
{
    public T Get()
    {
        __target_switch
        {
        case hlsl:
            __intrinsic_asm ".Get";
        }
    }
};

/// Read-only input array for a coalescing-launch node shader.
__generic<T>
[require(hlsl)]
__intrinsic_type(GroupNodeInputRecords)
public struct GroupNodeInputRecords
{
    public uint Count()
    {
        __target_switch
        {
        case hlsl:
            __intrinsic_asm ".Count";
        }
    }

    public T Get(uint index)
    {
        __target_switch
        {
        case hlsl:
            __intrinsic_asm ".Get";
        }
    }

    public __subscript(uint index) -> T
    {
        get { return Get(index); }
    }
};

[require(hlsl)]
__intrinsic_type(EmptyNodeInput)
public struct EmptyNodeInput
{
    public uint Count() { __intrinsic_asm ".Count"; }
};

/// Per-thread output records for node shader output; call OutputComplete() when done writing.
__generic<T>
[require(hlsl)]
__intrinsic_type(ThreadNodeOutputRecords)
public struct ThreadNodeOutputRecords
{
    public Ref<T> Get(uint index = 0)
    {
        __target_switch
        {
        case hlsl:
            __intrinsic_asm ".Get";
        }
    }

    public __subscript(uint index) -> T
    {
        [nonmutating]
        __intrinsic_op(nodeOutputRecordGetElementPtr)
        ref;
    }

    public void OutputComplete()
    {
        __target_switch
        {
        case hlsl:
            __intrinsic_asm ".OutputComplete";
        }
    }
};

/// Per-group (shared) output records for node shader output; call OutputComplete()
/// when done writing.
__generic<T>
[require(hlsl)]
__intrinsic_type(GroupNodeOutputRecords)
public struct GroupNodeOutputRecords
{
    public Ref<T> Get(uint index = 0)
    {
        __target_switch
        {
        case hlsl:
            __intrinsic_asm ".Get";
        }
    }

    public __subscript(uint index) -> T
    {
        [nonmutating]
        __intrinsic_op(nodeOutputRecordGetElementPtr)
        ref;
    }

    public void OutputComplete()
    {
        __target_switch
        {
        case hlsl:
            __intrinsic_asm ".OutputComplete";
        }
    }
};

/// Output node handle; use to obtain output record objects for the named output node.
__generic<T>
[require(hlsl)]
__intrinsic_type(NodeOutput)
public struct NodeOutput
{
    public bool IsValid()
    {
        __target_switch
        {
        case hlsl:
            __intrinsic_asm ".IsValid";
        }
    }

    public ThreadNodeOutputRecords<T> GetThreadNodeOutputRecords(uint count)
    {
        __target_switch
        {
        case hlsl:
            __intrinsic_asm ".GetThreadNodeOutputRecords";
        }
    }

    public GroupNodeOutputRecords<T> GetGroupNodeOutputRecords(uint count)
    {
        __target_switch
        {
        case hlsl:
            __intrinsic_asm ".GetGroupNodeOutputRecords";
        }
    }
};

/// Array of output node handles indexed by node array index.
__generic<T>
[require(hlsl)]
__intrinsic_type(NodeOutputArray)
public struct NodeOutputArray
{
    public NodeOutput<T> Get(uint index)
    {
        __target_switch
        {
        case hlsl:
            __intrinsic_asm ".operator[]";
        }
    }

    public __subscript(uint index) -> NodeOutput<T>
    {
        get { return Get(index); }
    }
};

[require(hlsl)]
__intrinsic_type(EmptyNodeOutput)
public struct EmptyNodeOutput
{
    public bool IsValid() { __intrinsic_asm ".IsValid"; }

    public void ThreadIncrementOutputCount(uint count)
    {
        __intrinsic_asm ".ThreadIncrementOutputCount";
    }

    public void GroupIncrementOutputCount(uint count)
    {
        __intrinsic_asm ".GroupIncrementOutputCount";
    }

    public void OutputComplete() { __intrinsic_asm ".OutputComplete"; }
};

[require(hlsl)]
__intrinsic_type(EmptyNodeOutputArray)
public struct EmptyNodeOutputArray
{
    public EmptyNodeOutput Get(uint index) { __intrinsic_asm ".operator[]"; }

    public __subscript(uint index) -> EmptyNodeOutput
    {
        get { return Get(index); }
    }
};

/// Memory type flags for the work-graph Barrier() intrinsic (SM 6.8, first argument).
/// Keep these numeric values in sync with source/slang/slang-type-system-shared.h.
/// That C++ header pins the mirrored values with static_assert checks.
public enum BarrierMemoryTypeFlags : uint
{
    UAV_MEMORY          = 0x01,
    GROUP_SHARED_MEMORY = 0x02,
    NODE_INPUT_MEMORY   = 0x04,
    NODE_OUTPUT_MEMORY  = 0x08,
    ALL_MEMORY          = 0x0f,
}

/// Semantic flags for the work-graph Barrier() intrinsic (SM 6.8, second argument).
/// Keep these numeric values in sync with source/slang/slang-type-system-shared.h.
/// That C++ header pins the mirrored values with static_assert checks.
public enum BarrierSemanticFlags : uint
{
    REORDER      = 0x00,
    GROUP_SYNC   = 0x01,
    GROUP_SCOPE  = 0x02,
    DEVICE_SCOPE = 0x04,
}

/// Maps a compile-time BarrierMemoryTypeFlags value to its target named-constant expression
/// (e.g. "(UAV_MEMORY)" or "(UAV_MEMORY | NODE_INPUT_MEMORY)"). Handled by the
/// HLSL emitter via kIROp_GetEnumBarrierMemoryTypeFlags; must only be called for HLSL.
__intrinsic_op(getEnumBarrierMemoryTypeFlags)
int GetEnumBarrierMemoryTypeFlags(constexpr BarrierMemoryTypeFlags f);

/// Maps a compile-time BarrierSemanticFlags value to its target named-constant expression
/// (e.g. "(GROUP_SYNC)" or "(DEVICE_SCOPE | GROUP_SCOPE)"). Handled by the
/// HLSL emitter via kIROp_GetEnumBarrierSemanticFlags; must only be called for HLSL.
__intrinsic_op(getEnumBarrierSemanticFlags)
int GetEnumBarrierSemanticFlags(constexpr BarrierSemanticFlags f);

[require(hlsl, sm_6_8)]
void __Barrier_hlsl(int memoryTypeFlags, int semanticFlags)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "Barrier";
    }
}

/// Work-graph Barrier intrinsic (SM 6.8).
[ForceInline]
[require(hlsl, sm_6_8)]
public void Barrier(
    constexpr BarrierMemoryTypeFlags memoryTypeFlags,
    constexpr BarrierSemanticFlags semanticFlags)
{
    __target_switch
    {
    case hlsl:
        __Barrier_hlsl(
            GetEnumBarrierMemoryTypeFlags(memoryTypeFlags),
            GetEnumBarrierSemanticFlags(semanticFlags));
    }
}
