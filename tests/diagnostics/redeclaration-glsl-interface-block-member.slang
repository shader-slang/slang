//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang glsl tests/diagnostics/redeclaration-glsl-interface-block-member.slang -target spirv -stage vertex -entry main

// The members of a GLSL interface block without an instance name are visible in the
// enclosing scope, so they conflict with other declarations of the same name
// (shader-slang/slang#13876).

layout(binding = 0) uniform Block
{
    float blockMember;
/*CHECK:
          ^^^^^^^^^^^ see previous declaration of 'blockMember'
*/
};

float blockMember;
/*CHECK:
      ^^^^^^^^^^^ warning E30204
      ^^^^^^^^^^^ declaration of 'blockMember' conflicts with existing declaration
*/

layout(std430, binding = 1) buffer Storage
{
    float storageMember;
/*CHECK:
          ^^^^^^^^^^^^^ see previous declaration of 'storageMember'
*/
};

float storageMember;
/*CHECK:
      ^^^^^^^^^^^^^ warning E30204
      ^^^^^^^^^^^^^ declaration of 'storageMember' conflicts with existing declaration
*/

layout(location = 0) in VertexIn
{
    vec3 sharedVarying;
/*CHECK:
         ^^^^^^^^^^^^^ see previous declaration of 'sharedVarying'
*/
};

layout(location = 0) out VertexOut
{
    vec3 sharedVarying;
/*CHECK:
         ^^^^^^^^^^^^^ warning E30204
         ^^^^^^^^^^^^^ declaration of 'sharedVarying' conflicts with existing declaration
*/
};

// The compiler ignores a redeclaration of a builtin block, so its members are not
// declared in this scope, and a later declaration of the same name does not conflict.

out gl_PerVertex
{
    vec4 gl_Position;
};

float gl_Position;

void main()
{
}
