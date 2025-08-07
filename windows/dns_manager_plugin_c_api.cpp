#include "include/dns_manager/dns_manager_plugin_c_api.h"
#include "include/dns_manager/dns_manager_plugin.h"

#include <flutter/plugin_registrar_windows.h>

extern "C" FLUTTER_PLUGIN_EXPORT void DnsManagerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  dns_manager::DnsManagerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
