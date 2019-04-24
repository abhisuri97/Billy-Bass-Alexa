// include the servo library
#include <Servo.h>

//initializing Servo object
Servo servo1;

const int inPin1 = A1;

const int ledPin = 10;
const int servoPin = 6;
const int lightPin = A5;

int dhtPin = 2;
uint8_t bits[5]; 
#define TIMEOUT 10000

#define DHTLIB_OK                0
#define DHTLIB_ERROR_CHECKSUM   -1
#define DHTLIB_ERROR_TIMEOUT    -2
#define DHTLIB_INVALID_VALUE    -999


//
Servo myservo;  // create servo object to control a servo
//int pos = 0;    // variable to store the servo position
//int factor = -1;
int resetTimeout = 10;
int resetLightTimeout = 20;
int timeout = resetTimeout;
int lightTimeout = resetLightTimeout;
int openPos =  20;
int pos = 0;
int mouthIsOpen = 0;
int previousInRead = 0;
int lightRead = 0;
double humidity = 0.0;
double temperature = 0.0;
int dhtCountdown = 100;
int soundThreshold = 15;

int lightVals[20];
int lightCount = 0;
int lightLow = 0;
void setup(){
  Serial.begin(9600); 
  delay(20);
 
  myservo.attach(servoPin); 
  myservo.write(pos);
  pinMode(ledPin, OUTPUT);
  pinMode(lightPin, INPUT);
}

void loop() {
//
//  //read the analog values from the accelerometer
  int inRead1 = analogRead(inPin1);
  Serial.print(inRead1);
  
  lightVals[lightCount] = analogRead(lightPin);
  lightCount++;
  if (lightCount == 20) {
    lightCount = 0;
    int sum = 0;
    for (int i = 0; i < 20; i++) { sum += lightVals[i]; }
    lightRead = sum/20;
    for (int i = 0; i < 20; i++) { lightVals[i] = 0; }
  }

  Serial.print(" ");
  Serial.print(myservo.read());
  Serial.print(" ");
  Serial.print(lightRead);

  if (dhtCountdown <= 0) {
    int rv = dht_read(dhtPin);
    humidity    = bits[0];  // bit[1] == 0;
    temperature = bits[2];  // bits[3] == 0;
  
    // TEST CHECKSUM
    uint8_t sum = bits[0] + bits[2]; // bits[1] && bits[3] both 0
    if (bits[4] != sum) {
      Serial.println("ERROR, " + sum);
    }
    Serial.print(" "); 
    Serial.print(humidity);
    Serial.print(" "); 
    Serial.println(temperature);
    dhtCountdown = 100;

  }
  Serial.print(" "); 
  Serial.print(humidity);
  Serial.print(" "); 
  Serial.println(temperature);

  
  if (inRead1 > soundThreshold) {
    openMouth();
  } else {
    closeMouth();
  }

  
  timeout--;
  dhtCountdown--;
}

void openMouth() {
  if (timeout <= 0 && mouthIsOpen == 0) {
    pos = openPos;
    mouthIsOpen = 1;
    myservo.write(pos);
    timeout = resetTimeout;
    digitalWrite(ledPin, HIGH);
  }
}

void closeMouth() {
  if (timeout <= 0 && mouthIsOpen == 1) {
    pos = 0;
    mouthIsOpen = 0;
    myservo.write(pos);
    timeout = resetTimeout;
    digitalWrite(ledPin, LOW);
  }
}

int dht_read(uint8_t pin)
{
        // INIT BUFFERVAR TO RECEIVE DATA
        uint8_t cnt = 7;
        uint8_t idx = 0;

        // EMPTY BUFFER
        for (int i=0; i< 5; i++) bits[i] = 0;

        // REQUEST SAMPLE
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        delay(20);
        digitalWrite(pin, HIGH);
        delayMicroseconds(40);
        pinMode(pin, INPUT);

        // GET ACKNOWLEDGE or TIMEOUT
        unsigned int loopCnt = TIMEOUT;
        while(digitalRead(pin) == LOW)
                if (loopCnt-- == 0) return DHTLIB_ERROR_TIMEOUT;

        loopCnt = TIMEOUT;
        while(digitalRead(pin) == HIGH)
                if (loopCnt-- == 0) return DHTLIB_ERROR_TIMEOUT;

        // READ THE OUTPUT - 40 BITS => 5 BYTES
        for (int i=0; i<40; i++)
        {
                loopCnt = TIMEOUT;
                while(digitalRead(pin) == LOW)
                        if (loopCnt-- == 0) return DHTLIB_ERROR_TIMEOUT;

                unsigned long t = micros();

                loopCnt = TIMEOUT;
                while(digitalRead(pin) == HIGH)
                        if (loopCnt-- == 0) return DHTLIB_ERROR_TIMEOUT;

                if ((micros() - t) > 40) bits[idx] |= (1 << cnt);
                if (cnt == 0)   // next byte?
                {
                        cnt = 7;   
                        idx++;      
                }
                else cnt--;
        }

        return DHTLIB_OK;
}
