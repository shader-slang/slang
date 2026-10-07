//TEST_IGNORE_FILE:
__exported import "specialize-per-conformance-interface";

public struct Second : IOperation
{
    static bool evaluate(uint context, out uint candidate)
    {
        candidate = context + 20;
        return (context & 1) != 0;
    }
}
