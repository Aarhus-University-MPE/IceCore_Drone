/******************************************************************************
 This is example sketch for Arduino.
 Shows how to control SimpleBGC-driven gimbal via Serial API.
 API specs are available at http://www.basecamelectronics.com/

 Connection:
 Arduino GND -> SimpleBGC GND
 Arduino TX -> SimpleBGC RX

 Power Arduino separatly or via +5V from onboard FTDI connector
 (C) Aleksey Moskalenko
*******************************************************************************/
#include <inttypes.h>
// default is 115200 but may be changed in the Advanced tab of the SimpleBGC GUI
#define SERIAL_SPEED 115200
// delay between commands, ms
#define SBGC_CMD_DELAY 20
// Some definitions required to send commands
#define SBGC_CMD_CONTROL 'C'
#define SBGC_CMD_TRIGGER 'T'
#define SBGC_CONTROL_MODE_SPEED 1
#define SBGC_CONTROL_MODE_ANGLE 2
#define SBGC_CONTROL_MODE_SPEED_ANGLE 3
// Pins that may be triggered
#define SBGC_RC_INPUT_ROLL 1
#define SBGC_RC_INPUT_PITCH 2
#define SBGC_RC_INPUT_EXT_ROLL 3
#define SBGC_RC_INPUT_EXT_PITCH 4
#define SBGC_RC_INPUT_YAW 5 // not connected in 1.0 board
#define SBGC_PIN_AUX1 16
#define SBGC_PIN_AUX2 17
#define SBGC_PIN_AUX3 18
#define SBGC_PIN_BUZZER 32
// Conversion from degree/sec to units that command understand
#define SBGC_SPEED_SCALE (1.0f/0.1220740379f)
#define ANGLE_TO_DEGREE(d) ((float)(d)*0.02197265625f)
#define DEGREE_TO_ANGLE(d) ((int16_t)((float)(d)*(1.0f/0.02197265625f)))
// Holder for command parameters
typedef struct {
 uint8_t mode;
 int16_t speedROLL;
 int16_t angleROLL;
 int16_t speedPITCH;
 int16_t anglePITCH;
 int16_t speedYAW;
 int16_t angleYAW;
} SBGC_cmd_control_data;
typedef struct {
 uint8_t pin;
 int8_t state;
} SBGC_cmd_trigger_data;
// This helper function formats and sends a command to SimpleBGC Serial API
void SBGC_sendCommand(uint8_t cmd, void *data, uint8_t size) {
 uint8_t i, checksum=0;

 // Header
 Serial.write('>');
 Serial.write(cmd);
 Serial.write(size);
 Serial.write(cmd+size);

 // Body
 for(i=0;i<size;i++) {
 checksum+= ((uint8_t*)data)[i];
 Serial.write(((uint8_t*)data)[i]);
 }
 Serial.write(checksum);
}
/*****************************************************************************/
#define LED_ON() { digitalWrite(13, HIGH); }
#define LED_OFF() { digitalWrite(13, LOW); }
void blink_led(uint8_t cnt) {
 for(uint8_t i=0; i<cnt; i++) {
 LED_OFF();
 delay(200);
 LED_ON();
 delay(300);
 }
}
void setup() {
 Serial.begin(SERIAL_SPEED);
 pinMode(13, OUTPUT);
 // Take a pause to let gimbal controller to initialize
 delay(3000);
}
void loop() {
 // LED ON: start demo
 LED_ON();
 SBGC_cmd_control_data c = { 0, 0, 0, 0, 0, 0, 0 };
 SBGC_cmd_trigger_data t = { 0, 0 };
 // Move camera to initial position (all angles are zero)
 // Set speed 30 degree/sec
 c.mode = SBGC_CONTROL_MODE_ANGLE;
 c.speedROLL = c.speedPITCH = c.speedYAW = 30 * SBGC_SPEED_SCALE;
 SBGC_sendCommand(SBGC_CMD_CONTROL, &c, sizeof(c));
 delay(3000);

 Serial.print(SBGC_cmd_get_angle_roll);

 // Demo 1. PITCH and YAW gimbal by 40 and 30 degrees both sides and return back. Actual speed depends on PID setting.
 // Whait 5 sec to finish
 c.mode = SBGC_CONTROL_MODE_ANGLE;
 c.anglePITCH = DEGREE_TO_ANGLE(40);
 c.angleYAW = DEGREE_TO_ANGLE(30);
 SBGC_sendCommand(SBGC_CMD_CONTROL, &c, sizeof(c));
 delay(4000);
 c.anglePITCH = DEGREE_TO_ANGLE(-40);
 c.angleYAW = DEGREE_TO_ANGLE(-30);
 SBGC_sendCommand(SBGC_CMD_CONTROL, &c, sizeof(c));
 delay(8000);
 // .. and back
 c.anglePITCH = 0;
 c.angleYAW = 0;
 SBGC_sendCommand(SBGC_CMD_CONTROL, &c, sizeof(c));
 delay(4000);
 
 /*
 // Demo 2. Pitch gimbal down with constant speed 10 degree/sec by 50 degree (it takes 5 sec)
 // (this is simplified version of speed control. To prevent jerks, you should add acceleration and de-acceleration phase)
 blink_led(2);
 c.mode = SBGC_CONTROL_MODE_SPEED;
 c.speedPITCH = 10 * SBGC_SPEED_SCALE;
 c.speedROLL = 0;
 c.speedYAW = 0;
 SBGC_sendCommand(SBGC_CMD_CONTROL, &c, sizeof(c));
 delay(5000);
 // Stop
 c.speedPITCH = 0;
 SBGC_sendCommand(SBGC_CMD_CONTROL, &c, sizeof(c));
 delay(1000);
 // .. and back

 c.speedPITCH = -10 * SBGC_SPEED_SCALE;
 SBGC_sendCommand(SBGC_CMD_CONTROL, &c, sizeof(c));
 delay(5000);
 // Stop
 c.speedPITCH = 0;
 SBGC_sendCommand(SBGC_CMD_CONTROL, &c, sizeof(c));
 delay(1000);
 // Demo3: Return control back to RC for 5 seconds
 blink_led(3);
 c.mode = 0;
 SBGC_sendCommand(SBGC_CMD_CONTROL, &c, sizeof(c));
 delay(5000);
 // Demo 4. More complicated example: Pitch gimbal by 40 degrees with the full control of speed and angle.
 // - send control command with the fixed frame rate
 // - angle is calculated by the integration of the speed
 blink_led(4);
 float speed = 0, angle = 0;
 c.mode = SBGC_CONTROL_MODE_SPEED_ANGLE;
 // acceleration phase
 while(angle < 20.0f) {
 speed+= 0.5f;
 c.speedPITCH = speed * SBGC_SPEED_SCALE;
 angle+= speed * SBGC_CMD_DELAY / 1000.0f; // degree/sec -> degree/ms
 c.anglePITCH = DEGREE_TO_ANGLE(angle);
 SBGC_sendCommand(SBGC_CMD_CONTROL, &c, sizeof(c));
 delay(SBGC_CMD_DELAY);
 }
 // de-acceleration phase
 while(angle < 40.0f && speed > 0.0f) {
 speed-= 0.5f;
 c.speedPITCH = speed * SBGC_SPEED_SCALE;
 angle+= speed * SBGC_CMD_DELAY / 1000.0f;
 c.anglePITCH = DEGREE_TO_ANGLE(angle);
 SBGC_sendCommand(SBGC_CMD_CONTROL, &c, sizeof(c));
 delay(SBGC_CMD_DELAY);
 }

 // Demo 5: Trigger AUX1 pin state HIGH and make BUZZER to buzz for 0.5 sec
 blink_led(5);
 t.pin = SBGC_PIN_AUX1;
 t.state = 1;
 SBGC_sendCommand(SBGC_CMD_TRIGGER, &t, sizeof(t));
 t.pin = SBGC_PIN_BUZZER;
 t.state = 1;
 SBGC_sendCommand(SBGC_CMD_TRIGGER, &t, sizeof(t));
 delay(500);
 t.pin = SBGC_PIN_AUX1;
 t.state = 0;
 SBGC_sendCommand(SBGC_CMD_TRIGGER, &t, sizeof(t));
 t.pin = SBGC_PIN_BUZZER;
 t.state = 0;
 SBGC_sendCommand(SBGC_CMD_TRIGGER, &t, sizeof(t));
 // Turn off serial control
 c.mode = 0;
 SBGC_sendCommand(SBGC_CMD_CONTROL, &c, sizeof(c));
 LED_OFF();
 */
} 
