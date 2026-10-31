//TEST:SIMPLE(filecheck=CHECK): -target hpp -line-directive-mode none
//TEST:SIMPLE(filecheck=OBF): -target hpp -line-directive-mode none -obfuscate

// Header prototypes name their parameters from the source parameter names
// (https://github.com/shader-slang/slang/issues/14028). A function that is only
// declared has no parameters to take names from, and `-obfuscate` strips the
// names, so both fall back to positional `_<i>` names.

__extern_cpp float hostProvided(float hostArg);
public export __extern_cpp float callHost(float callArg) { return hostProvided(callArg); }

//CHECK: float hostProvided(float _0);
//CHECK: float callHost(float callArg_{{[0-9]+}});
//OBF-NOT: callArg
//OBF-NOT: hostArg
//OBF: (float _0);

[shader("compute")]
[numthreads(1, 1, 1)]
void dummyEntryPoint() {}
