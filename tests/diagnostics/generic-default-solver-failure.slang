//TEST:SIMPLE(filecheck=CHECK):

// CHECK: error[E39999]: could not specialize generic for arguments of type ()
// CHECK: defaultNeedsUnsatisfiedWitness();
// CHECK: note: see declaration of func defaultNeedsUnsatisfiedWitness<T, U> -> void
// CHECK: defaultNeedsUnsatisfiedWitness<T : IElement = NotElement, U : IData = T.DataType>()
// CHECK: error[E39999]: could not specialize generic for arguments of type ()
// CHECK: defaultChainFailsAtSecondWitness();
// CHECK: note: see declaration of func defaultChainFailsAtSecondWitness<T, U, V> -> void
// CHECK: defaultChainFailsAtSecondWitness<T : IElement = GoodElement, U : IData = T.DataType, V : IStrongData = U>()

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
{
}

void defaultChainFailsAtSecondWitness<T : IElement = GoodElement, U : IData = T.DataType, V : IStrongData = U>()
{
}

void useDefaultNeedsUnsatisfiedWitness()
{
    defaultNeedsUnsatisfiedWitness();

    defaultChainFailsAtSecondWitness();
}
