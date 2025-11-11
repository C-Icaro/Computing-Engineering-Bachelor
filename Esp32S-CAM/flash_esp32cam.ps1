# Script para fazer erase e upload correto da ESP32-CAM
# Uso: .\flash_esp32cam.ps1 -Port COM3

param(
    [Parameter(Mandatory=$true)]
    [string]$Port
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "ESP32-CAM Flash Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Verificar se esptool está disponível
$esptool = Get-Command esptool.py -ErrorAction SilentlyContinue
if (-not $esptool) {
    Write-Host "ERRO: esptool.py não encontrado!" -ForegroundColor Red
    Write-Host "Instale com: pip install esptool" -ForegroundColor Yellow
    exit 1
}

Write-Host "1. Fazendo erase completo da flash..." -ForegroundColor Yellow
python -m esptool --port $Port erase_flash
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERRO ao fazer erase da flash!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "2. Compilando o sketch com configurações corretas..." -ForegroundColor Yellow
arduino-cli compile --fqbn esp32:esp32:esp32cam `
    --build-property "build.flash_mode=dio" `
    --build-property "build.flash_freq=80m" `
    --build-property "build.flash_size=4MB" `
    --build-property "build.partitions=default" `
    --build-property "build.psram_type=opi" `
    Esp32S-CAM\esp32s-cam.ino

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERRO na compilação!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "3. Fazendo upload do firmware..." -ForegroundColor Yellow
arduino-cli upload --fqbn esp32:esp32:esp32cam `
    --upload-property "upload.flash_mode=dio" `
    --upload-property "upload.flash_freq=80m" `
    --upload-property "upload.flash_size=4MB" `
    -p $Port `
    Esp32S-CAM\esp32s-cam.ino

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERRO no upload!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Upload concluído com sucesso!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Agora abra o Serial Monitor (115200 baud) para ver o IP da ESP32-CAM" -ForegroundColor Cyan

