/**
 * ============================================================
 * wifi_ctrl.h — ESP32 WiFi 控制模块
 * ============================================================
 *
 * 【功能概述】
 *   本文件封装了 ESP32 的 WiFi 控制功能，支持三种工作模式：
 *   - AP 模式（热点）：ESP32 自己发出 WiFi 信号，供其他设备连接
 *   - STA 模式（客户端）：ESP32 连接到已有的路由器 WiFi
 *   - AP+STA 模式（混合）：同时开启 AP 和 STA
 *
 * 【依赖库】
 *   #include <LittleFS.h>      — 文件系统，用于在 Flash 中存储 WiFi 配置
 *   #include <WIFI.h>          — ESP32 WiFi 库
 *   #include <ArduinoJson.h>   — JSON 解析库，用于解析配置文件
 *
 * 【配置文件】
 *   WiFi 配置保存在 Flash 文件系统的 /wifiConfig.json 中，
 *   可通过以下链接了解如何上传配置文件到 ESP32 Flash：
 *   https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
 *
 * 【前置条件】
 *   - 需要先初始化 Serial（串口）
 *   - 需要定义全局变量 bool InfoPrint（控制调试信息的串口打印开关）
 *   - 使用前调用 initWifi() 即可完成全部初始化
 * ============================================================
 */

// ==================== WiFi 启动模式配置 ====================
// WIFI_MODE_ON_BOOT 控制开机时的 WiFi 行为：
//   0: OFF    — 关闭 WiFi（需要通过串口命令或重新上传配置文件来重新开启）
//   1: AP     — 仅 AP 热点模式（出厂默认模式）
//   2: STA    — 仅 STA 客户端模式
//   3: AP+STA — 混合模式（首次 STA 连接成功后自动切换到的默认模式）
byte WIFI_MODE_ON_BOOT = 1;

// STA 模式（连接路由器）的 SSID 和密码，初始为 "none"
const char* sta_ssid = "none";
const char* sta_password = "none";

// AP 模式（自身热点）的 SSID 和密码，默认热点名为 "UGV"，密码 "12345678"
const char* ap_ssid = "ugv374041";
const char* ap_password = "12345678";

// 当首次 STA 连接成功后，是否自动将 WIFI_MODE_ON_BOOT 改为 3（AP+STA）
bool defaultModeToAPSTA = false;

// ==================== 配置文件格式说明 ====================
// 配置文件 /wifiConfig.json 的 JSON 格式示例：
// {
//   "wifi_mode_on_boot": 3,
//   "sta_ssid": "你的WiFi名称",
//   "sta_password": "你的WiFi密码",
//   "ap_ssid": "UGV热点名称",
//   "ap_password": "UGV热点密码"
// }
File wifiConfigYaml;

// ==================== 其他全局变量 ====================
unsigned long connectionStartTime;        // WiFi 连接开始时间（毫秒），用于超时判断
unsigned long connectionTimeout = 15000;  // WiFi 连接超时时间，默认 15 秒
byte WIFI_CURRENT_MODE = -1;              // 当前 WiFi 工作模式（-1 表示未初始化）
IPAddress localIP;                        // 当前设备的 IP 地址
DynamicJsonDocument wifiDoc(256);         // JSON 文档对象，用于解析和构建 JSON 数据
bool wifiConfigFound = false;             // 是否成功从 Flash 加载到配置文件


/**
 * 根据当前 WiFi 状态更新 OLED 显示屏上的 WiFi 信息
 *   模式 0 (OFF):    显示 "AP: OFF" / "ST: OFF"
 *   模式 1 (AP):     显示 AP 的 SSID / "ST: OFF"
 *   模式 2 (STA):    显示 "AP: OFF" / STA 的 IP 地址
 *   模式 3 (AP+STA): 显示 AP 的 SSID / STA 的 IP 地址
 */
void updateOledWifiInfo() {
  switch(WIFI_CURRENT_MODE) {
  case 0:
    screenLine_0 = "AP: OFF";
    screenLine_1 = "ST: OFF";
    break;
  case 1:
    screenLine_0 = String("AP:") + ap_ssid;
    screenLine_1 = "ST: OFF";
    break;
  case 2:
    screenLine_0 = "AP: OFF";
    screenLine_1 = String("ST:") + localIP.toString();
    break;
  case 3:
    screenLine_0 = String("AP:") + ap_ssid;
    screenLine_1 = String("ST:") + localIP.toString();
    break;
  }
  oled_update();
}


