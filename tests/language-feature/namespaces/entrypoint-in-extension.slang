//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry Pixel.MyType.Main -stage fragment

// Entry lookup must see extensions declared in the module being compiled.
// CHECK: OpEntryPoint Fragment

namespace Pixel
{
    struct MyType {}

    extension MyType
    {
        static float4 Main(float4 color : COLOR) : SV_Target
        {
            return color;
        }
    }
}
