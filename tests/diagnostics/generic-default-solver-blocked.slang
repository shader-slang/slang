//TEST:SIMPLE(filecheck=CHECK):

// CHECK: error[E39999]: could not specialize generic for arguments of type ()
// CHECK: blockedDependentDefault();
// CHECK: note: see declaration of func blockedDependentDefault<T, U> -> void
// CHECK: blockedDependentDefault<T : IElement, U : IData = T.DataType>()

// This leaves `T` unsolved while the default for `U` depends on `T.DataType`.
// There is no source-level default cycle here; the solver must reject the
// generic application when its work list becomes empty with blocked work left.

interface IData
{
}

interface IElement
{
    associatedtype DataType : IData;
}

void blockedDependentDefault<T : IElement, U : IData = T.DataType>()
{
}

void useBlockedDependentDefault()
{
    blockedDependentDefault();
}
