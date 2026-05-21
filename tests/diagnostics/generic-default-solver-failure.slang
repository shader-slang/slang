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

struct GoodData : IData
{
}

struct GoodElement : IElement
{
    typealias DataType = GoodData;
}

interface IStrongData
{
}

void defaultNeedsUnsatisfiedWitness<T : IElement = NotElement, U : IData = T.DataType>()
/*CHECK:
     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ see declaration of func defaultNeedsUnsatisfiedWitness<T, U> -> void
*/
{
}

void defaultChainFailsAtSecondWitness<
/*CHECK:
     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ see declaration of func defaultChainFailsAtSecondWitness<T, U, V> -> void
*/
    T : IElement = GoodElement,
    U : IData = T.DataType,
    V : IStrongData = U>()
{
}

void useDefaultNeedsUnsatisfiedWitness()
{
    defaultNeedsUnsatisfiedWitness();
//CHECK:                          ^ could not specialize generic for arguments of type ()

    defaultChainFailsAtSecondWitness();
//CHECK:                            ^ could not specialize generic for arguments of type ()
}
