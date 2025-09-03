
// DummyVPNService.kt
package com.example.dns_manager

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.Intent
import android.net.VpnService
import android.os.Build
import android.os.ParcelFileDescriptor
import android.util.Log
import androidx.core.app.NotificationCompat
import java.net.InetAddress

class DummyVPNService : VpnService() {
    private var vpnInterface: ParcelFileDescriptor? = null
    private var isRunning = false
    
    companion object {
        private const val TAG = "DummyVPNService"
        private const val NOTIFICATION_ID = 1001
        private const val CHANNEL_ID = "dns_vpn_channel"
        private var currentDnsServers = mutableListOf<String>()
        
        fun getCurrentDNS(): List<String> = currentDnsServers.toList()
    }

    override fun onCreate() {
        super.onCreate()
        createNotificationChannel()
        Log.d(TAG, "Service created")
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        Log.d(TAG, "onStartCommand called with action: ${intent?.action}")
        
        when (intent?.action) {
            "START_VPN" -> {
                val dnsServers = intent.getStringArrayListExtra("dns_servers")
                if (dnsServers != null && dnsServers.isNotEmpty()) {
                    startVPN(dnsServers)
                } else {
                    Log.e(TAG, "No DNS servers provided")
                    stopSelf()
                }
            }
            "STOP_VPN" -> {
                stopVPN()
            }
            else -> {
                Log.e(TAG, "Unknown action: ${intent?.action}")
                stopSelf()
            }
        }
        
        return START_NOT_STICKY // Don't restart if killed
    }

    private fun startVPN(dnsServers: List<String>) {
        try {
            Log.d(TAG, "Starting VPN with DNS servers: ${dnsServers.joinToString(",")}")
            
            // Stop existing VPN if running
            if (isRunning) {
                stopVPN()
            }
            
            val builder = Builder().apply {
                // Set a dummy IP range that doesn't conflict with local network
                addAddress("10.0.0.2", 32)
                addRoute("0.0.0.0", 0) // Route all traffic through VPN
                
                // Add DNS servers
                dnsServers.forEach { dns ->
                    try {
                        addDnsServer(InetAddress.getByName(dns))
                        Log.d(TAG, "Added DNS server: $dns")
                    } catch (e: Exception) {
                        Log.e(TAG, "Failed to add DNS server $dns: ${e.message}")
                    }
                }
                
                setSession("DNS VPN")
                setBlocking(false)
                
                // Allow bypass for this app to prevent loops
                try {
                    addDisallowedApplication(packageName)
                } catch (e: Exception) {
                    Log.w(TAG, "Could not disallow own package: ${e.message}")
                }
            }

            vpnInterface = builder.establish()
            
            if (vpnInterface != null) {
                currentDnsServers.clear()
                currentDnsServers.addAll(dnsServers)
                
                startForeground(NOTIFICATION_ID, createNotification())
                isRunning = true
                
                Log.d(TAG, "VPN started successfully with DNS servers: ${dnsServers.joinToString(",")}")
            } else {
                Log.e(TAG, "Failed to establish VPN interface")
                stopSelf()
            }
            
        } catch (e: Exception) {
            Log.e(TAG, "Error starting VPN: ${e.message}")
            stopSelf()
        }
    }

    private fun stopVPN() {
        try {
            Log.d(TAG, "Stopping VPN")
            isRunning = false
            
            vpnInterface?.close()
            vpnInterface = null
            
            currentDnsServers.clear()
            
            stopForeground(true)
            stopSelf()
            
            Log.d(TAG, "VPN stopped successfully")
        } catch (e: Exception) {
            Log.e(TAG, "Error stopping VPN: ${e.message}")
        }
    }

    override fun onDestroy() {
        Log.d(TAG, "Service destroyed")
        stopVPN()
        super.onDestroy()
    }

    override fun onRevoke() {
        Log.d(TAG, "VPN permission revoked")
        stopVPN()
        super.onRevoke()
    }

    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val channel = NotificationChannel(
                CHANNEL_ID,
                "DNS VPN Service",
                NotificationManager.IMPORTANCE_LOW
            ).apply {
                description = "Notification for DNS VPN service"
                setShowBadge(false)
            }
            
            val notificationManager = getSystemService(NotificationManager::class.java)
            notificationManager.createNotificationChannel(channel)
        }
    }

    private fun createNotification(): Notification {
        val intent = packageManager.getLaunchIntentForPackage(packageName)
        val pendingIntent = PendingIntent.getActivity(
            this, 0, intent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        val dnsText = if (currentDnsServers.isNotEmpty()) {
            "DNS: ${currentDnsServers.joinToString(",")}"
        } else {
            "DNS VPN Active"
        }

        return NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("DNS VPN Active")
            .setContentText(dnsText)
            .setSmallIcon(android.R.drawable.ic_menu_info_details)
            .setContentIntent(pendingIntent)
            .setOngoing(true)
            .setPriority(NotificationCompat.PRIORITY_LOW)
            .build()
    }
}