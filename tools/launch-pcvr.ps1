param([string]$RomDirectory = (Join-Path $PSScriptRoot 'roms'))
$ErrorActionPreference='Stop'
$taskRoot=$PSScriptRoot
if(!(Test-Path -LiteralPath (Join-Path $taskRoot 'bel.exe'))) {$taskRoot=Split-Path -Parent $PSScriptRoot}
$taskRom=[IO.Path]::GetFullPath($RomDirectory)
foreach($taskFile in @('bel.zip')) {
    if(!(Test-Path -LiteralPath (Join-Path $taskRom $taskFile) -PathType Leaf)) {throw "Missing owned game file: $taskRom\$taskFile"}
}
Push-Location -LiteralPath $taskRoot
try {
    & (Join-Path $taskRoot 'bel.exe') -rompath $taskRom -verifyroms bel
    if($LASTEXITCODE){throw 'MAME rejected the game data. Use the original Behind Enemy Lines arcade bel.zip with all 31 verified payloads. No CHD is used.'}
    $taskData=Join-Path $taskRoot 'userdata'
    foreach($taskDirectory in @('nvram','cfg','diff','state','snap')) {New-Item -ItemType Directory -Path (Join-Path $taskData $taskDirectory) -Force | Out-Null}
    $env:BEL_VR='1'
    $env:BEL_FAST_RASTER='1'
    $env:BEL_XR_LIBRARY=Join-Path $taskRoot 'bel_xr.dll'
    $env:BEL_ASSETS=Join-Path $taskRoot 'assets'
    $env:BEL_CUSTOM_GUN=Join-Path $taskRoot 'models/guns/custom.glb'
    $env:BEL_SETTINGS=Join-Path $taskData 'settings.ini'
    $env:BEL_XR_LOG=Join-Path $taskData 'bel-xr.log'
    & (Join-Path $taskRoot 'bel.exe') bel -rompath $taskRom -nvram_directory (Join-Path $taskData 'nvram') `
        -cfg_directory (Join-Path $taskData 'cfg') -diff_directory (Join-Path $taskData 'diff') `
        -state_directory (Join-Path $taskData 'state') -snapshot_directory (Join-Path $taskData 'snap') `
        -video none -sound auto -skip_gameinfo -noreadconfig
    $taskPlayExit=$LASTEXITCODE
    if(Test-Path -LiteralPath $env:BEL_XR_LOG) {Get-Content -LiteralPath $env:BEL_XR_LOG -Tail 15}
    if($taskPlayExit){throw "Behind Enemy Lines session exited with code $taskPlayExit."}
} finally {Pop-Location}
