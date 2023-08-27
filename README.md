# PGM-FI_Emulator_LittleCab

https://www.suke-blog.com/cub_ecu_emulator/ を外部からボリュームで回転数を変更、USBシリアルとI2C接続のディスプレイ（例えばhttps://www.amazon.co.jp/dp/B088FLH7DG/)に表示できるよう手を加えました。
※I2Cを利用する場合、Arduino NanoはATmega168搭載の場合Flash容量不足で書き込みできません。I2C接続に関する部分をコメントアウトするか、ATmega328P搭載品を使用してください。