/**
 * 从 Flash 文件系统加载 WiFi 配置文件 /wifiConfig.json
 * 读取 JSON 配置并解析出 WiFi 模式、SSID、密码等参数
 * @return true  — 加载成功
 * @return false — 未找到配置文件
 */
// load the wifiConfig.json from Flash.
// the file name is wifiConfig.json in root path.
bool loadWifiConfig() {
	wifiConfigYaml = LittleFS.open("/wifiConfig.json", "r");
	if (wifiConfigYaml) {
		if (InfoPrint == 1) {Serial.println("/wifiConfig.json load succeed.");}
		
		String line = wifiConfigYaml.readStringUntil('\n');

		// parse the YAML file using ArduinoJson.
		deserializeJson(wifiDoc, line);

		// read configuration values.
		WIFI_MODE_ON_BOOT = wifiDoc["wifi_mode_on_boot"];
		sta_ssid = wifiDoc["sta_ssid"];
		sta_password = wifiDoc["sta_password"];
		ap_ssid = wifiDoc["ap_ssid"];
		ap_password = wifiDoc["ap_password"];

		if (InfoPrint == 1) {
			Serial.println(line);
		}

		wifiConfigYaml.close();
		wifiConfigFound = true;
		jsonInfoHttp.clear();
  	jsonInfoHttp["ip"] = "/wifiConfig.json load succeed.";
 		jsonInfoHttp["wifi_mode_on_boot"] = WIFI_MODE_ON_BOOT;
 		jsonInfoHttp["sta_ssid"] = sta_ssid;
 		jsonInfoHttp["sta_password"] = sta_password;
 		jsonInfoHttp["ap_ssid"] = ap_ssid;
 		jsonInfoHttp["ap_password"] = ap_password;
		return true;

	} else {
		if (InfoPrint == 1) {Serial.println("cound not found wifiConfig.json.");}
		wifiConfigFound = false;
		return false;
	}
}


/**
 * 获取当前设备的 IP 地址并存入 jsonInfoHttp
 * @param inputMode WiFi 模式（未在函数内直接使用）
 * @return 当前本地 IP 地址
 */
IPAddress getIPAddress(byte inputMode) {
	localIP = WiFi.localIP();
	if (InfoPrint == 1) {
		Serial.print("IP: ");
		Serial.println(localIP.toString());
	}

	jsonInfoHttp.clear();
  jsonInfoHttp["ip"] = localIP.toString();
	return localIP;
}


/**
 * 根据当前已有的配置参数创建 /wifiConfig.json 文件
 * 将 WIFI_MODE_ON_BOOT、SSID、密码等写入 Flash 文件系统
 * 注意：WIFI_MODE_ON_BOOT 为 0 或 -1 时不执行创建
 * @return true  — 创建成功
 * @return false — 创建失败或当前模式不支持
 */
bool createWifiConfigFileByStatus() {
	if (WIFI_MODE_ON_BOOT != 0 || WIFI_MODE_ON_BOOT != -1){
		wifiDoc.clear();
		wifiDoc["wifi_mode_on_boot"] = WIFI_MODE_ON_BOOT;
		wifiDoc["sta_ssid"] = sta_ssid;
		wifiDoc["sta_password"] = sta_password;
		wifiDoc["ap_ssid"] = ap_ssid;
		wifiDoc["ap_password"] = ap_password;

		File configFile = LittleFS.open("/wifiConfig.json", "w");
		if (configFile) {
			serializeJson(wifiDoc, configFile);
			configFile.close();
			if (InfoPrint == 1) {
				Serial.println("/wifiConfig.json created.");
			}
			jsonInfoHttp.clear();
  		jsonInfoHttp["info"] = "/wifiConfig.json created.";
			jsonInfoHttp["wifi_mode_on_boot"] = WIFI_MODE_ON_BOOT;
			jsonInfoHttp["sta_ssid"] = sta_ssid;
			jsonInfoHttp["sta_password"] = sta_password;
			jsonInfoHttp["ap_ssid"] = ap_ssid;
			jsonInfoHttp["ap_password"] = ap_password;
			return true;
		} else {
			jsonInfoHttp.clear();
  		jsonInfoHttp["info"] = "/wifiConfig.json open failed.";
			return false;
		}
	} else {
		jsonInfoHttp.clear();
  	jsonInfoHttp["info"] = "not for this wifi_mode_on_boot.";
		return false;
	}
}


