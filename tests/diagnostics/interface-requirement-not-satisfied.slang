// interface-requirement-not-satisfied.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHK):

interface IFoo
{
	void bar();
//CHK:^^^ see declaration of 'bar'
}

struct T : IFoo
//CHK:     ^^^^ missing interface member
//CHK:     ^^^^ type 'T' does not provide required interface member 'bar'
{
}
