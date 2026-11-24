//TEST:SIMPLE(filecheck=IR): -entry main -target hlsl -dump-ir -no-codegen
//TEST:COMPILE: tests/language-feature/known-builtin-name/known_builtin_library.slang -o tests/language-feature/known-builtin-name/known-builtin-cache.slang-module
//TEST:SIMPLE(filecheck=IR): -r tests/language-feature/known-builtin-name/known-builtin-cache.slang-module -entry main -target hlsl -dump-ir -no-codegen

import known_builtin_library;

// The cache filename differs from the import name so the first run loads source.
// Reloading the saved module must preserve the same metadata on the called functions.
// IR: [KnownBuiltinDecoration(0 : Int)]
// IR-NEXT: [{{import|export}}({{.*}})]
// IR-NEXT: [nameHint("fromName")]
// IR-NEXT: func [[NAME:%[A-Za-z0-9_]+]]
// IR: [KnownBuiltinDecoration(0 : Int)]
// IR-NEXT: [{{import|export}}({{.*}})]
// IR-NEXT: [nameHint("fromInteger")]
// IR-NEXT: func [[INTEGER:%[A-Za-z0-9_]+]]
// IR: [KnownBuiltinDecoration(0 : Int)]
// IR-NEXT: [{{import|export}}({{.*}})]
// IR-NEXT: [nameHint("fromEnum")]
// IR-NEXT: func [[ENUM:%[A-Za-z0-9_]+]]
// IR: call [[NAME]](
// IR: call [[INTEGER]](
// IR: call [[ENUM]](

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = fromName(tid.x) + fromInteger(tid.x) + fromEnum(tid.x);
}
