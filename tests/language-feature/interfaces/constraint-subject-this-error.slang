//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// The subject (left-hand side) of an interface `__constraint` must be one of the
// interface's (possibly nested) associated types, such as `This.A` -- never the
// bare `This` type. Adding a base to an interface is the job of an inheritance
// clause (`interface IFoo : IBar`); writing it as `__constraint This : IBar` is
// ill-formed (a checked predicate over the very base set being computed), so it is
// rejected here instead of surfacing as a confusing cyclic-reference error.

interface IBar
{
    int bar();
}

interface IFoo
{
    __constraint This : IBar;
//diag:          ^^^^ the subject of a constraint cannot be the 'This' type
//diag:          ^^^^ constrain an associated type such as 'This.A' here, or use an inheritance clause (e.g. 'interface IFoo : IBar') to give an interface a base
}
