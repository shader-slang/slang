// ref-accessor-body-mutation.slang
//
// An unannotated `ref` accessor is implicitly mutating (like `set`, per #9636), so its
// body may write a `this` field — `this` is an l-value inside the body. Before the fix,
// `visitThisExpr` granted an l-value `this` only for constructor / `set` / `[mutating]` /
// `[ref]`, so a plain `ref` body assigning a `this` field was spuriously rejected with
// "left of '=' is not an l-value", while the equivalent `[mutating] ref` compiled. Here
// the accessor's body writes a (different) `this` field as a side effect and returns a
// reference to another; an l-value use invokes the body (the write lands) and yields the
// caller's storage.
// https://github.com/shader-slang/slang/issues/9636, #12487

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct Counter
{
    int _v;
    int _witness;
    property int value
    {
        get { return _v; }
        // Unannotated `ref`: the body writes a `this` field (implicitly mutating).
        ref
        {
            _witness = 42;
            return _v;
        }
    }
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Counter c;
    c._v = 5;
    c._witness = 0;

    // An l-value use invokes the `ref` accessor: its body writes `_witness = 42` (the
    // this-field write the fix enables) and returns a reference to `_v`, into which the
    // assignment then stores 7.
    c.value = 7;

    // CHECK: 7
    outputBuffer[0] = c._v;
    // CHECK-NEXT: 42
    outputBuffer[1] = c._witness;
}
