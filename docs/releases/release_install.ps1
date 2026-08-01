# Moss release installation script.
# This script copies the moss binary and libraries to expected locations.
#
# Usage: .\install.ps1

$MossDir = "$env:LocalAppData\moss"
$BinDir = "$MossDir\bin"

try {
    Write-Host "Installing Moss..."

    # Create directories
    New-Item -ItemType Directory -Force -Path $BinDir -ErrorAction Stop | Out-Null

    # Copy executable
    Write-Host "Copying moss.exe..."
    Copy-Item "moss.exe" $BinDir -Force -ErrorAction Stop
    Copy-Item "python*.dll" $BinDir -Force -ErrorAction Stop

    # Copy .msb files
    Write-Host "Copying .msb files..."
    Copy-Item "*.msb" $MossDir -Force -ErrorAction Stop
    Copy-Item "*.css" $MossDir -Force -ErrorAction Stop

    Write-Host "Installation complete."
    Write-Host "Installed to: $MossDir"
}
catch {
    Write-Host "Installation failed!" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}
