
#include <Servo.h> 
#include <SoftwareSerial.h>

// RFID //
#define enablePin  10   // Connects to the RFID's ENABLE pin
#define rxPin      2  // Serial input (connects to the RFID's SOUT pin)
#define txPin      12  // Serial output (unused)

#define BUFSIZE    11  // Size of receive buffer (in bytes) (10-byte unique ID + null character)

#define RFID_START  0x0A  // RFID Reader Start and Stop bytes
#define RFID_STOP   0x0D

// SERVO //
Servo myservo;

#define servoPin      9
#define STOP         95
#define GO          105 // at 5V, gives one rev/~4.5 seconds
#define CYCLE_TIME  744 // milliseconds for 1/6 rev

char *ALLOWED = "0F03042223";

char rfidData[BUFSIZE];  // Buffer for incoming data
char offset = 0;         // Offset into buffer

void setup() 
{
  // init RFID
  pinMode(enablePin, OUTPUT);
  pinMode(rxPin, INPUT);
  digitalWrite(enablePin, HIGH); // disable
 
  // attach servo and stop motor
  myservo.attach(servoPin);
  myservo.write(STOP);

//  Serial.begin(9600);
  Serial.begin(2400);
  while(!Serial) ;
  Serial.print("READY\n");
  delay(300);
}

void dispense()
{
  Serial.print("dispensing...");
  myservo.write(GO);
  delay(CYCLE_TIME);
  myservo.write(STOP);
  Serial.print("done.\n");
  delay(500);
  rfidData[0] = 0;         // Clear the buffer   
}

void loop()
{
  digitalWrite(enablePin, LOW);   // enable the RFID Reader
  
  // Wait for a response from the RFID Reader
  // See Arduino readBytesUntil() as an alternative solution to read data from the reader

if (Serial.available() > 0) // If there are any bytes available to read, then the RFID Reader has probably seen a valid tag
  {
    rfidData[offset] = Serial.read();  // Get the byte and store it in our buffer
    if (rfidData[offset] == RFID_START)    // If we receive the start byte from the RFID Reader, then get ready to receive the tag's unique ID
    {
      offset = -1;     // Clear offset (will be incremented back to 0 at the end of the loop)
    }
    else if (rfidData[offset] == RFID_STOP)  // If we receive the stop byte from the RFID Reader, then the tag's entire unique ID has been sent
    {
      rfidData[offset] = 0; // Null terminate the string of bytes we just received
      receive_string();
      // reset the state
      offset = -1;
      rfidData[0] = 0;
    }
        
    offset++;  // Increment offset into array
    if (offset >= BUFSIZE) offset = 0; // If the incoming data string is longer than our buffer, wrap around to avoid going out-of-bounds
  }
}

void receive_string() {
  digitalWrite(enablePin, HIGH);
  Serial.println(rfidData);       // The rfidData string should now contain the tag's unique ID with a null termination, so display it on the Serial Monitor
  if (strcmp(ALLOWED, rfidData) == 0) {
    Serial.println("ALLOWED");
    dispense();
  } else {
    Serial.println("DENIED");
  }
  delay(1000);
}
