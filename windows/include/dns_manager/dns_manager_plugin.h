#ifndef FLUTTER_PLUGIN_DNS_MANAGER_PLUGIN_H_
#define FLUTTER_PLUGIN_DNS_MANAGER_PLUGIN_H_

// Prevent winsock2.h conflicts
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/encodable_value.h>
#include <flutter/standard_method_codec.h> // Added for StandardMethodCodec
#include <flutter_plugin_registrar.h>

#include <memory>
#include <string>
#include <vector>

// C API function declaration
extern "C" void DnsManagerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar);

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

 private:
  // Core DNS operations
  std::string GetCurrentDNS();
  bool SetDNS(const std::string& dns);
  bool ResetDNS();
  std::string GetConnectionStatus();
  
  // Network adapter operations
  std::vector<std::string> GetActiveNetworkAdapters();
  
  // DNS parsing and validation
  std::vector<std::string> ParseDNSServers(const std::string& output);
  bool IsValidDNS(const std::string& dns);
  bool IsValidIPAddress(const std::string& ip);
  
  // System operations
  bool IsElevated();
  bool RequestElevation();
  bool RequestElevationWithDNS(const std::string& dns);
  bool RequestElevationForReset();
  std::string ExecuteCommand(const std::string& command);

  // Handle specific method calls
  void HandleSetDNS(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

}  // namespace dns_manager

#endif  // FLUTTER_PLUGIN_DNS_MANAGER_PLUGIN_H_