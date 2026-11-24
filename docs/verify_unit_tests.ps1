$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$build = Join-Path $root 'out\build\default'
& (Join-Path $build 'Lab10_CallCenter_Tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Pattern tests failed' }
& (Join-Path $build 'Lab10_CallCenter_DBTests.exe')
if ($LASTEXITCODE -ne 0) { throw 'SQLite integration tests failed' }
