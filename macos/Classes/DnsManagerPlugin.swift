import Cocoa
import FlutterMacOS

public class DnsManagerPlugin: NSObject, FlutterPlugin {
  public static func register(with registrar: FlutterPluginRegistrar) {
    let channel = FlutterMethodChannel(name: "dns_manager", binaryMessenger: registrar.messenger)
    let instance = DnsManagerPlugin()
    registrar.addMethodCallDelegate(instance, channel: channel)
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
    case "getDNS":
      result("macOS " + ProcessInfo.processInfo.operatingSystemVersionString)
    case "setDNS":
      result("macOS " + ProcessInfo.processInfo.operatingSystemVersionString)
    case "resetDNS":
      result("macOS " + ProcessInfo.processInfo.operatingSystemVersionString)
    default:
      result(FlutterMethodNotImplemented)
    }
  }
}
