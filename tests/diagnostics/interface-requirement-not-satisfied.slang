// interface-requirement-not-satisfied.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHK):

interface IFoo
{
	void bar();
//CHK:^^^ see declaration of 'bar'
}

struct T : IFoo
//CHK:     ^^^^ error E38100
//CHK:     ^^^^ type 'T' does not provide required interface member 'bar'
{
}
