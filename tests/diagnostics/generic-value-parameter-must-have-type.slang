//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv

interface ITest
{
    static const uint kValue0;
    static const uint kValue1;
};

// `TValue1` does not have an explicit type, this should fail to compile.
struct TestImpl<let TValue0 : uint, let TValue1> : ITest
//diag:                                 ^^^^^^^ a generic value parameter must be given an explicit type
//diag:                                 ^^^^^^^ a generic value parameter must be given an explicit type
//diag:                                 ^^^^^^^ generic value parameter type 'error' is not supported; only integer and enum types are allowed
//diag:                                 ^^^^^^^ generic value parameter type 'error' is not supported; only integer and enum types are allowed
{
    static const uint kValue0 = TValue0;
    static const uint kValue1 = TValue1;
};

void computeMain()
{
}
