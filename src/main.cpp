// https://www.suke-blog.com/cub_ecu_emulator/

/**
* PGM-FI Emulator
*/

#include <Arduino.h>

#define SERIAL_RATE 9600
#define SERIAL_TIMEOUT 1000
 
#define SENSOR_O2 3
#define SENSOR_THL 5
#define SENSOR_TA 6
#define SENSOR_TO 9
#define SENSOR_PB 10

#define PCP 8
 
const uint8_t analogOutPins[] = {3,5,6,9,10,11};

uint16_t RPM = 1700;
uint16_t pulseHigh;
uint16_t pulseLow;

// forDebug
static FILE uartout;
static int uart_putchar(char c, FILE *stream) {
  if(Serial.write(c) > 0) {
    return 0;
  } else {
    return -1;
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
  pinMode(PCP, OUTPUT); //CrankPulse
  
  // sensor initial value
  analogWrite(SENSOR_O2, 25); //0-1v(0-51)
  analogWrite(SENSOR_THL, 22); //0.4-4.8V(21-245)
  analogWrite(SENSOR_TA, 138); //2.7-3.1V(138-158)
  analogWrite(SENSOR_TO, 138); //2.7-3.1V(138-158)
  analogWrite(SENSOR_PB, 138); //2.7-3.1V(138-158)
  
  pulseHigh = 10000000 / 12 / RPM;        //pulseHigh = 60 / RPM * 1000 / 12 * (5 / 30) * 1000;
  pulseLow = 5000000 / RPM - pulseHigh;   //pulseLow = (60 / RPM * 1000 / 12) * 1000 - pulseHigh;
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
      }
      delayMicroseconds(pulseHigh);
 
      if(i < 9){
        digitalWrite(PCP, LOW);
      }
      delayMicroseconds(pulseLow);
    }
  }
}