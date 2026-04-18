param(
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string]$Configuration = 'Release',

    [string]$BuildDir = 'build',

    [string]$Generator = '',

    [ValidateSet('x64', 'Win32', 'ARM64')]
    [string]$Architecture = 'x64'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Fail([string]$Message) {
    Write-Error $Message
    exit 1
}

function Test-IsWindows {
    $isWindowsVariable = Get-Variable -Name IsWindows -ErrorAction SilentlyContinue
    if ($null -ne $isWindowsVariable) {
        return [bool]$isWindowsVariable.Value
    }

    return [System.Environment]::OSVersion.Platform -eq [System.PlatformID]::Win32NT
}

function Resolve-VisualStudioGenerator([string]$RequestedGenerator) {
    if (-not [string]::IsNullOrWhiteSpace($RequestedGenerator)) {
        return $RequestedGenerator
    }

    $vswhereRoot = [System.Environment]::GetEnvironmentVariable('ProgramFiles(x86)')
    if (-not [string]::IsNullOrWhiteSpace($vswhereRoot)) {
        $vswherePath = Join-Path $vswhereRoot 'Microsoft Visual Studio\Installer\vswhere.exe'
        if (Test-Path $vswherePath) {
            $installationVersion = & $vswherePath -latest -products * -requires Microsoft.Component.MSBuild -property installationVersion
            if ($LASTEXITCODE -eq 0 -and -not [string]::IsNullOrWhiteSpace($installationVersion)) {
                $majorVersion = $installationVersion.Trim().Split('.')[0]
                switch ($majorVersion) {
                    '18' { return 'Visual Studio 18 2026' }
                    '17' { return 'Visual Studio 17 2022' }
                }
            }
        }
    }

    return 'Visual Studio 17 2022'
}

function Get-CachedGenerator([string]$BuildPath) {
    $cachePath = Join-Path $BuildPath 'CMakeCache.txt'
    if (-not (Test-Path $cachePath)) {
        return $null
    }

    $generatorLine = Get-Content $cachePath | Where-Object { $_ -like 'CMAKE_GENERATOR:INTERNAL=*' } | Select-Object -First 1
    if ([string]::IsNullOrWhiteSpace($generatorLine)) {
        return $null
    }

    return $generatorLine.Substring('CMAKE_GENERATOR:INTERNAL='.Length)
}

if (-not (Test-IsWindows)) {
    Fail 'This build script only supports Windows because supra_view depends on Win32, DXGI Desktop Duplication, and Direct3D 11.'
}

$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($null -eq $cmake) {
    Fail 'CMake was not found in PATH. Install Visual Studio C++ tools with CMake support or install CMake from cmake.org.'
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir
$buildPath = Join-Path $repoRoot $BuildDir
$resolvedGenerator = Resolve-VisualStudioGenerator $Generator
$cachedGenerator = Get-CachedGenerator $buildPath

if ($null -ne $cachedGenerator -and $cachedGenerator -ne $resolvedGenerator) {
    Fail "Build directory '$buildPath' is already configured for '$cachedGenerator'. Remove it or pass -BuildDir with a fresh directory for '$resolvedGenerator'."
}

Write-Host "[supra_view] Source directory : $repoRoot"
Write-Host "[supra_view] Build directory  : $buildPath"
Write-Host "[supra_view] Generator        : $resolvedGenerator"
Write-Host "[supra_view] Architecture    : $Architecture"
Write-Host "[supra_view] Configuration   : $Configuration"

& cmake -S $repoRoot -B $buildPath -G $resolvedGenerator -A $Architecture
if ($LASTEXITCODE -ne 0) {
    Fail 'CMake configure step failed.'
}

& cmake --build $buildPath --config $Configuration
if ($LASTEXITCODE -ne 0) {
    Fail 'CMake build step failed.'
}

Write-Host '[supra_view] Build completed successfully.'
