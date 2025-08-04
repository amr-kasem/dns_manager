import 'package:flutter_test/flutter_test.dart';
import 'package:dns_manager/dns_manager.dart';
import 'package:dns_manager/dns_manager_platform_interface.dart';
import 'package:dns_manager/dns_manager_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockDnsManagerPlatform
    with MockPlatformInterfaceMixin
    implements DnsManagerPlatform {
  @override
  Future<String?> getDNS() => Future.value('42');

  @override
  Future<String?> setDNS(String dns) => Future.value('42');

  @override
  Future<String?> resetDNS() => Future.value('42');
}

void main() {
  final DnsManagerPlatform initialPlatform = DnsManagerPlatform.instance;

  test('$MethodChannelDnsManager is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelDnsManager>());
  });

  test('getPlatformVersion', () async {
    DnsManager dnsManagerPlugin = DnsManager();
    MockDnsManagerPlatform fakePlatform = MockDnsManagerPlatform();
    DnsManagerPlatform.instance = fakePlatform;

    expect(await dnsManagerPlugin.getDNS(), '42');
  });
}
