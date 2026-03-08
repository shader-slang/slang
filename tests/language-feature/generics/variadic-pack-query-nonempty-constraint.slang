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

struct BadConstraint<each T> where nonempty(int) {}
//CHECK:                                    ^^^ `nonempty(...)` requires a generic type pack or value pack parameter

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
    ^^^^^^^^^^^^^^^ empty pack does not satisfy `nonempty(...)` constraint
    ^^^^^^^^^^^^^^^ pack argument here is empty
*/
}

void badTrimmedForward<let each D : int>() where nonempty(D)
{
    consumeNonEmpty<__trimHead(D)>();
/*CHECK:
    ^^^^^^^^^^^^^^^ empty pack does not satisfy `nonempty(...)` constraint
    ^^^^^^^^^^^^^^^ pack argument here is empty
*/
}

int badSpecialization = goodValueUse<>();
//CHECK:                              ^ could not specialize generic for arguments of type ()
