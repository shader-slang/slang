package gcp

import (
	"context"
	"errors"
	"fmt"
	"slices"
	"strings"
	"sync"
	"testing"
	"time"

	computepb "cloud.google.com/go/compute/apiv1/computepb"
)

func TestCleanupFilter(t *testing.T) {
	got := cleanupFilter("win-runner")
	want := "name=win-runner-* AND status=TERMINATED"
	if got != want {
		t.Fatalf("cleanupFilter() = %q, want %q", got, want)
	}
}

func TestLiveFilter(t *testing.T) {
	got := liveFilter("linux-test")
	want := "name=linux-test-* AND (status=PROVISIONING OR status=STAGING OR status=RUNNING OR status=REPAIRING)"
	if got != want {
		t.Fatalf("liveFilter() = %q, want %q", got, want)
	}
}

func TestIsLiveStatus(t *testing.T) {
	tests := map[string]bool{
		"PROVISIONING": true,
		"STAGING":      true,
		"RUNNING":      true,
		"REPAIRING":    true,
		"STOPPING":     false,
		"TERMINATED":   false,
		"":             false,
	}

	for status, want := range tests {
		if got := isLiveStatus(status); got != want {
			t.Fatalf("isLiveStatus(%q) = %v, want %v", status, got, want)
		}
	}
}

func TestRemoveTrackedVMByVMName(t *testing.T) {
	m := &Manager{
		vms: map[string]*vmInfo{
			"runner-a": {vmName: "win-runner-1234"},
			"runner-b": {vmName: "win-runner-5678"},
		},
	}

	m.removeTrackedVMByVMName("win-runner-1234")

	if _, ok := m.vms["runner-a"]; ok {
		t.Fatalf("runner-a should be removed when vmName matches")
	}
	if _, ok := m.vms["runner-b"]; !ok {
		t.Fatalf("runner-b should remain")
	}
}

func TestSplitZonesTrimsAndDeduplicates(t *testing.T) {
	got := splitZones(" us-east1-d,us-east1-d,,us-east1-b, us-east1-b ")
	want := []string{"us-east1-d", "us-east1-b"}
	if !slices.Equal(got, want) {
		t.Fatalf("splitZones = %v, want %v", got, want)
	}
}

func TestRunCleanupLoopRunsImmediatePass(t *testing.T) {
	m := &Manager{}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	calls := 0
	m.cleanupPass = func(_ context.Context) {
		calls++
		cancel()
	}

	done := make(chan struct{})
	go func() {
		m.runCleanupLoop(ctx, make(chan time.Time))
		close(done)
	}()

	select {
	case <-done:
	case <-time.After(2 * time.Second):
		t.Fatal("cleanup loop did not exit")
	}

	if calls != 1 {
		t.Fatalf("cleanup pass calls = %d, want 1", calls)
	}
}

func TestNormalizeOrphanGracePeriod(t *testing.T) {
	if got := normalizeOrphanGracePeriod(0); got != defaultOrphanGracePeriod {
		t.Fatalf("zero grace should use default, got %v", got)
	}
	if got := normalizeOrphanGracePeriod(-time.Minute); got != -time.Minute {
		t.Fatalf("negative grace should remain disabled, got %v", got)
	}
	if got := normalizeOrphanGracePeriod(5 * time.Minute); got != 5*time.Minute {
		t.Fatalf("positive grace should be preserved, got %v", got)
	}
}

func TestRunCleanupLoopRunsOnTick(t *testing.T) {
	m := &Manager{}
	ticks := make(chan time.Time, 1)

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	calls := 0
	m.cleanupPass = func(_ context.Context) {
		calls++
		if calls == 2 {
			cancel()
		}
	}

	done := make(chan struct{})
	go func() {
		m.runCleanupLoop(ctx, ticks)
		close(done)
	}()

	ticks <- time.Now()

	select {
	case <-done:
	case <-time.After(2 * time.Second):
		t.Fatal("cleanup loop did not exit")
	}

	if calls != 2 {
		t.Fatalf("cleanup pass calls = %d, want 2", calls)
	}
}

