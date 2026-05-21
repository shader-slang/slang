//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface ICyclicElement
{
    associatedtype Next : ICyclicElement;
}

void cyclicDefault<T : ICyclicElement = T.Next>()
//CHECK:           ^ cyclic reference 'T'
{
}

void useCyclicDefault()
{
    cyclicDefault();
}
