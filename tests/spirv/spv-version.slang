//TEST:SIMPLE(filecheck=SPIRV13): -target spirv

//TEST:SIMPLE(filecheck=SPIRV14): -target spirv -profile spirv_1_4

// Test that we can use -profile to control the resulting spirv version.

// By default, we emit spirv 1.3

// SPIRV13: Version: 1.3

// If the user specified an explicit spirv version, we will use it.
// SPIRV14: Version: 1.4

[numthreads(1,1,1)]
void main()
{

}