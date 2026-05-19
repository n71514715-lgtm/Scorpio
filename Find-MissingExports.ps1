param(
    [string]$GameExe  = 'C:\Users\maja\Downloads\terraria_xb1_dump\terraria_xb1_dump\Mount\Terraria.exe',
    [string]$DebugDir = 'C:\Users\maja\source\repos\XOneLayer\x64\Debug',
    [string]$OutStubs = 'C:\Users\maja\source\repos\XOneLayer\missing_stubs.cpp'
)

# ── 1. Find dumpbin.exe ───────────────────────────────────────
function Find-Dumpbin {
    $db = Get-Command dumpbin.exe -ErrorAction SilentlyContinue
    if ($db) { return $db.Source }
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
        if ($hit) { return $hit.FullName }
    }
    return $null
}

$dumpbin = Find-Dumpbin
if (-not $dumpbin) {
    Write-Error 'dumpbin.exe not found. Run from x64 Native Tools Command Prompt for VS 2022.'
    exit 1
}
Write-Host ('dumpbin: ' + $dumpbin) -ForegroundColor Cyan
Write-Host ''

# ── 2. Stub DLLs ─────────────────────────────────────────────
$stubPaths = @(
    $DebugDir + '\AcpHal.dll',
    $DebugDir + '\d3d12_x.dll',
    $DebugDir + '\xg_x.dll',
    $DebugDir + '\GameInput.dll',
    $DebugDir + '\xmem.dll',
    $DebugDir + '\XFrontPanelDisplay.dll'
)

$stubMap = @{}
Write-Host '── Stub DLLs ──────────────────────────────────────────' -ForegroundColor DarkGray
foreach ($p in $stubPaths) {
    if (Test-Path $p) {
        $key = [IO.Path]::GetFileName($p).ToLower()
        $stubMap[$key] = $p
        Write-Host ('  [OK]  ' + $p) -ForegroundColor Green
    } else {
        Write-Host ('  [!!]  NOT FOUND: ' + $p) -ForegroundColor Yellow
    }
}
Write-Host ''

# ── 3. Parse imports from the game EXE ───────────────────────
function Get-GameImports([string]$exe) {
    $out = & $dumpbin /imports $exe 2>&1
    $map = @{}
    $cur = $null
    foreach ($line in $out) {
        if ($line -match '^\s{4}(\S+\.dll)\s*$') {
            $cur = $matches[1].ToLower()
            if (-not $map.ContainsKey($cur)) {
                $map[$cur] = [Collections.Generic.List[string]]::new()
            }
            continue
        }
        if ($cur -and ($line -match '^\s+[0-9A-Fa-f]+\s+([A-Za-z_][A-Za-z0-9_@?]+)\s*$')) {
            $map[$cur].Add($matches[1])
        }
    }
    return $map
}

# ── 4. Parse exports from a stub DLL ─────────────────────────
function Get-DllExports([string]$dll) {
    $out = & $dumpbin /exports $dll 2>&1
    $set = [Collections.Generic.HashSet[string]]::new()
    foreach ($line in $out) {
        if ($line -match '^\s+\d+\s+[\dA-Fa-f]+\s+[\dA-Fa-f]+\s+(\S+)') {
            [void]$set.Add($matches[1])
        }
    }
    return $set
}

# ── 5. Diff ───────────────────────────────────────────────────
Write-Host '── Scanning Terraria.exe imports ──────────────────────' -ForegroundColor DarkGray
$gameImports = Get-GameImports $GameExe

$allMissing = [ordered]@{}

foreach ($importedDll in ($gameImports.Keys | Sort-Object)) {
    if (-not $stubMap.ContainsKey($importedDll)) { continue }
    $exports = Get-DllExports $stubMap[$importedDll]
    $missing = $gameImports[$importedDll] | Where-Object { -not $exports.Contains($_) }
    if ($missing) {
        $allMissing[$importedDll] = @($missing | Sort-Object)
    }
}

# ── 6. Report ─────────────────────────────────────────────────
Write-Host ''
Write-Host '======================================================' -ForegroundColor Cyan
if ($allMissing.Count -eq 0) {
    Write-Host '  All stub exports match - no missing functions!' -ForegroundColor Green
    Write-Host '  If the game still crashes:' -ForegroundColor Yellow
    Write-Host '    - Make sure DLLs are copied into the Terraria folder' -ForegroundColor Yellow
    Write-Host '    - Use Process Monitor filter: NAME NOT FOUND' -ForegroundColor Yellow
} else {
    Write-Host '  MISSING EXPORTS:' -ForegroundColor Red
    Write-Host ''
    foreach ($dll in $allMissing.Keys) {
        Write-Host ('  ' + $dll) -ForegroundColor Magenta
        foreach ($fn in $allMissing[$dll]) {
            Write-Host ('      - ' + $fn) -ForegroundColor White
        }
        Write-Host ''
    }
}
Write-Host '======================================================' -ForegroundColor Cyan

# ── 7. Write missing_stubs.cpp ───────────────────────────────
if ($allMissing.Count -gt 0) {
    Write-Host ''
    Write-Host ('Generating -> ' + $OutStubs) -ForegroundColor Cyan

    $lines = [Collections.Generic.List[string]]::new()

    $lines.Add('// ============================================================')
    $lines.Add('// Scorpio - Auto-Generated Missing Stubs')
    $lines.Add('// Generated: ' + (Get-Date -Format 'yyyy-MM-dd HH:mm'))
    $lines.Add('//')
    $lines.Add('// Copy each section into the matching VS project and rebuild.')
    $lines.Add('// Every stub returns S_OK.')
    $lines.Add('// ============================================================')
    $lines.Add('#include <windows.h>')
    $lines.Add('')

    foreach ($dll in $allMissing.Keys) {
        $bar = '// -- ' + $dll + ' ' + ('-' * [Math]::Max(2, 50 - $dll.Length))
        $lines.Add($bar)
        foreach ($fn in $allMissing[$dll]) {
            $lines.Add('extern "C" __declspec(dllexport) HRESULT ' + $fn + '()')
            $lines.Add('{')
            $lines.Add('    return S_OK;')
            $lines.Add('}')
            $lines.Add('')
        }
    }

    Set-Content -Path $OutStubs -Value $lines -Encoding UTF8

    Write-Host ('  Written: ' + $OutStubs) -ForegroundColor Green
    Write-Host ''
    Write-Host '  Next steps:' -ForegroundColor Cyan
    Write-Host '    1. Open missing_stubs.cpp in VS 2022' -ForegroundColor White
    Write-Host '    2. Copy each function into the matching project' -ForegroundColor White
    Write-Host '    3. Rebuild All (Ctrl+Shift+B)' -ForegroundColor White
    Write-Host '    4. Re-run Scorpio' -ForegroundColor White
}
