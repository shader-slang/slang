//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

signed signedValue;
/*CHECK:
^^^^^^ 'signed' is not supported in HLSL
^^^^^^ 'signed' is not supported in HLSL; use 'int' instead
*/

signed int signedIntValue;
/*CHECK:
^^^^^^ 'signed' is not supported in HLSL
^^^^^^ 'signed' is not supported in HLSL; use 'int' instead
*/

unsigned char unsignedCharValue;
/*CHECK:
^^^^^^^^ traditional integer type name is not supported in HLSL
^^^^^^^^ traditional integer type name 'unsigned char' is not supported in HLSL
*/

unsigned short int unsignedShortValue;
/*CHECK:
^^^^^^^^ traditional integer type name is not supported in HLSL
^^^^^^^^ traditional integer type name 'unsigned short int' is not supported in HLSL
*/

unsigned long unsignedLongValue;
/*CHECK:
^^^^^^^^ traditional integer type name is not supported in HLSL
^^^^^^^^ traditional integer type name 'unsigned long' is not supported in HLSL
*/

unsigned long long int unsignedLongLongValue;
/*CHECK:
^^^^^^^^ traditional integer type name is not supported in HLSL
^^^^^^^^ traditional integer type name 'unsigned long long int' is not supported in HLSL
*/

vector<signed int, 4> signedVectorValue;
/*CHECK:
       ^^^^^^ 'signed' is not supported in HLSL
       ^^^^^^ 'signed' is not supported in HLSL; use 'int' instead
*/

vector<unsigned short, 4> unsignedShortVectorValue;
/*CHECK:
       ^^^^^^^^ traditional integer type name is not supported in HLSL
       ^^^^^^^^ traditional integer type name 'unsigned short' is not supported in HLSL
*/
