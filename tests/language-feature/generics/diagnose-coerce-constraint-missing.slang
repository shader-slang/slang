//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A type coercion constraint `where ToType(FromType)` requires that
// FromType is convertible to ToType. When no conversion exists at all,
// error 38043 is emitted.

struct Source
{
    int value;
}

struct Target
{
    int value;
}

ToType convert<ToType, FromType>(FromType input) where ToType(FromType)
/*CHECK:
       ^^^^^^^ see definition of 'convert'
*/
{
    return {};
}

void test()
{
    Source s = { 42 };
    Target t = convert<Target, Source>(s);
//CHECK:       ^^^^^^^ type coerce constraint not satisfied
//CHECK:       ^^^^^^^ 'Source' is not convertible to 'Target', not satisfying the type coerce constraint 'Target(Source)'
}
