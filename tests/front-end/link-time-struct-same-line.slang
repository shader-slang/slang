// Check that we can correctly parse link-time struct definitions on the same line.
// There was a bug where the parser is expecting an identifier after `struct` definition,
// even if the struct is a link-time struct ending with a `;`.

//TEST:SIMPLE: -target slangvm

interface IFoo{}
struct FooImpl : IFoo{}
export struct Foo1 : IFoo = FooImpl; export struct Foo2 : IFoo = FooImpl;