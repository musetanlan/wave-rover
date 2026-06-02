void jsonCmdReceiveHandler(){
	int cmdType = jsonCmdReceive["T"].as<int>();
	switch(cmdType){
	case CMD_SPEED_CTRL:	if (jsonCmdReceive.containsKey("T") &&
														jsonCmdReceive.containsKey("L") &&
														jsonCmdReceive.containsKey("R")){
													if (jsonCmdReceive["L"].is<float>() &&
															jsonCmdReceive["R"].is<float>()){
														// 导航激活时，忽略普通速度指令，避免干扰导航PID
														if (nav_target_active && !nav_goal_reached) {
															if (InfoPrint == 1) {
																Serial.println("[WARN] 导航进行中，速度指令被忽略。请先发送 T=702 停止导航。");
															}
															break;
														}
														heartbeatStopFlag = false;
														lastCmdRecvTime = millis();
														setGoalSpeed(
														jsonCmdReceive["L"],
														jsonCmdReceive["R"]);
													}
												} break;
	case CMD_PWM_INPUT:		// 导航激活时，忽略PWM指令
												if (nav_target_active && !nav_goal_reached) {
													if (InfoPrint == 1) {
														Serial.println("[WARN] 导航进行中，PWM指令被忽略。请先发送 T=702 停止导航。");
													}
													break;
												}
												usePIDCompute = false;
												heartbeatStopFlag = false;
												lastCmdRecvTime = millis();
												leftCtrl(jsonCmdReceive["L"]);
												rightCtrl(jsonCmdReceive["R"]);
												break;
	case CMD_ROS_CTRL:		// 导航激活时，忽略ROS指令
												if (nav_target_active && !nav_goal_reached) {
													if (InfoPrint == 1) {
														Serial.println("[WARN] 导航进行中，ROS指令被忽略。请先发送 T=702 停止导航。");
													}
													break;
												}
												rosCtrl(
												jsonCmdReceive["X"],
												jsonCmdReceive["Z"]);
												heartbeatStopFlag = false;
												lastCmdRecvTime = millis();break;
	case CMD_SET_MOTOR_PID:
												setPID(
												jsonCmdReceive["P"],
												jsonCmdReceive["I"],
												jsonCmdReceive["D"],
												jsonCmdReceive["L"]);break;
	case CMD_OLED_CTRL:		oledCtrl(
												jsonCmdReceive["lineNum"],
												jsonCmdReceive["Text"]);break;
	case CMD_OLED_DEFAULT:setOledDefault();break;
	case CMD_MODULE_TYPE:	changeModuleType(
												jsonCmdReceive["cmd"]);break;



	case CMD_GET_IMU_DATA:
												getIMUData();break;
	case CMD_CALI_IMU_STEP:
												imuCalibration();break;
	case CMD_GET_IMU_OFFSET:
												getIMUOffset();
												break;
	case CMD_SET_IMU_OFFSET:
												setIMUOffset(
												jsonCmdReceive["x"],
												jsonCmdReceive["y"],
												jsonCmdReceive["z"]);break;
	case CMD_BASE_FEEDBACK:
												baseInfoFeedback();break;
	case CMD_BASE_FEEDBACK_FLOW:
												setBaseInfoFeedbackMode(
												jsonCmdReceive["cmd"]);break;
	case CMD_FEEDBACK_FLOW_INTERVAL:
												setFeedbackFlowInterval(
												jsonCmdReceive["cmd"]);break;
	case CMD_UART_ECHO_MODE:
												setCmdEcho(
												jsonCmdReceive["cmd"]);break;
	case CMD_ARM_CTRL_UI: RoArmM2_uiCtrl(
												jsonCmdReceive["E"],
												jsonCmdReceive["Z"],
												jsonCmdReceive["R"]
												);break;



	case CMD_LED_CTRL:		led_pwm_ctrl(
												jsonCmdReceive["IO4"],
												jsonCmdReceive["IO5"]);break;
	case CMD_GIMBAL_CTRL_SIMPLE:
												gimbalCtrlSimple(
												jsonCmdReceive["X"],
												jsonCmdReceive["Y"],
												jsonCmdReceive["SPD"],
												jsonCmdReceive["ACC"]);break;
	case CMD_GIMBAL_CTRL_MOVE:
												gimbalCtrlMove(
												jsonCmdReceive["X"],
												jsonCmdReceive["Y"],
												jsonCmdReceive["SX"],
												jsonCmdReceive["SY"]);break;
	case CMD_GIMBAL_CTRL_STOP:
												gimbalCtrlStop();break;
	case CMD_HEART_BEAT_SET:
												changeHeartBeatDelay(
												jsonCmdReceive["cmd"]);break;
	case CMD_GIMBAL_STEADY:
												gimbalSteadySet(
												jsonCmdReceive["s"],
												jsonCmdReceive["y"]);break;
	case CMD_SET_SPD_RATE:
												setSpdRate(
												jsonCmdReceive["L"],
												jsonCmdReceive["R"]);break;
	case CMD_GET_SPD_RATE:
												getSpdRate();break;
	case CMD_SAVE_SPD_RATE:
												saveSpdRate();break;
	case CMD_GIMBAL_USER_CTRL:
												gimbalUserCtrl(
												jsonCmdReceive["X"],
												jsonCmdReceive["Y"],
												jsonCmdReceive["SPD"]);break;




	// EoAT type settings.
	case CMD_EOAT_TYPE:		configEEmodeType(
												jsonCmdReceive["mode"]);break;
	case CMD_CONFIG_EOAT: configEoAT(
												jsonCmdReceive["pos"],
												jsonCmdReceive["ea"],
												jsonCmdReceive["eb"]
												);break;



	// it moves to goal position directly
	// with interpolation.
	case CMD_MOVE_INIT:		RoArmM2_moveInit();break;
	case CMD_SINGLE_JOINT_CTRL:
												RoArmM2_singleJointAbsCtrl(
												jsonCmdReceive["joint"],
												jsonCmdReceive["rad"],
												jsonCmdReceive["spd"],
												jsonCmdReceive["acc"]
												);break;
	case CMD_JOINTS_RAD_CTRL:
												RoArmM2_allJointAbsCtrl(
												jsonCmdReceive["base"],
												jsonCmdReceive["shoulder"],
												jsonCmdReceive["elbow"],
												jsonCmdReceive["hand"],
												jsonCmdReceive["spd"],
												jsonCmdReceive["acc"]
												);break;
	case CMD_SINGLE_AXIS_CTRL:
												RoArmM2_singlePosAbsBesselCtrl(
												jsonCmdReceive["axis"],
												jsonCmdReceive["pos"],
												jsonCmdReceive["spd"]
												);break;
	case CMD_XYZT_GOAL_CTRL:
												RoArmM2_allPosAbsBesselCtrl(
												jsonCmdReceive["x"],
											  jsonCmdReceive["y"],
											  jsonCmdReceive["z"],
											  jsonCmdReceive["t"],
											  jsonCmdReceive["spd"]
											  );break;
	case CMD_XYZT_DIRECT_CTRL:
												RoArmM2_baseCoordinateCtrl(
												jsonCmdReceive["x"],
												jsonCmdReceive["y"],
												jsonCmdReceive["z"],
												jsonCmdReceive["t"]);
												RoArmM2_goalPosMove();
												break;
	case CMD_SERVO_RAD_FEEDBACK:
												RoArmM2_getPosByServoFeedback();
												RoArmM2_infoFeedback();
												break;

	case CMD_EOAT_HAND_CTRL:
												RoArmM2_handJointCtrlRad(1,
												jsonCmdReceive["cmd"],
												jsonCmdReceive["spd"],
												jsonCmdReceive["acc"]
												);break;
	case CMD_EOAT_GRAB_TORQUE:
												RoArmM2_handTorqueCtrl(
												jsonCmdReceive["tor"]
												);break;

	case CMD_SET_JOINT_PID:
												RoArmM2_setJointPID(
												jsonCmdReceive["joint"],
												jsonCmdReceive["p"],
												jsonCmdReceive["i"]
												);break;
	case CMD_RESET_PID:		RoArmM2_resetPID();break;

	// set a new x-axis.
	case CMD_SET_NEW_X: 	setNewAxisX(
												jsonCmdReceive["xAxisAngle"]
												);break;
	case CMD_DELAY_MILLIS:
												RoArmM2_delayMillis(
												jsonCmdReceive["cmd"]
												);break;
	case CMD_DYNAMIC_ADAPTATION:
												RoArmM2_dynamicAdaptation(
												jsonCmdReceive["mode"],
												jsonCmdReceive["b"],
												jsonCmdReceive["s"],
												jsonCmdReceive["e"],
												jsonCmdReceive["h"]
												);break;
	// this two funcs are NOT for UGV.
	// case CMD_SWITCH_CTRL: switchCtrl(
	// 											jsonCmdReceive["pwm_a"],
	// 											jsonCmdReceive["pwm_b"]
	// 											);break;
	// case CMD_LIGHT_CTRL:	lightCtrl(
	// 											jsonCmdReceive["led"]
	// 											);break;
	case CMD_SWITCH_OFF:  switchEmergencyStop();break;
	case CMD_SINGLE_JOINT_ANGLE:
												RoArmM2_singleJointAngleCtrl(
												jsonCmdReceive["joint"],
												jsonCmdReceive["angle"],
												jsonCmdReceive["spd"],
												jsonCmdReceive["acc"]
												);break;
	case CMD_JOINTS_ANGLE_CTRL:
												RoArmM2_allJointsAngleCtrl(
												jsonCmdReceive["b"],
												jsonCmdReceive["s"],
												jsonCmdReceive["e"],
												jsonCmdReceive["h"],
												jsonCmdReceive["spd"],
												jsonCmdReceive["acc"]
												);break;
// constant ctrl
// m: 0 - angle
//    1 - xyzt
// cmd: 0 - stop
// 		  1 - increase
// 		  2 - decrease
// {"T":123,"m":0,"axis":0,"cmd":0,"spd":0}
	case CMD_CONSTANT_CTRL:
												constantCtrl(
												jsonCmdReceive["m"],
												jsonCmdReceive["axis"],
												jsonCmdReceive["cmd"],
												jsonCmdReceive["spd"]
												);break;




	// mission & steps edit & file edit.
	case CMD_SCAN_FILES:  scanFlashContents();
												break;
	case CMD_CREATE_FILE: createFile(
												jsonCmdReceive["name"],
												jsonCmdReceive["content"]
												);break;
	case CMD_READ_FILE:		readFile(
												jsonCmdReceive["name"]
												);break;
	case CMD_DELETE_FILE: deleteFile(
												jsonCmdReceive["name"]
												);break;
	case CMD_APPEND_LINE:	appendLine(
												jsonCmdReceive["name"],
												jsonCmdReceive["content"]
												);break;
	case CMD_INSERT_LINE: insertLine(
												jsonCmdReceive["name"],
												jsonCmdReceive["lineNum"],
												jsonCmdReceive["content"]
												);break;
	case CMD_REPLACE_LINE:
												replaceLine(
												jsonCmdReceive["name"],
												jsonCmdReceive["lineNum"],
												jsonCmdReceive["content"]
												);break;
	case CMD_READ_LINE:   readSingleLine(
												jsonCmdReceive["name"],
												jsonCmdReceive["lineNum"]
												);break;
	case CMD_DELETE_LINE: deleteSingleLine(
												jsonCmdReceive["name"],
												jsonCmdReceive["lineNum"]
												);break;


	case CMD_TORQUE_CTRL: servoTorqueCtrl(254,
												jsonCmdReceive["cmd"]);
												break;


	case CMD_CREATE_MISSION:
												createMission(
												jsonCmdReceive["name"],
												jsonCmdReceive["intro"]
												);break;
	case CMD_MISSION_CONTENT:
												missionContent(
												jsonCmdReceive["name"]
												);break;
	case CMD_APPEND_STEP_JSON:
												appendStepJson(
												jsonCmdReceive["name"],
												jsonCmdReceive["step"]
												);break;
	case CMD_APPEND_STEP_FB:
												appendStepFB(
												jsonCmdReceive["name"],
												jsonCmdReceive["spd"]
												);break;
	case CMD_APPEND_DELAY:
												appendDelayCmd(
												jsonCmdReceive["name"],
												jsonCmdReceive["delay"]
												);break;
	case CMD_INSERT_STEP_JSON:
												insertStepJson(
												jsonCmdReceive["name"],
												jsonCmdReceive["stepNum"],
												jsonCmdReceive["step"]
												);break;
	case CMD_INSERT_STEP_FB:
												insertStepFB(
												jsonCmdReceive["name"],
												jsonCmdReceive["stepNum"],
												jsonCmdReceive["spd"]
												);break;
	case CMD_INSERT_DELAY:
												insertDelayCmd(
												jsonCmdReceive["name"],
												jsonCmdReceive["stepNum"],
												jsonCmdReceive["spd"]
												);break;
	case CMD_REPLACE_STEP_JSON:
												replaceStepJson(
												jsonCmdReceive["name"],
												jsonCmdReceive["stepNum"],
												jsonCmdReceive["step"]
												);break;
	case CMD_REPLACE_STEP_FB:
												replaceStepFB(
												jsonCmdReceive["name"],
												jsonCmdReceive["stepNum"],
												jsonCmdReceive["spd"]
												);break;
	case CMD_REPLACE_DELAY:
												replaceDelayCmd(
												jsonCmdReceive["name"],
												jsonCmdReceive["stepNum"],
												jsonCmdReceive["delay"]
												);break;
	case CMD_DELETE_STEP: deleteStep(
												jsonCmdReceive["name"],
												jsonCmdReceive["stepNum"]
												);break;

	case CMD_MOVE_TO_STEP:
												moveToStep(
												jsonCmdReceive["name"],
												jsonCmdReceive["stepNum"]
												);break;
	case CMD_MISSION_PLAY:
												missionPlay(
												jsonCmdReceive["name"],
												jsonCmdReceive["times"]
												);break;



	// esp-now settings.
  case CMD_BROADCAST_FOLLOWER:
  											changeBroadcastMode(
  											jsonCmdReceive["mode"],
  											jsonCmdReceive["mac"]
  											);break;
  case CMD_ESP_NOW_CONFIG:
  											changeEspNowMode(
  											jsonCmdReceive["mode"]
  											);break;
  case CMD_GET_MAC_ADDRESS:
  											getThisDevMacAddress();
  											break;
  case CMD_ESP_NOW_ADD_FOLLOWER:
  											registerNewFollowerToPeer(
  											jsonCmdReceive["mac"]);break;
  case CMD_ESP_NOW_REMOVE_FOLLOWER:
  											deleteFollower(
  											jsonCmdReceive["mac"]);break;
  case CMD_ESP_NOW_GROUP_CTRL:
  											espNowGroupSend(
  											jsonCmdReceive["dev"],
  											jsonCmdReceive["b"],
  											jsonCmdReceive["s"],
  											jsonCmdReceive["e"],
  											jsonCmdReceive["h"],
  											jsonCmdReceive["cmd"],
  											jsonCmdReceive["megs"]
  											);break;
  case CMD_ESP_NOW_SINGLE:
  											espNowSingleDevSend(
  											jsonCmdReceive["mac"],
  											jsonCmdReceive["dev"],
  											jsonCmdReceive["b"],
  											jsonCmdReceive["s"],
  											jsonCmdReceive["e"],
  											jsonCmdReceive["h"],
  											jsonCmdReceive["cmd"],
  											jsonCmdReceive["megs"]
  											);break;



	// wifi settings.
	case CMD_WIFI_ON_BOOT:
												configWifiModeOnBoot(
												jsonCmdReceive["cmd"]
												);break;
	case CMD_SET_AP: 			wifiModeAP(
									 			jsonCmdReceive["ssid"],
									 			jsonCmdReceive["password"]
									 			);break;
	case CMD_SET_STA: 		wifiModeSTA(
												jsonCmdReceive["ssid"],
												jsonCmdReceive["password"]
												);break;
	case CMD_WIFI_APSTA: 	wifiModeAPSTA(
										 	 	jsonCmdReceive["ap_ssid"],
											 	jsonCmdReceive["ap_password"],
											 	jsonCmdReceive["sta_ssid"],
											 	jsonCmdReceive["sta_password"]
											 	);break;
	case CMD_WIFI_INFO: 	wifiStatusFeedback();break;
	case CMD_WIFI_CONFIG_CREATE_BY_STATUS:
												createWifiConfigFileByStatus();break;
	case CMD_WIFI_CONFIG_CREATE_BY_INPUT:
												createWifiConfigFileByInput(
												jsonCmdReceive["mode"],
												jsonCmdReceive["ap_ssid"],
												jsonCmdReceive["ap_password"],
												jsonCmdReceive["sta_ssid"],
												jsonCmdReceive["sta_password"]
												);break;
	case CMD_WIFI_STOP: 	wifiStop();break;



	// servo settings.
	case CMD_SET_SERVO_ID:
												changeID(
												jsonCmdReceive["raw"],
												jsonCmdReceive["new"]
												);break;
	case CMD_SET_MIDDLE:  setMiddlePos(
												jsonCmdReceive["id"]
												);break;
	case CMD_SET_SERVO_PID:
												setServosPID(
												jsonCmdReceive["id"],
												jsonCmdReceive["p"]
												);break;

	// esp-32 dev ctrl.
	case CMD_REBOOT: 			esp_restart();break;
	case CMD_FREE_FLASH_SPACE:
												freeFlashSpace();break;
	case CMD_BOOT_MISSION_INFO:
												missionContent("boot");break;
	case CMD_RESET_BOOT_MISSION:
												deleteFile("boot.mission");
												createFile("boot", "these cmds run automatically at boot.");
												break;
	case CMD_NVS_CLEAR:		nvs_flash_erase();
												delay(1000);
												nvs_flash_init();
												break;
	case CMD_INFO_PRINT:	configInfoPrint(
												jsonCmdReceive["cmd"]
												);break;
	// case CMD_PID_RESET_A: PID_v2 pidA(__kp, __ki, __kd, PID::Direct);
	// 											PID_v2 pidB(__kp, __ki, __kd, PID::Direct);
	// 											pidControllerInit();break;

	// mainType & moduleType settings.
	case CMD_MM_TYPE_SET: mm_settings(
												jsonCmdReceive["main"],
												jsonCmdReceive["module"]
												);
												break;

	// 导航PID控制命令处理
	case CMD_NAV_SET_TARGET:             // 设置目标坐标，启动导航
												nav_set_target(
												jsonCmdReceive["x"],
												jsonCmdReceive["y"]);
												heartbeatStopFlag = false;
												lastCmdRecvTime = millis();
												break;
	case CMD_NAV_UPDATE_POS:             // 更新当前位姿（会用外部坐标覆盖里程计）
												nav_update_position(
												jsonCmdReceive["x"],
												jsonCmdReceive["y"],
												jsonCmdReceive["h"]);
												break;
	case CMD_NAV_STOP:                   // 停止导航
												nav_stop();
												break;
	case CMD_NAV_SET_PID:                // 在线调整PID参数
												if (jsonCmdReceive.containsKey("kp_d"))
													nav_kp_dist = jsonCmdReceive["kp_d"];
												if (jsonCmdReceive.containsKey("ki_d"))
													nav_ki_dist = jsonCmdReceive["ki_d"];
												if (jsonCmdReceive.containsKey("kd_d"))
													nav_kd_dist = jsonCmdReceive["kd_d"];
												if (jsonCmdReceive.containsKey("kp_h"))
													nav_kp_head = jsonCmdReceive["kp_h"];
												if (jsonCmdReceive.containsKey("ki_h"))
													nav_ki_head = jsonCmdReceive["ki_h"];
												if (jsonCmdReceive.containsKey("kd_h"))
													nav_kd_head = jsonCmdReceive["kd_h"];
												break;
	case CMD_NAV_ODOMETRY_RESET:         // 重置里程计原点（当前位置清零）
												nav_odometry_reset();
												break;
	case CMD_NAV_AUTO_ODOMETRY:          // 开关自动里程计
												if (jsonCmdReceive.containsKey("cmd"))
													nav_use_auto_odometry = (jsonCmdReceive["cmd"].as<int>() != 0);
												break;
	case CMD_NAV_SET_MAX_SPEED:          // 设置导航最大速度
												if (jsonCmdReceive.containsKey("spd"))
													nav_set_max_speed(jsonCmdReceive["spd"]);
												break;
	case CMD_NAV_GET_STATUS:             // 获取导航状态信息
												{
													jsonInfoHttp.clear();
													jsonInfoHttp["T"] = 708;
													jsonInfoHttp["active"] = nav_target_active;
													jsonInfoHttp["reached"] = nav_goal_reached;
													jsonInfoHttp["cx"] = nav_current_x;
													jsonInfoHttp["cy"] = nav_current_y;
													jsonInfoHttp["ch"] = nav_current_heading;
													jsonInfoHttp["tx"] = nav_target_x;
													jsonInfoHttp["ty"] = nav_target_y;
													jsonInfoHttp["dist"] = nav_get_distance_to_target();
													jsonInfoHttp["auto_odom"] = nav_use_auto_odometry;
													String _navStatusStr;
													serializeJson(jsonInfoHttp, _navStatusStr);
													Serial.println(_navStatusStr);
												}
												break;
	}
}


void serialCtrl() {
  static String receivedData;

  while (Serial.available() > 0) {
    char receivedChar = Serial.read();
    receivedData += receivedChar;

    // Detect the end of the JSON string based on a specific termination character
    if (receivedChar == '\n') {
      // Now we have received the complete JSON string
      DeserializationError err = deserializeJson(jsonCmdReceive, receivedData);
      if (err == DeserializationError::Ok) {
  			if (InfoPrint == 1 && uartCmdEcho) {
  				Serial.print(receivedData);
  			}
        jsonCmdReceiveHandler();
      } else {
        // Handle JSON parsing error here
      }
      // Reset the receivedData for the next JSON string
      receivedData = "";
    }
  }
}