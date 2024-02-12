# **１．概要**

CO2濃度、温度、湿度を15分単位で測定し、Ambientサーバーに保存する環境センサー。

Ambientサーバーに保存後、スマホなどのモバイル端末で確認できる。


# **２．システム構成**
Ambientサーバー　ー　インターネット　ー　WiFiルーター　センサ送信機(M5StickCPlus)　－　SHT35モジュール

                                                                                 ∟　SCD41モジュール

![kankyoSenser](https://github.com/km190112/kikurageKankyouSenseer/assets/46617422/76a97617-11c4-4df0-ac96-0c0b757eb553)


# **3．プログラムの書き込み(M5StickCPlus)**
ArduinoIDEで以下のプログラム(M5StatikC_SHT35Ambient_v4.ino)を開き、以下を設定する。

使用するライブラリ

SHT35(https://github.com/Seeed-Studio/Seeed_SHT35)

SCD41(https://github.com/DFRobot/DFRobot_SCD4X)

Ambient(https://github.com/AmbientDataInc/Ambient_ESP8266_lib)


WiFiルーターのSSIDとパスワードを入力。

30行目　#define WIFISSID "***************"  // ルーターのSSID

31行目　#define WIFIPASS "***************"  // ルーターのパスワード


Ambientのアカウント作成とチャネルIDを取得し以下を設定

35行目　uint32_t channelId = *****;                 // AmbientのチャネルID

36行目　const char* writeKey = "****************";  // AmbientのチャネルIDのライトキー


# **3．H/W制作**

(1)用意するもの

【センシング用】

・XIAO_ESP32C3

・SHT35(温湿度センサー)

・SCD41(CO2センサー)

・GROVE - 4ピンケーブル 50cm

・2ポート USB充電器(サンワサプライ,ACA-IP44W)

・UVレジン(センサー金属部の錆防止用、コーティング用)

・空調服ファン 5V

【防水用ケースなど】

・埋込コンセント(Panasonic,WN1001SW)

・PVKボックス(未来工業,PVK-BLNJ)

・塩ビパイプ 呼び径：100

・塩ビパイプ 呼び径：50

・塩ビ VU90°エルボ　呼び径：100

・塩ビ VU90°エルボ　呼び径：50

・VUインクリーザー　呼び径：100×50