func TestDoCleanupTerminatedVMsContinuesAfterListError(t *testing.T) {
	m := &Manager{
		config: ManagerConfig{
			Zones: "us-east1-c,us-west1-a",
		},
		vms: map[string]*vmInfo{
			"runner-a": {vmName: "win-runner-a"},
			"runner-b": {vmName: "win-runner-b"},
		},
	}

	listCalls := 0
	m.listTerminated = func(_ context.Context, zone string) ([]string, error) {
		listCalls++
		if zone == "us-east1-c" {
			return nil, errors.New("list failed")
		}
		return []string{"win-runner-b"}, nil
	}

	deleted := make([]string, 0, 1)
	m.deleteVMFunc = func(_ context.Context, vmName, _ string) error {
		deleted = append(deleted, vmName)
		return nil
	}

	m.doCleanupTerminatedVMs(context.Background())

	if listCalls != 2 {
		t.Fatalf("list calls = %d, want 2", listCalls)
	}
	if !slices.Equal(deleted, []string{"win-runner-b"}) {
		t.Fatalf("deleted VMs = %v, want [win-runner-b]", deleted)
	}
	if _, ok := m.vms["runner-b"]; ok {
		t.Fatalf("runner-b should be removed after successful cleanup delete")
	}
	if _, ok := m.vms["runner-a"]; !ok {
		t.Fatalf("runner-a should remain when its zone list failed")
	}
}

func TestDoCleanupTerminatedVMsDeleteErrorStillRemovesTrackedEntry(t *testing.T) {
	m := &Manager{
		config: ManagerConfig{
			Zones: "us-east1-c",
		},
		vms: map[string]*vmInfo{
			"runner-a": {vmName: "win-runner-a"},
		},
	}

	m.listTerminated = func(_ context.Context, _ string) ([]string, error) {
		return []string{"win-runner-a"}, nil
	}
	m.deleteVMFunc = func(_ context.Context, _, _ string) error {
		return errors.New("delete failed")
	}

	m.doCleanupTerminatedVMs(context.Background())

	if _, ok := m.vms["runner-a"]; ok {
		t.Fatalf("runner-a should be removed from tracked map even when delete fails")
	}
}

func TestDoCleanupTerminatedVMsDeletesPartialListResultsOnError(t *testing.T) {
	m := &Manager{
		config: ManagerConfig{
			Zones: "us-east1-c",
		},
		vms: map[string]*vmInfo{
			"runner-a": {vmName: "win-runner-a"},
		},
	}

	m.listTerminated = func(_ context.Context, _ string) ([]string, error) {
		return []string{"win-runner-a"}, errors.New("partial page read failed")
	}

	deleted := make([]string, 0, 1)
	m.deleteVMFunc = func(_ context.Context, vmName, _ string) error {
		deleted = append(deleted, vmName)
		return nil
	}

	m.doCleanupTerminatedVMs(context.Background())

	if !slices.Equal(deleted, []string{"win-runner-a"}) {
		t.Fatalf("deleted VMs = %v, want [win-runner-a]", deleted)
	}
	if _, ok := m.vms["runner-a"]; ok {
		t.Fatalf("runner-a should be removed after deleting partial list result")
	}
}

func TestReconcileKeepsLiveTrackedVMs(t *testing.T) {
	m := &Manager{
		vms: map[string]*vmInfo{
			"runner-a": {vmName: "linux-test-a", zone: "us-east1-c"},
		},
	}
	m.listLive = func(_ context.Context, zone string) ([]string, error) {
		if zone != "us-east1-c" {
			t.Fatalf("zone = %q, want us-east1-c", zone)
		}
		return []string{"linux-test-a"}, nil
	}

	m.reconcileTrackedVMs(context.Background())

	if _, ok := m.vms["runner-a"]; !ok {
		t.Fatalf("runner-a should remain while VM is live")
	}
}

func TestReconcileEvictsMissingTrackedVMs(t *testing.T) {
	m := &Manager{
		vms: map[string]*vmInfo{
			"runner-a": {vmName: "linux-test-a", zone: "us-east1-c"},
		},
	}
	m.listLive = func(_ context.Context, _ string) ([]string, error) {
		return nil, nil
	}

	m.reconcileTrackedVMs(context.Background())

	if _, ok := m.vms["runner-a"]; ok {
		t.Fatalf("runner-a should be removed when VM is no longer live")
	}
}

