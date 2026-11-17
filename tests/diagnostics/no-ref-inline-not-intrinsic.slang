// [noRefInline] opts a function out of the reference-parameter force-inlining
// pass. A non-intrinsic function that keeps its definition cannot faithfully
// express a `__ref` parameter on every target (HLSL emits the closest form
// `inout`, which copies and loses the aliasing `__ref` promises), so the
// attribute only applies to intrinsic declarations, whose bodies are never
// emitted as target function definitions.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> buf;

[noRefInline]
/*CHECK:
 ^^^^^^^^^^^ invalid attribute placement
 ^^^^^^^^^^^ attribute 'noRefInline' is not valid here
*/
[noinline] void bump(__ref int x) { x += 1; }

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    int v = buf[0];
    bump(v);
    buf[2] = v;
}
