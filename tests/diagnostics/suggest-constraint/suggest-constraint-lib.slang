// Helper module for the issue #13140 tests in this directory (no test directive of its own).

module "suggest-constraint-lib";

public interface IPublicThing
{
    int publicThing();
}

public interface IPublicBox<T>
{
    T publicItem();
}

internal interface IInternalThing
{
    int internalThing();
}