func TestReconcileKeepsTrackedVMsWhenListFails(t *testing.T) {
	m := &Manager{
		vms: map[string]*vmInfo{
			"runner-a": {vmName: "linux-test-a", zone: "us-east1-c"},
		},
	}
	m.listLive = func(_ context.Context, _ string) ([]string, error) {
		return nil, errors.New("list failed")
	}

	m.reconcileTrackedVMs(context.Background())

	if _, ok := m.vms["runner-a"]; !ok {
		t.Fatalf("runner-a should remain when live VM listing fails")
	}
}

func TestReconcileDoesNotEvictVMAddedAfterSnapshot(t *testing.T) {
	m := &Manager{
		vms: map[string]*vmInfo{
			"runner-a": {vmName: "linux-test-a", zone: "us-east1-c"},
		},
	}
	m.listLive = func(_ context.Context, _ string) ([]string, error) {
		m.mu.Lock()
		m.vms["runner-b"] = &vmInfo{vmName: "linux-test-b", zone: "us-east1-c"}
		m.mu.Unlock()
		return nil, nil
	}

	m.reconcileTrackedVMs(context.Background())

	if _, ok := m.vms["runner-a"]; ok {
		t.Fatalf("runner-a should be removed when snapshot VM is no longer live")
	}
	if _, ok := m.vms["runner-b"]; !ok {
		t.Fatalf("runner-b should remain because it was added after the snapshot")
	}
}

func TestSelectZoneErrorsOnEmptyCandidates(t *testing.T) {
	m := &Manager{}
	m.selectZonesFunc = func(context.Context) ([]zoneCandidate, error) {
		return nil, nil
	}

	if _, err := m.selectZone(context.Background()); err == nil {
		t.Fatal("selectZone should fail for empty candidate list")
	} else if !strings.Contains(err.Error(), "no zone candidates available") {
		t.Fatalf("selectZone error = %q, want empty-candidate failure", err)
	}
}

func TestSelectZonesErrorsOnInvalidZone(t *testing.T) {
	m := &Manager{
		config: ManagerConfig{
			Zones:   "us-east1-d,invalid-zone",
			GPUType: "nvidia-l4",
		},
	}

	_, err := m.selectZones(context.Background())
	if err == nil {
		t.Fatal("selectZones should fail for invalid zone config")
	}
	if !strings.Contains(err.Error(), "invalid-zone") {
		t.Fatalf("selectZones error = %q, want invalid zone name", err)
	}
}

func TestCreateVMTryNextZoneAfterStockout(t *testing.T) {
	m := &Manager{
		config: ManagerConfig{
			Project:          "test-project",
			Zones:            "us-east1-d,us-east1-b",
			InstanceTemplate: "linux-gpu-runner-sm80plus-l4",
			GPUType:          "nvidia-l4",
			Platform:         "linux",
		},
		vms:            map[string]*vmInfo{},
		pendingCreates: map[string]zoneCandidate{},
	}
	m.selectZonesFunc = func(context.Context) ([]zoneCandidate, error) {
		return []zoneCandidate{
			{zone: "us-east1-d", region: "us-east1", available: 16},
			{zone: "us-east1-b", region: "us-east1", available: 16},
		}, nil
	}

	var attempts []string
	m.insertVMFunc = func(_ context.Context, req *computepb.InsertInstanceRequest) error {
		attempts = append(attempts, req.GetZone())
		if req.GetInstanceResource().GetName() != "linux-sm80plus-test" {
			t.Fatalf("VM name = %q, want linux-sm80plus-test", req.GetInstanceResource().GetName())
		}
		if req.GetZone() == "us-east1-d" {
			return errors.New("ZONE_RESOURCE_POOL_EXHAUSTED_WITH_DETAILS: resource_availability")
		}
		return nil
	}

	vmName, err := m.CreateVM(context.Background(), "linux-sm80plus-test", "jit-config")
	if err != nil {
		t.Fatalf("CreateVM returned error: %v", err)
	}
	if vmName != "linux-sm80plus-test" {
		t.Fatalf("vmName = %q, want linux-sm80plus-test", vmName)
	}
	if !slices.Equal(attempts, []string{"us-east1-d", "us-east1-b"}) {
		t.Fatalf("attempted zones = %v, want [us-east1-d us-east1-b]", attempts)
	}
	tracked, ok := m.vms["linux-sm80plus-test"]
	if !ok {
		t.Fatal("expected VM to be tracked after successful CreateVM")
	}
	if tracked.zone != "us-east1-b" {
		t.Fatalf("tracked zone = %q, want us-east1-b", tracked.zone)
	}
}

