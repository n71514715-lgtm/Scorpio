# Scorpio Release Builder
# Builds all projects in Release x64 and copies DLLs to game folder

$solutionDir = "C:\Users\maja\source\repos\XOneLayer"
$releaseDir  = "$solutionDir\x64\Release"
$gameDir     = "C:\Users\maja\Downloads\terraria_xb1_dump\terraria_xb1_dump\Mount"

# Find MSBuild
$msbuild = "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
if (-not (Test-Path $msbuild)) {
    $msbuild = Get-ChildItem "C:\Program Files\Microsoft Visual Studio" -Filter "MSBuild.exe" -Recurse -ErrorAction SilentlyContinue |
               Select-Object -First 1 -ExpandProperty FullName
}

if (-not $msbuild) {
    Write-Host "MSBuild not found!" -ForegroundColor Red
    exit 1
}

Write-Host "Building Scorpio in Release x64..." -ForegroundColor Cyan
Write-Host "Using MSBuild: $msbuild" -ForegroundColor Gray
Write-Host ""

# Build the entire solution in Release x64
& $msbuild "$solutionDir\XOneLayer.sln" /p:Configuration=Release /p:Platform=x64 /t:Rebuild /v:minimal

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build FAILED!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Build succeeded! Copying DLLs to game folder..." -ForegroundColor Green
Write-Host ""

# DLLs to copy to game folder
$dlls = @(
    "d3d12_x.dll",
    "xg_x.dll",
    "GameInput.dll",
    "xmem.dll",
    "XFrontPanelDisplay.dll",
    "AcpHal.dll",
    "XGameRuntime.dll",
    "ScorpioHook.dll"
)

foreach ($dll in $dlls) {
    $src  = "$releaseDir\$dll"
    $dest = "$gameDir\$dll"
    if (Test-Path $src) {
        Copy-Item -Path $src -Destination $dest -Force
        Write-Host "  Copied: $dll" -ForegroundColor Green
    } else {
        Write-Host "  MISSING: $dll (not in Release folder)" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "Done! Run Scorpio:" -ForegroundColor Cyan
Write-Host "  $releaseDir\XOneLayer.exe" -ForegroundColor White
Write-Host "  `"$gameDir\Terraria.exe`"" -ForegroundColor White
