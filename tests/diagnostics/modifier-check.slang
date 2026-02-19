//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

public internal int v()
//diag:^^^^^^^^ duplicate modifier
//diag:^^^^^^^^ modifier 'internal' is redundant or conflicting with existing modifier 'public'
{}
