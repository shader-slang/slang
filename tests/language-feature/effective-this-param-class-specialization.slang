//TEST:SIMPLE: -no-codegen -Wno-38109

// When an interface requirement's `This` type is specialized to a class, its effective `this`
// parameter must use the class mode (`in`) before it is compared with the satisfying method.

interface IMutatingRequirement
{
    [mutating]
    void mutate();
}

class MutatingClass : IMutatingRequirement
{
    void mutate() {}
}

interface IConstRefRequirement
{
    [constref]
    int read();
}

class ConstRefClass : IConstRefRequirement
{
    int read()
    {
        return 0;
    }
}

interface IRefRequirement
{
    [__ref]
    void update();
}

class RefClass : IRefRequirement
{
    void update() {}
}

interface IFieldBackedProperty
{
    property value : int
    {
        get;
        set;
    }
}

// The synthesized accessors that adapt a field to this property requirement have a class
// receiver. In particular, the setter's canonical In mode must not be spelled as `[nonmutating]`,
// because parameter-mode attributes are invalid on class members.
class FieldBackedPropertyClass : IFieldBackedProperty
{
    int value;
}
