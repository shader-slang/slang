//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -entry main2 -stage compute

[require(spvShaderClockKHR)]
void leafFunc1() {}

[require(spvShaderNonUniform)]
void leafFunc2() {}

void caller()
{
    leafFunc1();
    leafFunc2();
}

[require(spirv, shaderclock)]
// CHECK: ([[# @LINE+1]]): error 36104:
void main1()
{
    caller(); // Error, shaderclock does not imply spvShaderNonUniform.
}


[require(spirv, shaderclock)]
void main2()
{
    // CHECK-NOT: error
    leafFunc1(); // OK, shaderclock implies spvShaderClockKHR.
}
