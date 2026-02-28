//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -profile vs_5_0
// non-exhaustive: cascading "undefined identifier 'main'" and "no function found matching entry point" errors

layout(
	binding = UNDEFINED_VK_BINDING,
//CHECK:   ^^^^^^^^^^^^^^^^^^^^ undefined identifier
//CHECK:   ^^^^^^^^^^^^^^^^^^^^ undefined identifier 'UNDEFINED_VK_BINDING'.
	set = UNDEFINED_VK_SET)
/*CHECK:
       ^^^^^^^^^^^^^^^^ undefined identifier
       ^^^^^^^^^^^^^^^^ undefined identifier 'UNDEFINED_VK_SET'.
*/
Texture2DArray<float4> Float4Texture2DArrays[] : register(t0, space100);

