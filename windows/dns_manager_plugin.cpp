#include "include/dns_manager/dns_manager_plugin.h"

#include <windows.h>
#include <iphlpapi.h>
#include <sstream>
#include <regex>
#include <memory>
#include <fstream>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include <shellapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "shell32.lib")

namespace dns_manager {

void DnsManagerPlugin::RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar) {
  auto channel = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
      registrar->messenger(), "dns_manager",
      &flutter::StandardMethodCodec::GetInstance());
  
  auto plugin = std::make_unique<DnsManagerPlugin>();
  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

DnsManagerPlugin::DnsManagerPlugin() = default;
DnsManagerPlugin::~DnsManagerPlugin() = default;

void DnsManagerPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name().compare("getDNS") == 0) {
    try {
      std::string dns = GetCurrentDNS();
      result->Success(flutter::EncodableValue(dns));
    } catch (const std::exception& e) {
      result->Error("DNS_ERROR", "Failed to get DNS: " + std::string(e.what()));
    }
  } else if (method_call.method_name().compare("setDNS") == 0) {
    HandleSetDNS(method_call, std::move(result));
  } else if (method_call.method_name().compare("resetDNS") == 0) {
    try {
      if (!IsElevated()) {
        // Show UAC dialog and perform DNS reset with elevation
        if (!RequestElevationForReset()) {
          result->Error("ELEVATION_REQUIRED", "Administrator privileges required. Please run the application as administrator to reset DNS settings.");
          return;
        }
        // If elevation was successful, return success
        result->Success(flutter::EncodableValue("DNS reset successfully with elevation"));
        return;
      }
      
      if (ResetDNS()) {
        result->Success(flutter::EncodableValue("DNS reset successfully"));
      } else {
        result->Error("RESET_DNS_FAILED", "Failed to reset DNS");
      }
    } catch (const std::exception& e) {
      result->Error("DNS_ERROR", "Failed to reset DNS: " + std::string(e.what()));
    }
  } else if (method_call.method_name().compare("getConnectionStatus") == 0) {
    try {
      std::string status = GetConnectionStatus();
      result->Success(flutter::EncodableValue(status));
    } catch (const std::exception& e) {
      result->Error("CONNECTION_ERROR", "Failed to get connection status: " + std::string(e.what()));
    }
  } else {
    result->NotImplemented();
  }
}

void DnsManagerPlugin::HandleSetDNS(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  std::string dns = "8.8.8.8"; // Default DNS for testing
  
  // Try to get DNS from arguments if available
  const flutter::EncodableValue& arguments = method_call.arguments();
  if (!arguments.IsNull()) {
    // First try to get as a map (like Android expects)
    const auto* args = std::get_if<flutter::EncodableMap>(&arguments);
    if (args) {
      auto dns_it = args->find(flutter::EncodableValue("dns"));
      if (dns_it != args->end()) {
        const auto* dns_value = std::get_if<std::string>(&dns_it->second);
        if (dns_value) {
          dns = *dns_value;
        }
      }
    } else {
      // Try to get as a direct string (fallback)
      const auto* dns_value = std::get_if<std::string>(&arguments);
      if (dns_value) {
        dns = *dns_value;
      }
    }
  }

  if (!IsValidDNS(dns)) {
    result->Error("INVALID_DNS", "Provided DNS is invalid: " + dns);
    return;
  }

  if (!IsElevated()) {
    // Show UAC dialog and perform DNS operation with elevation
    if (!RequestElevationWithDNS(dns)) {
      result->Error("ELEVATION_REQUIRED", "Administrator privileges required. Please run the application as administrator to change DNS settings.");
      return;
    }
    // If elevation was successful, return success
    result->Success(flutter::EncodableValue("DNS set successfully with elevation to: " + dns));
    return;
  }

           // If already elevated, perform the DNS operation directly
         if (SetDNS(dns)) {
           result->Success(flutter::EncodableValue("DNS set successfully to: " + dns + " - Please refresh to see updated status"));
         } else {
           result->Error("SET_DNS_FAILED", "Failed to set DNS");
         }
}

