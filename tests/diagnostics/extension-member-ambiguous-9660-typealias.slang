//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for shader-slang/slang#9660, type-kind conflict via `typealias`.
//
// A `typealias` is a `SimpleTypeDecl` rather than the `AggTypeDecl` already covered by the nested
// `struct` positive case. A same-named typealias on both the primary `struct` and an `extension`
// is a type-kind member, compared by name alone, so this must warn (41037).

struct ConcreteAlias
{
    typealias Handle = int;
//CHECK:      ^^^^^^ see the conflicting declaration of 'Handle' here
}

extension ConcreteAlias
{
    typealias Handle = float;
//CHECK:      ^^^^^^ member declared in more than one place
//CHECK:      ^^^^^^ 'Handle' is declared in more than one place (the extended type and/or its 'extension's); which declaration takes effect is currently undefined (see issue #9660)
}
