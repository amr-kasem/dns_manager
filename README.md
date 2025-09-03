# DNS Manager Plugin

A Flutter plugin for managing DNS settings on Linux systems using NetworkManager (`nmcli`).

## Features

- **Get DNS**: Retrieve current DNS settings for the active network connection
- **Set DNS**: Set custom DNS servers for the active network connection
- **Reset DNS**: Reset DNS settings to automatic (DHCP) mode

## Supported Platforms

- **Linux**: Uses NetworkManager (`nmcli`) to manage DNS settings
- Other platforms: Placeholder implementations (returns platform version)

## Installation

Add this plugin to your `pubspec.yaml`:

```yaml
dependencies:
  dns_manager: ^0.0.1
```

## Usage

### Basic Usage

```dart
import 'package:dns_manager/dns_manager.dart';

final dnsManager = DnsManager();

// Get current DNS settings
String? currentDns = await dnsManager.getDNS();
print('Current DNS: $currentDns');

// Set custom DNS servers
String? result = await dnsManager.setDNS('8.8.8.8,8.8.4.4');
print('Set DNS result: $result');

// Reset to automatic DNS
String? resetResult = await dnsManager.resetDNS();
print('Reset DNS result: $resetResult');
```

### Example App

The `example/` directory contains a complete Flutter app demonstrating the plugin usage.

## Linux Implementation Details

The Linux implementation uses NetworkManager's command-line interface (`nmcli`) to:

1. **Find Active Connection**: Automatically detects the active ethernet or WiFi connection
2. **Modify DNS Settings**: Uses `nmcli connection modify` to change DNS settings
3. **Apply Changes**: Restarts the connection to apply DNS changes immediately

### Commands Used

- `nmcli -t -f UUID,TYPE,DEVICE connection show --active`: List active connections
- `nmcli connection modify <UUID> ipv4.dns <DNS_SERVERS>`: Set DNS servers
- `nmcli connection down <UUID> && nmcli connection up <UUID>`: Restart connection

### Requirements

- NetworkManager must be installed and running
- The app must have sufficient permissions to modify network settings
- An active network connection (ethernet or WiFi)

## Error Handling

The plugin returns descriptive error messages for common issues:

- "Error: No active connection found" - No network connection detected
- "Error: DNS parameter required" - Missing DNS parameter in setDNS call
- "Error: Invalid arguments" - Incorrect argument format

## Building and Testing

### Build the Plugin

```bash
flutter build linux
```

### Run Tests

```bash
flutter test
```

### Run Linux Tests

```bash
cd linux
make test
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.