std::string DnsManagerPlugin::GetCurrentDNS() {
  std::string output = ExecuteCommand("netsh interface ip show dns");
  std::vector<std::string> servers = ParseDNSServers(output);
  if (servers.empty()) {
    // Try a different approach - get DNS for specific adapters
    std::vector<std::string> adapters = GetActiveNetworkAdapters();
    if (!adapters.empty()) {
      std::string adapterOutput = ExecuteCommand("netsh interface ip show dns \"" + adapters[0] + "\"");
      servers = ParseDNSServers(adapterOutput);
    }
  }
  if (servers.empty()) return "No DNS servers found";
  return servers[0];  // Return first one
}

bool DnsManagerPlugin::SetDNS(const std::string& dns) {
  std::vector<std::string> adapters = GetActiveNetworkAdapters();
  if (adapters.empty()) return false;

  for (const auto& adapter : adapters) {
    std::string command = "netsh interface ip set dns \"" + adapter + "\" static " + dns + " primary";
    if (system(command.c_str()) != 0) return false;
  }
  return true;
}

bool DnsManagerPlugin::ResetDNS() {
  std::vector<std::string> adapters = GetActiveNetworkAdapters();
  if (adapters.empty()) return false;

  for (const auto& adapter : adapters) {
    std::string command = "netsh interface ip set dns \"" + adapter + "\" dhcp";
    if (system(command.c_str()) != 0) return false;
  }
  return true;
}

std::string DnsManagerPlugin::GetConnectionStatus() {
  std::string output = ExecuteCommand("ping 8.8.8.8 -n 1");
  if (output.find("TTL=") != std::string::npos) {
    return "Connected";
  } else {
    return "Disconnected";
  }
}

std::vector<std::string> DnsManagerPlugin::GetActiveNetworkAdapters() {
  std::vector<std::string> adapters;
  std::string output = ExecuteCommand("netsh interface show interface");

  std::istringstream stream(output);
  std::string line;
  bool inInterfaceSection = false;
  
  while (std::getline(stream, line)) {
    // Skip header lines
    if (line.find("Admin State") != std::string::npos) {
      inInterfaceSection = true;
      continue;
    }
    
    if (inInterfaceSection && line.find("Connected") != std::string::npos) {
      // Parse the interface name - it's usually the last part of the line
      size_t lastColon = line.find_last_of(':');
      if (lastColon != std::string::npos) {
        std::string name = line.substr(lastColon + 1);
        // Trim whitespace
        name.erase(0, name.find_first_not_of(" \t\r\n"));
        name.erase(name.find_last_not_of(" \t\r\n") + 1);
        if (!name.empty()) {
          adapters.push_back(name);
        }
      }
    }
  }
  
  // If no adapters found, try a different approach
  if (adapters.empty()) {
    // Try to get all interfaces and filter by status
    std::string configOutput = ExecuteCommand("netsh interface ip show config");
    std::istringstream configStream(configOutput);
    std::string configLine;
    std::string currentAdapter;
    
    while (std::getline(configStream, configLine)) {
      if (configLine.find("Configuration for interface") != std::string::npos) {
        // Extract interface name
        size_t startQuote = configLine.find("\"");
        size_t endQuote = configLine.find("\"", startQuote + 1);
        if (startQuote != std::string::npos && endQuote != std::string::npos) {
          currentAdapter = configLine.substr(startQuote + 1, endQuote - startQuote - 1);
        }
      } else if (!currentAdapter.empty() && configLine.find("IP Address:") != std::string::npos) {
        // Check if this adapter has an IP (indicating it's active)
        size_t colonPos = configLine.find(":");
        if (colonPos != std::string::npos) {
          std::string ip = configLine.substr(colonPos + 1);
          ip.erase(0, ip.find_first_not_of(" \t"));
          if (!ip.empty() && ip != "0.0.0.0") {
            adapters.push_back(currentAdapter);
            currentAdapter.clear(); // Reset to avoid duplicates
          }
        }
      }
    }
  }
  
  return adapters;
}

