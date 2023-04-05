param (
    [Parameter(Mandatory = $true)]
    [string]$outputFile
)

$result = "#define SLANG_TAG_VERSION ""unknown"""

if (Get-Command git -ErrorAction SilentlyContinue) {
    try {
        $gitResult = git describe --tags
        if ($gitResult) {
            $result = "#define SLANG_TAG_VERSION ""$($gitResult.Trim())"""
        }
    } catch {
        # Handle any errors from the git command
    }
}

if (-not (Test-Path $outputFile)) {
    Set-Content -Path $outputFile -Value $result
} else {
    $currentContent = (Get-Content -Path $outputFile -Raw).Trim()
    if ($currentContent -ne $result) {
        Set-Content -Path $outputFile -Value $result
    }
}