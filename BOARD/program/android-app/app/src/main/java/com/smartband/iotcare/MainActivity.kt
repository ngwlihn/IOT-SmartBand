package com.smartband.iotcare

import android.os.Bundle
import android.Manifest
import android.annotation.SuppressLint
import android.bluetooth.*
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.pm.PackageManager
import android.os.Build
import android.os.Handler
import android.os.Looper
import android.content.Context
import android.content.Intent
import android.media.AudioAttributes
import android.media.AudioManager
import android.media.MediaPlayer
import android.media.RingtoneManager
import android.view.KeyEvent
import android.webkit.JavascriptInterface
import android.webkit.WebView
import android.webkit.WebViewClient
import android.webkit.WebResourceRequest
import android.webkit.WebResourceResponse
import android.webkit.CookieManager
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import java.util.UUID

class MainActivity : ComponentActivity() {
    private val serviceUuid = UUID.fromString("7d8f0001-8d5b-4d2f-9b2a-8b0a0f8a1001")
    private val wifiUuid = UUID.fromString("7d8f0002-8d5b-4d2f-9b2a-8b0a0f8a1001")
    private var webView: WebView? = null
    private var scanCallback: ScanCallback? = null
    private var bluetoothGatt: BluetoothGatt? = null
    private var alertPlayer: MediaPlayer? = null
    private var isEmergencyRinging = false
    private var previousAlarmVolume = -1
    private var previousMusicVolume = -1
    private val volumeHandler = Handler(Looper.getMainLooper())
    private val volumeEnforcer = object : Runnable {
        override fun run() {
            if (!isEmergencyRinging) return
            enforceMaxVolumes()
            volumeHandler.postDelayed(this, 300)
        }
    }