std::vector<std::string> DnsManagerPlugin::ParseDNSServers(const std::string& output) {
  std::vector<std::string> dns_list;
  std::regex dns_regex(R"((\d{1,3}\.){3}\d{1,3})");
  std::smatch match;
  std::string::const_iterator searchStart(output.cbegin());

  while (std::regex_search(searchStart, output.cend(), match, dns_regex)) {
    dns_list.push_back(match[0]);
    searchStart = match.suffix().first;
  }

  return dns_list;
}

bool DnsManagerPlugin::IsValidDNS(const std::string& dns) {
  return IsValidIPAddress(dns);
}

bool DnsManagerPlugin::IsValidIPAddress(const std::string& ip) {
  std::regex ip_regex(R"(^(\d{1,3}\.){3}\d{1,3}$)");
  if (!std::regex_match(ip, ip_regex)) return false;

  std::istringstream iss(ip);
  std::string octet;
  while (std::getline(iss, octet, '.')) {
    try {
      int value = std::stoi(octet);
      if (value < 0 || value > 255) return false;
    } catch (...) {
      return false;
    }
  }

  return true;
}

bool DnsManagerPlugin::IsElevated() {
  BOOL isElevated = FALSE;
  HANDLE token = NULL;

  if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
    TOKEN_ELEVATION elevation;
    DWORD size;
    if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size)) {
      isElevated = elevation.TokenIsElevated;
    }
    CloseHandle(token);
  }

  return isElevated;
}

std::string DnsManagerPlugin::ExecuteCommand(const std::string& cmd) {
  std::string result;
  char buffer[128];
  FILE* pipe = _popen(cmd.c_str(), "r");
  if (!pipe) return "Failed to run command";
  while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
    result += buffer;
  }
  _pclose(pipe);
  return result;
}

bool DnsManagerPlugin::RequestElevation() {
  // This is a fallback function - not used in the new implementation
  return RequestElevationWithDNS("8.8.8.8");
}

bool DnsManagerPlugin::RequestElevationWithDNS(const std::string& dns) {
  // Get active network adapters
  std::vector<std::string> adapters = GetActiveNetworkAdapters();
  if (adapters.empty()) {
    return false;
  }
  
  // Create PowerShell command to show UAC dialog and set DNS for all adapters
  std::string command = "powershell -Command \"";
  command += "Start-Process cmd -ArgumentList '/c ";
  
  // Add DNS commands for each adapter
  for (const auto& adapter : adapters) {
    command += "netsh interface ip set dns \\\"" + adapter + "\\\" static " + dns + " primary && ";
  }
  
  // Remove the last " && " and close the command
  if (command.length() > 4) {
    command = command.substr(0, command.length() - 4);
  }
  command += "' -Verb RunAs -WindowStyle Hidden\"";
  
  int result = system(command.c_str());
  
  // Check if the command was successful
  return result == 0;
}

bool DnsManagerPlugin::RequestElevationForReset() {
  // Get active network adapters
  std::vector<std::string> adapters = GetActiveNetworkAdapters();
  if (adapters.empty()) {
    return false;
  }
  
  // Create PowerShell command to show UAC dialog and reset DNS for all adapters
  std::string command = "powershell -Command \"";
  command += "Start-Process cmd -ArgumentList '/c ";
  
  // Add DNS reset commands for each adapter
  for (const auto& adapter : adapters) {
    command += "netsh interface ip set dns \\\"" + adapter + "\\\" dhcp && ";
  }
  
  // Remove the last " && " and close the command
  if (command.length() > 4) {
    command = command.substr(0, command.length() - 4);
  }
  command += "' -Verb RunAs -WindowStyle Hidden\"";
  
  int result = system(command.c_str());
  
  // Check if the command was successful
  return result == 0;
}

}  // namespace dns_manager