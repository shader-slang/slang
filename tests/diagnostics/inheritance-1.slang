//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// Tests that we will diagnose a warning on struct inheritance being unstable
// before Slang 2026.

#lang 2025

struct Base {}

//CHECK: ([[# @LINE+1]]): warning 30816:
struct Derived : Base {}

struct Base1 {}