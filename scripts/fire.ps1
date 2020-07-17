Write-Host "Building for M5Stack Fire"
$programs       = @('AstroClock', 'ClickBang', 'Clock', 'Distance', 'FileManager', 'HomeControl',
                    'IChing', 'ImuMonitor', 'Life', 'mDNSMonitor', 'menu', 'MQTTMonitor',
                    'PacketMonitor', 'SoundMonitor', 'SoundRecorder', 'Weather', 'WiFiScanner')
$firmware_path  = '.pio/build/m5stack-fire'             # path from project directory to built firmware
$script_dir     = $PSScriptRoot                         # Location of the script.
$project_dir    = "$script_dir/.."                      # Location of the project.
$prop_dir       = "$project_dir/SD"                     # Destination location
$apps_dir       = "$project_dir/Apps"                   # Location of M5Sys application projects
$iching_data    = "$apps_dir/IChing/data/hexagrams.*"   # Data files for IChing

Remove-Item "$prop_dir/*" -Recurse                      # clean the output directory
New-Item -Path "$prop_dir/data" -ItemType Directory     # create PROP/data
Copy-Item $iching_data "$prop_dir/data"                 # copy IChing data files to PROP/data
foreach ($prog in $programs) {
    Set-Location -Path "$apps_dir/$prog"
    Invoke-Expression 'platformio run --environment m5stack-fire'
    Copy-Item "$apps_dir/$prog/$firmware_path/$prog.bin" "$prop_dir/$prog.bin"
}
