package main

import "testing"

func TestParseCleanupIntervalValid(t *testing.T) {
	got, err := parseCleanupInterval("90s")
	if err != nil {
		t.Fatalf("parseCleanupInterval returned error: %v", err)
	}
	if got.String() != "1m30s" {
		t.Fatalf("parseCleanupInterval = %v, want 1m30s", got)
	}
}

func TestParseCleanupIntervalInvalid(t *testing.T) {
	if _, err := parseCleanupInterval("not-a-duration"); err == nil {
		t.Fatal("parseCleanupInterval should fail for invalid duration")
	}
}

func TestParseSessionMaxAgeValid(t *testing.T) {
	got, err := parseSessionMaxAge("2h")
	if err != nil {
		t.Fatalf("parseSessionMaxAge returned error: %v", err)
	}
	if got.String() != "2h0m0s" {
		t.Fatalf("parseSessionMaxAge = %v, want 2h0m0s", got)
	}
}

func TestParseSessionMaxAgeZero(t *testing.T) {
	got, err := parseSessionMaxAge("0")
	if err != nil {
		t.Fatalf("parseSessionMaxAge returned error: %v", err)
	}
	if got != 0 {
		t.Fatalf("parseSessionMaxAge = %v, want 0", got)
	}
}

func TestParseSessionMaxAgeNegative(t *testing.T) {
	if _, err := parseSessionMaxAge("-1s"); err == nil {
		t.Fatal("parseSessionMaxAge should fail for negative durations")
	}
}