    private fun enforceMaxVolumes() {
        try {
            val audioManager = getSystemService(Context.AUDIO_SERVICE) as AudioManager
            val maxAlarm = audioManager.getStreamMaxVolume(AudioManager.STREAM_ALARM)
            val maxMusic = audioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC)
            if (audioManager.getStreamVolume(AudioManager.STREAM_ALARM) < maxAlarm) {
                audioManager.setStreamVolume(AudioManager.STREAM_ALARM, maxAlarm, 0)
            }
            if (audioManager.getStreamVolume(AudioManager.STREAM_MUSIC) < maxMusic) {
                audioManager.setStreamVolume(AudioManager.STREAM_MUSIC, maxMusic, 0)
            }
        } catch (_: Exception) {}
    }

    private fun startEmergencyAlarmAudio() {
        isEmergencyRinging = true
        try {
            val audioManager = getSystemService(Context.AUDIO_SERVICE) as AudioManager
            if (previousAlarmVolume == -1) {
                previousAlarmVolume = audioManager.getStreamVolume(AudioManager.STREAM_ALARM)
            }
            if (previousMusicVolume == -1) {
                previousMusicVolume = audioManager.getStreamVolume(AudioManager.STREAM_MUSIC)
            }
            enforceMaxVolumes()
        } catch (_: Exception) {}

        volumeHandler.removeCallbacks(volumeEnforcer)
        volumeHandler.post(volumeEnforcer)

        if (alertPlayer == null) {
            val uri = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_ALARM)
            if (uri != null) {
                runCatching {
                    alertPlayer = MediaPlayer().apply {
                        setAudioAttributes(AudioAttributes.Builder().setUsage(AudioAttributes.USAGE_ALARM)
                            .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION).build())
                        setDataSource(this@MainActivity, uri)
                        isLooping = true
                        prepare()
                        start()
                    }
                }
            }
        }
    }

    private fun stopEmergencyAlarmAudio() {
        isEmergencyRinging = false
        volumeHandler.removeCallbacks(volumeEnforcer)
        try {
            val audioManager = getSystemService(Context.AUDIO_SERVICE) as AudioManager
            if (previousAlarmVolume >= 0) {
                audioManager.setStreamVolume(AudioManager.STREAM_ALARM, previousAlarmVolume, 0)
                previousAlarmVolume = -1
            }
            if (previousMusicVolume >= 0) {
                audioManager.setStreamVolume(AudioManager.STREAM_MUSIC, previousMusicVolume, 0)
                previousMusicVolume = -1
            }
        } catch (_: Exception) {}

        alertPlayer?.run {
            stop()
            release()
        }
        alertPlayer = null
    }

    override fun onKeyDown(keyCode: Int, event: KeyEvent?): Boolean {
        if (isEmergencyRinging && (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN || keyCode == KeyEvent.KEYCODE_VOLUME_MUTE)) {
            enforceMaxVolumes()
            return true
        }
        return super.onKeyDown(keyCode, event)
    }
    private val settings by lazy { getSharedPreferences("smartband_state", MODE_PRIVATE) }
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O_MR1) {
            setShowWhenLocked(true)
            setTurnScreenOn(true)
        } else {
            @Suppress("DEPRECATION")
            window.addFlags(
                android.view.WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED or
                android.view.WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON
            )
        }
        window.addFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        androidx.core.content.ContextCompat.startForegroundService(this, Intent(this, EmergencyAlertService::class.java))
        if (Build.VERSION.SDK_INT >= 33 && ContextCompat.checkSelfPermission(this, Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.POST_NOTIFICATIONS), 701)
        }
        val webView = WebView(this)
        this.webView = webView
        webView.settings.javaScriptEnabled = true
        webView.settings.domStorageEnabled = true
        webView.settings.allowFileAccess = true
        // The bundled HTML calls FormSubmit directly with fetch(). Android WebView
        // otherwise blocks file:// pages from reaching the HTTPS endpoint.
        @Suppress("DEPRECATION")
        webView.settings.allowUniversalAccessFromFileURLs = true
        @Suppress("DEPRECATION")
        webView.settings.allowFileAccessFromFileURLs = true
        webView.settings.mixedContentMode = android.webkit.WebSettings.MIXED_CONTENT_COMPATIBILITY_MODE
        webView.settings.loadsImagesAutomatically = true
        webView.settings.userAgentString = webView.settings.userAgentString + " SmartbandCareAndroid/1.0"
        CookieManager.getInstance().setAcceptCookie(true)
        CookieManager.getInstance().setAcceptThirdPartyCookies(webView, true)
        webView.webViewClient = object : WebViewClient() {
            override fun shouldInterceptRequest(view: WebView, request: WebResourceRequest): WebResourceResponse? {
                if (request.url.host == "127.0.0.1" && request.url.path == "/app-preview.html") {
                    return WebResourceResponse("text/html", "UTF-8", assets.open("app-preview.html"))
                }
                return super.shouldInterceptRequest(view, request)
            }
        }
        webView.addJavascriptInterface(BleProvisionBridge(), "NativeBridge")
        webView.loadUrl("http://127.0.0.1:5500/app-preview.html")
        setContentView(webView)
    }

    inner class BleProvisionBridge {
        @JavascriptInterface
        fun saveState(value: String) {
            settings.edit().putString("state", value).commit()
        }

        @JavascriptInterface
        fun loadState(): String = settings.getString("state", "{}") ?: "{}"
        @JavascriptInterface
        fun playEmergencyAlarm() {
            runOnUiThread {
                startEmergencyAlarmAudio()
            }
        }

        @JavascriptInterface
        fun stopEmergencyAlarm() {
            startService(Intent(this@MainActivity, EmergencyAlertService::class.java).setAction(EmergencyAlertService.ACTION_STOP_ALARM))
            runOnUiThread {
                stopEmergencyAlarmAudio()
            }
        }

        @JavascriptInterface
        fun setBackendUrl(url: String) {
            startService(Intent(this@MainActivity, EmergencyAlertService::class.java)
                .setAction(EmergencyAlertService.ACTION_UPDATE_BACKEND)
                .putExtra(EmergencyAlertService.EXTRA_BACKEND_URL, url))
        }

        @JavascriptInterface
        fun provisionWifi(ssid: String, password: String, backendUrl: String) {
            runOnUiThread {
                if (!hasBlePermission()) {
                    ActivityCompat.requestPermissions(this@MainActivity, blePermissions(), 700)
                    webView?.evaluateJavascript("window.onBleProvisionResult(false,'Cho phép Bluetooth rồi bấm lại')", null)
                    return@runOnUiThread
                }
                val payload = org.json.JSONObject().put("ssid", ssid).put("password", password)
                    .put("backend_url", backendUrl).toString()
                startBleScan(payload)
            }
        }

        @JavascriptInterface
        fun provisionScreen(line1: String, line2: String, line3: String) {
            runOnUiThread {
                if (!hasBlePermission()) {
                    ActivityCompat.requestPermissions(this@MainActivity, blePermissions(), 700)
                    return@runOnUiThread
                }
                val screen = org.json.JSONObject().put("line1", line1).put("line2", line2).put("line3", line3)
                startBleScan(org.json.JSONObject().put("screen", screen).toString())
            }
        }

        @JavascriptInterface
        fun provisionMedical(blood: String, condition: String, phone: String, qrUrl: String) {
            provisionMedicalProfile(blood, condition, phone, qrUrl, "")
        }

        @JavascriptInterface
        fun provisionMedicalProfile(blood: String, condition: String, phone: String, qrUrl: String, name: String) {
            runOnUiThread {
                if (!hasBlePermission()) {
                    ActivityCompat.requestPermissions(this@MainActivity, blePermissions(), 700)
                    return@runOnUiThread
                }
                val medical = org.json.JSONObject()
                    .put("blood", blood)
                    .put("condition", condition)
                    .put("phone", phone)
                    .put("url", qrUrl)
                if (name.isNotEmpty()) {
                    medical.put("name", name)
                }
                startBleScan(org.json.JSONObject().put("medical", medical).toString())
            }
        }
    }

    private fun blePermissions(): Array<String> = if (Build.VERSION.SDK_INT >= 31) arrayOf(
        Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT,
        Manifest.permission.ACCESS_FINE_LOCATION
    ) else arrayOf(Manifest.permission.ACCESS_FINE_LOCATION)

    private fun hasBlePermission() = blePermissions().all {
        ContextCompat.checkSelfPermission(this, it) == PackageManager.PERMISSION_GRANTED
    }

    @SuppressLint("MissingPermission")
    private fun startBleScan(payload: String) {
        bluetoothGatt?.close()
        bluetoothGatt = null
        val adapter = (getSystemService(BLUETOOTH_SERVICE) as BluetoothManager).adapter
        if (adapter == null || !adapter.isEnabled) {
            webView?.evaluateJavascript("window.onBleProvisionResult(false,'Hãy bật Bluetooth')", null)
            return
        }
        val scanner = adapter.bluetoothLeScanner
        val settings = ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build()
        val callback = object : ScanCallback() {
            override fun onScanResult(type: Int, result: ScanResult) {
                val advertisedName = result.scanRecord?.deviceName ?: result.device.name
                val advertisesService = result.scanRecord?.serviceUuids?.contains(android.os.ParcelUuid(serviceUuid)) == true
                if (advertisedName == "Smartband-001" || advertisesService) {
                    scanner.stopScan(this)
                    connectAndWrite(result.device, payload)
                }
            }
            override fun onScanFailed(errorCode: Int) {
                webView?.evaluateJavascript("window.onBleProvisionResult(false,'Không quét được kit BLE')", null)
            }
        }
        scanCallback = callback
        // Do not rely solely on a service-data filter: some ESP32 Android
        // advertisements expose the service UUID only after the scan starts.
        scanner.startScan(null, settings, callback)
        Handler(Looper.getMainLooper()).postDelayed({
            runCatching { scanner.stopScan(callback) }
            if (bluetoothGatt == null) webView?.evaluateJavascript("window.onBleProvisionResult(false,'Không tìm thấy Smartband-001')", null)
        }, 8000)
    }

    @SuppressLint("MissingPermission")
    private fun connectAndWrite(device: BluetoothDevice, payload: String) {
        bluetoothGatt = device.connectGatt(this, false, object : BluetoothGattCallback() {
            override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
                if (newState == BluetoothProfile.STATE_CONNECTED) gatt.requestMtu(247)
                else if (newState == BluetoothProfile.STATE_DISCONNECTED) webView?.evaluateJavascript("window.onBleProvisionResult(false,'Kit đã ngắt kết nối')", null)
            }
            override fun onMtuChanged(gatt: BluetoothGatt, mtu: Int, status: Int) {
                gatt.discoverServices()
            }
            override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
                val characteristic = gatt.getService(serviceUuid)?.getCharacteristic(wifiUuid)
                if (characteristic == null) {
                    webView?.evaluateJavascript("window.onBleProvisionResult(false,'Kit thiếu dịch vụ cấu hình Wi‑Fi')", null)
                    return
                }
                characteristic.value = payload.toByteArray(Charsets.UTF_8)
                if (!gatt.writeCharacteristic(characteristic)) {
                    runOnUiThread { webView?.evaluateJavascript("window.onBleProvisionResult(false,'Không ghi được cấu hình BLE')", null) }
                }
            }
            override fun onCharacteristicWrite(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic, status: Int) {
                runOnUiThread {
                    val result = if (status == BluetoothGatt.GATT_SUCCESS)
                        "window.onBleProvisionResult(true,'Đã gửi cấu hình; chờ kit khởi động lại')"
                    else "window.onBleProvisionResult(false,'Ghi cấu hình BLE thất bại')"
                    webView?.evaluateJavascript(result, null)
                }
            }
        })
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        setIntent(intent)
        if (intent.getBooleanExtra("is_emergency", false)) {
            val kind = intent.getStringExtra("alert_type") ?: "FALL"
            webView?.evaluateJavascript("window.onNativeEmergencyAlert && window.onNativeEmergencyAlert('$kind')", null)
        }
    }

    override fun onDestroy() {
        alertPlayer?.release()
        alertPlayer = null
        bluetoothGatt?.close()
        bluetoothGatt = null
        super.onDestroy()
    }
}
