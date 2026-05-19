# =============================================================
# Scorpio-Setup.ps1
# Run this from x64 Native Tools Command Prompt for VS 2022
# Right-click > Run as Administrator if you hit permission errors
# =============================================================

$DebugDir   = 'C:\Users\maja\source\repos\XOneLayer\x64\Debug'
$TerrariaDir = 'C:\Users\maja\Downloads\terraria_xb1_dump\terraria_xb1_dump\Mount'
$GameExe    = "$TerrariaDir\Terraria.exe"

$StubDlls = @(
    'AcpHal.dll',
    'd3d12_x.dll',
    'xg_x.dll',
    'GameInput.dll',
    'xmem.dll',
    'XFrontPanelDisplay.dll'
)

Write-Host ''
Write-Host '============================================================' -ForegroundColor Cyan
Write-Host '  Scorpio Setup - Copying stub DLLs to Terraria folder' -ForegroundColor Cyan
Write-Host '============================================================' -ForegroundColor Cyan
Write-Host ''

# ── Step 1: Copy all stub DLLs ───────────────────────────────
Write-Host '── Step 1: Copying stub DLLs ───────────────────────────────' -ForegroundColor DarkGray
$allCopied = $true
foreach ($dll in $StubDlls) {
    $src = Join-Path $DebugDir $dll
    $dst = Join-Path $TerrariaDir $dll
    if (Test-Path $src) {
        Copy-Item $src $dst -Force
        Write-Host "  [OK]  Copied $dll" -ForegroundColor Green
    } else {
        Write-Host "  [!!]  NOT FOUND in Debug: $src" -ForegroundColor Red
        $allCopied = $false
    }
}

Write-Host ''

# ── Step 2: Verify all DLLs are now in Terraria folder ───────
Write-Host '── Step 2: Verifying Terraria folder ───────────────────────' -ForegroundColor DarkGray
foreach ($dll in $StubDlls) {
    $dst = Join-Path $TerrariaDir $dll
    if (Test-Path $dst) {
        Write-Host "  [OK]  $dll present in Terraria folder" -ForegroundColor Green
    } else {
        Write-Host "  [!!]  MISSING from Terraria folder: $dll" -ForegroundColor Red
    }
}

Write-Host ''

# ── Step 3: Check for extra DLLs Terraria needs ──────────────
Write-Host '── Step 3: Checking for additional missing DLLs ────────────' -ForegroundColor DarkGray

$dumpbin = $null
$roots = @(
    "$env:ProgramFiles\Microsoft Visual Studio",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio"
)
foreach ($root in $roots) {
    if (-not (Test-Path $root)) { continue }
    $hit = Get-ChildItem -Path $root -Filter 'dumpbin.exe' -Recurse -ErrorAction SilentlyContinue |
           Where-Object { $_.FullName -match 'Hostx64|x64' } | Select-Object -First 1
    if (-not $hit) {
        $hit = Get-ChildItem -Path $root -Filter 'dumpbin.exe' -Recurse -ErrorAction SilentlyContinue |
               Select-Object -First 1
    }
    if ($hit) { $dumpbin = $hit.FullName; break }
}

