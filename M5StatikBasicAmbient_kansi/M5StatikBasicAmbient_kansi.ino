/**************************************
  温湿度計SHT35のセンサ情報を取得してAmbientにデータを送る　　　
 **************************************/
// M5StackBasicを利用する。
// #include <M5Stack.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson
#include <Arduino.h>
#include <time.h>
// #include <DateTime.h>

WiFiClient client;  //WiFi

// //ルーター設定
// #define WIFISSID "Rakuten-49D9"                                                      // ルーターのSSID
// #define WIFIPASS "7a6c1ec75136357af3e9b5228b303cf81403855e4c2e55bc757281d8b118f5fb"  //PSK ハッシュ値256bitのキー(https://mascii.github.io/wpa-psk-calc/)
#define WIFISSID "Buffalo-G-A240"
#define WIFIPASS "973e5dbed444a545626dfdbc3ad041514ba2c4cd5e781220e7b0d68ab310e399"

const uint32_t channelId1 = 63513;          // AmbientのチャネルID 弘源堂1
const char* readKey1 = "2f65c47420a8fc03";  // リードキー

const uint32_t channelId2 = 64437;          // AmbientのチャネルID  弘源堂2
const char* readKey2 = "c428ba5a06b66f70";  // リードキー

const int32_t DelayTime_ms = 30000;  // Ambient問い合わせ間隔ms 5分

StaticJsonDocument<194> doc;

// void lcdUpdate() {
//   // LCDにカウント数を表示する関数
//   M5.Lcd.setTextColor(WHITE);  // 文字色設定(color,backgroundcolor)引数2番目省略時は(WHITE, BLACK, RED, GREEN, BLUE, YELLOW...)
//   M5.Lcd.fillScreen(BLACK);    // 背景色設定 定数((WHITE, BLACK, RED, GREEN, BLUE, YELLOW...))
//   M5.Lcd.setTextFont(2);       // フォントを指定（1～8)1:Adafruit 8ピクセルASCIIフォント、2:16ピクセルASCIIフォント、4:26ピクセルASCIIフォント...)
//   M5.Lcd.setTextSize(2);       // 文字の大きさを設定（1～7）
//   M5.Lcd.setCursor(0, 0);

//   M5.Lcd.println("Kikurage Now");

//   //サーバーとの接続状況表示
//   if (WiFi.status() != WL_CONNECTED) {  //WiFiが接続できていない場合の表示
//     M5.Lcd.setTextColor(RED);
//     M5.Lcd.println("OFFLINE");

//     return;
//   }

//   M5.Lcd.setTextColor(BLUE);
//   M5.Lcd.print("ONLINE");

//   M5.Lcd.setTextColor(WHITE);  // 文字色設定(color,backgroundcolor)引数2番目省略時は(WHITE, BLACK, RED, GREEN, BLUE, YELLOW...)
//   M5.Lcd.setTextFont(2);       // フォントを指定（1～8)1:Adafruit 8ピクセルASCIIフォント、2:16ピクセルASCIIフォント、4:26ピクセルASCIIフォント...)
// }

String getAmbientJsonData(const uint32_t ambientChannelId, const char* ambientReadkey) {
  // WiFiがONの場合にAmbientからデータを取得する。
  // 取得するデータは最新の1件

  if (WiFi.status() != WL_CONNECTED) {
    return "";
  }

  u_int8_t n = 1;  //最新の1件を取得

  String url = "http://ambidata.io/api/v2/channels/";
  url += ambientChannelId;
  url += "/data?&readKey=";
  url += ambientReadkey;
  url += "&n=";
  url += n;

  Serial.print("Requesting URL ");
  Serial.println(url);

  HTTPClient httpClient;
  httpClient.begin(url);

  int httpCode = httpClient.GET();
  String httpResponse = httpClient.getString();
  httpClient.end();

  Serial.printf("httpCode:%d\n", httpCode);
  // Serial.println(httpResponse);

  return httpResponse;
}


void setup() {
  Serial.begin(115200);

  // // M5Stackの初期化
  // M5.begin();  // (LCD有効, POWER有効, Serial有効)

  // // M5StackのLCD表示設定
  // M5.Lcd.setBrightness(200);   //バックライトの明るさを0（消灯）～255（点灯）で制御
  // M5.Lcd.setRotation(1);       // 画面の向きを指定(0～3)
  // M5.Lcd.setTextColor(WHITE);  // 文字色設定(color,backgroundcolor)引数2番目省略時は(WHITE, BLACK, RED, GREEN, BLUE, YELLOW...)
  // M5.Lcd.fillScreen(BLACK);    // 背景色設定 定数((WHITE, BLACK, RED, GREEN, BLUE, YELLOW...))
  // M5.Lcd.setTextFont(2);       // フォントを指定（1～8)1:Adafruit 8ピクセルASCIIフォント、2:16ピクセルASCIIフォント、4:26ピクセルASCIIフォント...)
  // M5.Lcd.setTextSize(2);       // 文字の大きさを設定（1～7）
  // M5.Lcd.setCursor(0, 0);

  // M5.Lcd.println(F("Kikurage Sensor"));

  // M5.Lcd.setTextSize(1);
  // M5.Lcd.println(F("initialize..."));
  // M5.Lcd.print(F("WiFi > "));

  delay(50);

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

  // Serial.println(F("\n-------WiFi ON"));
  // Serial.print(F("IP address: "));
  // Serial.println(WiFi.localIP());  // 本機のIPアドレスをシリアル出力
  // M5.Lcd.println(F("ok!"));

  delay(10);

  Serial.println("Connected to the WiFi network!");

  Serial.println(F("Start!"));
  // M5.Lcd.println(F("Start!"));
}


void loop() {
  uint32_t startTime = micros();  // 開始時間保存

  // 無線が切れていたらマイコンをリセットさせ再接続させる。
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("WiFi Err ESP Reset"));
    delay(2000);
    esp_restart();
  }

  String jsonStr = getAmbientJsonData(channelId2, readKey2);  // AmbientからJSONデータを取得する。

  if (jsonStr != "[]") {

    // DynamicJsonDocument doc(194);  // 動的な場合はこっち

    DeserializationError error = deserializeJson(doc, jsonStr);

    serializeJson(doc, Serial);
    Serial.println("");

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());

    } else {

      JsonObject root_0 = doc[0];

      float ch1_temp = root_0["d1"];               // 23.52
      float ch1_hum = root_0["d2"];                // 68.02
      int ch1_co2Val = root_0["d3"];               // 1200
      const char* time_str = root_0["created"];  // "2023-08-01T16:03:58.178Z"

      // 時刻文字列を解析して、time_t型に変換します。
      time_t now = strptime(time_str, "%Y-%m-%dT%H:%M:%S.%fZ", NULL);

      // JSTに変換します。
      time_t jst = now + 9 * 3600;

      // 時刻を表示します。
      char buf[64];
      strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&jst));
      Serial.println(buf);

      // lcdUpdate();  // 画面更新の初期化関数呼び出し

      // M5.Lcd.println("Dt  :" + String(year()) + "/" + month() + "/" + day() + " " + hour() + ":" + minute() + ":" + second());
      // M5.Lcd.printf("Temp:  %3.1f C\n", ch1_temp);
      // M5.Lcd.printf("hum :  %3.1f RH\n", ch1_hum);
      // M5.Lcd.printf("CO2 : %5d ppm\n", ch1_co2Val);
    }
  }


  uint32_t processingTime = (micros() - startTime) / 1000;
  Serial.printf("processingTime: %d ms\n", processingTime);
  delay(DelayTime_ms - processingTime);  // Loopの頻度
}
