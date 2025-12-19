# GitHub Authentication Setup for ARC

ARC (Actions Runner Controller) requires authentication to GitHub to manage self-hosted runners. You have two options:

## Option 1: Personal Access Token (PAT) - Recommended for Pilot

**Best for**: Testing, proof-of-concept, single-user scenarios
**Setup Time**: 2 minutes
**Maintenance**: Tokens expire (need renewal)

### Creating a PAT

1. Go to GitHub Settings → Developer settings → Personal access tokens → Tokens (classic)
   - Direct link: https://github.com/settings/tokens

2. Click "Generate new token (classic)"

3. Configure the token:
   - **Note**: `ARC Controller for slang-ubuntu-runner-3`
   - **Expiration**: 90 days (for pilot)
   - **Select scopes**:
     - `repo` (Full control of private repositories)
     - `workflow` (Update GitHub Action workflows)
     - `admin:org` → `manage_runners:org` (if using organization-level runners)

   **For repository-level runners** (simpler, recommended for pilot):
   - Just `repo` scope is sufficient
   - ARC will register runners to `shader-slang/slang` repository only

4. Click "Generate token"

5. **IMPORTANT**: Copy the token immediately (you won't be able to see it again)
   ```bash
   # Save it securely
   export GITHUB_PAT="ghp_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
   ```

### Using the PAT with ARC

When you install ARC (Phase 2.4), you'll use:

```bash
helm install arc \
  --namespace actions-runner-system \
  --create-namespace \
  actions-runner-controller/actions-runner-controller \
  --set authSecret.github_token="$GITHUB_PAT"
```

### Security Considerations for PAT

✅ **Pros**:

- Simple to set up
- Easy to revoke if compromised
- Works immediately

⚠️ **Cons**:

- Tied to your user account (if you leave, token breaks)
- Requires periodic renewal (expiration)
- Broad permissions (has access to everything you do)

**Best Practice**: Create a dedicated "bot" GitHub user for ARC in production.

---

## Option 2: GitHub App - Recommended for Production

**Best for**: Production deployments, team environments, long-term use
**Setup Time**: 10-15 minutes
**Maintenance**: No expiration, more secure

### Creating a GitHub App

1. Go to Organization Settings → GitHub Apps → New GitHub App
   - For shader-slang: https://github.com/organizations/shader-slang/settings/apps/new
   - Or User Settings if using personal account

2. Configure the app:

   **Basic Information**:
   - **GitHub App name**: `Slang ARC Controller`
   - **Homepage URL**: `https://github.com/shader-slang/slang`
   - **Webhook**: Uncheck "Active" (ARC doesn't use webhooks)

   **Permissions** (Repository permissions):
   - `Actions`: Read and write
   - `Administration`: Read and write (for runner management)
   - `Metadata`: Read-only (automatically selected)

   **Organization permissions** (if using org-level runners):
   - `Self-hosted runners`: Read and write

   **Where can this GitHub App be installed?**
   - Select "Only on this account"

3. Click "Create GitHub App"

4. After creation:
   - Note the **App ID** (you'll need this)
   - Scroll down to "Private keys" section
   - Click "Generate a private key"
   - Download the `.pem` file (save it securely!)

5. Install the app:
   - Go to app settings → Install App
   - Click "Install" next to shader-slang organization
   - Select "Only select repositories" → Choose `shader-slang/slang`
   - Click "Install"
   - Note the **Installation ID** from the URL (e.g., `https://github.com/organizations/shader-slang/settings/installations/12345678`)

### Using the GitHub App with ARC

Create a Kubernetes secret with the app credentials:

```bash
# Set variables
APP_ID="123456"  # Your App ID
INSTALLATION_ID="12345678"  # Your Installation ID
PRIVATE_KEY_FILE="slang-arc-controller.2025-12-15.private-key.pem"

# Create secret
kubectl create secret generic github-app-secret \
  --namespace=actions-runner-system \
  --from-literal=github_app_id="$APP_ID" \
  --from-literal=github_app_installation_id="$INSTALLATION_ID" \
  --from-file=github_app_private_key="$PRIVATE_KEY_FILE"

# Install ARC with GitHub App
helm install arc \
  --namespace actions-runner-system \
  --create-namespace \
  actions-runner-controller/actions-runner-controller \
  --set authSecret.github_app_id="$APP_ID" \
  --set authSecret.github_app_installation_id="$INSTALLATION_ID" \
  --set authSecret.github_app_private_key_file="$PRIVATE_KEY_FILE"
```

### Security Considerations for GitHub App

✅ **Pros**:

- More secure (limited permissions)
- Not tied to a user account
- No expiration
- Better audit trail

⚠️ **Cons**:

- More complex setup
- Requires managing private key securely
- Organization admin rights needed to create

---

## Recommendation for Pilot

**Use Option 1 (PAT)** for the pilot:

- Faster to set up
- Easier to troubleshoot
- Can switch to GitHub App later if pilot is successful

**For Production** (after pilot):

- Migrate to Option 2 (GitHub App)
- Create dedicated "bot" user if GitHub App not feasible
- Implement key rotation procedures

---

## Troubleshooting

### PAT Issues

**Problem**: "Bad credentials" error

- Solution: Verify token hasn't expired, check scopes include `repo`

**Problem**: "Resource not accessible by integration"

- Solution: Ensure token has `admin:org` → `manage_runners:org` for org-level runners

### GitHub App Issues

**Problem**: "App not installed"

- Solution: Install the app on the repository/organization

**Problem**: "Private key invalid"

- Solution: Ensure you're using the downloaded `.pem` file, not modified

---

## Next Steps

After setting up authentication:

1. ✅ Save credentials securely
2. ✅ Test authentication (ARC will validate when installed)
3. ✅ Proceed to Phase 2.4: Install ARC controller
4. ✅ Mark Phase 1.4 as complete in todo list

**Security Reminder**: Never commit tokens or private keys to git repositories!
