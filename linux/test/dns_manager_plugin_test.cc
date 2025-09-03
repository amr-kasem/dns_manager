#include <flutter_linux/flutter_linux.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "include/dns_manager/dns_manager_plugin.h"
#include "dns_manager_plugin_private.h"

// This demonstrates a simple unit test of the C portion of this plugin's
// implementation.
//
// Once you have built the plugin's example app, you can run these tests
// from the command line. For instance, for a plugin called my_plugin
// built for x64 debug, run:
// $ build/linux/x64/debug/plugins/my_plugin/my_plugin_test

namespace dns_manager {
namespace test {

TEST(DnsManagerPlugin, GetDNS) {
  g_autoptr(FlMethodResponse) response = get_dns();
  ASSERT_NE(response, nullptr);
  ASSERT_TRUE(FL_IS_METHOD_SUCCESS_RESPONSE(response));
  FlValue* result = fl_method_success_response_get_result(
      FL_METHOD_SUCCESS_RESPONSE(response));
  ASSERT_EQ(fl_value_get_type(result), FL_VALUE_TYPE_STRING);
  // The result should be a string (either DNS servers or error message)
  EXPECT_FALSE(fl_value_get_string(result) == nullptr);
}

TEST(DnsManagerPlugin, SetDNS) {
  // Create a test arguments map
  g_autoptr(FlValue) args = fl_value_new_map();
  fl_value_set_string_take(args, "dns", fl_value_new_string("8.8.8.8"));
  
  g_autoptr(FlMethodResponse) response = set_dns(args);
  ASSERT_NE(response, nullptr);
  ASSERT_TRUE(FL_IS_METHOD_SUCCESS_RESPONSE(response));
  FlValue* result = fl_method_success_response_get_result(
      FL_METHOD_SUCCESS_RESPONSE(response));
  ASSERT_EQ(fl_value_get_type(result), FL_VALUE_TYPE_STRING);
  // The result should be a string (success or error message)
  EXPECT_FALSE(fl_value_get_string(result) == nullptr);
}

TEST(DnsManagerPlugin, ResetDNS) {
  g_autoptr(FlMethodResponse) response = reset_dns();
  ASSERT_NE(response, nullptr);
  ASSERT_TRUE(FL_IS_METHOD_SUCCESS_RESPONSE(response));
  FlValue* result = fl_method_success_response_get_result(
      FL_METHOD_SUCCESS_RESPONSE(response));
  ASSERT_EQ(fl_value_get_type(result), FL_VALUE_TYPE_STRING);
  // The result should be a string (success or error message)
  EXPECT_FALSE(fl_value_get_string(result) == nullptr);
}

}  // namespace test
}  // namespace dns_manager
