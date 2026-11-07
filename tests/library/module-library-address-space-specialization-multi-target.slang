//TEST_IGNORE_FILE:

// module-library-address-space-specialization-multi-target.slang

// A library for precompiled-multi-target-address-space-specialization.slang (#14004).

module "module-library-address-space-specialization-multi-target";

public struct Acc
{
    public float v;

    [mutating]
    public void add(float x)
    {
        v += x;
    }
};
