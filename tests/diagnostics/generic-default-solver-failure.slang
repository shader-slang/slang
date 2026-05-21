//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface IData
{
}

interface IElement
{
    associatedtype DataType : IData;
}

struct NotElement
{
}

void defaultNeedsUnsatisfiedWitness<T : IElement = NotElement, U : IData = T.DataType>()
/*CHECK:
     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ see declaration of func defaultNeedsUnsatisfiedWitness<T, U> -> void
*/
{
}

void useDefaultNeedsUnsatisfiedWitness()
{
    defaultNeedsUnsatisfiedWitness();
//CHECK:                          ^ could not specialize generic for arguments of type ()
}
