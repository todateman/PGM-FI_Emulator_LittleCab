// https://www.suke-blog.com/cub_ecu_emulator/

/**
* PGM-FI Emulator
*/

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Arduino.h>
#include <SPI.h>

#define SERIAL_RATE 9600
#define SERIAL_TIMEOUT 1000
 
#define SENSOR_O2 3
#define SENSOR_THL 5
#define SENSOR_TA 6
// #define SENSOR_TO 9
// #define SENSOR_PB 10
#define SENSOR_TO A2
#define SENSOR_PB A3

// #define PCP 8
#define PCP A1

const int Volune = A0;
 
//const uint8_t analogOutPins[] = {3,5,6,9,10,11};
const uint8_t analogOutPins[] = {3,5,6,A2,A3,A4};

uint16_t RPM = 1700;
uint16_t pulseHigh;
uint16_t pulseLow;

// SPI
#define TFT_CS        10
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// forDebug
static FILE uartout;
static int uart_putchar(char c, FILE *stream) {
  if(Serial.write(c) > 0) {
    return 0;
  } else {
    return -1;
  }
}

void RPM_Calc(){
  pulseHigh = 10000000 / 12 / RPM;        //pulseHigh = 60 / RPM * 1000 / 12 * (5 / 30) * 1000;
  pulseLow = 5000000 / RPM - pulseHigh;   //pulseLow = (60 / RPM * 1000 / 12) * 1000 - pulseHigh;

  Serial.print("RPM: ");
  Serial.println(RPM);

  // LCD Display
  tft.fillScreen(BLACK);
  tft.setCursor(0, 80);
  tft.setTextColor(RED);
  tft.setTextSize(10);
  tft.println(RPM);
  tft.drawRect(10, 200, 220, 30, WHITE);
  tft.fillRect(10, 200, map(RPM, 0, 9900, 0, 220), 30, WHITE);
}

void Volune_Tune(){
  digitalWrite(PCP, LOW);
  digitalWrite(LED_BUILTIN, LOW);

  uint16_t Value_HTL = analogRead( Volune );
  RPM = map(Value_HTL, 0, 1023, 3, 99)*100;  // delayMicrosecondsが16383usec以上は正常に動かないため、下限を300rpm(OFF時間13889usec)に制限
  RPM_Calc();

  delay(100);
}

void setup(){
  //forDebug
  fdev_setup_stream(&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &uartout;
  
  // config serial
  Serial.begin(SERIAL_RATE, SERIAL_8N1);
  Serial.setTimeout(SERIAL_TIMEOUT);

  // LCD Setting
  //tft.init(tft.width(), tft.height());            // Init ST7789 170x320
  tft.init(tft.width(), tft.height(), SPI_MODE2);   // Init ST7789 240x240
  tft.setRotation(3);            // 画面回転
  tft.setTextWrap(false);        // 行の折り返し無効
  
  // config output pin
  for(uint8_t i = 0; i<sizeof(analogOutPins); i++){
    pinMode(analogOutPins[i], OUTPUT);
  }
  pinMode(PCP, OUTPUT); // CrankPulse
  pinMode(2, INPUT_PULLUP);  // RPM_Tune_ON
  
  // sensor initial value
  analogWrite(SENSOR_O2, 25); //0-1v(0-51)
  analogWrite(SENSOR_THL, 22); //0.4-4.8V(21-245)
  analogWrite(SENSOR_TA, 138); //2.7-3.1V(138-158)
  analogWrite(SENSOR_TO, 138); //2.7-3.1V(138-158)
  analogWrite(SENSOR_PB, 138); //2.7-3.1V(138-158)
  
  RPM_Calc();

  attachInterrupt(digitalPinToInterrupt(2), Volune_Tune, LOW);
}

void loop(){

  uint8_t i = 0;
  //pulseHigh = 500;  //1700RPM
  //pulseLow = 2500;  //1700RPM

  //generate crank pulse
  while(1){
    for(i=0; i<12; i++){
      if(i < 9){
        digitalWrite(PCP, HIGH);
        digitalWrite(LED_BUILTIN, HIGH);
      }
      delayMicroseconds(pulseHigh);
 
      if(i < 9){
        digitalWrite(PCP, LOW);
        digitalWrite(LED_BUILTIN, LOW);
      }
      delayMicroseconds(pulseLow);
    }
  }
}