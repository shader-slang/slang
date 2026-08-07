// Force the current module's extension into the shared candidate cache before a differentiability
// attribute loads the autodiff supplement. Merging the supplement must not discard the extension
// that was already found in this module.
//
//TEST:SIMPLE:

struct LocalValue
{
    float value;
}

extension LocalValue
{
    static const int elementCount = 2;
}

struct UseExtensionBeforeAutodiff
{
    float values[LocalValue.elementCount];
}

[Differentiable]
float triggerSupplementLoad(float value)
{
    return sin(value);
}

struct UseExtensionAfterAutodiff
{
    float values[LocalValue.elementCount];
}
