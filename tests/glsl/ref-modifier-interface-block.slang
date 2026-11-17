// The ref and borrow passing-mode modifiers are parameter-only, so applying
// them to a GLSL interface block declaration is rejected with error 31201
// the same way it is on globals and struct fields.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang glsl tests/glsl/ref-modifier-interface-block.slang -stage compute -entry computeMain -target glsl
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang glsl tests/glsl/ref-modifier-interface-block.slang -stage compute -entry computeMain -target spirv -emit-spirv-directly

layout(local_size_x = 1) in;

__ref uniform RefBlock { uint d1; };
/*CHECK:
^^^^^ E31201
^^^^^ modifier '__ref' is not allowed here.
*/

__constref uniform ConstrefBlock { uint d2; };
/*CHECK:
^^^^^^^^^^ E31201
^^^^^^^^^^ modifier '__constref' is not allowed here.
*/

__borrow uniform BorrowBlock { uint d3; };
/*CHECK:
^^^^^^^^ E31201
^^^^^^^^ modifier '__borrow' is not allowed here.
*/

__ref_readonly uniform RefReadonlyBlock { uint d4; };
/*CHECK:
^^^^^^^^^^^^^^ E31201
^^^^^^^^^^^^^^ modifier '__ref_readonly' is not allowed here.
*/

__ref_writeonly uniform RefWriteonlyBlock { uint d5; };
/*CHECK:
^^^^^^^^^^^^^^^ E31201
^^^^^^^^^^^^^^^ modifier '__ref_writeonly' is not allowed here.
*/

void computeMain()
{
}
