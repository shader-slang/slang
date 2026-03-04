//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-directly

// CHECK-DAG: OpCapability Int64ImageEXT{{$}}
// CHECK-DAG: OpCapability Int64{{$}}
// CHECK-DAG: OpCapability Shader{{$}}
// CHECK-DAG: OpExtension "SPV_EXT_shader_image_int64"

uniform DescriptorHandle<Texture2D<uint64_t>> image;
uniform uint64_t* outval;

[shader("compute")]
void computeMain()
{
    *outval = image.Load(int3(0));
}
