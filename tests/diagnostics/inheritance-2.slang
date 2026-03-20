//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Tests that we will diagnose an error on struct inheritance in language 2026 or later.

#lang 2026

struct Base {}

struct Derived : Base {}
//CHECK:         ^^^^ struct 'Derived' cannot inherit from non-interface type 'Base'
