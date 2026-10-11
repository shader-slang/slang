// The `writeonly` memory qualifier on a Slang-syntax resource reports E30119 only for a variable
// initializer and for the right-hand side of an assignment. Indexing the resource, reading an
// element, passing it to a `writeonly` parameter and inferring a `let` or `var` from it are not
// reported. `__ref_writeonly` parameters have their own, stricter rule and do not change this one.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

writeonly RWTexture2D<float> gt;
RWStructuredBuffer<float> o;

float f(writeonly RWTexture2D<float> t)
{
    uint w, h;
    t.GetDimensions(w, h);
    float a = t[uint2(0, 0)];
    return t[uint2(0, 0)] + 1;
}

[numthreads(1, 1, 1)]
void main()
{
    float b = gt[uint2(0, 0)];
    o[0] = f(gt) + b;
    RWTexture2D<float> cpy = gt;
//CHECK:                     ^^ E30119
//CHECK:                     ^^ cannot read from writeonly, check modifiers.
    let la = gt;
    var vb = gt;
    RWTexture2D<float> c = gt;
//CHECK:                   ^^ E30119
//CHECK:                   ^^ cannot read from writeonly, check modifiers.
    c = gt;
//CHECK:^^ E30119
//CHECK:^^ cannot read from writeonly, check modifiers.
}
