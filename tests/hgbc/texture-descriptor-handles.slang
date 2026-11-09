//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main
//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -obfuscate

// The handle's semantic identity includes access, element type, and storage format.
[host]
void main(Texture2D<float4, 0, 10> input, RWTexture2D<uint, 0, 37> output,
    WTexture2D<float4, 0, 10> writeOnly,
    out Texture2D<float4, 0, 10>.Handle inputHandle,
    out RWTexture2D<uint, 0, 37>.Handle outputHandle,
    out WTexture2D<float4, 0, 10>.Handle writeOnlyHandle,
    out Texture2D<float4, 0, 10> recovered)
{
    inputHandle = input.getDescriptorHandle();
    outputHandle = output.getDescriptorHandle();
    writeOnlyHandle = writeOnly.getDescriptorHandle();
    recovered = *inputHandle;
}

// CHECK-DAG: "DescriptorHandle<_Texture<vector<float, 4>, __Shape2D, 0, 0, 0, 0, 0, 0, 10>>"
// CHECK-DAG: "DescriptorHandle<_Texture<uint, __Shape2D, 0, 0, 0, 1, 0, 0, 37>>"
// CHECK-DAG: "DescriptorHandle<_Texture<vector<float, 4>, __Shape2D, 0, 0, 0, 2, 0, 0, 10>>"
// CHECK-DAG: OpResourceToDescriptorHandle
// CHECK-DAG: OpDescriptorHandleToResource
