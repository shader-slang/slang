//TEST:SIMPLE(filecheck=CHECK):

// A generic default argument whose substituted type does not satisfy a source
// constraint now reports the focused non-conformance diagnostic instead of the
// general "could not specialize generic" fallback. The first failing witness in
// a default chain is reported: `NotElement` does not satisfy `IElement`, and in
// the chained case `GoodData` (the `U` default) does not satisfy `IStrongData`.

// CHECK: error[E38029]: type argument doesn't conform to interface
// CHECK: defaultNeedsUnsatisfiedWitness();
// CHECK: type argument 'NotElement' does not conform to the required interface 'IElement'
// CHECK: note: see declaration of func defaultNeedsUnsatisfiedWitness<T, U> -> void
// CHECK: defaultNeedsUnsatisfiedWitness<T : IElement = NotElement, U : IData = T.DataType>()
// CHECK: error[E38029]: type argument doesn't conform to interface
// CHECK: defaultChainFailsAtSecondWitness();
// CHECK: type argument 'GoodData' does not conform to the required interface 'IStrongData'
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
