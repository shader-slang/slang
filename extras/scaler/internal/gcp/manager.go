// Package gcp provides GCP Compute Engine VM lifecycle management for
// ephemeral GitHub Actions runners.
package gcp

import (
	"context"
	_ "embed"
	"fmt"
	"log/slog"
	"sort"
	"strings"
	"sync"
	"time"

	compute "cloud.google.com/go/compute/apiv1"
	computepb "cloud.google.com/go/compute/apiv1/computepb"
	"google.golang.org/api/iterator"
	"google.golang.org/protobuf/proto"

	regionspb "cloud.google.com/go/compute/apiv1/computepb"
)

const (
	cleanupZoneScanTimeout = 30 * time.Second
	cleanupDeleteTimeout   = 45 * time.Second
	defaultCleanupInterval = 2 * time.Minute
)

//go:embed startup.ps1
var windowsStartupScript string

//go:embed startup.sh
var linuxStartupScript string

// ManagerConfig holds the GCP configuration for VM management.
type ManagerConfig struct {
	Project          string // GCP project ID
	Zones            string // Comma-separated preferred zones (e.g., "us-east1-c,us-west1-a")
	InstanceTemplate string // Name of the instance template
	GPUType          string // GPU accelerator type (e.g., "nvidia-tesla-t4")
	Platform         string // "windows" or "linux"
	VMPrefix         string // VM name prefix for cleanup (e.g., "win-runner" or "linux-runner")
	CleanupInterval  time.Duration
}

type vmInfo struct {
	vmName string
	zone   string
	busy   bool
}

type zoneCandidate struct {
	zone      string
	region    string
	available float64
}

// Manager handles creating and deleting GCP VMs for GitHub Actions runners.
type Manager struct {
	config          ManagerConfig
	instancesClient *compute.InstancesClient
	regionsClient   *compute.RegionsClient
	cancelCleanup   context.CancelFunc
	cleanupPass     func(context.Context)
	listTerminated  func(context.Context, string) ([]string, error)
	deleteVMFunc    func(context.Context, string, string) error
	selectZonesFunc func(context.Context) ([]zoneCandidate, error)
	insertVMFunc    func(context.Context, *computepb.InsertInstanceRequest) error

	mu sync.Mutex
	// runnerName -> vmInfo
	vms map[string]*vmInfo
}

// NewManager creates a new GCP VM manager.
func NewManager(ctx context.Context, cfg ManagerConfig) (*Manager, error) {
	instancesClient, err := compute.NewInstancesRESTClient(ctx)
	if err != nil {
		return nil, fmt.Errorf("creating instances client: %w", err)
	}

	regionsClient, err := compute.NewRegionsRESTClient(ctx)
	if err != nil {
		instancesClient.Close()
		return nil, fmt.Errorf("creating regions client: %w", err)
	}

	if cfg.GPUType == "" {
		cfg.GPUType = "nvidia-tesla-t4"
	}
	if cfg.Platform == "" {
		cfg.Platform = "windows"
	}
	if cfg.CleanupInterval <= 0 {
		cfg.CleanupInterval = defaultCleanupInterval
	}

	cleanupCtx, cancelCleanup := context.WithCancel(ctx)

	mgr := &Manager{
		config:          cfg,
		instancesClient: instancesClient,
		regionsClient:   regionsClient,
		cancelCleanup:   cancelCleanup,
		vms:             make(map[string]*vmInfo),
	}

	// Start background loop to clean up TERMINATED VMs.
	// VMs self-terminate via shutdown in the startup script after the job
	// completes. The scaler normally deletes them via HandleJobCompleted,
	// but after a restart or if the deletion fails, they linger as
	// TERMINATED. This loop catches those orphans.
	if cfg.VMPrefix != "" {
		go mgr.cleanupTerminatedVMs(cleanupCtx)
	}

	return mgr, nil
}

// Close shuts down the manager.
func (m *Manager) Close() {
	m.cancelCleanup()
	m.instancesClient.Close()
	m.regionsClient.Close()
}

// ActiveCount returns the number of VMs currently tracked.
func (m *Manager) ActiveCount() int {
	m.mu.Lock()
	defer m.mu.Unlock()
	return len(m.vms)
}

// ActiveRunnerNames returns the names of all tracked runners.
func (m *Manager) ActiveRunnerNames() []string {
	m.mu.Lock()
	defer m.mu.Unlock()
	names := make([]string, 0, len(m.vms))
	for name := range m.vms {
		names = append(names, name)
	}
	return names
}

