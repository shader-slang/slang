// precompiled-spirv-buffer-element-import.slang

// Buffer-element-type lowering specializes the imported `readS` for the storage-typed
// `buf[id.x]` argument and keeps the original for the logical `local` argument. The
// specialization loses its linkage through `removeLinkageDecorations`, so it must also lose
// `AvailableInDownstreamIR`; otherwise its body is replaced by an import of a name it no longer
// has, and emitting SPIR-V crashes.

//TEST:COMPILE: tests/library/module-library-buffer-element-import.slang -o tests/library/module-library-buffer-element-import.slang-module -target spirv -embed-downstream-ir -incomplete-library
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry useMain -stage compute -fvk-use-entrypoint-name

// CHECK: OpEntryPoint GLCompute %useMain "useMain"

import "module-library-buffer-element-import";

RWStructuredBuffer<S> buf;
RWStructuredBuffer<float> outv;

[shader("compute")]
[numthreads(1, 1, 1)]
void useMain(uint3 id: SV_DispatchThreadID)
{
    S local = buf[1];
    local.x += 1;
    outv[id.x] = readS(buf[id.x]) + readS(local);
}
