#include <flutter_linux/flutter_linux.h>

#include "include/dns_manager/dns_manager_plugin.h"

// This file exposes some plugin internals for unit testing. See
// https://github.com/flutter/flutter/issues/88724 for current limitations
// in the unit-testable API.

// DNS management functions
FlMethodResponse* get_dns();
FlMethodResponse* set_dns(FlValue* arguments);
FlMethodResponse* reset_dns();
FlMethodResponse* get_connection_status();
