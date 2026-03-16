//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int badTypeUse<each T>()
{
    return countof(Tuple<__first(T)>);
//CHECK:                 ^^^^^^^ cannot apply __first to a pack that may be empty
//CHECK:                 ^^^^^^^ __first requires a non-empty pack; add a `where nonempty(...)` constraint or use a structurally non-empty pack
//CHECK:   ^^^^^^^ invalid countof argument
//CHECK:   ^^^^^^^ argument to countof can only be a type pack or tuple
}

int badValueUse<let each D : int>()
{
    return __first(D);
//CHECK:   ^^^^^^^ cannot apply __first to a pack that may be empty
//CHECK:   ^^^^^^^ __first requires a non-empty pack; add a `where nonempty(...)` constraint or use a structurally non-empty pack
}

int badLastValueUse<let each D : int>()
{
    return __last(D);
//CHECK:   ^^^^^^ cannot apply __last to a pack that may be empty
//CHECK:   ^^^^^^ __last requires a non-empty pack; add a `where nonempty(...)` constraint or use a structurally non-empty pack
}

int badConcreteEmptyFirst()
{
    return __first(makeTuple());
//CHECK:   ^^^^^^^ cannot apply __first to an empty pack
//CHECK:   ^^^^^^^ __first requires a non-empty pack
}

int badConcreteEmptyLast()
{
    return __last(makeTuple());
//CHECK:   ^^^^^^ cannot apply __last to an empty pack
//CHECK:   ^^^^^^ __last requires a non-empty pack
}

struct BadConstraint<each T> where nonempty(int) {}
//CHECK:                                    ^^^ `nonempty(...)` requires a generic type pack or value pack parameter

struct Outer<let each D : int>
{
    struct Inner<let each E : int> where nonempty(D) {}
//CHECK:                                          ^ `nonempty(...)` target must be a generic pack parameter declared in the current generic
//CHECK:                                          ^ this pack parameter is declared outside the current generic
};

int badTypePackUse<each T>()
{
    return countof(Tuple<__last(T)>);
//CHECK:                 ^^^^^^ cannot apply __last to a pack that may be empty
//CHECK:                 ^^^^^^ __last requires a non-empty pack; add a `where nonempty(...)` constraint or use a structurally non-empty pack
//CHECK:   ^^^^^^^ invalid countof argument
//CHECK:   ^^^^^^^ argument to countof can only be a type pack or tuple
}

int goodTypePackUse<each T>() where nonempty(T)
{
    return countof(Tuple<__first(T), __last(T)>);
}

int optionalBad<let each D : int>() where optional nonempty(D)
//CHECK:                                                    ^ `optional nonempty(...)` is not meaningful
//CHECK:                                                    ^ remove `optional` from this `nonempty(...)` constraint
{
    return 0;
}

int goodValueUse<let each D : int>() where nonempty(D)
{
    return __first(D);
}

void consumeNonEmpty<let each D : int>() where nonempty(D) {}

void badForward<let each D : int>()
{
    consumeNonEmpty<D>();
/*CHECK:
    ^^^^^^^^^^^^^^^ cannot apply nonempty(...) to a pack that may be empty
    ^^^^^^^^^^^^^^^ nonempty(...) requires a non-empty pack; add a `where nonempty(...)` constraint or use a structurally non-empty pack
*/
}

void badTrimmedForward<let each D : int>() where nonempty(D)
{
    consumeNonEmpty<__trimHead(D)>();
/*CHECK:
    ^^^^^^^^^^^^^^^ cannot apply nonempty(...) to a pack that may be empty
    ^^^^^^^^^^^^^^^ nonempty(...) requires a non-empty pack; add a `where nonempty(...)` constraint or use a structurally non-empty pack
*/
}

int badSpecialization = goodValueUse<>();
//CHECK:                              ^ could not specialize generic for arguments of type ()

interface IMarker {}

struct BadEachConstraint<each T> where expand each __trimHead(T) : IMarker {}
//CHECK:                               ^^^^^^ type '__trimHead(T)' is not a valid left hand side of a type constraint
//CHECK:                               ^^^^^^ type '__trimHead(T)' is not a valid left hand side of a type constraint
