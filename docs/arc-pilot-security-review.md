# Security Review: GitHub Copilot on Self-Hosted ARC Runners

**Date**: 2025-12-15
**Project**: Slang Compiler - GitHub Copilot Pilot on Self-Hosted Runners
**Scope**: Deploy Actions Runner Controller (ARC) on slang-ubuntu-runner-3 for Copilot coding agent

## Executive Summary

This document outlines the security implications of enabling GitHub Copilot coding agent on self-hosted GPU runners. The deployment requires disabling Copilot's integrated firewall and implementing network-level security controls.

**Risk Level**: Medium
**Approval Required**: Yes (Security Team + Infrastructure Team)

## Overview

### What is Changing

- **Current State**: 3 traditional self-hosted GitHub Actions runners on GCP
- **Pilot Change**: Convert 1 runner (slang-ubuntu-runner-3) to ARC-managed for Copilot support
- **Production Goal**: Enable Copilot coding agent to run on self-hosted infrastructure with GPU access

### Why This Change is Needed

- Enable Copilot access to internal resources (NVIDIA-specific packages, internal build systems)
- Provide GPU access for testing Slang compiler features
- Evaluate Copilot coding agent for potential productivity improvements

## Security Implications

### 1. Disabled Copilot Firewall

**Requirement**: GitHub Copilot's integrated firewall must be disabled for self-hosted runners.

**Impact**:

- ⚠️ **Reduced Isolation**: Network isolation between Copilot and self-hosted environment is reduced
- ⚠️ **Increased Attack Surface**: Copilot can potentially access more network resources
- ⚠️ **Trust Requirement**: We assume GitHub Copilot service is trustworthy

**Mitigation**:

- ✅ Implement GCP firewall rules to restrict egress traffic (see Section 4)
- ✅ Monitor network traffic from runner VMs
- ✅ Limit Copilot access to specific repositories/teams (configure in GitHub settings)
- ✅ Regular security audits of runner activity logs

### 2. Required Network Access

**Copilot requires outbound HTTPS access to**:

- `api.githubcopilot.com` - Copilot API endpoint
- `uploads.github.com` - Artifact and file uploads
- `user-images.githubusercontent.com` - User-generated content
- `*.actions.githubusercontent.com` - GitHub Actions infrastructure
- `github.com` - GitHub platform

**Risk Assessment**:

- ✅ **Low Risk**: All endpoints are official GitHub services over HTTPS
- ✅ **Encrypted**: All traffic uses TLS 1.2+
- ⚠️ **Data Exfiltration**: Code and artifacts will be transmitted to GitHub

### 3. Kubernetes on Runner VM

**Change**: Installing k3s (lightweight Kubernetes) on slang-ubuntu-runner-3

**Impact**:

- ⚠️ **Additional Services**: k3s runs additional processes (kubelet, containerd, etc.)
- ⚠️ **Increased Complexity**: More components to patch and monitor
- ⚠️ **Container Breakout Risk**: Standard Kubernetes container security concerns

**Mitigation**:

- ✅ Use official k3s from Rancher (well-maintained, security-focused)
- ✅ Enable k3s with minimal components (no Traefik, no ServiceLB)
- ✅ Keep k3s updated with security patches
- ✅ Use NVIDIA device plugin from official NVIDIA GitHub

### 4. ARC Runner Pods

**Change**: GitHub Actions jobs will run in Kubernetes pods instead of bare metal

**Impact**:

- ✅ **Better Isolation**: Jobs run in containers with defined resource limits
- ✅ **Ephemeral**: Pods are destroyed after each job (no state persistence)
- ⚠️ **Privileged GPU Access**: Pods need device access for GPU

**Mitigation**:

- ✅ Use Docker-in-Docker (DinD) for additional isolation
- ✅ Mount only required host paths (Vulkan ICD, EGL vendor config)
- ✅ Define resource limits (CPU, memory, GPU) in pod specs
- ✅ Use read-only mounts where possible

## Proposed Network Security Controls

### GCP Firewall Rules

**Egress Rules for slang-ubuntu-runner-3** (Priority: 1000):

```
# Allow GitHub Copilot endpoints
gcloud compute firewall-rules create allow-copilot-egress \
  --direction=EGRESS \
  --priority=1000 \
  --network=default \
  --action=ALLOW \
  --rules=tcp:443 \
  --destination-ranges=0.0.0.0/0 \
  --target-tags=slang-runner \
  --description="Allow HTTPS to GitHub Copilot and Actions endpoints"

# Apply tag to runner VM
gcloud compute instances add-tags slang-ubuntu-runner-3 \
  --zone=us-west1-a \
  --tags=slang-runner
```

**Recommendation**: Consider restricting destination-ranges to GitHub IP ranges if available.

### Endpoint Monitoring

**Implementation**:

- Enable VPC Flow Logs for runner subnet
- Set up alerts for connections to non-GitHub domains
- Regular review of outbound connections

**Monitoring Command** (run on runner VM):

```bash
# Monitor outbound HTTPS connections
sudo tcpdump -i any -n 'tcp port 443' -w /tmp/runner-traffic.pcap

# Analyze with tshark
tshark -r /tmp/runner-traffic.pcap -T fields -e ip.dst_host | sort -u
```

## Data Sensitivity

### What Data Will Copilot Access?

1. **Source Code**: Full access to shader-slang/slang repository
   - ✅ **Public Repository**: Already publicly accessible
   - ✅ **Low Risk**: No proprietary NVIDIA code in public Slang repo

