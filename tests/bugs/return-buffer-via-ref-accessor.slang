// Regression test for #10174: a `ref` accessor that returns a reference to a
// device-buffer element must survive atomic-destination validation.  Previously
// the ref accessor was lowered to a function returning a plain `Ptr<T>` whose
// body actually returned the loaded value, so the call-site pointer had no
// storage-class information and atomic operations through it were rejected by
// the IR validator.
//
// Compilation-only test: specify `-o` so slang-test doesn't treat the SPIR-V
// assembly on stdout as a diff failure.

//TEST:SIMPLE:-target spirv -o return-buffer-via-ref-accessor.spv

RWStructuredBuffer<int> buf;

struct Wrapper
{
    __subscript(uint i) -> int
    {
        ref { return buf[i]; }
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Wrapper w;
    InterlockedAdd(w[0], 1);
}
