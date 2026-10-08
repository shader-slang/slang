//TEST_IGNORE_FILE:

module "private-init-cross-module-helper";

public struct B
{
    // A single-parameter constructor with default (internal) visibility. This
    // matches the reproducer in issue #7152: calling it across a module
    // boundary used to crash the compiler; it must now produce an
    // accessibility diagnostic instead.
    __init(float b) {}
}

public struct C
{
    // The same shape with an *explicit* `private` modifier, to also cover the
    // explicit-visibility resolution path.
    private __init(float b) {}
}
