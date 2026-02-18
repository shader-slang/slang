//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -emit-spirv-directly -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_IGNORE_CAPS): -target spirv -emit-spirv-directly -entry computeMain -stage compute -ignore-capabilities

[require(spirv, hlsl)]
/*CHECK:
 ^^^^^^^ disallowed capability
 ^^^^^^^ '' resolves into a disallowed `Invalid` Capability.
*/
void computeMain()
{

}
