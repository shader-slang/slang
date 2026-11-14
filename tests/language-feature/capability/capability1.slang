//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -emit-spirv-directly -entry main2 -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_IGNORE_CAPS): -target spirv -emit-spirv-directly -entry main2 -stage compute -ignore-capabilities

[require(spvShaderClockKHR)]
void leafFunc1() {}

[require(spvShaderNonUniform)]
void leafFunc2() {}
/*CHECK:
     ^^^^^^^^^ see definition of 'leafFunc2'
*/

void caller()
{
    leafFunc1();
    leafFunc2();
/*CHECK:
    ^^^^^^^^^ see using of 'leafFunc2'
*/
}

[require(spirv, shaderclock)]
void main1()
/*CHECK:
     ^^^^^ undeclared capability used
     ^^^^^ 'main1' uses undeclared capability 'spvShaderNonUniform'
*/
{
    caller(); // Error, shaderclock does not imply spvShaderNonUniform.
/*CHECK:
    ^^^^^^ see using of 'caller'
*/
}


[require(spirv, shaderclock)]
void main2()
{
    leafFunc1(); // OK, shaderclock implies spvShaderClockKHR.
}
