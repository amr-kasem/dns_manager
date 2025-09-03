// DnsManagerPlugin.kt
package com.example.dns_manager

import android.content.Context
import android.content.Intent
import android.net.VpnService
import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.embedding.engine.plugins.activity.ActivityAware
import io.flutter.embedding.engine.plugins.activity.ActivityPluginBinding
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result
import io.flutter.plugin.common.PluginRegistry

/** DnsManagerPlugin */
class DnsManagerPlugin: FlutterPlugin, MethodCallHandler, ActivityAware, PluginRegistry.ActivityResultListener {
  private lateinit var channel: MethodChannel
  private var context: Context? = null
  private var activity: android.app.Activity? = null
  private var pendingResult: Result? = null
  private var pendingDnsServers: List<String>? = null
  
  companion object {
    private const val VPN_REQUEST_CODE = 1001
  }

  override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
    channel = MethodChannel(flutterPluginBinding.binaryMessenger, "dns_manager")
    channel.setMethodCallHandler(this)
    context = flutterPluginBinding.applicationContext
  }

  override fun onMethodCall(call: MethodCall, result: Result) {
    when (call.method) {
      "getDNS" -> {
        val currentDns = DummyVPNService.getCurrentDNS()
        if (currentDns.isNotEmpty()) {
          result.success(currentDns.joinToString(","))
        } else {
          result.success("No custom DNS set")
        }
      }
      "setDNS" -> {
        val dnsServers = call.argument<String>("dns")
        if (dnsServers.isNullOrEmpty()) {
          result.error("INVALID_DNS", "DNS servers cannot be empty", null)
          return
        }
        
        val dnsList = dnsServers.split(",").map { it.trim() }
        if (dnsList.any { !isValidIP(it) }) {
          result.error("INVALID_DNS", "Invalid DNS server format", null)
          return
        }
        
        startVpnService(dnsList, result)
      }
      "resetDNS" -> {
        stopVpnService()
        result.success("DNS reset successfully")
      }
      else -> {
        result.notImplemented()
      }
    }
  }
  
  private fun isValidIP(ip: String): Boolean {
    val parts = ip.split(".")
    if (parts.size != 4) return false
    return parts.all { 
      try {
        val num = it.toInt()
        num in 0..255
      } catch (e: NumberFormatException) {
        false
      }
    }
  }
  
  private fun startVpnService(dnsServers: List<String>, result: Result) {
    val intent = VpnService.prepare(context)
    if (intent != null) {
      // Need VPN permission
      pendingResult = result
      pendingDnsServers = dnsServers
      activity?.startActivityForResult(intent, VPN_REQUEST_CODE)
    } else {
      // Permission already granted, start service immediately
      startVpnServiceWithPermission(dnsServers, result)
    }
  }
  
  private fun startVpnServiceWithPermission(dnsServers: List<String>, result: Result) {
    try {
      val serviceIntent = Intent(context, DummyVPNService::class.java).apply {
        action = "START_VPN"
        putStringArrayListExtra("dns_servers", ArrayList(dnsServers))
      }
      context?.startForegroundService(serviceIntent)
      result.success("DNS set successfully: ${dnsServers.joinToString(",")}")
    } catch (e: Exception) {
      result.error("VPN_START_ERROR", "Failed to start VPN service: ${e.message}", null)
    }
  }
  
  private fun stopVpnService() {
    try {
      val serviceIntent = Intent(context, DummyVPNService::class.java).apply {
        action = "STOP_VPN"
      }
      context?.startService(serviceIntent)
    } catch (e: Exception) {
      // If service isn't running, stopping it will fail - that's okay
    }
  }

  override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?): Boolean {
    if (requestCode == VPN_REQUEST_CODE) {
      val result = pendingResult
      val dnsServers = pendingDnsServers
      pendingResult = null
      pendingDnsServers = null
      
      if (resultCode == android.app.Activity.RESULT_OK && dnsServers != null && result != null) {
        // Permission granted, start VPN service with DNS servers
        startVpnServiceWithPermission(dnsServers, result)
      } else {
        result?.error("VPN_PERMISSION_DENIED", "VPN permission denied by user", null)
      }
      return true
    }
    return false
  }

  override fun onAttachedToActivity(binding: ActivityPluginBinding) {
    activity = binding.activity
    binding.addActivityResultListener(this)
  }

  override fun onDetachedFromActivityForConfigChanges() {
    activity = null
  }

  override fun onReattachedToActivityForConfigChanges(binding: ActivityPluginBinding) {
    activity = binding.activity
    binding.addActivityResultListener(this)
  }

  override fun onDetachedFromActivity() {
    activity = null
  }

  override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
    channel.setMethodCallHandler(null)
  }
}
