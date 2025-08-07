#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <sstream>

// Simple test functions to verify DNS operations
std::string ExecuteCommand(const std::string& cmd) {
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

std::vector<std::string> ParseDNSServers(const std::string& output) {
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

std::vector<std::string> GetActiveNetworkAdapters() {
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
  
  return adapters;
}

int main() {
  std::cout << "=== DNS Manager Test ===" << std::endl;
  
  // Test 1: Get current DNS
  std::cout << "\n1. Testing Get Current DNS:" << std::endl;
  std::string output = ExecuteCommand("netsh interface ip show dns");
  std::vector<std::string> servers = ParseDNSServers(output);
  if (servers.empty()) {
    std::cout << "No DNS servers found in general output, trying specific adapters..." << std::endl;
    std::vector<std::string> adapters = GetActiveNetworkAdapters();
    if (!adapters.empty()) {
      std::cout << "Found adapters: ";
      for (const auto& adapter : adapters) {
        std::cout << adapter << " ";
      }
      std::cout << std::endl;
      
      for (const auto& adapter : adapters) {
        std::string adapterOutput = ExecuteCommand("netsh interface ip show dns \"" + adapter + "\"");
        std::vector<std::string> adapterServers = ParseDNSServers(adapterOutput);
        if (!adapterServers.empty()) {
          std::cout << "DNS servers for " << adapter << ": ";
          for (const auto& server : adapterServers) {
            std::cout << server << " ";
          }
          std::cout << std::endl;
        }
      }
    }
  } else {
    std::cout << "Found DNS servers: ";
    for (const auto& server : servers) {
      std::cout << server << " ";
    }
    std::cout << std::endl;
  }
  
  // Test 2: Get network adapters
  std::cout << "\n2. Testing Get Network Adapters:" << std::endl;
  std::vector<std::string> adapters = GetActiveNetworkAdapters();
  std::cout << "Active network adapters: ";
  for (const auto& adapter : adapters) {
    std::cout << adapter << " ";
  }
  std::cout << std::endl;
  
  // Test 3: Test connection status
  std::cout << "\n3. Testing Connection Status:" << std::endl;
  std::string pingOutput = ExecuteCommand("ping 8.8.8.8 -n 1");
  if (pingOutput.find("TTL=") != std::string::npos) {
    std::cout << "Connection: Connected" << std::endl;
  } else {
    std::cout << "Connection: Disconnected" << std::endl;
  }
  
  std::cout << "\n=== Test Complete ===" << std::endl;
  return 0;
} 