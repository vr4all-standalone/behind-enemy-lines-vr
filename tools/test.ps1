param([string]$PythonPath='D:/Python314/python.exe',[string]$MsysRoot='D:/msys64')
$ErrorActionPreference='Stop'
$taskProject=Split-Path -Parent $PSScriptRoot
$taskBuild=Join-Path $taskProject 'build'
$taskTemp=Join-Path $taskBuild 'test-temp'
New-Item -ItemType Directory -Path $taskTemp -Force | Out-Null
$taskOldPath,$taskOldTemp,$taskOldTmp=$env:PATH,$env:TEMP,$env:TMP
try {
    $env:PATH=(Join-Path $MsysRoot 'mingw64/bin')+';'+$env:PATH
    $env:TEMP=$env:TMP=$taskTemp
    foreach($taskSuite in @('tests','android/tests')) {
        & $PythonPath -B -m unittest discover -s (Join-Path $taskProject $taskSuite) -v
        if($LASTEXITCODE){throw "Python regressions failed: $taskSuite"}
    }
    foreach($taskTest in @('reticle_options','burst_feedback','native_reticle','fire_feedback','hud_aim','gun_setup','world_scale','cabinet_io','input_policy','game_state','native_aim_calibration','aim_policy','operator_policy','raster_policy','settings_io','launch_policy','audio_probe','uart_clock_policy','adaptive_audio_bridge','capture','trace_commit','gun_pose')) {
        $taskExe=Join-Path $taskBuild ($taskTest+'_test.exe')
        & (Join-Path $MsysRoot 'mingw64/bin/g++.exe') -std=c++20 -Wall -Wextra "-I$taskProject/vendor/glm" (Join-Path $taskProject ('tests/'+$taskTest+'_test.cpp')) -o $taskExe
        if($LASTEXITCODE){throw "Cannot compile $taskTest"}
        if($taskTest -eq 'trace_commit'){& $taskExe $taskTemp}else{& $taskExe}
        if($LASTEXITCODE){throw "Native regression failed: $taskTest"}
    }
} finally {$env:PATH=$taskOldPath;$env:TEMP=$taskOldTemp;$env:TMP=$taskOldTmp}
