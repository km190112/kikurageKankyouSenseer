/**************************************
  温湿度計SHT35のセンサ情報を取得してAmbientにデータを送る　　　
 **************************************/
// M5StickCPlusを利用する。
#include <M5StickCPlus.h>

// Anbient
#include <WiFi.h>
#include "Ambient.h"

//SHT35(https://github.com/Seeed-Studio/Seeed_SHT35)
#include <Seeed_SHT35.h>

#define SDAPIN 32
#define SCLPIN 33
#define RSTPIN 7
#define SERIAL SerialUSB

SHT35 sensor(SCLPIN);

// SCD41(https://github.com/DFRobot/DFRobot_SCD4X)
#include <DFRobot_SCD4X.h>
DFRobot_SCD4X SCD4X(&Wire, /*i2cAddr = */ SCD4X_I2C_ADDR);  //'0x62'

//WiFi
WiFiClient client;

// //ルーター設定
#define WIFISSID "***************"  // ルーターのSSID
#define WIFIPASS "***************"  //PSK ハッシュ値256bitのキー(https://mascii.github.io/wpa-psk-calc/)

//Ambient
Ambient ambient;
uint32_t channelId = *****;                 // AmbientのチャネルID
const char* writeKey = "****************";  // ライトキー


volatile bool AmbientConnectionFlg = false;
const int32_t DelayTime_ms = 900000;  // センサ読み取り間隔ms 15分


void lcdUpdate(const float temp, const float hum, const uint32_t averageCO2ppm) {
  // LCDにカウント数を表示する関数
  M5.Lcd.setTextColor(WHITE);  // 文字色設定(color,backgroundcolor)引数2番目省略時は(WHITE, BLACK, RED, GREEN, BLUE, YELLOW...)
  M5.Lcd.fillScreen(BLACK);    // 背景色設定 定数((WHITE, BLACK, RED, GREEN, BLUE, YELLOW...))
  M5.Lcd.setTextFont(1);       // フォントを指定（1～8)1:Adafruit 8ピクセルASCIIフォント、2:16ピクセルASCIIフォント、4:26ピクセルASCIIフォント...)
  M5.Lcd.setTextSize(2);       // 文字の大きさを設定（1～7）
  M5.Lcd.setCursor(0, 0);

  M5.Lcd.println("Kikurage Sensor v4");

  M5.Lcd.printf("Temp:  %3.1f C\n", temp);
  M5.Lcd.printf("hum :  %3.1f RH\n", hum);
  M5.Lcd.printf("CO2 : %5d ppm\n", averageCO2ppm);

  //サーバーとの接続状況表示
  if (WiFi.status() != WL_CONNECTED) {  //WiFiが接続できていない場合の表示
    M5.Lcd.setTextColor(RED);
    M5.Lcd.println("OFFLINE");

  } else {  //WiFiが接続できた場合の表示
    M5.Lcd.setTextColor(BLUE);
    M5.Lcd.print("ONLINE");

    if (AmbientConnectionFlg) {  // サーバとのリンクが確立している場合の表示
      M5.Lcd.println(" Ambient LINK\n");
      M5.Lcd.printf("channel ID:%d", channelId);

    } else {
      M5.Lcd.setTextColor(RED);
      M5.Lcd.println(" Ambient connection error");
    }
  }
}

void sendAmbient(const float temp, const float hum, const uint32_t co2Val) {
  // WiFiがONの場合は温度、湿度の値をAmbientに送信する
  // portENTER_CRITICAL(&timerMux);
  if (WiFi.status() == WL_CONNECTED) {

    ambient.set(1, String(temp).c_str());
    ambient.set(2, String(hum).c_str());
    ambient.set(3, String(co2Val).c_str());

    AmbientConnectionFlg = ambient.send();

    if (AmbientConnectionFlg) {
      Serial.println(F("Ambient Send"));
      Serial.printf("channel ID: %d\n", channelId);


    } else {
      Serial.println(F("Server connection error"));
    }

    delay(100);
  }
}

uint32_t SCD4X_CO2Val() {
  // CO2センサSCD4Xを立ち上げ、センサの値を5回取得した平均を返す。
  Serial.println("Waking sensor...");
  SCD4X.setSleepMode(SCD4X_WAKE_UP);

  DFRobot_SCD4X::sSensorMeasurement_t SCD41data[6];
  memset(SCD41data, 0, sizeof(SCD41data));

  Serial.println("Measuring...");

  // 5回の平均をとる。
  uint32_t averageCO2ppm = 0;

  for (int i = 0; i <= 5; i++) {
    SCD4X.measureSingleShot(SCD4X_MEASURE_SINGLE_SHOT);
    while (!SCD4X.getDataReadyStatus()) {
      delay(100);
    }

    SCD4X.readMeasurement(&SCD41data[i]);

    if (i > 0) {  // ウェイクアップ後に得られた最初の読み取り値が無効であることをチップ データシートが示しているため、最初のデータ セットを破棄します
      averageCO2ppm += SCD41data[i].CO2ppm;

      Serial.printf("val %d > %d ppm\n", i, SCD41data[i].CO2ppm);
    }
  }

  averageCO2ppm = averageCO2ppm / 5;
  Serial.printf("\nave SCD41_CO2:%5d ppm\n", averageCO2ppm);

  Serial.println(F("Sleeping sensor..."));
  SCD4X.setSleepMode(SCD4X_POWER_DOWN);

  return averageCO2ppm;
}

