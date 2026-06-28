// output-no-target.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target dxbc -target spirv -entry main -stage compute -o bad.hlsl
//CHECK: no specified '-target' option matches the output path 'tests/diagnostics/command-line/bad.hlsl', which implies the 'hlsl' format