func TestCreateVMAllCandidateZonesStockout(t *testing.T) {
	m := &Manager{
		config: ManagerConfig{
			Project:          "test-project",
			Zones:            "us-east1-d,us-east1-b",
			InstanceTemplate: "linux-gpu-runner-sm80plus-l4",
			GPUType:          "nvidia-l4",
			Platform:         "linux",
		},
		vms:            map[string]*vmInfo{},
		pendingCreates: map[string]zoneCandidate{},
	}
	m.selectZonesFunc = func(context.Context) ([]zoneCandidate, error) {
		return []zoneCandidate{
			{zone: "us-east1-d", region: "us-east1", available: 16},
			{zone: "us-east1-b", region: "us-east1", available: 16},
		}, nil
	}

	var attempts []string
	m.insertVMFunc = func(_ context.Context, req *computepb.InsertInstanceRequest) error {
		attempts = append(attempts, req.GetZone())
		return errors.New("ZONE_RESOURCE_POOL_EXHAUSTED_WITH_DETAILS: resource_availability")
	}

	_, err := m.CreateVM(context.Background(), "linux-sm80plus-test", "jit-config")
	if err == nil {
		t.Fatal("CreateVM should fail when all candidate zones are out of stock")
	}
	if !slices.Equal(attempts, []string{"us-east1-d", "us-east1-b"}) {
		t.Fatalf("attempted zones = %v, want [us-east1-d us-east1-b]", attempts)
	}
	for _, want := range []string{
		"all candidate zones are out of stock",
		"us-east1-d",
		"us-east1-b",
	} {
		if !strings.Contains(err.Error(), want) {
			t.Fatalf("CreateVM error = %q, want substring %q", err, want)
		}
	}
	if len(m.vms) != 0 {
		t.Fatalf("tracked VM count = %d, want 0", len(m.vms))
	}
}

func TestCreateVMStopsOnNonStockoutError(t *testing.T) {
	m := &Manager{
		config: ManagerConfig{
			Project:          "test-project",
			Zones:            "us-east1-d,us-east1-b",
			InstanceTemplate: "linux-gpu-runner-sm80plus-l4",
			GPUType:          "nvidia-l4",
			Platform:         "linux",
		},
		vms:            map[string]*vmInfo{},
		pendingCreates: map[string]zoneCandidate{},
	}
	m.selectZonesFunc = func(context.Context) ([]zoneCandidate, error) {
		return []zoneCandidate{
			{zone: "us-east1-d", region: "us-east1", available: 16},
			{zone: "us-east1-b", region: "us-east1", available: 16},
		}, nil
	}

	var attempts []string
	m.insertVMFunc = func(_ context.Context, req *computepb.InsertInstanceRequest) error {
		attempts = append(attempts, req.GetZone())
		return errors.New("permission denied")
	}

	if _, err := m.CreateVM(context.Background(), "linux-sm80plus-test", "jit-config"); err == nil {
		t.Fatal("CreateVM should fail on non-stockout errors")
	}
	if !slices.Equal(attempts, []string{"us-east1-d"}) {
		t.Fatalf("attempted zones = %v, want [us-east1-d]", attempts)
	}
	if len(m.vms) != 0 {
		t.Fatalf("tracked VM count = %d, want 0", len(m.vms))
	}
	if got := m.ActiveCount(); got != 0 {
		t.Fatalf("active count after failed CreateVM = %d, want 0", got)
	}
}