2. **Build Artifacts**: Compiled binaries, test results
   - ✅ **Low Risk**: Generated from public source code
   - ✅ **Already Available**: CI artifacts are public

3. **Environment Variables**: May include paths, system info
   - ⚠️ **Medium Risk**: Ensure no secrets in environment
   - ✅ **Mitigation**: Review workflow files for secret exposure

4. **Network Resources**: Internal packages, build systems
   - ⚠️ **Medium Risk**: Copilot could access NVIDIA internal resources
   - ✅ **Mitigation**: Firewall rules, no credentials on runner VM

### Sensitive Data Protection

**Actions Required**:

- [ ] Audit all workflow files for hardcoded secrets
- [ ] Ensure GitHub Secrets are used correctly (not echoed to logs)
- [ ] Review runner environment variables
- [ ] Confirm no NVIDIA internal credentials stored on runner VM

## Compliance Considerations

### NVIDIA Security Policies

**Questions to Answer**:

1. Does this deployment comply with NVIDIA's cloud security policy?
2. Are there restrictions on AI services accessing development infrastructure?
3. Is GitHub Copilot an approved service?
4. Do we need additional approvals for GPU resource usage?

**Action**: Confirm with NVIDIA Security Team before proceeding.

### GitHub Terms of Service

**Copilot Terms**: Review [GitHub Copilot Terms](https://github.com/features/copilot#terms)

- Data usage for Copilot improvement
- Privacy implications
- Liability and warranty disclaimers

## Pilot Constraints

### Scope Limitation

**Pilot Phase**:

- ✅ Only 1 runner (slang-ubuntu-runner-3)
- ✅ Only test/evaluation workloads
- ✅ No production CI dependencies
- ✅ Can be rolled back easily

**Production Requires**:

- ✅ Successful pilot evaluation
- ✅ Security audit of logs and network traffic
- ✅ No incidents during pilot period (minimum 2 weeks)
- ✅ Additional security team approval

### Rollback Plan

If security concerns arise:

1. Delete ARC scale set: `kubectl delete namespace slang-runners`
2. Stop k3s: `sudo systemctl stop k3s`
3. Reinstall traditional runner: `./config.sh` from backup
4. Re-enable Copilot firewall in GitHub settings

**Rollback Time**: < 30 minutes

## Approval Checklist

### Required Approvals

- [ ] **Security Team**: Approve firewall changes and network access
- [ ] **Infrastructure Team**: Approve k3s installation on runner VM
- [ ] **Engineering Management**: Approve Copilot usage for team
- [ ] **Legal/Compliance** (if required): Approve GitHub Copilot terms

### Pre-Deployment Checklist

- [ ] GCP firewall rules configured
- [ ] Network connectivity tested
- [ ] GitHub PAT/App created with minimal required scopes
- [ ] Rollback plan documented and tested
- [ ] Monitoring and alerting configured
- [ ] Incident response plan updated

### Post-Deployment Checklist

- [ ] Verify Copilot runs successfully on ARC runner
- [ ] Verify GPU access works correctly
- [ ] Review network traffic logs (first 48 hours)
- [ ] Monitor for anomalies (first 2 weeks)
- [ ] Document lessons learned
- [ ] Schedule security review after 2-week pilot

## Risk Assessment Summary

| Risk                       | Likelihood | Impact | Mitigation                        | Residual Risk |
| -------------------------- | ---------- | ------ | --------------------------------- | ------------- |
| Network data exfiltration  | Low        | Medium | Firewall rules, monitoring        | Low           |
| Container breakout         | Low        | High   | k3s security, ephemeral pods      | Low           |
| Copilot service compromise | Very Low   | High   | Trust GitHub, monitor logs        | Low           |
| Credential exposure        | Low        | High   | No secrets on VM, audit workflows | Very Low      |
| GPU driver exploit         | Low        | Medium | Keep drivers updated              | Low           |

**Overall Risk**: **Low to Medium** with mitigations in place

## Recommendation

✅ **APPROVE** pilot deployment with the following conditions:

1. Implement all network security controls (GCP firewall rules)
2. Enable monitoring and alerting for network traffic
3. Limit pilot to 2-4 weeks with regular security reviews
4. Require additional approval before production rollout
5. Document and review all findings before scaling beyond pilot

## Approvers

| Role                     | Name | Date | Signature |
| ------------------------ | ---- | ---- | --------- |
| Security Team Lead       |      |      |           |
| Infrastructure Team Lead |      |      |           |
| Engineering Manager      |      |      |           |
| Compliance (if required) |      |      |           |

## References

- [GitHub Copilot Self-Hosted Runners](https://github.blog/changelog/2025-10-28-copilot-coding-agent-now-supports-self-hosted-runners/)
- [Customizing Copilot Agent Environment](https://docs.github.com/en/enterprise-cloud@latest/copilot/how-tos/use-copilot-agents/coding-agent/customize-the-agent-environment)
- [GitHub Actions Self-Hosted Runner Security](https://docs.github.com/en/actions/hosting-your-own-runners/managing-self-hosted-runners/about-self-hosted-runners#self-hosted-runner-security)
- [k3s Security Hardening](https://docs.k3s.io/security/hardening-guide)
- Implementation Plan: `/Users/jkiviluoto/.claude/plans/tingly-wobbling-hopper.md`
