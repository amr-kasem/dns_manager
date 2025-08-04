#ifndef FLUTTER_PLUGIN_DNS_MANAGER_PLUGIN_H_
#define FLUTTER_PLUGIN_DNS_MANAGER_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>

namespace dns_manager {

class DnsManagerPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  DnsManagerPlugin();

  virtual ~DnsManagerPlugin();

  // Disallow copy and assign.
  DnsManagerPlugin(const DnsManagerPlugin&) = delete;
  DnsManagerPlugin& operator=(const DnsManagerPlugin&) = delete;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

}  // namespace dns_manager

#endif  // FLUTTER_PLUGIN_DNS_MANAGER_PLUGIN_H_
