import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'dns_manager_method_channel.dart';

abstract class DnsManagerPlatform extends PlatformInterface {
  /// Constructs a DnsManagerPlatform.
  DnsManagerPlatform() : super(token: _token);

  static final Object _token = Object();

  static DnsManagerPlatform _instance = MethodChannelDnsManager();

  /// The default instance of [DnsManagerPlatform] to use.
  ///
  /// Defaults to [MethodChannelDnsManager].
  static DnsManagerPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [DnsManagerPlatform] when
  /// they register themselves.
  static set instance(DnsManagerPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getDNS() {
    throw UnimplementedError('getDNS() has not been implemented.');
  }

  Future<String?> setDNS(String dns) {
    throw UnimplementedError('setDNS() has not been implemented.');
  }

  Future<String?> resetDNS() {
    throw UnimplementedError('resetDNS() has not been implemented.');
  }
}
