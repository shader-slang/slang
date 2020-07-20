// extension-import-helper.slang

//TEST_IGNORE_FILE:

interface IThing
{
	float getValue();
}

extension float : IThing
{
	float getValue() { return this; }
}