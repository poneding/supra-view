param(
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string]$Configuration = 'Release',

    [string]$BuildDir = 'build'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Fail([string]$Message) {
    Write-Error $Message
    exit 1
}

if (-not $IsWindows) {
    Fail 'This run script only supports Windows because supra_view is a native Win32 and Direct3D 11 application.'
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir
$binaryPath = Join-Path $repoRoot "$BuildDir\$Configuration\supra_view.exe"

if (-not (Test-Path $binaryPath)) {
    Fail "Build output not found at '$binaryPath'. Run .\scripts\build.ps1 -Configuration $Configuration first."
}

Write-Host "[supra_view] Launching $binaryPath"
& $binaryPath
