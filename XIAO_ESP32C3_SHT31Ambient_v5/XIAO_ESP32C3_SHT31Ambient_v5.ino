/**************************************
  温湿度とCO2のセンサから取得した情報をAmbientサーバーにデータを送る　　　
 **************************************/

// XIAO_ESP32C3

// Anbient
#include <WiFi.h>
#include "Ambient.h"
#include <Arduino.h>

//"https://github.com/adafruit/Adafruit_SHT4X/blob/master/examples/SHT4test/SHT4test.ino"
#include "Adafruit_SHT4x.h"
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

// SCD40,SCD41(https://github.com/DFRobot/DFRobot_SCD4X)
#include <DFRobot_SCD4X.h>
DFRobot_SCD4X SCD4X(&Wire, /*i2cAddr = */ SCD4X_I2C_ADDR);  //'0x62'

//WiFi
WiFiClient client;

// //ルーター設定
#define WIFISSID "Rakuten-49D9"                                                      // ルーターのSSID
#define WIFIPASS "7a6c1ec75136357af3e9b5228b303cf81403855e4c2e55bc757281d8b118f5fb"  //PSK ハッシュ値256bitのキー(https://mascii.github.io/wpa-psk-calc/)
// #define WIFISSID "Buffalo-G-A240"
// #define WIFIPASS "973e5dbed444a545626dfdbc3ad041514ba2c4cd5e781220e7b0d68ab310e399"

//Ambient
Ambient ambient;
uint32_t channelId = 63513;                 // AmbientのチャネルID 弘源堂1
const char* writeKey = "83c534ad38a2f650";  // ライトキー

// unsigned int channelId = 64437;             // AmbientのチャネルID  弘源堂2
// const char* writeKey = "65ccc35e89245be3";  // ライトキー

#define TIME_TO_SLEEP 900  // 測定周期（秒）
// #define TIME_TO_SLEEP 180  // 測定周期（秒）