/**
 * 设置 WiFi 为 AP（热点）模式
 * ESP32 自身发出 WiFi 信号，默认网关地址 192.168.4.1
 * 内部使用 WIFI_AP_STA 接口以保留后续切换 STA 的能力
 * 调用后更新 OLED 显示
 * @param input_ssid     热点名称
 * @param input_password 热点密码
 * @return 始终返回 true
 */
bool wifiModeAP(const char* input_ssid, const char* input_password) {
	WiFi.disconnect();
	if (InfoPrint == 1) {Serial.println("wifi mode on boot: AP");}
	// WiFi.mode(WIFI_AP);
	WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(input_ssid, input_password);
	if (InfoPrint == 1) {
		Serial.println("AP mode starts...");
		Serial.print("SSID: ");
		Serial.println(input_ssid);
		Serial.print("Password: ");
		Serial.println(input_password);
		Serial.println("AP Address: 192.168.4.1");
	}
	WIFI_CURRENT_MODE = 1;
	localIP = WiFi.localIP();
	ap_ssid = input_ssid;
	ap_password = input_password;

	updateOledWifiInfo();

	jsonInfoHttp.clear();
  jsonInfoHttp["info"] = "AP mode starts";
  jsonInfoHttp["ap_ssid"] = ap_ssid;
  jsonInfoHttp["ap_password"] = ap_password;

	return true;
}


/**
 * 设置 WiFi 为 STA（客户端）模式
 * ESP32 连接到指定的路由器 WiFi，带超时机制（默认 15 秒）
 * 连接超时后会自动回退到 AP 模式
 * 首次 STA 连接成功时，如果 defaultModeToAPSTA 为 true，
 *   自动将开机模式切换为 AP+STA 并保存配置
 * @param input_ssid     要连接的路由器 SSID
 * @param input_password 要连接的路由器密码
 * @return true  — STA 连接成功
 * @return false — 连接超时
 */
bool wifiModeSTA(const char* input_ssid, const char* input_password) {
	WiFi.disconnect();
	if (InfoPrint == 1) {Serial.println("wifi mode on boot: STA");}
	// WiFi.mode(WIFI_STA);
	WiFi.mode(WIFI_AP_STA);
	WiFi.begin(input_ssid, input_password);
	connectionStartTime = millis();

	if (InfoPrint == 1) {Serial.println("STA mode starts: connecting to ");
					Serial.println(input_ssid);}
	while (WiFi.status() != WL_CONNECTED) {
		unsigned long currentTime = millis();
		if (InfoPrint == 1) {Serial.print(".");}
		delay(500);

		if (currentTime - connectionStartTime >= connectionTimeout) {
			WIFI_CURRENT_MODE = -1;
			if (InfoPrint == 1) {Serial.println(".");Serial.println("STA connection timeout.");}
			wifiModeAP(ap_ssid, ap_password);
			updateOledWifiInfo();

			jsonInfoHttp.clear();
			jsonInfoHttp["info"] = "STA connection timeout.";

			return false;
			break;
		}
	}

	if (InfoPrint == 1) {Serial.println(".");Serial.println("STA connection succeed.");}
	WIFI_CURRENT_MODE = 2;
	getIPAddress(WIFI_CURRENT_MODE);
	sta_ssid = input_ssid;
	sta_password = input_password;

	jsonInfoHttp.clear();
	jsonInfoHttp["info"] = "STA connection succeed.";
	jsonInfoHttp["wifi_mode_on_boot"] = WIFI_MODE_ON_BOOT;
	jsonInfoHttp["sta_ssid"] = sta_ssid;
	jsonInfoHttp["sta_password"] = sta_password;
	jsonInfoHttp["ap_ssid"] = ap_ssid;
	jsonInfoHttp["ap_password"] = ap_password;

	if (defaultModeToAPSTA && !wifiConfigFound) {
		WIFI_MODE_ON_BOOT = 3;
		if (InfoPrint == 1) {Serial.println("[default] wifi mode on boot: AP+STA");}
		jsonInfoHttp["info"] = "[default] wifi mode on boot: AP+STA";
		createWifiConfigFileByStatus();
	}
	updateOledWifiInfo();

	return true;
}


