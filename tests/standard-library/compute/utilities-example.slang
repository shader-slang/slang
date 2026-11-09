// TEST:INTERPRET(hgvm,filecheck=CHECK): -api vulkan
// TEST:INTERPRET(hgvm,filecheck=CHECK): -api d3d12

import "../../../examples/host-compute/buffer-texture-utils.slang";

[host]
void main()
{
    runBufferTextureExample();
}

// CHECK: 438.000000
// CHECK: 342.000000
