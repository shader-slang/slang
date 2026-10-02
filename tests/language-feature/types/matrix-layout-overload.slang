// Two functions whose parameter types differ only in matrix layout have the same signature, as in
// HLSL, so a second body is a redefinition. This holds for both parameter spellings, which
// produce the same type, and for a layout on a matrix inside an array or behind a pointer.
// Mangled names do not encode matrix layout (shader-slang/slang#13383), so accepting these as
// overloads would emit one function for both.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -profile cs_6_0 -entry main

float traditionalSpelling(row_major float2x3 m[2]) { return 1; }
/*CHECK:
      ^^^^^^^^^^^^^^^^^^^ see previous definition of 'traditionalSpelling'
*/
float traditionalSpelling(column_major float2x3 m[2]) { return 2; }
/*CHECK:
      ^^^^^^^^^^^^^^^^^^^ E30201
      ^^^^^^^^^^^^^^^^^^^ redeclared here
*/

func modernSpelling(m: row_major float2x3[2]) -> float { return 1; }
/*CHECK:
     ^^^^^^^^^^^^^^ see previous definition of 'modernSpelling'
*/
func modernSpelling(m: column_major float2x3[2]) -> float { return 2; }
/*CHECK:
     ^^^^^^^^^^^^^^ E30201
     ^^^^^^^^^^^^^^ redeclared here
*/

func explicitAndDefault(m: row_major float2x3[2]) -> float { return 1; }
/*CHECK:
     ^^^^^^^^^^^^^^^^^^ see previous definition of 'explicitAndDefault'
*/
func explicitAndDefault(m: float2x3[2]) -> float { return 2; }
/*CHECK:
     ^^^^^^^^^^^^^^^^^^ E30201
     ^^^^^^^^^^^^^^^^^^ redeclared here
*/

float pointee(row_major float2x3* p) { return 1; }
/*CHECK:
      ^^^^^^^ see previous definition of 'pointee'
*/
float pointee(column_major float2x3* p) { return 2; }
/*CHECK:
      ^^^^^^^ E30201
      ^^^^^^^ redeclared here
*/

float unsizedArray(row_major float2x3 m[]) { return 1; }
/*CHECK:
      ^^^^^^^^^^^^ see previous definition of 'unsizedArray'
*/
float unsizedArray(column_major float2x3 m[]) { return 2; }
/*CHECK:
      ^^^^^^^^^^^^ E30201
      ^^^^^^^^^^^^ redeclared here
*/

// A layout difference does not hide a real difference elsewhere in the type.
float differentShape(row_major float2x3 m[2]) { return 1; }
float differentShape(column_major float3x2 m[2]) { return 2; }
float differentCount(row_major float2x3 m[2]) { return 1; }
float differentCount(column_major float2x3 m[3]) { return 2; }

RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = 0;
}
