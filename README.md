# **１．概要**

CO2濃度、温度、湿度を15分単位で測定し、Ambientサーバーに保存する環境センサー。

Ambientサーバーに保存後、スマホなどのモバイル端末で確認できる。


# **２．システム構成**
Ambientサーバー　ー　インターネット　ー　WiFiルーター　センサ送信機(M5StickCPlus)　－　SHT35モジュール
　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　∟　SCD41モジュール


# **3．セットアップ方法**
ArduinoIDEで以下のプログラムを開き、以下を設定する。


SHT35(https://github.com/Seeed-Studio/Seeed_SHT35)

SCD41(https://github.com/DFRobot/DFRobot_SCD4X)

WiFiルーターのSSIDとパスワードを入力。

M5StatikC_SHT35Ambient_v4.ino

30行目　#define WIFISSID "***************"  // ルーターのSSID
31行目　#define WIFIPASS "***************"  // ルーターのパスワード
