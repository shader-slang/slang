//DIAGNOSTIC_TEST(dxc):SIMPLE(diag=DXIL): -Gec -target dxil -profile cs_6_0 -entry computeMain -O0 -validate-ir
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -source-language hlsl -xslang -Gec -output-using-type

// With `-Gec`, the compiler gives the fixed-size resource array `selectedTextures` a writable
// shadow. Resource-global legalization must initialize the entire entry-point local from the
// shader parameter before `selectReplacement` changes one element. The generated writable
// parameter for that helper must update the entry-point local, and `readSelected` must receive the
// updated array value. The CPU run checks those semantics.
//
// DXIL cannot represent a mutable local whose type is an array of textures. Inlining the helpers
// leaves the assignment to element one in the entry point; removing the remaining local would
// require scalarizing the array rather than specializing another call. The DXIL run must report
// this target restriction.

//TEST_INPUT: Texture2D(size=4, content = one):name=selectedTextures[0]
//TEST_INPUT: Texture2D(size=4, content = one):name=selectedTextures[1]
Texture2D<float4> selectedTextures[2];

//TEST_INPUT: Texture2D(size=4, content = zero):name=replacementTexture
Texture2D<float4> replacementTexture;

// The output starts with nonzero data. The expected one-valued first element proves that
// whole-array initialization preserved the element we do not replace. The expected zero-valued
// second element proves that the helper changed the selected element.
//TEST_INPUT: ubuffer(data=[2 2 2 2 2 2 2 2], stride=16):out,name=output
RWStructuredBuffer<float4> output;

[noinline]
void selectReplacement()
{
    selectedTextures[1] = replacementTexture;
}

[noinline]
float4 readSelected(uint index)
{
    return selectedTextures[index].Load(int3(0, 0, 0));
}

[numthreads(1, 1, 1)]
void computeMain()
{
    selectReplacement();
//DXIL:              ^ target cannot represent an opaque value in mutable local storage
//DXIL:              ^ mutable local storage contains an opaque value of type 'Texture2D', which target 'dxil' cannot represent; use or pass the resource value directly instead of assigning it to a mutable local or `static` variable
    output[0] = readSelected(0);
    output[1] = readSelected(1);
}

// CHECK: type: float
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