uint32_t SCD4X_CO2Val() {
  // SDC4X initialize
  Serial.print(F("SCD4X > "));
  uint16_t i = 0;

  while (!SCD4X.begin()) {
    // Serial.print('.');
    // Serial.println(F("Communication with device failed, please check connection"));

    if (i > 5) {  // 接続できないときはリセット
      // Serial.println(F("SHT35 initialize Error > ESP Reset"));
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

  // CO2センサSCD4Xを立ち上げ、センサの値を5回取得した平均を返す。
  Serial.println("Waking sensor...");
  SCD4X.setSleepMode(SCD4X_WAKE_UP);

  DFRobot_SCD4X::sSensorMeasurement_t SCD4Xdata[6];
  memset(SCD4Xdata, 0, sizeof(SCD4Xdata));

  // Serial.println("Measuring...");

  // 4回の平均をとる。
  uint32_t averageCO2ppm = 0;

  for (i = 1; i <= 5; i++) {
    SCD4X.measureSingleShot(SCD4X_MEASURE_SINGLE_SHOT);
    while (!SCD4X.getDataReadyStatus()) {
      delay(100);
    }

    SCD4X.readMeasurement(&SCD4Xdata[i]);

    if (i >= 3) {  // ウェイクアップ後に得られた最初の読み取り値が無効であることをチップ データシートが示しているため、最初のデータ セットを破棄します
      averageCO2ppm += SCD4Xdata[i].CO2ppm;
      // Serial.printf("val %d > %d ppm\n", i, SCD4Xdata[i].CO2ppm);
    }
  }

  averageCO2ppm = averageCO2ppm / 3;
  Serial.printf("\nave SCD41_CO2:%5d ppm\n", averageCO2ppm);
  Serial.println(F("Sleeping sensor..."));
  SCD4X.setSleepMode(SCD4X_POWER_DOWN);

  return averageCO2ppm;
}

void setup() {
  uint64_t startTime = micros();  // 開始時間保存

  delay(500);

  Serial.begin(115200);
  //  while (!Serial) {
  //    delay(50);
  //  }

  // Serial.println("Adafruit SHT4x Bigin");
  if (!sht4.begin()) {
    // Serial.println("Couldn't find SHT4x");
    delay(2000);
    esp_restart();
  }
  sht4.readSerial();

  Serial.println("Found SHT4x sensor");
  Serial.print("Serial number 0x");
  Serial.println(sht4.readSerial(), HEX);

  sht4.setPrecision(SHT4X_HIGH_PRECISION);

  sht4.setHeater(SHT4X_NO_HEATER);
  switch (sht4.getHeater()) {
    case SHT4X_NO_HEATER:
      Serial.println("No heater");
      break;
    case SHT4X_HIGH_HEATER_1S:
      Serial.println("High heat for 1 second");
      break;
    case SHT4X_HIGH_HEATER_100MS:
      Serial.println("High heat for 0.1 second");
      break;
    case SHT4X_MED_HEATER_1S:
      Serial.println("Medium heat for 1 second");
      break;
    case SHT4X_MED_HEATER_100MS:
      Serial.println("Medium heat for 0.1 second");
      break;
    case SHT4X_LOW_HEATER_1S:
      Serial.println("Low heat for 1 second");
      break;
    case SHT4X_LOW_HEATER_100MS:
      Serial.println("Low heat for 0.1 second");
      break;
  }

  //温湿度計SHT4x情報取得
  sensors_event_t humidity, temp;

  sht4.getEvent(&humidity, &temp);
  Serial.printf("SHT4X_Temperature: %3.1f C\n", temp.temperature);
  Serial.printf("SHT4X_Humidity   : %3.1f RH\n", humidity.relative_humidity);

  // CO2濃度を取得
  int co2Val = SCD4X_CO2Val();
  Serial.printf("SCD4X_CO2 : %d ppm\n", co2Val);

  //　Wi-Fi機能をONにしてネットワークに接続。
  WiFi.mode(WIFI_STA);             // 無線LANをSTAモードに設定
  WiFi.begin(WIFISSID, WIFIPASS);  // 無線LANに接続
  // Serial.println(F("connecting WiFi... "));
  // Serial.println(WIFISSID);

  // 接続するまで待機
  uint16_t waiting = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    // Serial.print('.');
    waiting = waiting + 1;

    if (waiting == 600) {  // 60sで接続してない場合は再接続を試みる
      Serial.println(F("WiFi reconnect..."));
      WiFi.disconnect();
      delay(50);
      WiFi.reconnect();
    }

    if (waiting >= 1800) {  //タイムアウト:約180s
      Serial.println(F("WiFi TimeOut Error ESP Reset"));
      delay(200);
      esp_restart();
    }
  }

  Serial.println(F("\nWiFi ON"));
  // Serial.print(F("IP address: "));
  // Serial.println(WiFi.localIP());  // 本機のIPアドレスをシリアル出力

  // WiFiがONの場合に温度、湿度の値をAmbientに送信する。
  if (WiFi.status() != WL_CONNECTED) {

    // Serial.println(F("WiFi Err ESP Reset"));
    delay(5000);
    esp_restart();
  }

  ambient.begin(channelId, writeKey, &client);  // チャネルIDとライトキーを指定してAmbientの初期化
  // Serial.printf("Ambient > ok! ID: %d\n", channelId);

  if (WiFi.status() == WL_CONNECTED) {

    ambient.set(1, temp.temperature);
    ambient.set(2, humidity.relative_humidity);
    ambient.set(3, co2Val);

    bool AmbientConnectionFlg = ambient.send();

    if (AmbientConnectionFlg) {
      Serial.println(F("Ambient Send"));
      Serial.printf("channel ID: %d\n", channelId);
    } else {
      Serial.println(F("Server connection error"));
    }
  }
  delay(300);

  WiFi.mode(WIFI_OFF);

  // Deep sleepする時間（μs）を計算する
  uint64_t sleeptime = TIME_TO_SLEEP * 1000000 - (micros() - startTime);
  Serial.printf("sleepTime: %d us\n", sleeptime);
  esp_sleep_enable_timer_wakeup(sleeptime);  // DeepSleepモードに移行
  esp_deep_sleep_start();

  while (1) {
    delay(1);
  }
}

void loop() {
}
