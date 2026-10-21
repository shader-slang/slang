//TEST_IGNORE_FILE:

// Companion module for extern-overload-*.slang: defines the `extern` `float` overload of `f`.
module overloadimpl;

public export float f(float x)
{
    return x * 2.0f;
}