func TestCreateVMConcurrentNonGPUCreatesReserveZones(t *testing.T) {
	m := &Manager{
		config: ManagerConfig{
			Project:          "test-project",
			Zones:            "us-east1-c,us-east1-d,us-central1-a",
			InstanceTemplate: "linux-build-runner",
			GPUType:          "none",
			Platform:         "linux",
		},
		vms:            map[string]*vmInfo{},
		pendingCreates: map[string]zoneCandidate{},
	}

	const createCount = 5
	insertEntered := make(chan struct{}, createCount)
	releaseInserts := make(chan struct{})
	var releaseInsertsOnce sync.Once
	releasePendingInserts := func() {
		releaseInsertsOnce.Do(func() { close(releaseInserts) })
	}
	defer releasePendingInserts()
	var mu sync.Mutex
	var zones []string

	m.insertVMFunc = func(_ context.Context, req *computepb.InsertInstanceRequest) error {
		mu.Lock()
		zones = append(zones, req.GetZone())
		mu.Unlock()

		insertEntered <- struct{}{}
		<-releaseInserts
		return nil
	}

	var wg sync.WaitGroup
	errs := make(chan error, createCount)
	for i := range createCount {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			runnerName := fmt.Sprintf("linux-build-test-%d", i)
			_, err := m.CreateVM(context.Background(), runnerName, "jit-config")
			errs <- err
		}(i)
	}

	for i := 0; i < createCount; i++ {
		select {
		case <-insertEntered:
		case <-time.After(2 * time.Second):
			t.Fatal("timed out waiting for concurrent inserts to start")
		}
	}

	if got := m.ActiveCount(); got != createCount {
		t.Fatalf("active count while inserts are pending = %d, want %d", got, createCount)
	}

	releasePendingInserts()
	wg.Wait()
	close(errs)
	for err := range errs {
		if err != nil {
			t.Fatalf("CreateVM returned error: %v", err)
		}
	}

	wantZoneCounts := map[string]int{
		"us-east1-c":    2,
		"us-east1-d":    2,
		"us-central1-a": 1,
	}
	gotZoneCounts := map[string]int{}
	mu.Lock()
	for _, zone := range zones {
		gotZoneCounts[zone]++
	}
	mu.Unlock()
	if len(zones) != createCount {
		t.Fatalf("insert count = %d, want %d", len(zones), createCount)
	}
	for zone, want := range wantZoneCounts {
		if got := gotZoneCounts[zone]; got != want {
			t.Fatalf("zone %s insert count = %d, want %d (all zones: %v)", zone, got, want, gotZoneCounts)
		}
	}
	if got := m.ActiveCount(); got != createCount {
		t.Fatalf("active count after CreateVM completes = %d, want %d", got, createCount)
	}
}

func TestCreateVMConcurrentGPUCreatesRespectPendingQuota(t *testing.T) {
	m := &Manager{
		config: ManagerConfig{
			Project:          "test-project",
			Zones:            "us-east1-d",
			InstanceTemplate: "linux-gpu-runner-sm80plus-l4",
			GPUType:          "nvidia-l4",
			Platform:         "linux",
		},
		vms:            map[string]*vmInfo{},
		pendingCreates: map[string]zoneCandidate{},
	}
	m.selectZonesFunc = func(context.Context) ([]zoneCandidate, error) {
		return []zoneCandidate{{zone: "us-east1-d", region: "us-east1", available: 1}}, nil
	}

	insertEntered := make(chan struct{}, 1)
	releaseInsert := make(chan struct{})
	m.insertVMFunc = func(context.Context, *computepb.InsertInstanceRequest) error {
		insertEntered <- struct{}{}
		<-releaseInsert
		return nil
	}

	firstErr := make(chan error, 1)
	go func() {
		_, err := m.CreateVM(context.Background(), "linux-sm80plus-a", "jit-config")
		firstErr <- err
	}()

	select {
	case <-insertEntered:
	case <-time.After(2 * time.Second):
		t.Fatal("timed out waiting for first insert to start")
	}

	if got := m.ActiveCount(); got != 1 {
		t.Fatalf("active count while first insert is pending = %d, want 1", got)
	}

	_, err := m.CreateVM(context.Background(), "linux-sm80plus-b", "jit-config")
	if err == nil {
		t.Fatal("second CreateVM should fail while reported GPU quota is fully reserved")
	}
	if !strings.Contains(err.Error(), "no candidate zones have unreserved nvidia-l4 quota") {
		t.Fatalf("second CreateVM error = %q, want unreserved quota error", err)
	}
	if got := m.ActiveCount(); got != 1 {
		t.Fatalf("active count after rejected second create = %d, want 1", got)
	}

	close(releaseInsert)
	if err := <-firstErr; err != nil {
		t.Fatalf("first CreateVM returned error: %v", err)
	}
	if got := m.ActiveCount(); got != 1 {
		t.Fatalf("active count after first CreateVM completes = %d, want 1", got)
	}
}

