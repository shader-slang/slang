//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -emit-spirv-directly -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_IGNORE_CAPS,exit-code=0): -target spirv -emit-spirv-directly -entry computeMain -stage compute -ignore-capabilities

[require(spirv, hlsl)]
/*CHECK:
 ^^^^^^^ disallowed capability
 ^^^^^^^ 'require' resolves into a disallowed `Invalid` Capability.
*/
void computeMain()
{

}
