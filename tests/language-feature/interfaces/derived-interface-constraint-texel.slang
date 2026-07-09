//TEST:INTERPRET(filecheck=CHECK):

// Exercises the combination this PR's other tests do not:
//   * a *base* interface bounds an associated type with a subtype constraint
//     (`DataType : IMyTexelData`), and
//   * a *derived* interface adds an equality constraint on the same associated
//     type (`__constraint DataType == This`), and
//   * a generic `wrapper` relies on `T.DataType == T` to return a `T.DataType`
//     value where a `T` is expected.
//
// (Modeled on a texel-element pattern. A builtin type like `int` as the element
// is intentionally avoided here: `int`'s conformance to an interface with an
// `__init` requirement currently resolves differently between standalone
// compilation and the in-process API, which is a separate issue from the
// `DataType == This` constraint exercised by this test.)

interface IMyTexelData
{
    int value();
}

interface IMyTexelElement
{
    associatedtype DataType : IMyTexelData;
}

interface IMyTexelElement2 : IMyTexelElement
{
    __constraint DataType == This;
}

struct Texel : IMyTexelData, IMyTexelElement2
{
    int v;
    typealias DataType = Texel;
    int value() { return v; }
}

struct MyTexture<T : IMyTexelElement>
{
    T.DataType data;
    T.DataType fetch() { return data; }
}

T wrapper<T : IMyTexelElement2>(MyTexture<T> texture)
{
    // `fetch()` returns `T.DataType`; the return type is `T`. This type-checks
    // only because `T : IMyTexelElement2` provides `T.DataType == T`.
    return texture.fetch();
}

void main()
{
    MyTexture<Texel> texture;
    texture.data.v = 42;
    Texel r = wrapper(texture);

    // CHECK: 42
    printf("%d\n", r.value());
}
