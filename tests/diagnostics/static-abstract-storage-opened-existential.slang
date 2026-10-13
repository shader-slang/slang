//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// `createLookupResultExpr` replaces a static subscript's unevaluated value qualifier and carries
// the selected subscript's abstract-storage contract onto the replacement path.

interface IStaticStorage
{
    static __subscript(int index) -> int { set; }
}

struct StaticStorage : IStaticStorage
{
    static int storedValue;

    static __subscript(int index) -> int
    {
        set { storedValue = newValue; }
    }
}

int readWriteOnlyStaticStorage(IStaticStorage value)
{
    // `maybeOpenExistential` introduces a local binding for `value` that the static requirement's
    // witness uses. After lookup selects the subscript, `createLookupResultExpr` replaces the
    // unevaluated qualifier and attaches the subscript's accessor path to that retained binding
    // instead of copying the discarded qualifier's path.
    return value[0];
//CHECK:        ^ property or subscript cannot be read
//CHECK:        ^ cannot read this subscript because it has no `get` or `ref` accessor
}
