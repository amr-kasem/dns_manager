#include "include/dns_manager/dns_manager_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "dns_manager_plugin.h"

void DnsManagerPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  dns_manager::DnsManagerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
