//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Verify that a generic typealias with interface constraints
// rejects types that do not implement the required interface.

interface IValueProvider
{
    int getValue();
}

struct Wrapper<T : IValueProvider>
{
    T provider;

    int extract() { return provider.getValue(); }
}

typealias WrappedProvider<T : IValueProvider> = Wrapper<T>;

struct NotAProvider
{
    int data;
}

void test()
{
    WrappedProvider<NotAProvider> w;
/*CHECK:
    ^^^^^^^^^^^^^^^ type argument doesn't conform to interface
    ^^^^^^^^^^^^^^^ type argument 'NotAProvider' does not conform to the required interface 'IValueProvider'
*/
}
