//DIAGNOSTIC_TEST(dxc):SIMPLE(diag=DXIL): -Gec -target dxil -profile cs_6_0 -entry computeMain -O0 -validate-ir
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -source-language hlsl -xslang -Gec -output-using-type

// With `-Gec`, the compiler gives `selectedTexture` a writable shadow. Resource-global legalization
// must replace that shadow with a value initialized separately for each entry-point invocation.
// Invocation zero replaces its copy, while invocation one leaves its copy unchanged. The two
// different CPU results prove that one invocation cannot modify another invocation's value.
//
// Specialization replaces the conditional assignment with a resource-valued phi. The DXIL
// pipeline eliminates that phi by introducing mutable local storage, but DXIL cannot represent a
// resource in such storage. The DXIL run must report this target restriction.

//TEST_INPUT: Texture2D(size=4, content = one):name=selectedTexture
Texture2D<float4> selectedTexture;
//DXIL:           ^^^^^^^^^^^^^^^ target cannot represent an opaque value in mutable local storage
//DXIL:           ^^^^^^^^^^^^^^^ mutable local storage contains an opaque value of type 'Texture2D', which target 'dxil' cannot represent; use or pass the resource value directly instead of assigning it to a mutable local or `static` variable

//TEST_INPUT: Texture2D(size=4, content = zero):name=replacementTexture
Texture2D<float4> replacementTexture;

// The output starts with nonzero data so that both expected results prove that the shader performed
// the stores.
//TEST_INPUT: ubuffer(data=[2 2 2 2 2 2 2 2], stride=16):out,name=output
RWStructuredBuffer<float4> output;

[noinline]
void selectReplacement()
{
    selectedTexture = replacementTexture;
}

[noinline]
float4 readSelected()
{
    return selectedTexture.Load(int3(0, 0, 0));
}

[numthreads(2, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x == 0)
        selectReplacement();
    output[dispatchThreadID.x] = readSelected();
}

// CHECK: type: float
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
