Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

param(
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string]$Configuration = 'Release',

    [string]$BuildDir = 'build',

    [string]$Generator = 'Visual Studio 17 2022',

    [ValidateSet('x64', 'Win32', 'ARM64')]
    [string]$Architecture = 'x64'
)

function Fail([string]$Message) {
    Write-Error $Message
    exit 1
}

if (-not $IsWindows) {
    Fail 'This build script only supports Windows because supra_view depends on Win32, DXGI Desktop Duplication, and Direct3D 11.'
}

$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($null -eq $cmake) {
    Fail 'CMake was not found in PATH. Install Visual Studio C++ tools with CMake support or install CMake from cmake.org.'
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir
$buildPath = Join-Path $repoRoot $BuildDir

Write-Host "[supra_view] Source directory : $repoRoot"
Write-Host "[supra_view] Build directory  : $buildPath"
Write-Host "[supra_view] Generator        : $Generator"
Write-Host "[supra_view] Architecture    : $Architecture"
Write-Host "[supra_view] Configuration   : $Configuration"

& cmake -S $repoRoot -B $buildPath -G $Generator -A $Architecture
if ($LASTEXITCODE -ne 0) {
    Fail 'CMake configure step failed.'
}

& cmake --build $buildPath --config $Configuration
if ($LASTEXITCODE -ne 0) {
    Fail 'CMake build step failed.'
}

Write-Host "[supra_view] Build completed successfully."