// TestCreateVMConcurrentGPUCreatesSpreadAcrossZonesInRegion verifies that when
// a region has multiple zones and enough quota for several VMs, concurrent
// reservations spread across the zones instead of all herding onto the first
// one. Without intra-region spreading the burst would pile onto a single zone
// and turn zonal stockouts into avoidable retries.
func TestCreateVMConcurrentGPUCreatesSpreadAcrossZonesInRegion(t *testing.T) {
	m := &Manager{
		config: ManagerConfig{
			Project:          "test-project",
			Zones:            "us-east1-d,us-east1-b",
			InstanceTemplate: "linux-gpu-runner-sm80plus-l4",
			GPUType:          "nvidia-l4",
			Platform:         "linux",
		},
		vms:            map[string]*vmInfo{},
		pendingCreates: map[string]zoneCandidate{},
	}
	// One region, two zones, room for two concurrent VMs.
	m.selectZonesFunc = func(context.Context) ([]zoneCandidate, error) {
		return []zoneCandidate{
			{zone: "us-east1-d", region: "us-east1", available: 2},
			{zone: "us-east1-b", region: "us-east1", available: 2},
		}, nil
	}

	const createCount = 2
	insertEntered := make(chan struct{}, createCount)
	releaseInserts := make(chan struct{})
	var mu sync.Mutex
	var zones []string
	m.insertVMFunc = func(_ context.Context, req *computepb.InsertInstanceRequest) error {
		mu.Lock()
		zones = append(zones, req.GetZone())
		mu.Unlock()
		insertEntered <- struct{}{}
		<-releaseInserts
		return nil
	}

	var wg sync.WaitGroup
	errs := make(chan error, createCount)
	for i := range createCount {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			_, err := m.CreateVM(context.Background(), fmt.Sprintf("linux-sm80plus-%d", i), "jit-config")
			errs <- err
		}(i)
	}

	for i := 0; i < createCount; i++ {
		select {
		case <-insertEntered:
		case <-time.After(2 * time.Second):
			t.Fatal("timed out waiting for concurrent inserts to start")
		}
	}

	close(releaseInserts)
	wg.Wait()
	close(errs)
	for err := range errs {
		if err != nil {
			t.Fatalf("CreateVM returned error: %v", err)
		}
	}

	mu.Lock()
	defer mu.Unlock()
	gotZoneCounts := map[string]int{}
	for _, zone := range zones {
		gotZoneCounts[zone]++
	}
	wantZoneCounts := map[string]int{"us-east1-d": 1, "us-east1-b": 1}
	for zone, want := range wantZoneCounts {
		if got := gotZoneCounts[zone]; got != want {
			t.Fatalf("zone %s insert count = %d, want %d (all zones: %v)", zone, got, want, gotZoneCounts)
		}
	}
}

// TestCreateVMStampsExpectGPUMetadata verifies that CreateVM stamps the
// "expect-gpu" instance-metadata key from the pool's GPUType: "true" for GPU
// pools and "false" for CPU-only pools (GPUType == "none"). The Linux startup
// script reads this to decide whether a missing accelerator is fatal (GPU pool)
// or expected (CPU-only build/analytics pool), so the contract must be explicit
// rather than inferred from the VM's PCI state.
func TestCreateVMStampsExpectGPUMetadata(t *testing.T) {
	cases := []struct {
		name    string
		gpuType string
		want    string
	}{
		{"gpu pool", "nvidia-l4", "true"},
		{"cpu-only pool", "none", "false"},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			m := &Manager{
				config: ManagerConfig{
					Project:          "test-project",
					Zones:            "us-east1-d",
					InstanceTemplate: "linux-build-runner",
					GPUType:          tc.gpuType,
					Platform:         "linux",
				},
				vms:            map[string]*vmInfo{},
				pendingCreates: map[string]zoneCandidate{},
			}
			m.selectZonesFunc = func(context.Context) ([]zoneCandidate, error) {
				return []zoneCandidate{{zone: "us-east1-d", region: "us-east1", available: 16}}, nil
			}

			var got string
			var found bool
			m.insertVMFunc = func(_ context.Context, req *computepb.InsertInstanceRequest) error {
				for _, item := range req.GetInstanceResource().GetMetadata().GetItems() {
					if item.GetKey() == "expect-gpu" {
						got = item.GetValue()
						found = true
					}
				}
				return nil
			}

			if _, err := m.CreateVM(context.Background(), "build-test", "jit-config"); err != nil {
				t.Fatalf("CreateVM returned error: %v", err)
			}
			if !found {
				t.Fatal("expect-gpu metadata key was not set")
			}
			if got != tc.want {
				t.Fatalf("expect-gpu = %q, want %q", got, tc.want)
			}
		})
	}
}

