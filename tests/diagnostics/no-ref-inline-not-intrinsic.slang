// `[noRefInline]` opts a function out of the reference-parameter force-inlining
// pass. A non-intrinsic function that keeps its definition cannot faithfully
// express a `__ref` parameter on every target (HLSL emits the closest form
// `inout`, which copies and loses the aliasing `__ref` promises), so the
// attribute only applies to intrinsic declarations, whose bodies are never
// emitted as target function definitions. The `bump` case carries `[noinline]`
// as well: even with both attributes an ordinary function cannot keep a
// `__ref` call boundary, so `[noRefInline]` is rejected regardless. The
// `gsWrite` case also pins the removal that follows the rejection: the
// attribute is gone, so the call takes the default force-inlining path and
// the groupshared parameter never reaches the HLSL inlining check (exactly
// the two E31002 rows below, and no E30708 for `gsWrite`).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> buf;

[noRefInline]
/*CHECK:
 ^^^^^^^^^^^ invalid attribute placement
 ^^^^^^^^^^^ attribute 'noRefInline' is not valid here
*/
[noinline] void bump(__ref int x) { x += 1; }

// An empty body is not an intrinsic definition.
[noRefInline]
/*CHECK:
 ^^^^^^^^^^^ invalid attribute placement
 ^^^^^^^^^^^ attribute 'noRefInline' is not valid here
*/
void emptyBody(__ref int x) {}

// A `default:` arm with real code makes the function non-intrinsic even though
// one arm is intrinsic assembly.
[noRefInline]
/*CHECK:
 ^^^^^^^^^^^ invalid attribute placement
 ^^^^^^^^^^^ attribute 'noRefInline' is not valid here
*/
void mixed(__ref int x)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "InterlockedAdd($0, 1)";
    default:
        x += 1;
    }
}

// A `__target_intrinsic` modifier covers only the targets it names; the body
// would be the catch-all definition elsewhere, so it must be intrinsic
// assembly to qualify.
[noRefInline]
/*CHECK:
 ^^^^^^^^^^^ invalid attribute placement
 ^^^^^^^^^^^ attribute 'noRefInline' is not valid here
*/
__target_intrinsic(hlsl, "InterlockedAddPlaceholder($0)")
void targetIntrinsicWithBody(__ref int x) { x += 1; }

// The attribute belongs on the definition; a bodyless declaration is not an
// intrinsic.
[noRefInline]
/*CHECK:
 ^^^^^^^^^^^ invalid attribute placement
 ^^^^^^^^^^^ attribute 'noRefInline' is not valid here
*/
void forwardOnly(__ref int x);

// The rejected attribute is removed, so this call takes the default
// force-inlining path; the groupshared parameter must not produce the
// groupshared-on-HLSL diagnostic the removed attribute would have caused.
[noRefInline]
/*CHECK:
 ^^^^^^^^^^^ invalid attribute placement
 ^^^^^^^^^^^ attribute 'noRefInline' is not valid here
*/
[noinline] void gsWrite(uint tid, groupshared uint scratch[8]) { scratch[tid] = tid; }

groupshared uint gsScratch[8];

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    int v = buf[0];
    bump(v);
    mixed(v);
    targetIntrinsicWithBody(v);
    gsWrite(0u, gsScratch);
    buf[2] = v;
}
