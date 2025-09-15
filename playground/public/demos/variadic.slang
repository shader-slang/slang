// This example demonstrates how to use variadic generic in Slang.
// Slang's syntax for variadic generics is similar to that of Swift.
// The main difference is that Slang uses the `expand` keyword to
// expand the variadic arguments, while Swift uses the `repeat` keyword.

import playground;

// Define an interface that requires a conforming type to provide a print function.
//
interface IMyPrintable
{
    void print();
}

// Define a variadic function that can print any number of arguments.
//
void printAllWithTypePrefix<each T>(expand each T args) where T : IMyPrintable
{
    expand (each args).print();
}

// Main function to test the variadic `printAllWithTypePrefix` function.
//
void printMain()
{
    printAllWithTypePrefix(1, 2, 3.0, 4.0);
}

// Make `float` conform to the `IMyPrintable` interface.
extension float : IMyPrintable
{
    void print()
    {
        printf("float: %f\n", this);
    }
}

// Make `int` conform to the `IMyPrintable` interface.
extension int : IMyPrintable
{
    void print()
    {
        printf("int: %d\n", this);
    }
}
