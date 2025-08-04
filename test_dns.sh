#!/bin/bash

echo "=== DNS Manager Plugin Test ==="
echo

# Get active connection
echo "1. Getting active connection..."
CONNECTION=$(nmcli -t -f UUID,TYPE,DEVICE connection show --active | grep "802-11-wireless" | head -1 | cut -d: -f1)
if [ -z "$CONNECTION" ]; then
    CONNECTION=$(nmcli -t -f UUID,TYPE,DEVICE connection show --active | grep ethernet | head -1 | cut -d: -f1)
fi

if [ -z "$CONNECTION" ]; then
    echo "Error: No active connection found"
    exit 1
fi

echo "Active connection: $CONNECTION"
echo

# Get current DNS
echo "2. Getting current DNS settings..."
CURRENT_DNS=$(nmcli connection show "$CONNECTION" | grep ipv4.dns | cut -d: -f2 | xargs)
echo "Current DNS: $CURRENT_DNS"
echo

# Test setting DNS
echo "3. Testing DNS setting..."
echo "Setting DNS to 1.1.1.1,1.0.0.1..."
nmcli connection modify "$CONNECTION" ipv4.dns "1.1.1.1,1.0.0.1"
if [ $? -eq 0 ]; then
    echo "DNS set successfully"
else
    echo "Error setting DNS"
fi
echo

# Get DNS again to verify
echo "4. Verifying DNS change..."
NEW_DNS=$(nmcli connection show "$CONNECTION" | grep ipv4.dns | cut -d: -f2 | xargs)
echo "New DNS: $NEW_DNS"
echo

# Reset DNS
echo "5. Resetting DNS to automatic..."
nmcli connection modify "$CONNECTION" ipv4.dns ""
if [ $? -eq 0 ]; then
    echo "DNS reset successfully"
else
    echo "Error resetting DNS"
fi
echo

# Get DNS one more time
echo "6. Final DNS check..."
FINAL_DNS=$(nmcli connection show "$CONNECTION" | grep ipv4.dns | cut -d: -f2 | xargs)
echo "Final DNS: $FINAL_DNS"
echo

echo "=== Test Complete ===" 