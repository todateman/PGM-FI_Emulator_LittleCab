// https://www.suke-blog.com/cub_ecu_emulator/

/**
* PGM-FI Emulator
*/
 
#include <Arduino.h>

// マルチタスク
#include <Arduino_FreeRTOS.h>
void PCPOUT( void *pvParameters );	// 信号出力用ポインタ
void DISPOUT( void *pvParameters );	// 表示用ポインタ

// I2c外部ディスプレイの設定(A4:SDA A5:SCL)
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
 // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SERIAL_RATE 9600
#define SERIAL_TIMEOUT 1000
 
#define SENSOR_O2 3
#define SENSOR_THL 5
#define SENSOR_TA 6
#define SENSOR_TO 9
#define SENSOR_PB 10
const int Volume_THL = A0;

#define PCP 8
 
const uint8_t analogOutPins[] = {3,5,6,9,10,11};

volatile uint16_t RPM = 1700;
volatile uint16_t pulseHigh;
volatile uint16_t pulseLow;

// forDebug
static FILE uartout;
static int uart_putchar(char c, FILE *stream) {
  if(Serial.write(c) > 0) {
    return 0;
  } else {
    return -1;
  }
}
 

void PCPOUT(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  pinMode(PCP, OUTPUT); //CrankPulse
  while(1){
    volatile uint8_t i = 0;
    for(i=0; i<12; i++){
      if(i < 9){
        digitalWrite(PCP, HIGH);
      }
      vTaskDelay( pulseHigh / portTICK_PERIOD_MS ); // wait
 
      if(i < 9){
        digitalWrite(PCP, LOW);
      }
      vTaskDelay( pulseLow / portTICK_PERIOD_MS ); // wait
    }
  }
}


void DISPOUT(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  while(1){
    volatile uint16_t Value_HTL = analogRead( Volume_THL );
    RPM = map(Value_HTL, 0, 1023, 0, 100) * 100;

    pulseHigh = 10000 / 12 / RPM;        //pulseHigh = 60 / RPM * 1000 / 12 * (5 / 30) * 1000;
    pulseLow = 5000 / RPM - pulseHigh;   //pulseLow = (60 / RPM * 1000 / 12) * 1000 - pulseHigh;
  
    Serial.print(F("rpm: "));
    Serial.println(RPM);

    // I2c
    display.clearDisplay();
    display.setTextSize(2);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.print(RPM);
    display.println(F(" rpm"));
    display.display();
    vTaskDelay( 500 / portTICK_PERIOD_MS );
  }
}

void setup(){
  //forDebug
  fdev_setup_stream(&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &uartout;
  
  // config serial
  Serial.begin(SERIAL_RATE);
  Serial.setTimeout(SERIAL_TIMEOUT);
  
  // config output pin
  for(uint8_t i = 0; i<sizeof(analogOutPins); i++){
    pinMode(analogOutPins[i], OUTPUT);
  }
  //pinMode(PCP, DISPOUT); //CrankPulse
  
  // sensor initial value
  analogWrite(SENSOR_O2, 25); //0-1v(0-51)
  analogWrite(SENSOR_THL, 22); //0.4-4.8V(21-245)
  analogWrite(SENSOR_TA, 138); //2.7-3.1V(138-158)
  analogWrite(SENSOR_TO, 138); //2.7-3.1V(138-158)
  analogWrite(SENSOR_PB, 138); //2.7-3.1V(138-158)
  
  // I2C
  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    delay(3000);
  }

	xTaskCreate(
	  PCPOUT,		      // タスクの関数名
	  "PCPOUT",	      // 名前 テキトーに
		128,				          // 割り当てるメモリサイズ bytes
		NULL,			            // タスクの関数に渡すパラメータへのポインタ 特にパラメータを受け取る処理は書いてないのでNULLで。
		1,	// プライオリティ 0-20 アイドリングよりも3ほど優先させる
	  NULL			  // タスクハンドラを渡して関連付ける
  );
  xTaskCreate(
		DISPOUT,
		"DISPOUT",
	  512,
		NULL,
		2,
		NULL
	);
  vTaskStartScheduler();
}

void loop(){
  delay(1);
/*
  uint8_t i = 0;
  //uint32_t pulseHigh = 500;//1700RPM
  //uint32_t pulseLow = 2500;//1700RPM

  //generate crank pulse
  while(1){
    uint16_t Value_HTL = analogRead( Volume_THL );
    RPM = map(Value_HTL, 0, 1023, 0, 100) * 100;

    pulseHigh = 10000000 / 12 / RPM;        //pulseHigh = 60 / RPM * 1000 / 12 * (5 / 30) * 1000;
    pulseLow = 5000000 / RPM - pulseHigh;   //pulseLow = (60 / RPM * 1000 / 12) * 1000 - pulseHigh;
  
    Serial.print(F("rpm: "));
    Serial.println(RPM);

    // I2c
    display.clearDisplay();
    display.setTextSize(2);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.print(RPM);
    display.println(F(" rpm"));
    display.display();

    for(i=0; i<12; i++){
      if(i < 9){
        digitalWrite(PCP, HIGH);
      }
      delayMicroseconds(pulseHigh);
 
      if(i < 9){
        digitalWrite(PCP, LOW);
      }
      delayMicroseconds(pulseLow);
    }
  }
*/
}