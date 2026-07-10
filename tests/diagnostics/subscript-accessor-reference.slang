//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature

struct ReadOnly
{
    __subscript(int index) -> float
    {
        get { return float(index); }
    }
}

struct RefOnly
{
    float value;

    __subscript(int index) -> float
    {
        ref { return value; }
    }
}

__func_extension fwd_diff(ReadOnly::__subscript::bogus)() -> void
//CHECK:                                         ^^^^^ member not found
//CHECK:                                         ^^^^^ 'bogus' is not a member of '(int) -> float'.
{
}

void testDirectCall()
{
    ReadOnly::__subscript::get();
//CHECK:      ^^^^^^^^^^^ type cannot refer to non-static member
//CHECK:      ^^^^^^^^^^^ type 'ReadOnly' cannot be used to refer to non-static member 'operator[]'

    RefOnly::__subscript::ref();
//CHECK:     ^^^^^^^^^^^ type cannot refer to non-static member
//CHECK:     ^^^^^^^^^^^ type 'RefOnly' cannot be used to refer to non-static member 'operator[]'
}
