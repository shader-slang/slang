// vk-bindings.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-entry main -target spirv
// CHECK: warning[E39029]
// CHECK: error[E39015]
// CHECK: warning[E39001]
// CHECK: note:

// D3D `register` without VK binding
Texture2D t : register(t0);
/*CHECK:
          ^ see declaration of 't'
              ^^^^^^^^ D3D register without Vulkan binding or shift
              ^^^^^^^^ shader parameter 't' has a 'register' specified for D3D
*/

[[vk::binding(3)]]
Texture2D t1 : register(t3);

struct S { float4 a; };

// Parameter block with non-zero binding:
[[vk::binding(2,1)]]
/*CHECK:
      ^^^^^^^ whole descriptor set requires binding 0
      ^^^^^^^ shader parameter 'b' consumes whole descriptor sets
*/
ParameterBlock<S> b;

[[vk::push_constant]] \
ConstantBuffer<int> gint : register(b0, space0);
/*CHECK:
                    ^^^^ explicit binding overlap
                    ^^^^ explicit binding for parameter 'gint' overlaps with parameter 't'
*/

layout(shaderRecordEXT) ConstantBuffer<int> GeometrySBT : register(b0, space99999);

[shader("compute")]
void main()
{}
