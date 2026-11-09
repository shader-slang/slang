// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

import slang.runtime.resource.simple_texture;

[host]
void main(uint selectedMip)
{
    let color = allocateTexture2D<float4, TextureFormat.rgba32Float, 4>(8, 8);
    let depth = allocateDepthTarget2D<TextureFormat.d32FloatS8Uint>(8, 8);
    var range = SubresourceRange();
    range.mip = selectedMip;
    range.mipCount = 1;
    color.clear(float4(0.0), range);
    depth.clear(DepthStencilValue(1.0, 7u));
    depth.clear<TextureAspect.DepthOnly>(0.5);
    depth.clear<TextureAspect.StencilOnly>(3u);
}

// CHECK-DAG: OpTexture2DType {{.*}} ReadOnly D32FloatS8Uint
// CHECK: OpAllocateResource Texture Texture2D RGBA32Float DeviceLocal UnorderedAccess {{.*}} 4 1 0
// CHECK: OpClearTexture {{.*}} All
// CHECK: OpClearTexture {{.*}} All
// CHECK: OpClearTexture {{.*}} DepthOnly
// CHECK: OpClearTexture {{.*}} StencilOnly
