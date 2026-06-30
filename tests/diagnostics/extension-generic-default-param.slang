// Regression test for #11330: a default value on an `extension`'s
// generic parameter is meaningless — extensions are bound by unifying
// their parameters with the target type at the application site, so
// there is no caller that could leave one unset and have the default
// apply. Pre-fix the compiler silently accepted both the type-param
// (`T=int`) and value-param (`int N=42`) shapes.
//
// `_validateExtensionDeclGenericParams` in
// `source/slang/slang-check-decl.cpp` now diagnoses any non-null
// `GenericTypeParamDecl::initType` / `GenericValueParamDecl::initExpr`,
// gated on the parameter being referenced by the target type so the
// new E30857 doesn't stack on the existing E30855/E30856 diagnostics
// for unreferenced params. Plain non-extension generics (e.g.
// `struct Foo<T = int>`) keep working — see
// `tests/language-feature/generics/generic-default-type-param.slang`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Foo<T> {}

extension<T = int> Foo<T> {}
//CHECK:  ^ default value on generic parameter 'T' is not allowed in an 'extension' declaration; the parameter is bound by unifying with the target type, so the default could never be used
//CHECK:  ^ default value on generic parameter 'T' has no effect in an 'extension'

struct Bar<int N> {}

extension<int N = 42> Bar<N> {}
//CHECK:      ^ default value on generic parameter 'N' is not allowed in an 'extension' declaration; the parameter is bound by unifying with the target type, so the default could never be used
//CHECK:      ^ default value on generic parameter 'N' has no effect in an 'extension'

// Multi-parameter cases — exercise the
// `getDirectMemberDecls()` loop progression.

struct Baz<T, U> {}

// Default on the second type-param.
extension<T, U = int> Baz<T, U> {}
//CHECK:     ^ default value on generic parameter 'U' is not allowed in an 'extension' declaration; the parameter is bound by unifying with the target type, so the default could never be used
//CHECK:     ^ default value on generic parameter 'U' has no effect in an 'extension'

// Default on the first type-param.
extension<T = int, U> Baz<T, U> {}
//CHECK:  ^ default value on generic parameter 'T' is not allowed in an 'extension' declaration; the parameter is bound by unifying with the target type, so the default could never be used
//CHECK:  ^ default value on generic parameter 'T' has no effect in an 'extension'

struct Qux<T, int M> {}

// Mixed type-param + value-param defaults — both must fire.
extension<T = int, int M = 5> Qux<T, M> {}
//CHECK:  ^ default value on generic parameter 'T' is not allowed in an 'extension' declaration; the parameter is bound by unifying with the target type, so the default could never be used
//CHECK:  ^ default value on generic parameter 'T' has no effect in an 'extension'
//CHECK:               ^ default value on generic parameter 'M' is not allowed in an 'extension' declaration; the parameter is bound by unifying with the target type, so the default could never be used
//CHECK:               ^ default value on generic parameter 'M' has no effect in an 'extension'
