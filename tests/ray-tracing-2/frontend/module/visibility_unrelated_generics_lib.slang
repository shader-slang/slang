//TEST_IGNORE_FILE:

module visibility_unrelated_generics_lib;

// These declarations mirror the generic patterns in SlangPy's core module.
// They are intentionally unrelated to structural ray tracing.
public struct TextureWrapper<T : ITexelElement>
{
    Texture1D<T> value;
}

public extension<T> TextureWrapper<vector<T, 1>> where vector<T, 1> : ITexelElement
{}

namespace conversion
{
    public interface IConvertibleFrom<From>
    {}

    public extension<T : __BuiltinIntegerType> T : IConvertibleFrom<int>
    {}
}

public void consumeUnrelatedGenericModule()
{}