/**
 * 设置 WiFi 为 AP+STA 混合模式
 * 同时开启 AP 热点和 STA 客户端连接
 * 先启动 AP，再尝试连接 STA，STA 连接超时则仅保留 AP 模式
 * 首次连接成功时，如果 defaultModeToAPSTA 为 true 且无配置文件，
 *   自动保存 AP+STA 模式为默认启动模式
 * @param input_ap_ssid      AP 热点名称
 * @param input_ap_password  AP 热点密码
 * @param input_sta_ssid     要连接的 WiFi 名称
 * @param input_sta_password 要连接的 WiFi 密码
 * @return true  — AP+STA 模式启动成功
 * @return false — STA 连接超时（AP 仍正常运行）
 */
bool wifiModeAPSTA(const char* input_ap_ssid, const char* input_ap_password, const char* input_sta_ssid, const char* input_sta_password) {
	WiFi.disconnect();
	if (InfoPrint == 1) {Serial.println("wifi mode on boot: AP+STA");}
	WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(input_ap_ssid, input_ap_password);
	if (InfoPrint == 1) {
		Serial.println("AP/AP+STA mode starts...");
		Serial.print("AP SSID: ");
		Serial.println(input_ap_ssid);
		Serial.print("AP Password: ");
		Serial.println(input_ap_password);
		Serial.println("AP Address: 192.168.4.1");
	}
	ap_ssid = input_ap_ssid;
	ap_password = input_ap_password;
	
	WiFi.begin(input_sta_ssid, input_sta_password);
	connectionStartTime = millis();

	if (InfoPrint == 1) {Serial.print("STA/AP+STA mode starts: connecting to ");
					Serial.println(input_sta_ssid);}
	while (WiFi.status() != WL_CONNECTED) {
		unsigned long currentTime = millis();
		if (InfoPrint == 1) {Serial.print(".");}
		delay(500);

		if (currentTime - connectionStartTime >= connectionTimeout) {
			WIFI_CURRENT_MODE = -1;
			if (InfoPrint == 1) {Serial.println(".");Serial.println("STA connection timeout.");}
			wifiModeAP(ap_ssid, ap_password);
			updateOledWifiInfo();

			jsonInfoHttp.clear();
			jsonInfoHttp["info"] = "STA connection timeout.";

			return false;
			break;
		}
	}

	if (InfoPrint == 1) {Serial.println("STA connection succeed.");}
	WIFI_CURRENT_MODE = 3;
	getIPAddress(WIFI_CURRENT_MODE);
	sta_ssid = input_sta_ssid;
	sta_password = input_sta_password;
	if (defaultModeToAPSTA && !wifiConfigFound) {
		WIFI_MODE_ON_BOOT = 3;
		if (InfoPrint == 1) {Serial.println("[default] wifi mode on boot: AP+STA");}
		createWifiConfigFileByStatus();
	}
	updateOledWifiInfo();

	jsonInfoHttp.clear();
	jsonInfoHttp["info"] = "STA connection succeed.";
	jsonInfoHttp["wifi_mode_on_boot"] = WIFI_MODE_ON_BOOT;
	jsonInfoHttp["sta_ssid"] = sta_ssid;
	jsonInfoHttp["sta_password"] = sta_password;
	jsonInfoHttp["ap_ssid"] = ap_ssid;
	jsonInfoHttp["ap_password"] = ap_password;
	
	return true;
}


/**
 * 断开所有 WiFi 连接
 * WiFi 硬件保持 WIFI_AP_STA 模式但不连接任何网络
 * WIFI_CURRENT_MODE 设为 0（OFF），并更新 OLED 显示
 */
void wifiStop() {
	WiFi.disconnect();
	WIFI_CURRENT_MODE = 0;
	WiFi.mode(WIFI_AP_STA);
	updateOledWifiInfo();
}


