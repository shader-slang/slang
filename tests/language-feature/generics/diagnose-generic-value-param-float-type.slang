//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Generic value parameters only support integer and enum types.
// Float, double, and half are all rejected.
// Each diagnostic is emitted twice (initial check + re-check phase).

void withFloat<let F : float>(int x) {}
//CHECK:           ^ generic value parameter type 'float' is not supported; only integer and enum types are allowed
//CHECK:           ^ generic value parameter type 'float' is not supported; only integer and enum types are allowed

void withDouble<let D : double>(int x) {}
//CHECK:            ^ generic value parameter type 'double' is not supported; only integer and enum types are allowed
//CHECK:            ^ generic value parameter type 'double' is not supported; only integer and enum types are allowed

void withHalf<let H : half>(int x) {}
//CHECK:          ^ generic value parameter type 'half' is not supported; only integer and enum types are allowed
//CHECK:          ^ generic value parameter type 'half' is not supported; only integer and enum types are allowed

void main()
{
}
