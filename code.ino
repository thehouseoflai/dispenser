
#include <Servo.h> 
#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include <services.h>

#define TIMEOUT 10000 // global state timeout in ms

// RFID //
#define enablePin  10   // Connects to the RFID's ENABLE pin

#define BUFSIZE    11  // Size of receive buffer (in bytes) (10-byte unique ID + null character)

#define RFID_START  0x0A  // RFID Reader Start and Stop bytes
#define RFID_STOP   0x0D

// SERVO //
Servo myservo;

#define servoPin      7
#define STOP         95
#define GO          105 // at 5V, gives one rev/~4.5 seconds
#define CYCLE_TIME  744 // milliseconds for 1/6 rev

#define DEBUG

char rfidData[BUFSIZE];  // Buffer for incoming data
unsigned char offset = 0;         // Offset into buffer

enum STATE {
  starting,
  rfid_wait,
  rfid_recv,
  ble_wait,
  ble_send,
  ble_recv,
  servo_go
} state = starting;

enum STATE last_state; // for timeout checking

String state_msgs[] = {
  "starting",
  "rfid_wait",
  "rfid_recv",
  "ble_wait",
  "ble_send",
  "ble_recv",
  "servo_go"
};

unsigned long last_time;

void set_state( enum STATE new_state ) {
  last_time = millis();
  state = new_state;
#ifdef DEBUG
  Serial.println(state_msgs[(int)state]);
#endif
}

void setup() 
{
  set_state(starting);
  // init RFID
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH); // disable
 
  Serial.begin(2400);

  // init Bluetooth
  //ble_set_pins(9, 8); // default
  ble_set_name("Dispenser");
  ble_begin();

  // attach servo and stop motor
  myservo.attach(servoPin);
  myservo.write(STOP);

  set_state(rfid_wait);
}

void dispense()
{
  myservo.write(GO);
  delay(CYCLE_TIME);
  myservo.write(STOP);
  delay(500);
  rfidData[0] = 0;         // Clear the buffer   
}

int response = 0; // value received from BLE client

void loop()
{
  last_state = state;
  switch (state) {
    case starting:
      setup();
      //fall through

    case rfid_wait:
      // waiting for RFID reader to do something
      digitalWrite(enablePin, LOW);   // enable the RFID Reader
    
      // if there's data, receive it
      if (Serial.available() > 0) {
        offset = 0;
        digitalWrite(enablePin, HIGH); // stop reading
        set_state(rfid_recv);
      }
      break;
  
    case rfid_recv:
      if (Serial.available() == 0)
        break;

      // get a byte from the reader
      rfidData[offset] = Serial.read();

      if (rfidData[offset] == RFID_START) {

        offset = 0;

      } else if (rfidData[offset] == RFID_STOP) {

        rfidData[offset] = 0; // Null terminate

        // wait for BLE so we can push data
#ifdef DEBUG
        Serial.println(rfidData);
#endif
        set_state(ble_wait);

      } else { // it's a data byte

        offset++;  // Increment offset into array
        if (offset >= BUFSIZE) offset = 0; // If the incoming data string is longer than our buffer, wrap around to avoid going out-of-bounds

      }

      break;

    case ble_wait:
      //busy-wait for BLE to connect
      if (ble_connected() ) {
        set_state(ble_send);
      } else {
        delay(100);
      }
      break;

    case ble_send:
      for(offset = 0; offset < BUFSIZE; offset++) {
        ble_write(rfidData[offset]);
      }
      set_state(ble_recv);
      // fall through

    case ble_recv:
      if ( ble_available() ) {
        // we will get back a single int
        response = ble_read();
        if ( response > 0 ) { // this is the actual protocol
          set_state(servo_go);
        } else {
          set_state(rfid_wait);
        }
      } else {
        delay(100);
      }
      break;

    case servo_go:
      dispense();
      set_state(rfid_wait);
      break;

  } // switch state

  // check timeout
  if (   (last_state == state)
      && (state != rfid_wait)
      && (millis() > last_time + TIMEOUT)) {

    Serial.println("timeout");
    set_state(rfid_wait);

  }

  ble_do_events(); // let the BLE stack run
}


