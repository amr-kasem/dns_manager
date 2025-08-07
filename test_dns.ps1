Write-Host "=== DNS Manager Test ===" -ForegroundColor Green

# Test 1: Get current DNS
Write-Host "`n1. Testing Get Current DNS:" -ForegroundColor Yellow
$output = netsh interface ip show dns
Write-Host "DNS Output:"
Write-Host $output

# Test 2: Get network adapters
Write-Host "`n2. Testing Get Network Adapters:" -ForegroundColor Yellow
$adapters = netsh interface show interface
Write-Host "Network Adapters Output:"
Write-Host $adapters

# Test 3: Test connection status
Write-Host "`n3. Testing Connection Status:" -ForegroundColor Yellow
$pingResult = ping 8.8.8.8 -n 1
Write-Host "Ping Result:"
Write-Host $pingResult

Write-Host "`n=== Test Complete ===" -ForegroundColor Green 