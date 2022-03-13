# M5Stack_SG90_StackChan_Unified_BTSPK_MultiDisp_LED
M5Stack_SG90_StackChan_Unified_BTSPK_MultiDisp_LED

OLED/LCD表示、LEDレベルメーター付きBluetooth Speaker版Mｽﾀｯｸﾁｬﾝファームです。<br>
@mongonta555 さんの[ｽﾀｯｸﾁｬﾝ M5GoBottom版組み立てキット](https://raspberrypi.mongonta.com/about-products-stackchan-m5gobottom-version/ "Title")に対応しています。<br>

Avatar表示は、meganetaaanさんのm5stack-avatarをベースにさせていただきました。<br>
オリジナルはこちら。<br>
An M5Stack library for rendering avatar faces <https://github.com/meganetaaan/m5stack-avator><br>

---

### M5GoBottom版ｽﾀｯｸﾁｬﾝ本体を作るのに必要な物、及び作り方 ###
こちらを参照してください。<br>
* [ｽﾀｯｸﾁｬﾝ M5GoBottom版組み立てキット](https://raspberrypi.mongonta.com/about-products-stackchan-m5gobottom-version/ "Title")<br>

### このプログラムを動かすのに必要な物 ###
* [M5Stack](http://www.m5stack.com/ "Title") (M5Stack Core2 for AWSで動作確認をしました。)<br>
* [OLEDディスプレイユニット](https://www.switch-science.com/catalog/7233/ "Title") または [ LCDディスプレイユニット](https://www.switch-science.com/catalog/7358/ "Title")
* Arduino IDE (バージョン 1.8.15で動作確認をしました。)<br>
* [M5Unified](https://github.com/m5stack/M5Unified/tree/develop/ "Title")ライブラリ((developバージョンで動作確認をしました。))<br>
* [M5GFX](https://github.com/m5stack/M5GFX/tree/develop/ "Title")ライブラリ(バージョン 0.0.18で動作確認をしました。)<br>
* [ServoEasing](https://github.com/ArminJo/ServoEasing/ "Title")ライブラリ(バージョン 2.4.0で動作確認をしました。)<br>
* [ESP32Servo](https://github.com/madhephaestus/ESP32Servo/ "Title")ライブラリ(バージョン 0.9.0で動作確認をしました。)<br>
* [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP/ "Title")ライブラリ<br>
* [FastLED](https://github.com/FastLED/FastLED/ "Title")ライブラリ<br>
<br>

---
### M5Stack-SD-Updaterに対応させる場合 ###

* [M5Stack-SD-Updater](https://github.com/tobozo/M5Stack-SD-Updater/ "Title")ライブラリが必要です。<br>
* "M5Stack_SG90_StackChan_Unified_BTSPK_MultiDisp_LED.ino"の28行目のコメントを外して"#define USE_SD_UPDATER"を有効にします。<br>

M5Stack-SD-Updaterの使い方はこちらを参照してください。：<https://github.com/tobozo/M5Stack-SD-Updater>


---

### 使い方 ###
* スマホなどでペアリングを選択すると"ESP32"というデバイスが表示されるので選択して接続してください。
<br><br>
<br><br>
