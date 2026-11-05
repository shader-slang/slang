// x-arg-tool-unknown.slang

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=1):-entry main -stage compute -o myKernel.dxbc -o myKernel.dxbc -Xthingy

// {{.*}}CHECK: downstream tool name not known, allowed names are
