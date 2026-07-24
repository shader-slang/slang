// Regression test for https://github.com/shader-slang/slang/issues/12197
//
// `HitObject` is the other opaque non-copyable handle NRVO fires on (isNonCopyableOpaqueType covers
// both IRRayQueryType and IRHitObjectType). A function that builds a `HitObject` into a named local
// and returns it by value must not lower that return as an opaque whole-handle copy — the same
// defect that disconnected the ray query. This exercises the `decl->initExpr` branch of the NRVO
// install (the local is declared with an initializer, `HitObject h = ...`).
//
// The SER HitObject intrinsics cause `makeHit` to be inlined, so the check is at the caller: there
// must be exactly ONE hit-object variable, the record op targets it, and the value the caller reads
// (`IsEmpty`) is that same variable — with no whole-handle load/store/copy of the HitObject between
// them. Before the fix the by-value return produced a second, unrecorded hit-object.
// (`HitObject` cannot run on the CPU, so this pins the SPIR-V lowering shape.)

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -stage raygeneration -entry main -O0 -capability spvShaderInvocationReorderNV

RWStructuredBuffer<uint> g_out;

HitObject makeHit()
{
    HitObject h = HitObject::MakeNop();
    return h;
}

[shader("raygeneration")]
void main()
{
    HitObject h = makeHit();
    g_out[0] = uint(h.IsNop());
}

// One hit-object variable; the record op writes it and the query reads that same variable, with no
// intervening whole-handle copy.
// CHECK: [[H:%[A-Za-z0-9_]+]] = OpVariable %{{[A-Za-z0-9_]+}} Function
// CHECK-NOT: OpVariable %{{[A-Za-z0-9_]+}} Function
// CHECK: OpHitObjectRecordEmptyNV [[H]]
// CHECK-NOT: OpCopyMemory
// CHECK: OpHitObjectIsEmptyNV %{{[A-Za-z0-9_]+}} [[H]]
