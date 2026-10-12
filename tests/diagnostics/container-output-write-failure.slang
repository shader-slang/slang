// Container outputs (.slang-module, .zip) that cannot be written must be diagnosed with E00004
// and fail the compile, just like ordinary target outputs.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_MODULE): -o nonexistent-dir/container-output-write-failure.slang-module
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_SKIP): -skip-codegen -o nonexistent-dir/container-output-write-failure.slang-module
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_ZIP): -target spirv -entry main -stage fragment -o nonexistent-dir/container-output-write-failure.zip

// CHECK_MODULE: E00004
// CHECK_SKIP: E00004
// CHECK_ZIP: E00004

float4 main() : SV_Target
{
    return float4(0.0);
}
