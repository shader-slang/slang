//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// A parameter's `no_diff` modifier affects differentiation but not overload identity. A declaration
// and definition that differ only by this modifier must form one redeclaration family, and two
// definitions must be rejected instead of forming an ambiguous overload set.

float declarationAndDefinition(no_diff float value);
float declarationAndDefinition(float value)
{
    return value;
}

float duplicateDefinition(no_diff float value)
/*CHECK:
      ^^^^^^^^^^^^^^^^^^^ see previous definition of 'duplicateDefinition'
*/
{
    return value;
}

float duplicateDefinition(float value)
/*CHECK:
      ^^^^^^^^^^^^^^^^^^^ function 'duplicateDefinition' already has a body
      ^^^^^^^^^^^^^^^^^^^ redeclared here
*/
{
    return value;
}

float useRedeclaration(float value)
{
    return declarationAndDefinition(value);
}

// Other semantic type modifiers remain part of overload identity. These aliases make `no_diff`
// an outer modifier around `unorm` or `snorm`, so overload comparison must remove only `no_diff`
// from the nested modified types. The plain `float` overload also verifies that neither
// normalization modifier collapses to its base type.
typealias U = unorm float;
typealias S = snorm float;

func normalizedModifierOverload(value: no_diff U) -> float
{
    return value;
}

func normalizedModifierOverload(value: no_diff S) -> float
{
    return value;
}

func normalizedModifierOverload(value: no_diff float) -> float
{
    return value;
}
