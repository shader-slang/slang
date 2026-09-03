//TEST_IGNORE_FILE:

module "overload-import-overrides-local-helper";

// An exported overload that is an *exact* match for a `uint` argument. The
// importing module has its own `exampleFunction(int)`, which only matches after
// a `uint -> int` conversion, so this declaration wins overload resolution at
// the call site in the importing module.
public uint exampleFunction(uint val)
{
    return val * 42U;
}
