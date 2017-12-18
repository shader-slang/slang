//TEST_IGNORE_FILE:

struct A { int x; };

struct B { float y; };

struct C { Texture2D t; SamplerState s; };

struct M
{
	A a;
	B b;
	C c;
};
