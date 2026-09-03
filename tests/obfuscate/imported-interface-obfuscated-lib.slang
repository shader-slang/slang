//TEST_IGNORE_FILE:

// Library half of `imported-interface-obfuscated.slang`.

module "imported-interface-obfuscated-lib";

public interface IScaledObf
{
    float scaled(float x);
}

public struct DoublerObf : IScaledObf
{
    public __init() {}

    public float scaled(float x) { return x * 2.0; }
}
