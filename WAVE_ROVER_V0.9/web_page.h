const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>UGV01_BASE_WEB</title>
    <meta name="viewport" content="width=device-width,initial-scale=1.0">
    <!-- <script src="http://code.jquery.com/jquery-1.9.1.min.js"></script> -->
    <style type="text/css">
    html {
        display: inline-block;
        text-align: center;
        font-family: sans-serif;
    }
    body {
        background: #f0f2f5;
        font-family: "roboto",helt "sans-serif";
        font-weight: 400;
        background-position: center 0;
        background-attachment: fixed;
        color: #333;
        font-size: 14px;
    }
    .cc-btn {
        border: 0;
        cursor: pointer;
        color: #1a1a1a;
        background: rgba(164,169,186,0);
        font-size: 1em;
        width: 125px;
        height: 125px;
         -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none; 
    }
    .cc-middle{
        width: 120px;
        height: 120px;
        border-radius: 50%;
        background-color: #e0e3e8;
    }
    .cc-btn:hover svg, .cc-middle:hover {
        opacity: 0.6;
    }
    .cc-btn:active svg { opacity: 0.8; }
    .cc-middle:hover { opacity: 0.7; }
    .controlor-c > div{
        width: 360px;
        height: 360px;
        background-color: rgba(0,0,0,0.04);
        border-radius: 40px;
        box-shadow: 10px 10px 10px rgba(0,0,0,0.05);
        margin: auto;
    }
    .controlor-c > div > div{
        display: flex;
    }
    main {
        width: 960px;
        margin: auto;
    }
    section{margin: 40px 0;}
    .for-move {
        display: flex;
        align-items: center;
    }
    .for-move-a, .for-move-b{
        flex: 1;
        margin: 0 20px;
    }
    .h2-tt {
        font-size: 2em;
        font-weight: 700;
        color: #1a1a1a;
        text-transform: uppercase;
    }
    .info-device-box .info-box{display: flex;}
    .info-device-box .info-box{padding: 20px 0;}
    .num-box-big > div, .num-box-sma > div{flex: 1;}
    .num-box-big > div:first-child{border-right: 1px solid #e0e0e0;}
    .num-box-mid {
        flex-wrap: wrap;
        justify-content: space-between;
    }
    .num-box-mid div{
        width:33.3333%;
        margin: 20px 0;
    }
    .info-device-box .info-box > div > span {
        display: block;
        font-weight: 500;
    }
    .info-box {
        background: #fff;
        margin: 20px auto;
        border: 1px solid #e0e0e0;
        box-shadow: 0 2px 8px rgba(0,0,0,0.06);
        border-radius: 8px;
        color: #1a1a1a;
    }
    .big-num{font-size: 3em;}
    .mid-num{font-size: 2em;}
    .sma-num{font-size: 1.2em;}
    .num-color{
        background-image: linear-gradient(#1a73e8,#1557b0);
        background-clip: text;
        color:transparent;
        -webkit-background-clip: text;
        -moz-background-clip: text;
        -ms-background-clip: text;
        font-weight: 900;
        line-height: 1em;
        margin: 0.5em 0;
    }
    .num-color-red{
        background-image: linear-gradient(#d93025,#a50e0e);
        background-clip: text;
        color:transparent;
        -webkit-background-clip: text;
        -moz-background-clip: text;
        -ms-background-clip: text;
        font-weight: 900;
        line-height: 1em;
        margin: 0.5em 0;
    }
    .controlor > div {margin: 80px 0;}
    .json-cmd-info{
        display: flex;
        flex-wrap: wrap;
    }
    .json-cmd-info > div {
        width: 33.33333%;
        padding: 10px 0;
    }
    .json-cmd-info p{
        line-height: 30px;
        margin: 0;
    }
    .json-cmd-info p span {
        display: block;
        color: #1a1a1a;
    }
    .small-btn{
        color: #1a1a1a;
        background-color: #e8eaed;
        border: none;
        height: 48px;
        border-radius: 4px;
        font-weight: 500;
    }
    .small-btn-active{
        background-color: rgba(26,115,232,0.08);
        color: #1a73e8;
        border: 1px solid #1a73e8;
        height: 48px;
        border-radius: 4px;
    }
    .feedb-p input{
        width: 100%;
        height: 46px;
        background-color: #fff;
        padding: 0 10px;
        border: 1px solid #ddd;
        border-radius: 4px;
        color: #333;
        font-size: 1.2em;
        margin-right: 10px;
    }
    .control-speed > div {
        width: 290px;
        margin: auto;
    }
    .control-speed > div > div{display: flex;}
    .control-speed label {flex: 1;}
    .small-btn, .small-btn-active{
        width: 90px;
    }
    .feedb-p{ display: flex;}
    .fb-input-info{
        margin: 0 20px;
    }
    .fb-info {margin: 20px;}
    .fb-info > span{line-height: 2.4em;}
    .btn-send:hover, .small-btn:hover{background-color: #1a73e8;color:#fff;}
    .btn-send:active, .small-btn:active{background-color: #1557b0;}
    .w-btn{
        color: #1a73e8;
        background: transparent;
        padding: 10px;
        border: none;
        font-weight: 500;
    }
    .w-btn:hover{color: #1557b0;}
    .w-btn:active{color: #0d47a1;}
    @media screen and (min-width: 768px) and (max-width: 1200px){
        body{font-size: 16px;}
        main {
            width: 100%;
        }
        .for-move {
            display: block;
        }
        /* .controlor-c > div{width: 600px;height: 600px;}
        .cc-btn{width: 200px;height: 200px;} */
        .json-cmd-info{display: block;}
        .json-cmd-info p span{display: inline;}
        .json-cmd-info > div{
            display: flex;
            width: auto;
            padding: 20px;
            flex-wrap: wrap;
            justify-content: space-between;
        }
        .control-speed > div{width: 600px;}
        section{margin: 20px 0;}
    }
    @media screen and (min-width: 360px) and (max-width: 767px){
        main {
            width: 100%;
        }
        .for-move {
            display: block;
        }
        .json-cmd-info{display: block;}
        .json-cmd-info p span{display: inline;}
        .json-cmd-info > div{
            display: flex;
            width: auto;
            padding: 20px;
            flex-wrap: wrap;
            justify-content: space-between;
        }
        section{margin: 10px 0;}
        .info-box{margin: 10px auto;}
        .info-device-box .info-box{padding: 10px;}
        .num-box-mid div{margin: 10px 0;}
        .controlor-c > div{
            width: 270px;
            height: 270px;
        }
        .cc-btn{
            width: 90px;
            height: 90px;;
        }
        .big-num{font-size: 2em;}
        .controlor > div{margin: 40px 0;}
    }
    #navMapContainer { margin-top:16px; text-align:center; }
    #navMap { border:1px solid #ddd; border-radius:4px; background:#fafafa; }
    </style>
</head>
<body>
    <main>
        <section>
            <div>
                <h2 class="h2-tt" id="deviceInfo">控制面板</h2>
            </div>
            <div class="for-move">
                <div class="for-move-a">
                    <div class="info-device-box">
                        <div class="info-box num-box-big">
                            <div >
                                <span class="big-num num-color" id="V">-1.01</span>
                                <span id="Vn">电池电压(V)</span>
                            </div>
                            <div>
                                <span class="big-num num-color" id="RSSI">-1.01</span>
                                <span id="RSSIn">WiFi信号</span>
                            </div>
                        </div>
                    </div>
                    <div class="info-device-box">
                        <div class="info-box num-box-mid">
                            <div>
                                <span class="num-color mid-num" id="r">-1.01</span>
                                <span id="rn">横滚角度(°)</span>
                            </div>
                            <div>
                                <span class="num-color mid-num" id="p">-1.01</span>
                                <span id="pn">俯仰角度(°)</span>
                            </div>
                            <div>
                                <span class="num-color mid-num" id="y">-1.01</span>
                                <span id="yn">偏航角度(°)</span>
                            </div>
                            <div>
                                <span class="num-color mid-num" id="mZ">-1.01</span>
                                <span id="mZn">速度倍率</span>
                            </div>
                        </div>
                    </div>
                    <div class="info-device-box">
                        <div class="info-box num-box-mid">
                            <div>
                                <span class="num-color mid-num" id="ltX">--</span>
                                <span>UWB X(m)</span>
                            </div>
                            <div>
                                <span class="num-color mid-num" id="ltY">--</span>
                                <span>UWB Y(m)</span>
                            </div>
                            <div>
                                <span class="num-color mid-num" id="ltYaw">--</span>
                                <span>UWB Yaw(°)</span>
                            </div>
                            <div>
                                <span class="num-color mid-num" id="ltEopX">--</span>
                                <span>EOP X</span>
                            </div>
                            <div>
                                <span class="num-color mid-num" id="ltEopY">--</span>
                                <span>EOP Y</span>
                            </div>
                            <div>
                                <span class="num-color mid-num" id="ltCnt">--</span>
                                <span>Frames</span>
                            </div>
                        </div>
                    </div>
                </div>
                <div class="for-move-b controlor">
                    <div class="controlor-c">
                        <div>
                            <div>
                                <label><button class="cc-btn" onmousedown="movtionButton(0.3,0.5);" ontouchstart="movtionButton(0.3,0.5);" onmouseup="movtionButton(0,0);" ontouchend="movtionButton(0,0);"><svg fill="none" version="1.1" width="23" height="23" viewBox="0 0 23 23"><g style="mix-blend-mode:passthrough"><path d="M0,2L0,18.1716C0,19.9534,2.15428,20.8457,3.41421,19.5858L19.5858,3.41421C20.8457,2.15428,19.9534,0,18.1716,0L2,0C0.895431,0,0,0.895431,0,2Z" fill="#5f6368" fill-opacity="0.30"/></g></svg></button></label>
                                <label><button class="cc-btn" onmousedown="movtionButton(0.5,0.5);" ontouchstart="movtionButton(0.5,0.5);" onmouseup="movtionButton(0,0);" ontouchend="movtionButton(0,0);"><svg fill="none" version="1.1" width="26.87807685863504" height="15.435028109846826" viewBox="0 0 26.87807685863504 15.435028109846826"><g style="mix-blend-mode:passthrough" transform="matrix(0.9999999403953552,0,0,0.9999999403953552,0,0)"><path d="M12.0248,0.585787L0.589796,12.0208C-0.670133,13.2807,0.222199,15.435,2.00401,15.435L24.8741,15.435C26.6559,15.435,27.5482,13.2807,26.2883,12.0208L14.8533,0.585787C14.0722,-0.195262,12.8059,-0.195262,12.0248,0.585787Z" fill="#5f6368" fill-opacity="0.85"/></g></svg></button></label>
                                <label><button class="cc-btn" onmousedown="movtionButton(0.5,0.3);" ontouchstart="movtionButton(0.5,0.3);" onmouseup="movtionButton(0,0);" ontouchend="movtionButton(0,0);"><svg fill="none" version="1.1" width="23" height="23" viewBox="0 0 23 23"><g style="mix-blend-mode:passthrough" transform="matrix(0,1,-1,0,23,-23)"><path d="M23,2L23,18.1716C23,19.9534,25.15428,20.8457,26.41421,19.5858L42.5858,3.41421C43.8457,2.15428,42.9534,0,41.1716,0L25,0C23.895431,0,23,0.895431,23,2Z" fill="#5f6368" fill-opacity="0.30"/></g></svg></button></label>
                            </div>
                            <div>
                                <label><button class="cc-btn" onmousedown="movtionButton(-0.5,0.5);" ontouchstart="movtionButton(-0.5,0.5);" onmouseup="movtionButton(0,0);" ontouchend="movtionButton(0,0);"><svg fill="none" version="1.1" width="15.435028109846769" height="26.87807685863504" viewBox="0 0 15.435028109846769 26.87807685863504"><g style="mix-blend-mode:passthrough" transform="matrix(0.9999999403953552,0,0,0.9999999403953552,0,0)"><path d="M0.585787,14.8533L12.0208,26.2883C13.2807,27.5482,15.435,26.6559,15.435,24.8741L15.435,2.00401C15.435,0.222199,13.2807,-0.670133,12.0208,0.589795L0.585787,12.0248C-0.195262,12.8059,-0.195262,14.0722,0.585787,14.8533Z" fill="#5f6368" fill-opacity="0.85"/></g></svg></button></label>
                                <label><button class="cc-btn cc-middle" onmousedown="movtionButton(0,0);" ontouchstart="movtionButton(0,0);" onmouseup="movtionButton(0,0);" ontouchend="movtionButton(0,0);">停止</button></label>
                                <label><button class="cc-btn" onmousedown="movtionButton(0.5,-0.5);" ontouchstart="movtionButton(0.5,-0.5);" onmouseup="movtionButton(0,0);" ontouchend="movtionButton(0,0);"><svg fill="none" version="1.1" width="15.435030017195288" height="26.87807685863504" viewBox="0 0 15.435030017195288 26.87807685863504"><g style="mix-blend-mode:passthrough" transform="matrix(0.9999999403953552,0,0,0.9999999403953552,0,0)"><path d="M14.8492,12.0248L3.41422,0.589796C2.15429,-0.670133,-9.53674e-7,0.222199,9.53674e-7,2.00401L9.53674e-7,24.8741C-9.53674e-7,26.6559,2.15429,27.5482,3.41421,26.2883L14.8492,14.8533C15.6303,14.0722,15.6303,12.8059,14.8492,12.0248Z" fill="#5f6368" fill-opacity="0.85"/></g></svg></button></label>
                            </div>
                            <div>
                                <label><button class="cc-btn" onmousedown="movtionButton(-0.3,-0.5);" ontouchstart="movtionButton(-0.3,-0.5);" onmouseup="movtionButton(0,0);" ontouchend="movtionButton(0,0);"><svg fill="none" version="1.1" width="23" height="23" viewBox="0 0 23 23"><g style="mix-blend-mode:passthrough" transform="matrix(0,-1,1,0,-23,23)"><path d="M0,25L0,41.1716C0,42.9534,2.15428,43.8457,3.41421,42.5858L19.5858,26.41421C20.8457,25.15428,19.9534,23,18.1716,23L2,23C0.895431,23,0,23.895431,0,25Z" fill="#5f6368" fill-opacity="0.30"/></g></svg></button></label>
                                <label><button class="cc-btn" onmousedown="movtionButton(-0.5,-0.5);" ontouchstart="movtionButton(-0.5,-0.5);" onmouseup="movtionButton(0,0);" ontouchend="movtionButton(0,0);"><svg fill="none" version="1.1" width="26.87807685863504" height="15.435030017195231" viewBox="0 0 26.87807685863504 15.435030017195231"><g style="mix-blend-mode:passthrough" transform="matrix(0.9999999403953552,0,0,0.9999999403953552,0,0)"><path d="M14.8533,14.8492L26.2883,3.41422C27.5482,2.15429,26.6559,-9.53674e-7,24.8741,9.53674e-7L2.00401,9.53674e-7C0.222199,-9.53674e-7,-0.670133,2.15429,0.589795,3.41421L12.0248,14.8492C12.8059,15.6303,14.0722,15.6303,14.8533,14.8492Z" fill="#5f6368" fill-opacity="0.85"/></g></svg></button></label>
                                <label><button class="cc-btn" onmousedown="movtionButton(-0.5,-0.3);" ontouchstart="movtionButton(-0.5,-0.3);" onmouseup="movtionButton(0,0);" ontouchend="movtionButton(0,0);"><svg fill="none" version="1.1" width="23" height="23" viewBox="0 0 23 23"><g style="mix-blend-mode:passthrough" transform="matrix(-1,0,0,-1,46,46)"><path d="M23,25L23,41.1716C23,42.9534,25.15428,43.8457,26.41421,42.5858L42.5858,26.41421C43.8457,25.15428,42.9534,23,41.1716,23L25,23C23.895431,23,23,23.895431,23,25Z" fill="#5f6368" fill-opacity="0.30"/></g></svg></button></label>
                            </div>
                        </div>
                    </div>
                    <div class="control-speed">
                        <div>
                            <div id="device-speed-btn">
                                <label><button name="speedbtn" class="small-btn" onclick="changeSpeed(0.3);">慢速</button></label>
                                <label><button name="speedbtn" class="small-btn" onclick="changeSpeed(0.6);">中速</button></label>
                                <label><button name="speedbtn" class="small-btn" onclick="changeSpeed(1.0);">快速</button></label>
                            </div>
                        </div>
                        <br>
                        <div>
                            <div id="device-led-btn">
                                <label><button name="speedbtn" class="small-btn" onclick="ledCtrl(1);">灯1</button></label>
                                <label><button name="speedbtn" class="small-btn" onclick="ledCtrl(2);">灯2</button></label>
                                <label><button name="speedbtn" class="small-btn" onclick="ledCtrl(0);">关闭</button></label>
                            </div>
                        </div>
                    </div>
                    <div class="info-device-box">
                        <div class="info-box num-box-sma">
                            <div>
                                <span class="num-color sma-num" id="IP">192.168.10.67</span>
                                <span id="IPn">IP地址</span>
                            </div>
                            <div>
                                <span class="num-color sma-num" id="MAC">44:17:93:EE:F8:F8</span>
                                <span id="MACn">MAC地址</span>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </section>
        <section>
            <!-- 导航PID控制面板 -->
            <div class="info-box" style="padding:20px;">
                <h2 class="h2-tt" style="font-size:1.2em;margin:0 0 12px 0;">导航PID控制</h2>
                <div class="feedb-p" style="flex-wrap:wrap;gap:8px;">
                    <label style="color:#1a1a1a;">目标X(m):</label>
                    <input type="number" id="navTargetX" placeholder="X (m)" step="0.01" value="1.0" style="width:80px;height:36px;background:#fff;border:1px solid #ddd;border-radius:4px;color:#1a1a1a;text-align:center;">
                    <label style="color:#1a1a1a;">目标Y(m):</label>
                    <input type="number" id="navTargetY" placeholder="Y (m)" step="0.01" value="0.0" style="width:80px;height:36px;background:#fff;border:1px solid #ddd;border-radius:4px;color:#1a1a1a;text-align:center;">
                    <button class="small-btn" onclick="navSetTarget();" style="background-color:#1a73e8;color:#fff;">出发</button>
                    <button class="small-btn" onclick="navStop();">停止</button>
                    <button class="small-btn" onclick="navResetOdom();">归零</button>
                </div>
                <div class="info-box num-box-sma" style="margin-top:20px; padding: 16px 20%;">
                    <div>
                        <span class="num-color sma-num" id="navStatus">待命</span>
                        <span>状态</span>
                    </div>
                    <div>
                        <span class="num-color sma-num" id="navDist">--</span>
                        <span>距离(m)</span>
                    </div>
                    <div>
                        <span class="num-color sma-num" id="navCur">(0.00,0.00)</span>
                        <span>当前位置</span>
                    </div>
                </div>
                <div id="navMapContainer">
                    <canvas id="navMap" width="915" height="320"></canvas>
                </div>
            </div>
        </section>
        <section>
            <div class="fb-info">
                <h2 class="h2-tt" id="deviceInfo">反馈信息</h2>
                <span id="fbInfo" word-wrap="break-all">JSON反馈信息将显示在此处</span>
            </div>
            <div class="fb-input-info">
                <div class="feedb-p">
                    <input type="text" id="jsonData" placeholder="在此输入JSON指令">
                    <label><button class="small-btn btn-send" onclick="jsonSend();">发送</button></label>
                </div>
                <div class="info-box json-cmd-info">
                    <div>
                        <p>速度控制: <span id="cmd1" class="cmd-value">{"T":1,"L":0.5,"R":0.5}</span></p>
                        <button class="w-btn" onclick="cmdFill('jsonData', 'cmd1');">填入</button>
                    </div>
                    <div>
                        <p>心跳设置: <span id="cmd136" class="cmd-value">{"T":136,"cmd":0}</span></p>
                        <button class="w-btn" onclick="cmdFill('jsonData', 'cmd136');">填入</button>
                    </div>
                    <div>
                        <p>启动WiFi: <span id="cmd401" class="cmd-value">{"T":401,"cmd":3}</span></p>
                        <button class="w-btn" onclick="cmdFill('jsonData', 'cmd401');">填入</button>
                    </div>
                    <div>
                        <p>设置AP: <span id="cmd402" class="cmd-value">{"T":402,"ssid":"UGV","password":"12345678"}</span></p>
                        <button class="w-btn" onclick="cmdFill('jsonData', 'cmd402');">填入</button>
                    </div>
                    <div>
                        <p>停止WiFi: <span id="cmd408" class="cmd-value">{"T":408}</span></p>
                        <button class="w-btn" onclick="cmdFill('jsonData', 'cmd408');">填入</button>
                    </div>
                    <div>
                        <p>重启: <span id="cmd600" class="cmd-value">{"T":600}</span></p>
                        <button class="w-btn" onclick="cmdFill('jsonData', 'cmd600');">填入</button>
                    </div>
                    <div>
                        <p>可用Flash空间: <span id="cmd601" class="cmd-value">{"T":601}</span></p>
                        <button class="w-btn" onclick="cmdFill('jsonData', 'cmd601');">填入</button>
                    </div>
                    <div>
                        <p>重置启动任务: <span id="cmd603" class="cmd-value">{"T":603}</span></p>
                        <button class="w-btn" onclick="cmdFill('jsonData', 'cmd603');">填入</button>
                    </div>
                </div>
            </div>
        </section>
    </main>
<script>
    var cmdA;
    var cmdB;
    var cmdC;

    var forwardButton;   // 1
    var backwardButton;  // 2
    var fbNewer;

    // 导航地图数据
    var navMapCX = 0.0, navMapCY = 0.0;
    var navMapTX = null, navMapTY = null;
    var navMapScale = 50;  // 像素/米
    var navMapTrail = [];

    var leftButton;      // 1
    var rightButton;     // 2
    var lrNewer;

    var last_forwardButton;   // 1
    var last_backwardButton;  // 2
    var last_fbNewer;

    var last_leftButton;      // 1
    var last_rightButton;     // 2
    var last_lrNewer;

    var speed_rate  = 1; // 1:fast 0.6:middle 0.3:slow
    var left_speed  = 0;
    var right_speed = 0;
    var send_heartbeat = 0;

    var io4_status = 0;
    var io5_status = 0;


    getDevInfo();
    ledCtrl(0);
    setInterval(function() {
        getDevInfo();
    }, 2510);

    setInterval(function() {
        heartBeat();
    }, 1500);

    setInterval(function() {
        infoUpdate();
    }, 1000);

    function cmdFill(rawInfo, fillInfo) {
        document.getElementById(rawInfo).value = document.getElementById(fillInfo).innerHTML;
    }
    function jsonSend() {
        send_heartbeat = 0;
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              document.getElementById("fbInfo").innerHTML =
              this.responseText;
            }
        };
        xhttp.open("GET", "js?json="+document.getElementById('jsonData').value, true);
        xhttp.send();
    }
    function drawNavMap() {
        var canvas = document.getElementById("navMap");
        if (!canvas) return;
        var ctx = canvas.getContext("2d");
        var w = canvas.width, h = canvas.height;
        var cx = w / 2, cy = h / 2;
        var scale = navMapScale;

        ctx.clearRect(0, 0, w, h);
        ctx.fillStyle = "#fafafa";
        ctx.fillRect(0, 0, w, h);

        // 网格线
        var gridRange = Math.ceil(Math.max(w, h) / 2 / scale) + 1;
        ctx.strokeStyle = "#e8e8e8";
        ctx.lineWidth = 1;
        for (var i = -gridRange; i <= gridRange; i++) {
            var px = cx + i * scale;
            ctx.beginPath();
            ctx.moveTo(px, 0); ctx.lineTo(px, h); ctx.stroke();
            var py = cy + i * scale;
            ctx.beginPath();
            ctx.moveTo(0, py); ctx.lineTo(w, py); ctx.stroke();
        }

        // 坐标轴
        ctx.strokeStyle = "#ccc";
        ctx.lineWidth = 1.5;
        ctx.beginPath(); ctx.moveTo(cx, 0); ctx.lineTo(cx, h); ctx.stroke();
        ctx.beginPath(); ctx.moveTo(0, cy); ctx.lineTo(w, cy); ctx.stroke();

        // 刻度
        ctx.fillStyle = "#999";
        ctx.font = "10px sans-serif";
        ctx.textAlign = "center";
        for (var i = -gridRange; i <= gridRange; i++) {
            if (i !== 0) {
                ctx.fillText(i + "m", cx + i * scale, cy + 14);
                ctx.save();
                ctx.translate(cx - 14, cy + i * scale);
                ctx.rotate(-Math.PI / 2);
                ctx.fillText(i + "m", 0, 0);
                ctx.restore();
            }
        }
        ctx.fillText("0", cx, cy + 14);

        // 目标位置
        if (navMapTX !== null && navMapTY !== null) {
            var tx = cx + navMapTX * scale;
            var ty = cy - navMapTY * scale;
            ctx.fillStyle = "#e53935";
            ctx.beginPath();
            ctx.arc(tx, ty, 6, 0, 2 * Math.PI);
            ctx.fill();
            ctx.fillStyle = "#fff";
            ctx.font = "bold 9px sans-serif";
            ctx.textAlign = "center";
            ctx.fillText("T", tx, ty + 4);
        }

        // 当前位置
        var px = cx + navMapCX * scale;
        var py = cy - navMapCY * scale;

        // 轨迹
        navMapTrail.push({x: px, y: py});
        if (navMapTrail.length > 50) navMapTrail.shift();
        if (navMapTrail.length > 1) {
            ctx.strokeStyle = "rgba(26,115,232,0.3)";
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.moveTo(navMapTrail[0].x, navMapTrail[0].y);
            for (var t = 1; t < navMapTrail.length; t++) {
                ctx.lineTo(navMapTrail[t].x, navMapTrail[t].y);
            }
            ctx.stroke();
        }

        // 当前点
        ctx.fillStyle = "#1a73e8";
        ctx.beginPath();
        ctx.arc(px, py, 7, 0, 2 * Math.PI);
        ctx.fill();
        ctx.strokeStyle = "#fff";
        ctx.lineWidth = 2;
        ctx.stroke();
    }
    function infoUpdate() {
        var jsonCmd = {
            "T": 130
        }
        var jsonString = JSON.stringify(jsonCmd);
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                var jsonResponse = JSON.parse(this.responseText);
                document.getElementById("V").innerHTML = jsonResponse.v.toFixed(2);
                if (jsonResponse.V<11.06) {
                    document.getElementById("V").classList.remove("num-color");
                    document.getElementById("V").classList.add("num-color-red");
                }else{
                    document.getElementById("V").classList.remove("num-color-red");
                    document.getElementById("V").classList.add("num-color");
                }

                document.getElementById("r").innerHTML = jsonResponse.r?.toFixed(2);
                document.getElementById("p").innerHTML = jsonResponse.p?.toFixed(2);
                document.getElementById("y").innerHTML = jsonResponse.y?.toFixed(2);
                document.getElementById("mZ").innerHTML = speed_rate;


                // LinkTrack UWB 定位数据更新
                if (jsonResponse.hasOwnProperty('lt_x')) {
                    document.getElementById("ltX").innerHTML = jsonResponse.lt_x?.toFixed(2) || "--";
                    document.getElementById("ltY").innerHTML = jsonResponse.lt_y?.toFixed(2) || "--";
                    document.getElementById("ltYaw").innerHTML = jsonResponse.lt_yaw?.toFixed(1) || "--";
                    document.getElementById("ltEopX").innerHTML = jsonResponse.lt_eop_x?.toFixed(2) || "--";
                    document.getElementById("ltEopY").innerHTML = jsonResponse.lt_eop_y?.toFixed(2) || "--";
                    document.getElementById("ltCnt").innerHTML = jsonResponse.lt_cnt || "--";
                }

                // 导航状态更新
                if (jsonResponse.hasOwnProperty('nav_active')) {
                    if (jsonResponse.nav_active && !jsonResponse.nav_reached) {
                        document.getElementById("navStatus").innerHTML = "行驶中";
                        document.getElementById("navStatus").style.color = "#4CAF50";
                    } else if (jsonResponse.nav_reached) {
                        document.getElementById("navStatus").innerHTML = "已到达";
                        document.getElementById("navStatus").style.color = "#FFC107";
                    } else {
                        document.getElementById("navStatus").innerHTML = "待命";
                        document.getElementById("navStatus").style.color = "";
                    }
                    document.getElementById("navDist").innerHTML = jsonResponse.nav_dist?.toFixed(3) || "--";
                    document.getElementById("navCur").innerHTML = "(" + (jsonResponse.nav_cx?.toFixed(2)||"0.00") + "," + (jsonResponse.nav_cy?.toFixed(2)||"0.00") + ")";
                    // 更新地图数据
                    navMapCX = jsonResponse.nav_cx || 0.0;
                    navMapCY = jsonResponse.nav_cy || 0.0;
                    if (jsonResponse.hasOwnProperty('nav_tx')) {
                        navMapTX = jsonResponse.nav_tx;
                        navMapTY = jsonResponse.nav_ty;
                    }
                    drawNavMap();
                }
            }
        };
        xhttp.open("GET", "js?json=" + jsonString, true);
        xhttp.send();
    }
    function getDevInfo() {
        var jsonCmd = {
            "T": 405
        }
        var jsonString = JSON.stringify(jsonCmd);
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                var jsonResponse = JSON.parse(this.responseText);

                document.getElementById("IP").innerHTML = jsonResponse.ip;
                document.getElementById("MAC").innerHTML = jsonResponse.mac;
                document.getElementById("RSSI").innerHTML = jsonResponse.rssi;
            }
        };
        xhttp.open("GET", "js?json=" + jsonString, true);
        xhttp.send();
    }
    function changeSpeed(inputSpd) {
        speed_rate = inputSpd;
    }
    function heartBeat() {
        if (send_heartbeat == 1) {
            var jsonCmd = {
                "T":1,
                "L":left_speed,
                "R":right_speed
            }
            var jsonString = JSON.stringify(jsonCmd);
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "js?json=" + jsonString, true);
            xhr.send();
        }
    }
    function movtionButton(spdL, spdR){
        left_speed  = spdL*speed_rate;
        right_speed = spdR*speed_rate;
        send_heartbeat = 1;
        var jsonCmd = {
            "T":1,
            "L":left_speed,
            "R":right_speed
        }
        var jsonString = JSON.stringify(jsonCmd);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "js?json=" + jsonString, true);
        xhr.send();
    }
    function ledCtrl(inputCmd){
        if (inputCmd == 0) {
            io4_status = 0;
            io5_status = 0;
        }
        else if (inputCmd == 1) {
            if (io4_status == 0) {
                io4_status = 255;
            }
            else {
                io4_status = 0;
            }
        }
        else if (inputCmd == 2) {
            if (io5_status == 0) {
                io5_status = 255;
            }
            else {
                io5_status = 0;
            }
        }
        var jsonCmd = {
            "T":132,
            "IO4":io4_status,
            "IO5":io5_status
        }
        var jsonString = JSON.stringify(jsonCmd);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "js?json=" + jsonString, true);
        xhr.send();
    }

    function cmdProcess(){
        if (forwardButton == 0 && backwardButton == 0 && leftButton == 0 && rightButton == 0) {
            movtionButton(0, 0);
        }
        else if (forwardButton == 1 && backwardButton == 0 && leftButton == 0 && rightButton == 0){
            movtionButton(0.5, 0.5);
        }else if (forwardButton == 1 && backwardButton == 1 && fbNewer == 1 && leftButton == 0 && rightButton == 0){
            movtionButton(0.5, 0.5);
        }else if (forwardButton == 1 && backwardButton == 1 && fbNewer == 2 && leftButton == 0 && rightButton == 0){
            movtionButton(-0.5, -0.5);
        }else if (forwardButton == 0 && backwardButton == 1 && leftButton == 0 && rightButton == 0){
            movtionButton(-0.5, -0.5);
        }
        else if (forwardButton == 0 && backwardButton == 0 && leftButton == 1 && rightButton == 0){
            movtionButton(-0.5, 0.5);
        }else if (forwardButton == 0 && backwardButton == 0 && leftButton == 1 && rightButton == 1 && lrNewer == 1){
            movtionButton(-0.5, 0.5);
        }else if (forwardButton == 0 && backwardButton == 0 && leftButton == 0 && rightButton == 1){
            movtionButton(0.5, -0.5);
        }else if (forwardButton == 0 && backwardButton == 0 && leftButton == 1 && rightButton == 1 && lrNewer == 2){
            movtionButton(0.5, -0.5);
        }
        else if (forwardButton == 1 && backwardButton == 0 && leftButton == 1 && rightButton == 0){
            movtionButton(0.3, 0.5);
        }else if (forwardButton == 1 && backwardButton == 1 && fbNewer == 1 && leftButton == 1 && rightButton == 0){
            movtionButton(0.3, 0.5);
        }else if (forwardButton == 1 && backwardButton == 1 && fbNewer == 1 && leftButton == 1 && rightButton == 1 && lrNewer == 1){
            movtionButton(0.3, 0.5);
        }else if (forwardButton == 1 && backwardButton == 0 && leftButton == 1 && rightButton == 1 && lrNewer == 1){
            movtionButton(0.3, 0.5);
        }
        else if (forwardButton == 1 && backwardButton == 0 && leftButton == 0 && rightButton == 1){
            movtionButton(0.5, 0.3);
        }else if (forwardButton == 1 && backwardButton == 1 && fbNewer == 1 && leftButton == 0 && rightButton == 1){
            movtionButton(0.5, 0.3);
        }else if (forwardButton == 1 && backwardButton == 1 && fbNewer == 1 && leftButton == 1 && rightButton == 1 && lrNewer == 2){
            movtionButton(0.5, 0.3);
        }else if (forwardButton == 1 && backwardButton == 0 && leftButton == 1 && rightButton == 1 && lrNewer == 2){
            movtionButton(0.5, 0.3);
        }
        else if (forwardButton == 0 && backwardButton == 1 && leftButton == 1 && rightButton == 0){
            movtionButton(-0.3, -0.5);
        }else if (forwardButton == 1 && backwardButton == 1 && fbNewer == 2 && leftButton == 1 && rightButton == 0){
            movtionButton(-0.3, -0.5);
        }else if (forwardButton == 1 && backwardButton == 1 && fbNewer == 2 && leftButton == 1 && rightButton == 1 && lrNewer == 1){
            movtionButton(-0.3, -0.5);
        }else if (forwardButton == 0 && backwardButton == 1 && leftButton == 1 && rightButton == 1 && lrNewer == 1){
            movtionButton(-0.3, -0.5);
        }
        else if (forwardButton == 0 && backwardButton == 1 && leftButton == 0 && rightButton == 1){
            movtionButton(-0.5, -0.3);
        }else if (forwardButton == 1 && backwardButton == 1 && fbNewer == 2 && leftButton == 0 && rightButton == 1){
            movtionButton(-0.5, -0.3);
        }else if (forwardButton == 1 && backwardButton == 1 && fbNewer == 2 && leftButton == 1 && rightButton == 1 && lrNewer == 2){
            movtionButton(-0.5, -0.3);
        }else if (forwardButton == 0 && backwardButton == 1 && leftButton == 1 && rightButton == 1 && lrNewer == 2){
            movtionButton(-0.5, -0.3);
        }
    }

    document.onkeydown = function (event) {
        var e = event || window.event || arguments.callee.caller.arguments[0];
        if (e && e.keyCode == 65) {
            // alert ("A down");
            leftButton = 1;
            lrNewer = 1;
        }else if (e && e.keyCode == 87) {
            // alert ("W down");
            forwardButton = 1;
            fbNewer = 1;
        }else if (e && e.keyCode == 83) {
            // alert ("S down");
            backwardButton = 1;
            fbNewer = 2;
        }else if (e && e.keyCode == 68) {
            // alert ("D down");
            rightButton = 1;
            lrNewer = 2;
        }
        else if (e && e.keyCode == 13) {
            // alert ("Enter down");
            jsonSend();
        }
        else if (e && e.keyCode == 37) {
            // alert ("left down");
            leftButton = 1;
            lrNewer = 1;
        }else if (e && e.keyCode == 38) {
            // alert ("up down");
            forwardButton = 1;
            fbNewer = 1;
        }else if (e && e.keyCode == 40) {
            // alert ("down down");
            backwardButton = 1;
            fbNewer = 2;
        }else if (e && e.keyCode == 39) {
            // alert ("right down");
            rightButton = 1;
            lrNewer = 2;
        }

        if(forwardButton != last_forwardButton || backwardButton != last_backwardButton || fbNewer != last_fbNewer || leftButton != last_leftButton || last_rightButton != rightButton || lrNewer != last_fbNewer) {
            cmdProcess();
            last_forwardButton = forwardButton;
            last_backwardButton = backwardButton;
            last_fbNewer = fbNewer;

            last_leftButton = leftButton;
            last_rightButton = rightButton;
            last_lrNewer = lrNewer;
        }
    }

    document.onkeyup = function (event) {
        var e = event || window.event || arguments.callee.caller.arguments[0];
        if (e && e.keyCode == 65) {
            // alert ("A up");
            leftButton = 0;
        }else if (e && e.keyCode == 87) {
            // alert ("W up");
            forwardButton = 0;
        }else if (e && e.keyCode == 83) {
            // alert ("S up");
            backwardButton = 0;
        }else if (e && e.keyCode == 68) {
            // alert ("D up");
            rightButton = 0;
        }
        else if (e && e.keyCode == 37) {
            // alert ("left up");
            leftButton = 0;
        }else if (e && e.keyCode == 38) {
            // alert ("up up");
            forwardButton = 0;
        }else if (e && e.keyCode == 40) {
            // alert ("down up");
            backwardButton = 0;
        }else if (e && e.keyCode == 39) {
            // alert ("right up");
            rightButton = 0;
        }

        cmdProcess();

        last_forwardButton = forwardButton;
        last_backwardButton = backwardButton;
        last_fbNewer = fbNewer;

        last_leftButton = leftButton;
        last_rightButton = rightButton;
        last_lrNewer = lrNewer;
    }

    // ====== 导航PID控制函数 ======
    function navSetTarget() {
        var tx = parseFloat(document.getElementById("navTargetX").value) || 0;
        var ty = parseFloat(document.getElementById("navTargetY").value) || 0;
        var jsonCmd = {"T":700,"x":tx,"y":ty};
        var jsonString = JSON.stringify(jsonCmd);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "js?json=" + jsonString, true);
        xhr.send();
        document.getElementById("fbInfo").innerHTML = "导航目标已发送: (" + tx + ", " + ty + ")";
    }
    function navStop() {
        var jsonCmd = {"T":702};
        var jsonString = JSON.stringify(jsonCmd);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "js?json=" + jsonString, true);
        xhr.send();
        document.getElementById("fbInfo").innerHTML = "导航停止指令已发送";
    }
    function navResetOdom() {
        var jsonCmd = {"T":704};
        var jsonString = JSON.stringify(jsonCmd);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "js?json=" + jsonString, true);
        xhr.send();
        document.getElementById("fbInfo").innerHTML = "里程计归零指令已发送";
    }
    function navGetStatus() {
        var jsonCmd = {"T":707};
        var jsonString = JSON.stringify(jsonCmd);
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("fbInfo").innerHTML = this.responseText;
            }
        };
        xhr.open("GET", "js?json=" + jsonString, true);
        xhr.send();
    }
</script>
</body>
</html>
)rawliteral";