// MarkBusy marks a runner as busy (job started).
func (m *Manager) MarkBusy(runnerName string) {
	m.mu.Lock()
	defer m.mu.Unlock()
	if vm, ok := m.vms[runnerName]; ok {
		vm.busy = true
	}
}

func splitZones(zonesValue string) []string {
	parts := strings.Split(zonesValue, ",")
	zones := make([]string, 0, len(parts))
	for _, zone := range parts {
		zone = strings.TrimSpace(zone)
		if zone != "" {
			zones = append(zones, zone)
		}
	}
	return zones
}

func zoneRegion(zone string) string {
	parts := strings.Split(zone, "-")
	if len(parts) < 3 {
		return ""
	}
	return parts[0] + "-" + parts[1]
}

// selectZones picks candidate zones for creating a VM. For GPU VMs, it checks
// quota availability across regions. For non-GPU VMs (GPUType == "none"),
// it round-robins through configured zones.
func (m *Manager) selectZones(ctx context.Context) ([]zoneCandidate, error) {
	if m.selectZonesFunc != nil {
		return m.selectZonesFunc(ctx)
	}

	zones := splitZones(m.config.Zones)
	if len(zones) == 0 {
		return nil, fmt.Errorf("no zones configured")
	}

	// Non-GPU VMs: simple round-robin, no quota check needed
	if m.config.GPUType == "none" {
		m.mu.Lock()
		count := len(m.vms)
		m.mu.Unlock()
		zone := zones[count%len(zones)]
		return []zoneCandidate{{zone: zone, region: zoneRegion(zone)}}, nil
	}

	// GPU VMs: select zone by quota availability

	// Group zones by region (e.g., "us-east1-c" -> "us-east1")
	regionZones := make(map[string][]string)
	regionOrder := make(map[string]int)
	for _, z := range zones {
		region := zoneRegion(z)
		if region == "" {
			continue
		}
		if _, ok := regionOrder[region]; !ok {
			regionOrder[region] = len(regionOrder)
		}
		regionZones[region] = append(regionZones[region], z)
	}

	// Check quota for each region
	type regionQuota struct {
		region    string
		available float64
		order     int
	}

	var quotas []regionQuota

	quotaMetric := gpuQuotaMetric(m.config.GPUType)

	for region := range regionZones {
		req := &regionspb.GetRegionRequest{
			Project: m.config.Project,
			Region:  region,
		}
		regionInfo, err := m.regionsClient.Get(ctx, req)
		if err != nil {
			slog.Warn("failed to get region info", "region", region, "error", err)
			continue
		}

		for _, q := range regionInfo.GetQuotas() {
			if q.GetMetric() == quotaMetric {
				// GCP's reported usage already includes our running VMs,
				// so we only need limit - usage (no double-subtraction).
				available := q.GetLimit() - q.GetUsage()
				quotas = append(quotas, regionQuota{
					region:    region,
					available: available,
					order:     regionOrder[region],
				})
				slog.Debug("region quota",
					"region", region,
					"limit", q.GetLimit(),
					"usage", q.GetUsage(),
					"available", available,
				)
				break
			}
		}
	}

	if len(quotas) == 0 {
		return nil, fmt.Errorf("no regions with %s quota found", m.config.GPUType)
	}

	// Sort by available quota (most available first)
	sort.Slice(quotas, func(i, j int) bool {
		if quotas[i].available == quotas[j].available {
			return quotas[i].order < quotas[j].order
		}
		return quotas[i].available > quotas[j].available
	})

	best := quotas[0]
	if best.available <= 0 {
		return nil, fmt.Errorf("no GPU quota available in any configured region (best: %s with %.0f available)", best.region, best.available)
	}

	candidates := make([]zoneCandidate, 0, len(zones))
	for _, quota := range quotas {
		if quota.available <= 0 {
			continue
		}
		for _, zone := range regionZones[quota.region] {
			candidates = append(candidates, zoneCandidate{
				zone:      zone,
				region:    quota.region,
				available: quota.available,
			})
		}
	}
	if len(candidates) == 0 {
		return nil, fmt.Errorf("no GPU quota available in any configured region")
	}
	return candidates, nil
}

