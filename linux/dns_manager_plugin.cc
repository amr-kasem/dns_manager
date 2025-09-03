#include "include/dns_manager/dns_manager_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <sys/utsname.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cstring>

#include "dns_manager_plugin_private.h"

#define DNS_MANAGER_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), dns_manager_plugin_get_type(), \
                              DnsManagerPlugin))

struct _DnsManagerPlugin {
  GObject parent_instance;
};

G_DEFINE_TYPE(DnsManagerPlugin, dns_manager_plugin, g_object_get_type())

// Called when a method call is received from Flutter.
static void dns_manager_plugin_handle_method_call(
    DnsManagerPlugin* self,
    FlMethodCall* method_call) {
  g_autoptr(FlMethodResponse) response = nullptr;

  const gchar* method = fl_method_call_get_name(method_call);
  FlValue* arguments = fl_method_call_get_args(method_call);

  if (strcmp(method, "getDNS") == 0) {
    response = get_dns();
  } else if (strcmp(method, "setDNS") == 0) {
    response = set_dns(arguments);
  } else if (strcmp(method, "resetDNS") == 0) {
    response = reset_dns();
  } else if (strcmp(method, "getConnectionStatus") == 0) {
    response = get_connection_status();
  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  fl_method_call_respond(method_call, response, nullptr);
}

// Helper function to execute shell commands
static gchar* execute_command(const gchar* command) {
  FILE* pipe = popen(command, "r");
  if (!pipe) {
    return g_strdup("Error: Could not execute command");
  }
  
  gchar* result = g_strdup("");
  gchar buffer[256];
  
  while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
    gchar* temp = result;
    result = g_strdup_printf("%s%s", result, buffer);
    g_free(temp);
  }
  
  pclose(pipe);
  return result;
}

// Helper function to get the active connection
static gchar* get_active_connection() {
  // Try to get ethernet connection first
  gchar* result = execute_command("nmcli -t -f UUID,TYPE,DEVICE connection show --active | grep ethernet | head -1 | cut -d: -f1");
  if (result && strlen(result) > 0) {
    // Remove trailing newline and whitespace
    g_strchomp(result);
    if (strlen(result) > 0) {
      return result;
    }
  }
  g_free(result);
  
  // If no ethernet, try wifi
  result = execute_command("nmcli -t -f UUID,TYPE,DEVICE connection show --active | grep \"802-11-wireless\" | head -1 | cut -d: -f1");
  if (result && strlen(result) > 0) {
    // Remove trailing newline and whitespace
    g_strchomp(result);
    if (strlen(result) > 0) {
      return result;
    }
  }
  g_free(result);
  
  return g_strdup("");
}

