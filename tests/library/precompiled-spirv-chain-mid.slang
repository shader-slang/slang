// Imported by precompiled-spirv-chain.slang; imports precompiled-spirv-chain-base.slang.
module "precompiled-spirv-chain-mid";

import "precompiled-spirv-chain-base";

public uint chainMid(uint x)
{
    uint sum = 0u;
    for (uint i = 0u; i < 4u; i++)
        sum += chainBase(x + i);
    return sum;
}