// selectZone is kept for focused tests and callers that only need the first
// candidate. CreateVM uses the full candidate list for stockout fallback.
func (m *Manager) selectZone(ctx context.Context) (string, error) {
	candidates, err := m.selectZones(ctx)
	if err != nil {
		return "", err
	}
	if len(candidates) == 0 {
		return "", fmt.Errorf("no zone candidates available")
	}
	return candidates[0].zone, nil
}

// gpuQuotaMetric returns the GCP quota metric name for a GPU type.
func gpuQuotaMetric(gpuType string) string {
	// GCP quota metric names are uppercase with underscores
	// e.g., "nvidia-tesla-t4" -> "NVIDIA_T4_GPUS"
	switch gpuType {
	case "nvidia-tesla-t4":
		return "NVIDIA_T4_GPUS"
	case "nvidia-tesla-v100":
		return "NVIDIA_V100_GPUS"
	case "nvidia-tesla-p4":
		return "NVIDIA_P4_GPUS"
	case "nvidia-tesla-p100":
		return "NVIDIA_P100_GPUS"
	case "nvidia-l4":
		return "NVIDIA_L4_GPUS"
	case "nvidia-tesla-a100":
		return "NVIDIA_A100_GPUS"
	default:
		// Best effort: uppercase and replace dashes
		return strings.ToUpper(strings.ReplaceAll(strings.TrimPrefix(gpuType, "nvidia-tesla-"), "-", "_")) + "_GPUS"
	}
}

// CreateVM creates a new GPU VM from the instance template, trying candidate
// zones in quota order and falling through on zonal resource stockouts.
func (m *Manager) CreateVM(ctx context.Context, runnerName, jitConfig string) (string, error) {
	candidates, err := m.selectZones(ctx)
	if err != nil {
		return "", fmt.Errorf("selecting zones: %w", err)
	}

	vmName := runnerName

	templateURL := fmt.Sprintf(
		"projects/%s/global/instanceTemplates/%s",
		m.config.Project, m.config.InstanceTemplate,
	)

	// Select the startup script and metadata key based on platform
	var scriptKey, scriptContent string
	if m.config.Platform == "linux" {
		scriptKey = "startup-script"
		scriptContent = linuxStartupScript
	} else {
		scriptKey = "windows-startup-script-ps1"
		scriptContent = windowsStartupScript
	}

	var stockoutErrors []string
	for _, candidate := range candidates {
		zone := candidate.zone
		slog.Info("selected zone", "zone", zone, "region", candidate.region, "available_gpus", candidate.available)

		req := &computepb.InsertInstanceRequest{
			Project: m.config.Project,
			Zone:    zone,
			InstanceResource: &computepb.Instance{
				Name: proto.String(vmName),
				Metadata: &computepb.Metadata{
					Items: []*computepb.Items{
						{
							Key:   proto.String("jit-config"),
							Value: proto.String(jitConfig),
						},
						{
							Key:   proto.String(scriptKey),
							Value: proto.String(scriptContent),
						},
					},
				},
			},
			SourceInstanceTemplate: proto.String(templateURL),
		}

		if err := m.insertVM(ctx, req); err != nil {
			if isZoneResourceExhausted(err) {
				slog.Warn("zone resource exhausted, trying next candidate zone", "zone", zone, "error", err)
				stockoutErrors = append(stockoutErrors, fmt.Sprintf("%s: %v", zone, err))
				continue
			}
			return "", err
		}

		m.mu.Lock()
		m.vms[runnerName] = &vmInfo{vmName: vmName, zone: zone}
		m.mu.Unlock()

		slog.Info("VM created", "vm", vmName, "zone", zone)
		return vmName, nil
	}

	if len(stockoutErrors) > 0 {
		return "", fmt.Errorf("all candidate zones are out of stock for %s: %s", m.config.GPUType, strings.Join(stockoutErrors, "; "))
	}
	return "", fmt.Errorf("no candidate zones available for %s", m.config.GPUType)
}

func (m *Manager) insertVM(ctx context.Context, req *computepb.InsertInstanceRequest) error {
	if m.insertVMFunc != nil {
		return m.insertVMFunc(ctx, req)
	}

	op, err := m.instancesClient.Insert(ctx, req)
	if err != nil {
		return fmt.Errorf("inserting instance in %s: %w", req.GetZone(), err)
	}

	if err := op.Wait(ctx); err != nil {
		return fmt.Errorf("waiting for instance creation in %s: %w", req.GetZone(), err)
	}

	return nil
}

