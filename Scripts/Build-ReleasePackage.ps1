$ErrorActionPreference = "Stop"

$repositoryRoot = Split-Path -Parent $PSScriptRoot
$executablePath = Join-Path $repositoryRoot "Release\DirectX9_Maze.exe"
$assetsPath = Join-Path $repositoryRoot "Assets"
$readmePath = Join-Path $repositoryRoot "README.md"
$projectGuidePath = Join-Path $repositoryRoot "PROJECT_GUIDE.md"
$sourceDocsPath = Join-Path $repositoryRoot "docs"
$sourceImagesPath = Join-Path $sourceDocsPath "images"
$artifactRoot = Join-Path $repositoryRoot "artifacts"
$packagePath = Join-Path $artifactRoot "DirectX9_Maze"
$packageDocsPath = Join-Path $packagePath "docs"
$archivePath = Join-Path $artifactRoot "DirectX9_Maze-Release-x86.zip"

$documentationFileNames = @(
    "ASSET_LICENSES.md"
    "CODING_CONVENTIONS.md"
    "IMPROVEMENTS.md"
    "LEARNING_NOTES.md"
    "PERFORMANCE.md"
    "ROADMAP.md"
    "SMOKE_TEST.md"
)

$requiredPaths = @(
    $executablePath
    $assetsPath
    $readmePath
    $projectGuidePath
    $sourceImagesPath
)

foreach ($requiredPath in $requiredPaths)
{
    if (-not (Test-Path -LiteralPath $requiredPath))
    {
        throw "Required package source not found: $requiredPath"
    }
}

foreach ($documentationFileName in $documentationFileNames)
{
    $documentationPath = Join-Path $sourceDocsPath $documentationFileName

    if (-not (Test-Path -LiteralPath $documentationPath))
    {
        throw "Required package document not found: $documentationPath"
    }
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
New-Item -ItemType Directory -Path $packageDocsPath -Force | Out-Null

Copy-Item -LiteralPath $executablePath -Destination $packagePath
Copy-Item -LiteralPath $assetsPath -Destination $packagePath -Recurse
Copy-Item -LiteralPath $readmePath -Destination $packagePath
Copy-Item -LiteralPath $projectGuidePath -Destination $packagePath

foreach ($documentationFileName in $documentationFileNames)
{
    Copy-Item `
        -LiteralPath (Join-Path $sourceDocsPath $documentationFileName) `
        -Destination $packageDocsPath
}

Copy-Item -LiteralPath $sourceImagesPath -Destination $packageDocsPath -Recurse

Compress-Archive `
    -Path $packagePath `
    -DestinationPath $archivePath

Write-Output "Package directory: $packagePath"
Write-Output "Package archive: $archivePath"
