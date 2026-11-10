//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):

// Tests that we will diagnose a warning on struct inheritance being unstable
// before Slang 2026.

#lang 2025

struct Base {}

struct Derived : Base {}
//CHECK:         ^^^^ support for inheritance is unstable and will be removed in future language versions, consider using composition instead

struct Base1 {}