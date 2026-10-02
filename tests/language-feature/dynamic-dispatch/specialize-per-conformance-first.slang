//TEST_IGNORE_FILE:
__exported import "specialize-per-conformance-interface";

// This provider knows nothing about the traversal helper or its state.
public struct First : IOperation
{
    static bool evaluate(uint context, out uint candidate)
    {
        candidate = context + 10;
        return (context & 1) == 0;
    }
}
