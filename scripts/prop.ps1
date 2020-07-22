# Copies the content from the staging directory for M5Sys to the SD card in drive G:

Write-Host "Copying drop to SD Card in drive G:"
$files          = @('AstroClock.bin', 'ClickBang.bin', 'Clock.bin', 'Distance.bin',
                    'FileManager.bin', 'HomeControl.bin', 'IChing.bin', 'ImuMonitor.bin',
                    'Life.bin', 'mDNSMonitor.bin', 'menu.bin', 'MQTTMonitor.bin',
                    'PacketMonitor.bin', 'SoundMonitor.bin', 'SoundRecorder.bin',
                    'Weather.bin', 'WiFiScanner.bin')
$data_files     = @('data/hexagrams.idx', 'data/hexagrams.isam')
$script_dir     = $PSScriptRoot                         # Location of the script.
$project_dir    = "$script_dir/.."                      # Location of the project.
$prop_dir       = "$project_dir/SD"                     # Drop location
$dest_dir       = 'G:/'                                 # SD card on my particular system
$dest_data      = "G:/data"

Remove-Item $dest_data -Recurse
Remove-Item 'G:\*.*' -Exclude 'System Volume Information' -Recurse
Set-Location -Path "$prop_dir"                          # Copy from SD
foreach ($file in $files) {
    Copy-Item $file $dest_dir                           # Copy over all programs
}
New-Item -Path "G:/" -Name "data" -ItemType "directory"
foreach ($file in $data_files) {
    Copy-Item $file $dest_data                          # Copy over all data files
}
Set-Location -Path "$script_dir"                        # Go back home