/**
 * 根据 WIFI_MODE_ON_BOOT 的值执行对应的开机 WiFi 模式
 *   0 → 关闭 WiFi
 *   1 → AP 模式
 *   2 → STA 模式
 *   3 → AP+STA 混合模式
 * 通常在 initWifi() 中调用
 * @return 对应模式的启动结果
 */
bool wifiModeOnBoot() {
	bool funcStatus = false;
	switch(WIFI_MODE_ON_BOOT) {
	case 0: 
		if (InfoPrint == 1) {
			Serial.println("wifi mode on boot: OFF");
		}
		funcStatus = true;
		WIFI_CURRENT_MODE = 0;
		WiFi.mode(WIFI_AP_STA);
		break;
	case 1:
		funcStatus = wifiModeAP(ap_ssid, ap_password);
		break;
	case 2:
		funcStatus = wifiModeSTA(sta_ssid, sta_password);
		break;
	case 3:
		funcStatus = wifiModeAPSTA(ap_ssid, ap_password, sta_ssid, sta_password);
		break;
	}
	return funcStatus;
}


/**
 * 修改开机 WiFi 模式并保存到配置文件
 * 先更新 WIFI_MODE_ON_BOOT，再调用 createWifiConfigFileByStatus() 持久化
 * @param inputMode 新的开机模式（0=OFF, 1=AP, 2=STA, 3=AP+STA）
 */
void configWifiModeOnBoot(byte inputMode) {
	WIFI_MODE_ON_BOOT = inputMode;
	if (InfoPrint == 1) {
		Serial.print("wifi_mode_on_boot: ");
		Serial.println(WIFI_MODE_ON_BOOT);
	}
	createWifiConfigFileByStatus();
}


/**
 * 根据用户输入的参数创建 WiFi 配置文件
 * 与 createWifiConfigFileByStatus() 的区别：
 *   本函数先用输入参数启动 AP+STA 模式，成功后再保存配置
 *   而 createWifiConfigFileByStatus() 直接使用当前全局变量保存
 * @param inputMode         开机模式
 * @param inputApSsid       AP 热点名称
 * @param inputApPassword   AP 热点密码
 * @param inputStaSsid      STA 客户端 SSID
 * @param inputStaPassword  STA 客户端密码
 */
void createWifiConfigFileByInput(byte inputMode, const char* inputApSsid, const char* inputApPassword, const char* inputStaSsid, const char* inputStaPassword) {
	WIFI_MODE_ON_BOOT = inputMode;
	wifiModeAPSTA(inputApSsid, inputApPassword, inputStaSsid, inputStaPassword);
	if (InfoPrint == 1) {
		Serial.print("wifi_mode_on_boot: ");
		Serial.println(WIFI_MODE_ON_BOOT);
	}
	createWifiConfigFileByStatus();
}


/**
 * WiFi 状态信息反馈
 * 将当前 WiFi 状态（IP、RSSI、模式、SSID、MAC 等）打包成 JSON
 * 同时通过串口输出和存入 jsonInfoHttp 供 HTTP 响应使用
 */
void wifiStatusFeedback() {
	wifiDoc["ip"] = localIP.toString();
	wifiDoc["rssi"] = WiFi.RSSI();
	serializeJson(wifiDoc, Serial);

	jsonInfoHttp.clear();
	jsonInfoHttp["ip"] = wifiDoc["ip"];
	jsonInfoHttp["rssi"] = wifiDoc["rssi"];
	jsonInfoHttp["wifi_mode_on_boot"] = WIFI_MODE_ON_BOOT;
	jsonInfoHttp["sta_ssid"] = sta_ssid;
	jsonInfoHttp["sta_password"] = sta_password;
	jsonInfoHttp["ap_ssid"] = ap_ssid;
	jsonInfoHttp["ap_password"] = ap_password;
	jsonInfoHttp["mac"] = thisMacStr;
}


/**
 * WiFi 初始化入口函数
 * 1. 从 Flash 加载配置文件 /wifiConfig.json
 * 2. 根据配置执行开机 WiFi 模式
 * 外部只需调用此函数即可完成 WiFi 的全部初始化
 */
void initWifi() {
	loadWifiConfig();
	wifiModeOnBoot();
}