func isZoneResourceExhausted(err error) bool {
	if err == nil {
		return false
	}
	msg := strings.ToLower(err.Error())
	return strings.Contains(msg, "zone_resource_pool_exhausted") ||
		strings.Contains(msg, "resource_availability") ||
		strings.Contains(msg, "does not have enough resources")
}

// DeleteByRunnerName deletes the VM associated with a runner name.
func (m *Manager) DeleteByRunnerName(ctx context.Context, runnerName string) error {
	m.mu.Lock()
	vm, ok := m.vms[runnerName]
	if !ok {
		m.mu.Unlock()
		return fmt.Errorf("no VM found for runner %q", runnerName)
	}
	vmName := vm.vmName
	zone := vm.zone
	delete(m.vms, runnerName)
	m.mu.Unlock()

	return m.deleteVM(ctx, vmName, zone)
}

// DeleteAll deletes all tracked VMs. Used during shutdown.
func (m *Manager) DeleteAll(ctx context.Context) {
	m.mu.Lock()
	vms := make(map[string]*vmInfo)
	for rn, vm := range m.vms {
		vms[rn] = vm
	}
	m.mu.Unlock()

	for rn, vm := range vms {
		if err := m.deleteVM(ctx, vm.vmName, vm.zone); err != nil {
			slog.Error("failed to delete VM during cleanup", "vm", vm.vmName, "error", err)
		}
		m.mu.Lock()
		delete(m.vms, rn)
		m.mu.Unlock()
	}
}

func (m *Manager) deleteVM(ctx context.Context, vmName, zone string) error {
	req := &computepb.DeleteInstanceRequest{
		Project:  m.config.Project,
		Zone:     zone,
		Instance: vmName,
	}

	op, err := m.instancesClient.Delete(ctx, req)
	if err != nil {
		return fmt.Errorf("deleting instance %s in %s: %w", vmName, zone, err)
	}

	if err := op.Wait(ctx); err != nil {
		return fmt.Errorf("waiting for instance deletion %s in %s: %w", vmName, zone, err)
	}

	slog.Info("VM deleted", "vm", vmName, "zone", zone)
	return nil
}

// cleanupTerminatedVMs periodically scans all configured zones for VMs
// matching our name prefix that are in TERMINATED state, and deletes them.
// This catches VMs that self-terminated (via shutdown in the startup script)
// but weren't cleaned up by the scaler (e.g., after a restart).
func (m *Manager) cleanupTerminatedVMs(ctx context.Context) {
	ticker := time.NewTicker(m.config.CleanupInterval)
	defer ticker.Stop()

	m.runCleanupLoop(ctx, ticker.C)
}

func (m *Manager) runCleanupLoop(ctx context.Context, ticks <-chan time.Time) {
	// Run one pass immediately on startup so orphaned VMs are reclaimed
	// without waiting for the first ticker interval.
	m.runCleanupPass(ctx)

	for {
		select {
		case <-ctx.Done():
			return
		case <-ticks:
			m.runCleanupPass(ctx)
		}
	}
}

func (m *Manager) runCleanupPass(ctx context.Context) {
	if m.cleanupPass != nil {
		m.cleanupPass(ctx)
		return
	}
	m.doCleanupTerminatedVMs(ctx)
}

func cleanupFilter(vmPrefix string) string {
	return fmt.Sprintf("name=%s-* AND status=TERMINATED", vmPrefix)
}

func (m *Manager) removeTrackedVMByVMName(vmName string) {
	m.mu.Lock()
	defer m.mu.Unlock()
	for runnerName, vm := range m.vms {
		if runnerName == vmName || vm.vmName == vmName {
			delete(m.vms, runnerName)
			return
		}
	}
}

func (m *Manager) listVMNamesByFilter(ctx context.Context, zone, filter string) ([]string, error) {
	req := &computepb.ListInstancesRequest{
		Project: m.config.Project,
		Zone:    zone,
		Filter:  proto.String(filter),
	}

	it := m.instancesClient.List(ctx, req)
	var names []string
	for {
		instance, err := it.Next()
		if err == iterator.Done {
			break
		}
		if err != nil {
			return names, err
		}
		names = append(names, instance.GetName())
	}
	return names, nil
}

func (m *Manager) listTerminatedVMNames(ctx context.Context, zone string) ([]string, error) {
	if m.listTerminated != nil {
		return m.listTerminated(ctx, zone)
	}
	return m.listVMNamesByFilter(ctx, zone, cleanupFilter(m.config.VMPrefix))
}

