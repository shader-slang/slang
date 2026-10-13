//TEST_IGNORE_FILE:

module "effective-this-param-mangling-hlsl-module";

public struct HLSLReceiver
{
    public int value;

    public int implicitMutable(int amount)
    {
        value += amount;
        return value;
    }

    public int trailingConst(int amount) const
    {
        return value + amount;
    }
};
