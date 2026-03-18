//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Box<T>
{
    struct Nested<U> {}
}

// `Box<X>` is an ancestor of `Box<X>.Nested<X>`.
void triggerA<X>() where Box<X> == Box<X>.Nested<X>
{
}

void triggerB<X>() where Box<X>.Nested<X> == Box<X>
{
}

void useIt()
{
    triggerA<int>();
/*CHECK:
    ^^^^^^^^ E38029
    ^^^^^^^^ type argument 'Box<int>.Nested<int>' does not conform to the required interface 'Box<int>'
*/
    triggerB<int>();
/*CHECK:
    ^^^^^^^^ E38029
    ^^^^^^^^ type argument 'Box<int>.Nested<int>' does not conform to the required interface 'Box<int>'
*/
}
