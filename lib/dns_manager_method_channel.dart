import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'dart:async';

import 'dns_manager_platform_interface.dart';

/// DNS operation status events
class DnsOperationEvent {
  final String operation;
  final String status;
  final String? result;
  final String? error;
  final String? connectionStatus;

  DnsOperationEvent({
    required this.operation,
    required this.status,
    this.result,
    this.error,
    this.connectionStatus,
  });
}

/// An implementation of [DnsManagerPlatform] that uses method channels.
class MethodChannelDnsManager extends DnsManagerPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('dns_manager');

  /// Stream controller for DNS operation events
  static final StreamController<DnsOperationEvent> _eventController = 
      StreamController<DnsOperationEvent>.broadcast();

  /// Stream of DNS operation events
  static Stream<DnsOperationEvent> get eventStream => _eventController.stream;

  @override
  Future<String?> getDNS() async {
    // Return immediately and publish result via stream
    _eventController.add(DnsOperationEvent(
      operation: 'getDNS',
      status: 'started',
    ));

    _executeOperation('getDNS', () => methodChannel.invokeMethod<String>('getDNS'));
    return null;
  }

  @override
  Future<String?> setDNS(String dns) async {
    // Return immediately and publish result via stream
    _eventController.add(DnsOperationEvent(
      operation: 'setDNS',
      status: 'started',
    ));

    _executeOperation('setDNS', () => methodChannel.invokeMethod<String>('setDNS', {'dns': dns}));
    return null;
  }

  @override
  Future<String?> resetDNS() async {
    // Return immediately and publish result via stream
    _eventController.add(DnsOperationEvent(
      operation: 'resetDNS',
      status: 'started',
    ));

    _executeOperation('resetDNS', () => methodChannel.invokeMethod<String>('resetDNS'));
    return null;
  }

  /// Execute operation asynchronously and publish results via stream
  static void _executeOperation(String operation, Future<String?> Function() methodCall) async {
    try {
      final result = await methodCall().timeout(
        const Duration(seconds: 10),
        onTimeout: () => 'Operation timed out. Please try again.',
      );

      _eventController.add(DnsOperationEvent(
        operation: operation,
        status: 'completed',
        result: result,
      ));

      // If the operation involves network restart, monitor connection status
      if (result?.contains('Network reconnecting') == true) {
        _monitorConnectionStatus();
      }
    } catch (e) {
      _eventController.add(DnsOperationEvent(
        operation: operation,
        status: 'error',
        error: 'Error: $e',
      ));
    }
  }

  /// Monitor network connection status
  static void _monitorConnectionStatus() async {
    const methodChannel = MethodChannel('dns_manager');
    
    // Check connection status every 2 seconds for up to 30 seconds
    for (int i = 0; i < 15; i++) {
      await Future.delayed(const Duration(seconds: 2));
      
      try {
        final status = await methodChannel.invokeMethod<String>('getConnectionStatus');
        _eventController.add(DnsOperationEvent(
          operation: 'connectionStatus',
          status: 'update',
          connectionStatus: status,
        ));
        
        // If connection is active, stop monitoring
        if (status?.contains('connected') == true || status?.contains('activated') == true) {
          _eventController.add(DnsOperationEvent(
            operation: 'connectionStatus',
            status: 'completed',
            connectionStatus: 'Network connection restored',
          ));
          break;
        }
      } catch (e) {
        // Continue monitoring even if status check fails
      }
    }
  }

  /// Dispose the event controller
  static void dispose() {
    _eventController.close();
  }
}