func TestIsZoneResourceExhausted(t *testing.T) {
	if !isZoneResourceExhausted(errors.New("ZONE_RESOURCE_POOL_EXHAUSTED_WITH_DETAILS")) {
		t.Fatal("expected ZONE_RESOURCE_POOL_EXHAUSTED_WITH_DETAILS to be treated as stockout")
	}
	if !isZoneResourceExhausted(errors.New("resource_availability: does not have enough resources")) {
		t.Fatal("expected resource_availability to be treated as stockout")
	}
	if !isZoneResourceExhausted(errors.New("Resource_Availability: Does Not Have Enough Resources")) {
		t.Fatal("expected mixed-case resource availability message to be treated as stockout")
	}
	if isZoneResourceExhausted(errors.New("permission denied")) {
		t.Fatal("permission denied should not be treated as stockout")
	}
}

// fakeClock returns a closure suitable for Manager.now that always
// returns the same fixed time. Tests use it to drive evictStaleOrphans
// deterministically without sleeping.
func fakeClock(t time.Time) func() time.Time {
	return func() time.Time { return t }
}

func TestEvictStaleOrphansRemovesIdleVMPastGrace(t *testing.T) {
	now := time.Date(2026, 5, 11, 0, 0, 0, 0, time.UTC)
	stale := now.Add(-45 * time.Minute)

	deleted := make(map[string]string)
	m := &Manager{
		config:  ManagerConfig{OrphanGracePeriod: 30 * time.Minute},
		nowFunc: fakeClock(now),
		vms: map[string]*vmInfo{
			"runner-orphan": {vmName: "linux-test-orphan", zone: "us-east1-c", createdAt: stale},
		},
		deleteVMFunc: func(_ context.Context, vmName, zone string) error {
			deleted[vmName] = zone
			return nil
		},
	}

	m.evictStaleOrphans(context.Background())

	if _, ok := m.vms["runner-orphan"]; ok {
		t.Fatal("expected stale idle orphan to be evicted from tracking")
	}
	if got := deleted["linux-test-orphan"]; got != "us-east1-c" {
		t.Fatalf("expected delete call for orphan in us-east1-c, got %q", got)
	}
}

func TestEvictStaleOrphansSparesFreshAndBusyVMs(t *testing.T) {
	now := time.Date(2026, 5, 11, 0, 0, 0, 0, time.UTC)

	deleted := 0
	m := &Manager{
		config:  ManagerConfig{OrphanGracePeriod: 30 * time.Minute},
		nowFunc: fakeClock(now),
		vms: map[string]*vmInfo{
			// Younger than grace period — keep.
			"runner-fresh": {vmName: "linux-test-fresh", zone: "us-east1-c", createdAt: now.Add(-5 * time.Minute)},
			// Older than grace period but busy — keep (it's running a job).
			"runner-busy": {vmName: "linux-test-busy", zone: "us-east1-c", busy: true, createdAt: now.Add(-2 * time.Hour)},
		},
		deleteVMFunc: func(context.Context, string, string) error {
			deleted++
			return nil
		},
	}

	m.evictStaleOrphans(context.Background())

	if _, ok := m.vms["runner-fresh"]; !ok {
		t.Fatal("fresh VM should not be evicted")
	}
	if _, ok := m.vms["runner-busy"]; !ok {
		t.Fatal("busy VM should not be evicted regardless of age")
	}
	if deleted != 0 {
		t.Fatalf("no VMs should have been deleted, got %d", deleted)
	}
}

func TestEvictStaleOrphansDisabledByZeroGrace(t *testing.T) {
	now := time.Date(2026, 5, 11, 0, 0, 0, 0, time.UTC)
	m := &Manager{
		// Grace period 0 disables eviction (per the field doc; NewManager
		// substitutes the default, but the Manager itself respects 0).
		config:  ManagerConfig{OrphanGracePeriod: 0},
		nowFunc: fakeClock(now),
		vms: map[string]*vmInfo{
			"runner-orphan": {vmName: "linux-test-orphan", zone: "us-east1-c", createdAt: now.Add(-24 * time.Hour)},
		},
		deleteVMFunc: func(context.Context, string, string) error {
			t.Fatal("delete should not be called when grace period is zero")
			return nil
		},
	}

	m.evictStaleOrphans(context.Background())

	if _, ok := m.vms["runner-orphan"]; !ok {
		t.Fatal("VM should remain tracked when grace period is zero")
	}
}

