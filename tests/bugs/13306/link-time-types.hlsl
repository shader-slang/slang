//TEST:SIMPLE(filecheck=CHECK): -Gec -no-codegen

// A local default definition cannot prove which storage a link-time replacement will require.
struct Data { uint value; };
extern struct Alias = Data;
extern struct External {};
uniform Alias aliasInput;
uniform External externalInput;

void replace()
{
    aliasInput = aliasInput;
    // CHECK: error[E30011]
    externalInput = externalInput;
    // CHECK: error[E30011]
}
