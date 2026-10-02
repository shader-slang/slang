> TODO

# Scope

TODO

## Runtime Scopes

TODO

When a scope is exited, [deferred statements](statements-defer.md) scheduled for the scope are executed,
regardless of how control is transferred from the runtime scope. An exception is the `do` body of the
[`do-catch` statement](statements-do-catch.md), which executes the pending deferred statements at the end of
the `catch` body when an error object is caught. See [`defer` statement](statements-defer.md) for details.

