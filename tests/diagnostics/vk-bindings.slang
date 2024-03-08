// vk-bindings.slang

//DIAGNOSTIC_TEST:SIMPLE:-entry main -target spirv

// D3D `register` without VK binding
Texture2D t : register(t0);

[[vk::binding(3)]]
Texture2D t1 : register(t3);

struct S { float4 a; };

// Parameter block with non-zero binding:
[[vk::binding(2,1)]]
ParameterBlock<S> b;

[shader("compute")]
void main()
{}