// DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

interface ITagged
{
    int tag();
}

void requireTag<T : ITagged>(T value) {}

void unchecked<each T>(expand each T values) where optional T : ITagged
{
    // A projected optional pack witness is still optional before its check.
    // CHECK: type argument 'each T' does not conform to the required interface 'ITagged'
    expand requireTag<each T>(each values);
}
