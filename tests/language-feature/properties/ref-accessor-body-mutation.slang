// ref-accessor-body-mutation.slang
//
// An unannotated `ref` accessor is implicitly mutating (like `set`, per #9636), so its
// body may write a `this` field: `this` is an l-value inside the body, for both an
// unqualified member write (`_a = ...`) and the explicit `this.field` form. This test
// exercises both. The body writes two `this` fields as a side effect and returns a
// reference to a third; an l-value use of the property invokes the body (so the
// side-effect writes land) and then stores through the returned reference into the
// caller's storage.
// https://github.com/shader-slang/slang/issues/9636, #12487

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct Counter
{
    int _v;
    int _a;
    int _b;
    property int value
    {
        get { return _v; }
        ref
        {
            _a = 42;      // unqualified `this`-member write
            this._b = 43; // explicit `this.`-member write
            return _v;
        }
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Counter c;
    c._v = 5;
    c._a = 0;
    c._b = 0;

    // An l-value use invokes the `ref` accessor: its body writes `_a` / `_b` (the this-field
    // writes the fix enables) and returns a reference to `_v`, into which 7 is stored.
    c.value = 7;

    // CHECK: 7
    outputBuffer[0] = c._v;
    // CHECK-NEXT: 42
    outputBuffer[1] = c._a;
    // CHECK-NEXT: 43
    outputBuffer[2] = c._b;
}
