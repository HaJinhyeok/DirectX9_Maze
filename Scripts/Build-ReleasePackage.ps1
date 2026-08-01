$ErrorActionPreference = "Stop"

$repositoryRoot = Split-Path -Parent $PSScriptRoot
$executablePath = Join-Path $repositoryRoot "Release\DirectX9_Maze.exe"
$assetsPath = Join-Path $repositoryRoot "Assets"
$artifactRoot = Join-Path $repositoryRoot "artifacts"
$packagePath = Join-Path $artifactRoot "DirectX9_Maze"
$archivePath = Join-Path $artifactRoot "DirectX9_Maze-Release-x86.zip"

if (-not (Test-Path -LiteralPath $executablePath))
{
    throw "Release executable not found. Build Release|x86 first."
}

if (-not (Test-Path -LiteralPath $assetsPath))
{
    throw "Assets directory not found."
}

if (Test-Path -LiteralPath $packagePath)
{
    Remove-Item -LiteralPath $packagePath -Recurse -Force
}

if (Test-Path -LiteralPath $archivePath)
{
    Remove-Item -LiteralPath $archivePath -Force
}

New-Item -ItemType Directory -Path $packagePath -Force | Out-Null

Copy-Item -LiteralPath $executablePath -Destination $packagePath
Copy-Item -LiteralPath $assetsPath -Destination $packagePath -Recurse
Copy-Item -LiteralPath (Join-Path $repositoryRoot "README.md") -Destination $packagePath
Copy-Item `
    -LiteralPath (Join-Path $repositoryRoot "docs\ASSET_LICENSES.md") `
    -Destination $packagePath

Compress-Archive `
    -Path (Join-Path $packagePath "*") `
    -DestinationPath $archivePath

Write-Output "Package directory: $packagePath"
Write-Output "Package archive: $archivePath"