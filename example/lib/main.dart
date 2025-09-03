import 'package:flutter/material.dart';
import 'dart:async';

import 'package:dns_manager/dns_manager.dart';
import 'package:dns_manager/dns_manager_method_channel.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _currentDns = 'Loading...';
  String _lastResult = '';
  bool _isLoading = false;
  final _dnsController = TextEditingController(text: '8.8.8.8,8.8.4.4');
  final _dnsManagerPlugin = DnsManager();
  StreamSubscription<DnsOperationEvent>? _eventSubscription;

  @override
  void initState() {
    super.initState();
    _setupEventStream();
    _loadCurrentDns();
  }

  @override
  void dispose() {
    _eventSubscription?.cancel();
    _dnsController.dispose();
    super.dispose();
  }

  void _setupEventStream() {
    _eventSubscription = MethodChannelDnsManager.eventStream.listen((event) {
      setState(() {
        _isLoading = false;

        switch (event.status) {
          case 'started':
            _isLoading = true;
            _lastResult = '${event.operation} operation started...';
            break;
          case 'completed':
            _lastResult = event.result ?? 'Operation completed successfully';
            if (event.operation == 'getDNS') {
              _currentDns = event.result ?? 'Failed to get DNS';
            }
            break;
          case 'update':
            if (event.operation == 'connectionStatus') {
              _lastResult = 'Network status: ${event.connectionStatus}';
            }
            break;
          case 'error':
            _lastResult = event.error ?? 'Operation failed';
            break;
        }
      });
    });
  }

  Future<void> _loadCurrentDns() async {
    // Operation will be handled by the event stream
    await _dnsManagerPlugin.getDNS();
  }

  Future<void> _setDns() async {
    // Operation will be handled by the event stream
    await _dnsManagerPlugin.setDNS(_dnsController.text);
    // Reload current DNS after a short delay to show the change
    Future.delayed(const Duration(seconds: 2), () {
      _loadCurrentDns();
    });
  }

  Future<void> _resetDns() async {
    // Operation will be handled by the event stream
    await _dnsManagerPlugin.resetDNS();
    // Reload current DNS after a short delay to show the change
    Future.delayed(const Duration(seconds: 2), () {
      _loadCurrentDns();
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('DNS Manager Example'),
          backgroundColor: Colors.blue,
          foregroundColor: Colors.white,
        ),
        body: Padding(
          padding: const EdgeInsets.all(16.0),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Card(
                child: Padding(
                  padding: const EdgeInsets.all(16.0),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      const Text(
                        'Current DNS Settings',
                        style: TextStyle(
                          fontSize: 18,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      const SizedBox(height: 8),
                      Text(_currentDns, style: const TextStyle(fontSize: 16)),
                      const SizedBox(height: 8),
                      ElevatedButton(
                        onPressed: _isLoading ? null : _loadCurrentDns,
                        child: _isLoading
                            ? const Row(
                                mainAxisSize: MainAxisSize.min,
                                children: [
                                  SizedBox(
                                    width: 16,
                                    height: 16,
                                    child: CircularProgressIndicator(
                                      strokeWidth: 2,
                                    ),
                                  ),
                                  SizedBox(width: 8),
                                  Text('Loading...'),
                                ],
                              )
                            : const Text('Refresh DNS'),
                      ),
                    ],
                  ),
                ),
              ),
              const SizedBox(height: 16),
              Card(
                child: Padding(
                  padding: const EdgeInsets.all(16.0),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      const Text(
                        'Set Custom DNS',
                        style: TextStyle(
                          fontSize: 18,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      const SizedBox(height: 8),
                      TextField(
                        controller: _dnsController,
                        decoration: const InputDecoration(
                          labelText: 'DNS Servers (comma-separated)',
                          hintText: '8.8.8.8,8.8.4.4',
                          border: OutlineInputBorder(),
                        ),
                      ),
                      const SizedBox(height: 8),
                      Row(
                        children: [
                          Expanded(
                            child: ElevatedButton(
                              onPressed: _isLoading ? null : _setDns,
                              style: ElevatedButton.styleFrom(
                                backgroundColor: Colors.green,
                                foregroundColor: Colors.white,
                              ),
                              child: _isLoading
                                  ? const Row(
                                      mainAxisSize: MainAxisSize.min,
                                      children: [
                                        SizedBox(
                                          width: 16,
                                          height: 16,
                                          child: CircularProgressIndicator(
                                            strokeWidth: 2,
                                            color: Colors.white,
                                          ),
                                        ),
                                        SizedBox(width: 8),
                                        Text('Setting...'),
                                      ],
                                    )
                                  : const Text('Set DNS'),
                            ),
                          ),
                          const SizedBox(width: 8),
                          Expanded(
                            child: ElevatedButton(
                              onPressed: _isLoading ? null : _resetDns,
                              style: ElevatedButton.styleFrom(
                                backgroundColor: Colors.orange,
                                foregroundColor: Colors.white,
                              ),
                              child: _isLoading
                                  ? const Row(
                                      mainAxisSize: MainAxisSize.min,
                                      children: [
                                        SizedBox(
                                          width: 16,
                                          height: 16,
                                          child: CircularProgressIndicator(
                                            strokeWidth: 2,
                                            color: Colors.white,
                                          ),
                                        ),
                                        SizedBox(width: 8),
                                        Text('Resetting...'),
                                      ],
                                    )
                                  : const Text('Reset to Auto'),
                            ),
                          ),
                        ],
                      ),
                    ],
                  ),
                ),
              ),
              if (_lastResult.isNotEmpty) ...[
                const SizedBox(height: 16),
                Card(
                  color: _lastResult.contains('Error')
                      ? Colors.red[100]
                      : Colors.green[100],
                  child: Padding(
                    padding: const EdgeInsets.all(16.0),
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        const Text(
                          'Last Operation Result',
                          style: TextStyle(
                            fontSize: 18,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                        const SizedBox(height: 8),
                        Text(_lastResult, style: const TextStyle(fontSize: 16)),
                      ],
                    ),
                  ),
                ),
              ],
            ],
          ),
        ),
      ),
    );
  }
}
