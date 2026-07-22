// This example demonstrates how to use interfaces and generics in Slang,
// and how to use the extension feature to make existing types conform to an interface.

// Import the playground module so we can use the `printf` utility function.
import playground;

// Define an interface that requires a conforming type to
// provide a hash function.
interface IMyHashable
{
    int getHash();
}

// Define a generic function that can print the hash of an object.
// Here the generic type parameter `T` is constraint by the interface `IMyHashable`,
// which means that calls to `printHash` must provide an object that conforms
// to `IMyHashable`.
//
void printHash<T: IMyHashable>(T obj)
{
    printf("Hash: %d\n", obj.getHash());
}

// Define a type that conforms to the `IMyHashable` interface.
struct Circle : IMyHashable
{
    float2 position;
    float radius;

    __init(float2 p, float r)
    {
        position = p;
        radius = r;
    }

    int getHash()
    {
        return bit_cast<int>(position.x) ^ bit_cast<int>(position.y) ^ bit_cast<int>(radius);
    }
}

// Additionally, we can use Slang's extension feature to make an existing builtin type
// conform to the `IMyHashable` interface.
extension float : IMyHashable
{
    int getHash()
    {
        return bit_cast<int>(this);
    }
}

// We can also make all vector<T> type conform to the `IMyHashable` interface using an extension,
// if `T` is hashable.
extension<T:IMyHashable, int N> vector<T, N> : IMyHashable
{
    int getHash()
    {
        int hash = 0;
        for (int i = 0; i < N; i++)
        {
            hash ^= this[i].getHash();
        }
        return hash;
    }
}

void printMain()
{
    // Construct a circle.
    let circle = Circle(float2(1.0, 2.0), 3.0);

    // Call the generic function to print the hash of the circle.
    // This is OK because `Circle` conforms to `IMyHashable`.
    printHash(circle);

    // Construct a float vector and print its hash.
    let v = float3(1.0, 2.0, 3.0);

    // This call works because we made `float` IMyHashable with the extension at line 44,
    // and because `float` is IMyHashable, the extension at line 54 will also apply to make
    // all vector<float ,N> types conform to IMyHashable and thus valid to use in the call.
    printHash(v);

    // The following lines will result in a compile error because `NonHashable` does not
    // conform to `IMyHashable`:
    //
    // struct NonHashable{int data;}
    // NonHashable obj = {0};
    // printHash(obj);
}
