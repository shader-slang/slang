# _Callable_ Shader Contract

Status: supporting analysis for [PROPOSAL.md](PROPOSAL.md). The proposal is normative.

_Callable_ shaders form an independent, dynamically indexed SBT section. They are not selected by
ray traversal and do not belong to a payload partition.

## Source model

A callable context fixes the trace context, callable-data ABI, and per-record data:

```slang
struct MaterialCallableContext : rt::ICallableContext
{
    typealias TraceContext = SceneTraceContext;
    typealias CallableData = MaterialCallData;
    typealias Record = MaterialRecord;
}

struct ShadeMaterial : rt::ICallableShader
{
    typealias Context = MaterialCallableContext;

    void invoke(rt::CallableInput<Context> input)
    {
        input.data.result *= input.record.factor;
    }
}
```

The schema lists the stage directly:

```slang
struct SceneSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = SceneTraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::CallableShaderList<ShadeMaterial>;
}
```

There is no callable-group wrapper and no shader-side slot. The compiler assigns each finalized
callable a schema-wide dense function index. The host may place one callable in any number of
physical callable records with different `Context.Record` values.

## Invocation and type rule

```slang
rt::RayTracer<SceneSchema> tracer;
tracer.callShader<MaterialCallableContext>(callableRecordIndex, descriptor, data);
```

The argument is a physical callable-record index, not a function index. The selected record names
the function index and supplies `input.record`.

Every callable in one completed schema must use the same `CallableData` type because a dynamic
index selects one table with one native callable-data ABI. Record types may differ because the
selected record identifies its own shape. The compiler diagnoses a mismatched callable-data type
at schema completion or at `callShader`.

`CallableData` must be a fixed-size, copyable plain-data type and may not be `void`. An empty
concrete struct is allowed; target lowering provides physical storage where the native ABI requires
an addressable value. `Record = void` is allowed.

The type rule cannot prove that a runtime index is in range or points at the application-intended
record. Those remain host-data invariants.

## Target mapping

| Target | Lowering |
| --- | --- |
| D3D | Native callable entry points, callable SBT records, and `CallShader` |
| Vulkan | Native callable entry points, callable SBT records, and `OpExecuteCallableKHR` |
| OptiX | Native callable entry point and callable SBT-data lowering |
| Metal | One schema-wide visible function table plus generated record-buffer lookup |

Metal resolves the physical callable record, reads its function index, passes its application data
pointer to the selected visible function, and uses the same descriptor for nested structural
dispatch.

_Callable_ dispatch is valid only from stages whose target capability permits it. A generated Metal
helper does not make an otherwise invalid source-stage call legal.

Use an ordinary function when the callee is statically known. Use a callable shader when shader
code must choose an SBT record dynamically.
