// Regression test for #11157: printf args were silently dropped on the C++/LLVM
// targets when a loop separated the variadic-pack `makeStruct` from the
// `Printf` instruction (the args reached the emitter as `IRLoad` of a temp).

//TEST:EXECUTABLE:

export __extern_cpp int main()
{
    int a = 0;
    int b = 0;
    int c = 0;

    for (size_t i = 0; i < 8; ++i)
        a++;

    printf("%d, %d, %d\n", a, b, c);
    return 0;
}
