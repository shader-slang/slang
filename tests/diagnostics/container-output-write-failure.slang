// Container outputs (.slang-module, .zip) that cannot be written must be diagnosed with E00004
// and fail the compile, just like ordinary target outputs.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_MODULE,exit-code=-1): -o nonexistent-dir/container-output-write-failure.slang-module
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_SKIP,exit-code=-1): -skip-codegen -o nonexistent-dir/container-output-write-failure.slang-module
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_ZIP,exit-code=-1): -target spirv -entry main -stage fragment -o nonexistent-dir/container-output-write-failure.zip

// CHECK_MODULE: E00004
// CHECK_SKIP: E00004
// CHECK_ZIP: E00004

float4 main() : SV_Target
{
    return float4(0.0);
}