FlMethodResponse* set_dns(FlValue* arguments) {
  if (fl_value_get_type(arguments) != FL_VALUE_TYPE_MAP) {
    g_autoptr(FlValue) result = fl_value_new_string("Error: Invalid arguments");
    return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  
  FlValue* dns_value = fl_value_lookup_string(arguments, "dns");
  if (!dns_value || fl_value_get_type(dns_value) != FL_VALUE_TYPE_STRING) {
    g_autoptr(FlValue) result = fl_value_new_string("Error: DNS parameter required");
    return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  
  const gchar* dns = fl_value_get_string(dns_value);
  g_autofree gchar* connection = get_active_connection();
  
  if (strlen(connection) == 0) {
    g_autoptr(FlValue) result = fl_value_new_string("Error: No active connection found");
    return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  
  // Clear existing DNS first, then set new DNS servers
  g_autofree gchar* command1 = g_strdup_printf("nmcli connection modify '%s' ipv4.ignore-auto-dns yes", connection);
  g_autofree gchar* output1 = execute_command(command1);
  
  g_autofree gchar* command2 = g_strdup_printf("nmcli connection modify '%s' ipv4.dns '%s'", connection, dns);
  g_autofree gchar* output2 = execute_command(command2);
  
  if (strstr(output1, "Error") != NULL || strstr(output2, "Error") != NULL) {
    g_autoptr(FlValue) result = fl_value_new_string("Error setting DNS");
    return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  
  // Restart the connection to apply changes (run in background)
  g_autofree gchar* restart_cmd = g_strdup_printf("nmcli connection down '%s' && nmcli connection up '%s' &", connection, connection);
  system(restart_cmd);
  
  g_autoptr(FlValue) result = fl_value_new_string("DNS set successfully - Network reconnecting...");
  return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
}

FlMethodResponse* reset_dns() {
  g_autofree gchar* connection = get_active_connection();
  
  if (strlen(connection) == 0) {
    g_autoptr(FlValue) result = fl_value_new_string("Error: No active connection found");
    return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  
  // Reset DNS to automatic by clearing DNS servers
  g_autofree gchar* command = g_strdup_printf("nmcli connection modify '%s' ipv4.ignore-auto-dns no && nmcli connection modify '%s' ipv4.dns ''", connection, connection);
  g_autofree gchar* output = execute_command(command);
  
  if (strstr(output, "Error") != NULL) {
    g_autoptr(FlValue) result = fl_value_new_string("Error resetting DNS");
    return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  
  // Restart the connection to apply changes (run in background)
  g_autofree gchar* restart_cmd = g_strdup_printf("nmcli connection down '%s' && nmcli connection up '%s' &", connection, connection);
  system(restart_cmd);
  
  g_autoptr(FlValue) result = fl_value_new_string("DNS reset successfully - Network reconnecting...");
  return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
}

FlMethodResponse* get_connection_status() {
  g_autofree gchar* connection = get_active_connection();
  
  if (strlen(connection) == 0) {
    g_autoptr(FlValue) result = fl_value_new_string("No active connection");
    return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  
  // Check connection status using GENERAL field
  g_autofree gchar* command = g_strdup_printf("nmcli -t -f GENERAL connection show '%s'", connection);
  g_autofree gchar* output = execute_command(command);
  
  if (strstr(output, "Error") != NULL) {
    g_autoptr(FlValue) result = fl_value_new_string("Error checking connection status");
    return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  
  // Remove trailing newline and whitespace
  g_strchomp(output);
  
  g_autoptr(FlValue) result = fl_value_new_string(output);
  return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
}

FlMethodResponse* get_dns() {
  g_autofree gchar* connection = get_active_connection();
  
  if (strlen(connection) == 0) {
    g_autoptr(FlValue) result = fl_value_new_string("Error: No active connection found");
    return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  
  // Get current DNS settings more efficiently
  g_autofree gchar* command = g_strdup_printf("nmcli -g ipv4.dns connection show '%s'", connection);
  g_autofree gchar* output = execute_command(command);
  
  if (strstr(output, "Error") != NULL) {
    g_autoptr(FlValue) result = fl_value_new_string(output);
    return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  
  // Remove trailing newline and whitespace
  g_strchomp(output);
  
  if (strlen(output) == 0) {
    g_autoptr(FlValue) result = fl_value_new_string("Automatic DNS (DHCP)");
    return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  } else {
    g_autoptr(FlValue) result = fl_value_new_string(output);
    return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
}

static void dns_manager_plugin_dispose(GObject* object) {
  G_OBJECT_CLASS(dns_manager_plugin_parent_class)->dispose(object);
}

static void dns_manager_plugin_class_init(DnsManagerPluginClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = dns_manager_plugin_dispose;
}

static void dns_manager_plugin_init(DnsManagerPlugin* self) {}

static void method_call_cb(FlMethodChannel* channel, FlMethodCall* method_call,
                           gpointer user_data) {
  DnsManagerPlugin* plugin = DNS_MANAGER_PLUGIN(user_data);
  dns_manager_plugin_handle_method_call(plugin, method_call);
}

void dns_manager_plugin_register_with_registrar(FlPluginRegistrar* registrar) {
  DnsManagerPlugin* plugin = DNS_MANAGER_PLUGIN(
      g_object_new(dns_manager_plugin_get_type(), nullptr));

  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  g_autoptr(FlMethodChannel) channel =
      fl_method_channel_new(fl_plugin_registrar_get_messenger(registrar),
                            "dns_manager",
                            FL_METHOD_CODEC(codec));
  fl_method_channel_set_method_call_handler(channel, method_call_cb,
                                            g_object_ref(plugin),
                                            g_object_unref);

  g_object_unref(plugin);
}
