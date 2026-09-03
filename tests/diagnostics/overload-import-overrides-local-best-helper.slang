//TEST_IGNORE_FILE:
module "overload-import-overrides-local-best-helper";

// An exact match for the `half` argument, so it outranks both of the importing module's overloads.
public half f(half x)
{
    return x * (half)2;
}
