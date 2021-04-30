$job = Start-Job -ArgumentList $PSScriptRoot -ScriptBlock {
    Set-Location $args[0]
    $code = (Get-Content -Raw -Path "../scripts/Program.cs").ToString()
    $assemblies = ("System.Core", "System.IO", "System.Collections")
    Add-Type -ReferencedAssemblies $assemblies -TypeDefinition $code -Language CSharp	
    [toc.Builder]::Run($args[0])
}
Wait-Job $job
Receive-Job -Job $job