func TestEvictStaleOrphansKeepsTrackingOnDeleteFailure(t *testing.T) {
	now := time.Date(2026, 5, 11, 0, 0, 0, 0, time.UTC)
	stale := now.Add(-45 * time.Minute)

	m := &Manager{
		config:  ManagerConfig{OrphanGracePeriod: 30 * time.Minute},
		nowFunc: fakeClock(now),
		vms: map[string]*vmInfo{
			"runner-orphan": {vmName: "linux-test-orphan", zone: "us-east1-c", createdAt: stale},
		},
		deleteVMFunc: func(context.Context, string, string) error {
			return errors.New("transient GCP error")
		},
	}

	m.evictStaleOrphans(context.Background())

	if _, ok := m.vms["runner-orphan"]; !ok {
		t.Fatal("tracking entry must survive a delete failure so the next pass retries")
	}
}

func TestEvictStaleOrphansSparesEntriesWithoutCreatedAt(t *testing.T) {
	now := time.Date(2026, 5, 11, 0, 0, 0, 0, time.UTC)
	m := &Manager{
		config:  ManagerConfig{OrphanGracePeriod: 30 * time.Minute},
		nowFunc: fakeClock(now),
		vms: map[string]*vmInfo{
			// Zero createdAt simulates legacy entries (pre-#11115 fix).
			"runner-legacy": {vmName: "linux-test-legacy", zone: "us-east1-c"},
		},
		deleteVMFunc: func(context.Context, string, string) error {
			t.Fatal("delete should not be called for entries with zero createdAt")
			return nil
		},
	}

	m.evictStaleOrphans(context.Background())

	if _, ok := m.vms["runner-legacy"]; !ok {
		t.Fatal("entry with zero createdAt should be left alone for the next pass")
	}
}

func TestEvictStaleOrphansSkipsWhenBusyBeforeDelete(t *testing.T) {
	now := time.Date(2026, 5, 11, 0, 0, 0, 0, time.UTC)
	stale := now.Add(-45 * time.Minute)

	var m *Manager
	m = &Manager{
		config:  ManagerConfig{OrphanGracePeriod: 30 * time.Minute},
		nowFunc: fakeClock(now),
		vms: map[string]*vmInfo{
			"runner-orphan": {vmName: "linux-test-orphan", zone: "us-east1-c", createdAt: stale},
		},
		beforeOrphanDelete: func(c orphanCandidate) {
			m.MarkBusy(c.runnerName)
		},
		deleteVMFunc: func(context.Context, string, string) error {
			t.Fatal("delete should not be called after the runner goes busy")
			return nil
		},
	}

	m.evictStaleOrphans(context.Background())

	if vm, ok := m.vms["runner-orphan"]; !ok {
		t.Fatal("tracking entry should be retained when the VM raced to busy")
	} else if !vm.busy {
		t.Fatal("busy flag should have survived")
	}
}

func TestEvictStaleOrphansRetainsTrackingWhenBusyDuringDelete(t *testing.T) {
	now := time.Date(2026, 5, 11, 0, 0, 0, 0, time.UTC)
	stale := now.Add(-45 * time.Minute)

	m := &Manager{
		config:  ManagerConfig{OrphanGracePeriod: 30 * time.Minute},
		nowFunc: fakeClock(now),
		vms: map[string]*vmInfo{
			"runner-orphan": {vmName: "linux-test-orphan", zone: "us-east1-c", createdAt: stale},
		},
	}
	m.deleteVMFunc = func(context.Context, string, string) error {
		// Simulate HandleJobStarted firing between the snapshot and the
		// delete completing — the runner is now busy.
		m.mu.Lock()
		m.vms["runner-orphan"].busy = true
		m.mu.Unlock()
		return nil
	}

	m.evictStaleOrphans(context.Background())

	if vm, ok := m.vms["runner-orphan"]; !ok {
		t.Fatal("tracking entry should be retained when the VM raced to busy")
	} else if !vm.busy {
		t.Fatal("busy flag should have survived")
	}
}
