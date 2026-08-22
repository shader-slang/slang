//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -no-codegen

// The directive runs in exhaustive mode with no `//CHECK` annotations below, so
// the test passes only if the front end emits *no* diagnostics for any of these
// accepted `template<...>` forms — that zero-diagnostic requirement is the guard.

template<typename T>
struct Wrapper
{
    T value;
};

template<class T>
struct Box
{
    T item;
};

template<int N>
struct FixedArray
{
    float data[N];
};

template<typename T, int N>
struct Buffer
{
    T elements[N];
};

template<typename T = int>
struct Defaulted
{
    T value;
};

template<int N = 4>
struct DefaultedValue
{
    float data[N];
};

template<typename T>
T identity(T x)
{
    return x;
}

// `template` introduces a declaration only at declaration position; used as an
// ordinary local variable name and in an expression it still parses in HLSL.
void useTemplateAsIdentifier()
{
    int template = 3;
    template += 1;
}
