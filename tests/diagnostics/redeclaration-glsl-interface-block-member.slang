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
      ^^^^^^^^^^^ conflicting declaration
      ^^^^^^^^^^^ declaration of 'blockMember' conflicts with existing declaration
*/

// The compiler ignores a redeclaration of a builtin block, so its members are not
// declared in this scope.

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
}
