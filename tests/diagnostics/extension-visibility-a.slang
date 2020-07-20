// extension-visibility-a.slang

interface IThing
{
	int getValue();
}

// Note: not implementing the interface here!
struct MyThing
{
	int value;
}

int helper<T : IThing>(T thing)
{
	return thing.getValue();
}