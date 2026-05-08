[CmdletBinding()]
param(
    [string]$StartPath = (Get-Location).Path,
    [string]$Target,
    [string]$UeRoot
)

$ErrorActionPreference = "Stop"

function Find-UpwardFile {
    param(
        [string]$Path,
        [string]$Filter
    )

    $current = (Resolve-Path $Path).Path
    while ($true) {
        $matches = Get-ChildItem -Path $current -Filter $Filter -File -ErrorAction SilentlyContinue
        if ($matches.Count -eq 1) {
            return $matches[0].FullName
        }
        if ($matches.Count -gt 1) {
            $names = $matches | ForEach-Object { $_.FullName }
            throw "Multiple '$Filter' files found in $current. Specify -StartPath closer to the desired project.`n$($names -join "`n")"
        }

        $parent = Split-Path -Parent $current
        if (-not $parent -or $parent -eq $current) {
            break
        }
        $current = $parent
    }

    return $null
}

function Find-UpwardDirectory {
    param(
        [string]$Path,
        [string]$DirName
    )

    $current = (Resolve-Path $Path).Path
    while ($true) {
        $candidate = Join-Path $current $DirName
        if (Test-Path $candidate) {
            return $candidate
        }

        $parent = Split-Path -Parent $current
        if (-not $parent -or $parent -eq $current) {
            break
        }
        $current = $parent
    }

    return $null
}

$projectFile = Find-UpwardFile -Path $StartPath -Filter "*.uproject"
if (-not $projectFile) {
    throw "No .uproject found above '$StartPath'."
}

$projectRoot = Split-Path -Parent $projectFile
$projectName = [IO.Path]::GetFileNameWithoutExtension($projectFile)

if (-not $Target) {
    $Target = "$projectName`Editor Win64 Development"
}

if (-not $UeRoot) {
    if ($env:UE_ROOT) {
        $UeRoot = $env:UE_ROOT
    } else {
        $UeRoot = Find-UpwardDirectory -Path $projectRoot -DirName "UE_5.3"
    }
}

if (-not $UeRoot) {
    throw "UE root not found. Set -UeRoot or UE_ROOT env var."
}

$buildBat = Join-Path $UeRoot "Engine\Build\BatchFiles\Build.bat"
if (-not (Test-Path $buildBat)) {
    throw "Build.bat not found at $buildBat"
}

Write-Host "Project: $projectFile"
Write-Host "Target:  $Target"
Write-Host "UE root: $UeRoot"

& $buildBat -Mode=GenerateClangDatabase -Project="$projectFile" -Target="$Target"

$clangDb = Join-Path $UeRoot "compile_commands.json"
if (-not (Test-Path $clangDb)) {
    throw "compile_commands.json not found at $clangDb"
}

$vscodeDir = Join-Path $projectRoot ".vscode"
New-Item -ItemType Directory -Path $vscodeDir -Force | Out-Null

$dest = Join-Path $vscodeDir "compile_commands.json"
Copy-Item -Path $clangDb -Destination $dest -Force

Write-Host "Copied to $dest"