func runningFilter(vmPrefix string) string {
	return fmt.Sprintf("name=%s-* AND status=RUNNING", vmPrefix)
}

func (m *Manager) listRunningVMNames(ctx context.Context, zone string) ([]string, error) {
	if m.instancesClient == nil {
		return nil, nil
	}
	return m.listVMNamesByFilter(ctx, zone, runningFilter(m.config.VMPrefix))
}

func (m *Manager) deleteVMForCleanup(ctx context.Context, vmName, zone string) error {
	if m.deleteVMFunc != nil {
		return m.deleteVMFunc(ctx, vmName, zone)
	}
	return m.deleteVM(ctx, vmName, zone)
}

func (m *Manager) doCleanupTerminatedVMs(ctx context.Context) {
	zones := strings.Split(m.config.Zones, ",")
	deletedCount := 0

	for _, zone := range zones {
		zone = strings.TrimSpace(zone)
		if zone == "" {
			continue
		}

		listCtx, cancelList := context.WithTimeout(ctx, cleanupZoneScanTimeout)
		names, err := m.listTerminatedVMNames(listCtx, zone)
		cancelList()
		if err != nil {
			slog.Warn("failed to list instances for cleanup", "zone", zone, "error", err)
			if len(names) == 0 {
				continue
			}
		}

		for _, name := range names {
			slog.Info("cleaning up terminated VM", "vm", name, "zone", zone)
			deleteCtx, cancelDelete := context.WithTimeout(ctx, cleanupDeleteTimeout)
			err = m.deleteVMForCleanup(deleteCtx, name, zone)
			cancelDelete()
			if err != nil {
				slog.Warn("failed to delete terminated VM", "vm", name, "zone", zone, "error", err)
			} else {
				deletedCount++
			}

			// Also remove from tracked VMs if still there
			m.removeTrackedVMByVMName(name)
		}
	}

	slog.Info("terminated VM cleanup pass completed", "terminated_vms_deleted", deletedCount)

	// Reconcile: remove tracked VMs that no longer exist as RUNNING instances.
	// This prevents ActiveCount() from drifting above reality, which would
	// cause the scaler to stop creating new VMs.
	m.reconcileTrackedVMs(ctx)
}

// reconcileTrackedVMs checks all tracked VMs against actual GCP instance state
// and removes entries for VMs that are no longer RUNNING. This prevents the
// in-memory tracker from drifting when VMs terminate outside the scaler's
// control (e.g., via shutdown in the startup script).
func (m *Manager) reconcileTrackedVMs(ctx context.Context) {
	if m.instancesClient == nil {
		return // No GCP client (test mode), skip reconciliation
	}

	m.mu.Lock()
	if len(m.vms) == 0 {
		m.mu.Unlock()
		return
	}

	// Snapshot tracked VMs grouped by zone
	zoneVMs := make(map[string][]string) // zone -> []runnerName
	for runnerName, vm := range m.vms {
		zoneVMs[vm.zone] = append(zoneVMs[vm.zone], runnerName)
	}
	m.mu.Unlock()

	// Collect all RUNNING VM names across zones
	runningVMs := make(map[string]bool)
	failedZones := make(map[string]bool)
	for zone := range zoneVMs {
		listCtx, cancel := context.WithTimeout(ctx, cleanupZoneScanTimeout)
		names, err := m.listRunningVMNames(listCtx, zone)
		cancel()
		if err != nil {
			slog.Warn("reconcile: failed to list running VMs", "zone", zone, "error", err)
			failedZones[zone] = true
			continue
		}
		for _, name := range names {
			runningVMs[name] = true
		}
	}

	// Remove tracked entries whose VMs are no longer RUNNING.
	// Skip VMs in zones where the list call failed.
	m.mu.Lock()
	evicted := 0
	for runnerName, vm := range m.vms {
		if failedZones[vm.zone] {
			continue
		}
		if !runningVMs[vm.vmName] {
			slog.Info("reconcile: removing stale tracked VM", "runner", runnerName, "vm", vm.vmName, "zone", vm.zone)
			delete(m.vms, runnerName)
			evicted++
		}
	}
	m.mu.Unlock()

	if evicted > 0 {
		slog.Info("reconcile: evicted stale VM entries", "count", evicted, "tracked_after", m.ActiveCount())
	}
}
