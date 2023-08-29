// https://www.suke-blog.com/cub_ecu_emulator/

/**
* PGM-FI Emulator
*/
#include <Arduino.h>

// マルチタスク
#include <Arduino_FreeRTOS.h>
void TaskAnalogRead( void *pvParameters );
void TaskSignal( void *pvParameters );
// キュー
#include <queue.h>
QueueHandle_t integerQueue;

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

//volatile uint16_t RPM = 1700;
//volatile uint16_t pulseHigh;
//volatile uint16_t pulseLow;

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
  //pinMode(PCP, OUTPUT); //CrankPulse
  
  // sensor initial value
  analogWrite(SENSOR_O2, 25); //0-1v(0-51)
  analogWrite(SENSOR_THL, 22); //0.4-4.8V(21-245)
  analogWrite(SENSOR_TA, 138); //2.7-3.1V(138-158)
  analogWrite(SENSOR_TO, 138); //2.7-3.1V(138-158)
  analogWrite(SENSOR_PB, 138); //2.7-3.1V(138-158)
  
  
  integerQueue = xQueueCreate(1, // Queue length
                              sizeof(int) // Queue item size
                              );

  if (integerQueue != NULL) {
    xTaskCreate(TaskAnalogRead, // Task function
                "AnalogRead", // Task name
                128,  // Stack size
                NULL, 
                1, // Priority
                NULL);
    xTaskCreate(TaskSignal, // Task function
                "Signal", // Task name
                128, // Stack size 
                NULL, 
                2, // Priority
                NULL );
  }
}

void TaskAnalogRead(void *pvParameters) {
  (void) pvParameters;

  Serial.begin(SERIAL_RATE);
  while (!Serial) {
    vTaskDelay(1);
  }

  for (;;){
    int sensorValue = analogRead(A0);
    int RPM = map(sensorValue, 0, 1023, 1, 100) * 100;
    Serial.println(RPM);

    xQueueOverwrite(integerQueue, &RPM);

    // One tick delay (15ms) in between reads for stability
    vTaskDelay(1);
  }
}

void TaskSignal(void *pvParameters) {
  (void) pvParameters;

  pinMode(PCP, OUTPUT); //CrankPulse
  int RPM = 0;
  //portTickType xLastWakeTime;
  //xLastWakeTime = xTaskGetTickCount();
  for (;;){
    uint8_t i = 0;
    if (xQueuePeek(integerQueue, &RPM, portMAX_DELAY) == pdPASS) {
      int pulseHigh = 10000 / 12 / RPM * 1000;        //pulseHigh = 60 / RPM * 1000 / 12 * (5 / 30);
      int pulseLow = 5000 / RPM * 1000 - pulseHigh;   //pulseLow = (60 / RPM * 1000 / 12) - pulseHigh;
      
      for(i=0; i<12; i++){
        if(i < 9){
          digitalWrite(PCP, HIGH);
        }
        //vTaskDelayUntil(&xLastWakeTime, pulseHigh / portTICK_PERIOD_MS);
        vTaskDelay( pulseHigh / portTICK_PERIOD_MS ); // wait
 
        if(i < 9){
          digitalWrite(PCP, LOW);
        }
        //vTaskDelayUntil(&xLastWakeTime, pulseLow / portTICK_PERIOD_MS);
        vTaskDelay( pulseLow / portTICK_PERIOD_MS ); // wait
      }
    }
  }
}

void loop() {}