if ($dumpbin -and (Test-Path $GameExe)) {
    $out = & $dumpbin /imports $GameExe 2>&1
    $allImports = @()
    foreach ($line in $out) {
        if ($line -match '^\s{4}(\S+\.dll)\s*$') {
            $allImports += $matches[1].ToLower()
        }
    }

    $knownDlls = $StubDlls | ForEach-Object { $_.ToLower() }

    # DLLs that ship with Windows - we don't need to stub these
    $windowsDlls = @(
        'kernel32.dll','ntdll.dll','user32.dll','gdi32.dll','advapi32.dll',
        'ole32.dll','oleaut32.dll','shell32.dll','shlwapi.dll','winmm.dll',
        'msvcrt.dll','vcruntime140.dll','vcruntime140_1.dll','msvcp140.dll',
        'api-ms-win-core-console-l1-1-0.dll','api-ms-win-core-datetime-l1-1-0.dll',
        'api-ms-win-core-debug-l1-1-0.dll','api-ms-win-core-errorhandling-l1-1-0.dll',
        'api-ms-win-core-file-l1-1-0.dll','api-ms-win-core-file-l1-2-0.dll',
        'api-ms-win-core-file-l2-1-0.dll','api-ms-win-core-handle-l1-1-0.dll',
        'api-ms-win-core-heap-l1-1-0.dll','api-ms-win-core-interlocked-l1-1-0.dll',
        'api-ms-win-core-libraryloader-l1-1-0.dll','api-ms-win-core-localization-l1-2-0.dll',
        'api-ms-win-core-memory-l1-1-0.dll','api-ms-win-core-namedpipe-l1-1-0.dll',
        'api-ms-win-core-processenvironment-l1-1-0.dll','api-ms-win-core-processthreads-l1-1-0.dll',
        'api-ms-win-core-processthreads-l1-1-2.dll','api-ms-win-core-profile-l1-1-0.dll',
        'api-ms-win-core-rtlsupport-l1-1-0.dll','api-ms-win-core-string-l1-1-0.dll',
        'api-ms-win-core-synch-l1-1-0.dll','api-ms-win-core-synch-l1-2-0.dll',
        'api-ms-win-core-sysinfo-l1-1-0.dll','api-ms-win-core-timezone-l1-1-0.dll',
        'api-ms-win-core-util-l1-1-0.dll','api-ms-win-crt-conio-l1-1-0.dll',
        'api-ms-win-crt-convert-l1-1-0.dll','api-ms-win-crt-environment-l1-1-0.dll',
        'api-ms-win-crt-filesystem-l1-1-0.dll','api-ms-win-crt-heap-l1-1-0.dll',
        'api-ms-win-crt-locale-l1-1-0.dll','api-ms-win-crt-math-l1-1-0.dll',
        'api-ms-win-crt-multibyte-l1-1-0.dll','api-ms-win-crt-private-l1-1-0.dll',
        'api-ms-win-crt-process-l1-1-0.dll','api-ms-win-crt-runtime-l1-1-0.dll',
        'api-ms-win-crt-stdio-l1-1-0.dll','api-ms-win-crt-string-l1-1-0.dll',
        'api-ms-win-crt-time-l1-1-0.dll','api-ms-win-crt-utility-l1-1-0.dll',
        'bcrypt.dll','ncrypt.dll','crypt32.dll','ws2_32.dll','wsock32.dll',
        'rpcrt4.dll','combase.dll','comdlg32.dll','imm32.dll','wininet.dll',
        'version.dll','setupapi.dll','cfgmgr32.dll','dbghelp.dll',
        'dxgi.dll','d3d11.dll','d3d12.dll','d3dcompiler_47.dll',
        'xinput1_4.dll','xinput1_3.dll','xinput9_1_0.dll',
        'mf.dll','mfplat.dll','mfreadwrite.dll','mfuuid.dll',
        'xaudio2_9.dll','xaudio2_8.dll',
        'openal32.dll'
    )

    $needsStub = @()
    foreach ($imp in ($allImports | Sort-Object -Unique)) {
        if ($knownDlls -contains $imp) { continue }
        if ($windowsDlls -contains $imp) { continue }
        # Check if it actually exists on this machine
        $exists = [System.IO.File]::Exists("C:\Windows\System32\$imp") -or
                  [System.IO.File]::Exists("C:\Windows\SysWOW64\$imp") -or
                  (Test-Path (Join-Path $TerrariaDir $imp))
        if (-not $exists) {
            $needsStub += $imp
        }
    }

    if ($needsStub.Count -eq 0) {
        Write-Host '  [OK]  No additional unknown DLLs found!' -ForegroundColor Green
    } else {
        Write-Host '  [!!]  These DLLs may also need stubs:' -ForegroundColor Yellow
        foreach ($d in $needsStub) {
            Write-Host "        - $d" -ForegroundColor Yellow
        }
    }
} else {
    Write-Host '  [--]  Skipped (dumpbin or Terraria.exe not found)' -ForegroundColor DarkGray
}

Write-Host ''
Write-Host '============================================================' -ForegroundColor Cyan
Write-Host '  Done! Next steps:' -ForegroundColor Cyan
Write-Host '    1. Run Scorpio:' -ForegroundColor White
Write-Host '       "C:\Users\maja\source\repos\XOneLayer\x64\Debug\XOneLayer.exe"' -ForegroundColor Gray
Write-Host '       "C:\Users\maja\Downloads\terraria_xb1_dump\terraria_xb1_dump\Mount\Terraria.exe"' -ForegroundColor Gray
Write-Host '    2. If it still crashes 0xC0000135, open Process Monitor:' -ForegroundColor White
Write-Host '       Filter: Process Name = Terraria.exe AND Result = NAME NOT FOUND' -ForegroundColor Gray
Write-Host '    3. Paste any new missing DLL names back to Claude!' -ForegroundColor White
Write-Host '============================================================' -ForegroundColor Cyan
Write-Host ''