void setup() {
  // M5Stackの初期化
  M5.begin();  // (LCD有効, POWER有効, Serial有効)

  // M5StackのLCD表示設定
  M5.Axp.ScreenBreath(9);      // バックライトの明るさ(0~12),文字が読めるのは7~12
  M5.Lcd.setRotation(1);       // 画面の向きを指定(0～3)
  M5.Lcd.setTextColor(WHITE);  // 文字色設定(color,backgroundcolor)引数2番目省略時は(WHITE, BLACK, RED, GREEN, BLUE, YELLOW...)
  M5.Lcd.fillScreen(BLACK);    // 背景色設定 定数((WHITE, BLACK, RED, GREEN, BLUE, YELLOW...))
  M5.Lcd.setTextFont(2);       // フォントを指定（1～8)1:Adafruit 8ピクセルASCIIフォント、2:16ピクセルASCIIフォント、4:26ピクセルASCIIフォント...)
  M5.Lcd.setTextSize(2);       // 文字の大きさを設定（1～7）
  M5.Lcd.setCursor(0, 0);

  M5.Lcd.println(F("Kikurage Sensor"));

  M5.Lcd.setTextSize(1);
  M5.Lcd.println(F("initialize..."));
  M5.Lcd.print(F("WiFi > "));

  delay(500);

  //　Wi-Fi機能をONにしてネットワークに接続。
  // WiFi.mode(WIFI_STA);             // 無線LANをSTAモードに設定
  WiFi.begin(WIFISSID, WIFIPASS);  // 無線LANに接続
  Serial.println(F("connecting WiFi... "));
  Serial.println(WIFISSID);

  // 接続するまで待機
  uint16_t waiting = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print('.');
    waiting = waiting + 1;

    if (waiting == 600) {  // 60sで接続してない場合は再接続を試みる
      Serial.println(F("WiFi reconnect..."));
      WiFi.disconnect();
      WiFi.reconnect();
    }

    if (waiting >= 1800) {  //タイムアウト:約180s
      Serial.println(F("WiFi Error:TimeOut"));
      Serial.println(F("WiFi Error ESP Reset"));
      delay(1000);
      esp_restart();
    }
  }

  Serial.println(F("\n-------WiFi ON"));
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());  // 本機のIPアドレスをシリアル出力
  M5.Lcd.println(F("ok!"));

  delay(100);

  // チャネルIDとライトキーを指定してAmbientの初期化
  ambient.begin(channelId, writeKey, &client);
  Serial.printf("Ambient > ok! ID: %d\n", channelId);
  M5.Lcd.printf("Ambient > ok! ID: %d\n", channelId);

  // SDC4X initialize
  Serial.print(F("SCD4X > "));
  M5.Lcd.print(F("SCD4X > "));
  delay(200);
  uint16_t i = 0;

  while (!SCD4X.begin()) {
    Serial.print('.');
    Serial.println(F("Communication with device failed, please check connection"));

    if (i > 5) {  // 接続できないときはリセット
      Serial.println(F("SHT35 initialize Error > ESP Reset"));
      delay(2000);
      esp_restart();
    }
    i++;
    delay(3000);
  }

  SCD4X.enablePeriodMeasure(SCD4X_STOP_PERIODIC_MEASURE);
  if (0 != SCD4X.performSelfTest()) {
    Serial.println(F("Malfunction detected!"));
    delay(2000);
    esp_restart();
  }
  Serial.println(F("ok!"));
  M5.Lcd.println(F("ok!"));

  //  SHT35 initialize
  Serial.print(F("SHT35 > "));
  M5.Lcd.print(F("SHT35 > "));
  delay(500);

  if (sensor.init()) {  // 接続できないときはリセット
    Serial.println(F("sensor init failed!!!"));
    Serial.println(F("SHT35 initialize Error > ESP Reset"));
    delay(2000);
    esp_restart();
  }

  Serial.println(F("ok!"));
  M5.Lcd.println(F("ok!"));

  Serial.println(F("Start!"));
  M5.Lcd.println(F("Start!"));
}


void loop() {
  uint32_t startTime = micros();  // 開始時間保存

  // CO2濃度を取得
  uint32_t co2Val = SCD4X_CO2Val();
  Serial.printf("SCD41_CO2 : %d ppm\n", co2Val);

  //温湿度計SHT31情報取得
  float temp = -100.0;
  float hum = -100.0;

  u16 value = 0;
  u8 data[6] = { 0 };

  if (NO_ERROR != sensor.read_meas_data_single_shot(HIGH_REP_WITH_STRCH, &temp, &hum)) {
    Serial.println(F("SHT35 read temp failed!!"));  //温湿度が異常の場合の表示
    delay(500);

    // エラーだったときはもう一度データ取得を試みる。
    temp = -100.0, hum = -100.0;
    sensor.read_meas_data_single_shot(HIGH_REP_WITH_STRCH, &temp, &hum);
  }

  Serial.printf("SHT35_Temp: %3.1f C\n", temp);
  Serial.printf("SHT35_Hum : %3.1f RH\n", hum);


  // 無線が切れていたらマイコンをリセットさせ再接続させる。
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("WiFi Err ESP Reset"));
    delay(2000);
    esp_restart();
  }

  // WiFiがONの場合は温度、湿度の値をAmbientに送信する。
  sendAmbient(temp, hum, co2Val);

  lcdUpdate(temp, hum, co2Val);  // 画面更新の関数呼び出し

  uint32_t processingTime = (micros() - startTime) / 1000;
  Serial.printf("processingTime: %d ms\n", processingTime);
  delay(DelayTime_ms - processingTime);  // Loopの頻度
}
