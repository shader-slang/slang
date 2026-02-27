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
