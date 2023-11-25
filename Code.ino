#include "functions.h"
#include "pitches.h"

#define LDR_PIN 26

void setup() {
  // put your setup code here, to run once:
  Wire1.setSDA(I2C1_SDA);
  Wire1.setSCL(I2C1_SCL);
  Wire1.begin(); 
  
  oled.begin(&Adafruit128x32, OLED_I2CAADR); 

  Serial.begin(115200); //set up serial library baud rate to 115200
  delay(2000);
  Serial.println("Range Sensor & OLED Test");
  
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(10); // Set BRIGHTNESS to about 1/5 (max = 255)
  randomSeed(analogRead(0));

  oled.setFont(Adafruit5x7);
  oled.clear();
  oled.println("Mode: 0");

  pinMode(LDR_PIN, INPUT);
  pinMode(SPK_OUT, OUTPUT);
  noTone(SPK_OUT);
  
  pinMode(MotorA_1, OUTPUT);
  pinMode(MotorA_2, OUTPUT);
  pinMode(MotorB_1, OUTPUT);
  pinMode(MotorB_2, OUTPUT);

  attachInterrupt(IR_RECV, irISR, FALLING);
  pinMode(IR_RECV, INPUT);

  randomSeed(analogRead(0));
}

int mode = 1;

bool invertTurn = false;

int lightValue;
int turnTimer = 0;

//unsigned short colors[][3] = {{0, 0, 200}, {0, 0, 180}, {0, 0, 120}, {0, 0, 80}, {0, 0, 20}};
unsigned short colors[][3] = {{200, 0, 200}, {180, 0, 180}, {120, 0, 120}, {80, 0, 80}, {20, 0, 20}};
// unsigned short colors[][3] = {{200, 0, 0}, {180, 0, 0}, {120, 0, 0}, {80, 0, 0}, {20, 0, 0}};
//unsigned short colors[][3] = {{0, 200, 0}, {0, 180, 0}, {0, 120, 0}, {0, 80, 0}, {0, 20, 0}};
const short colLength = 5;

char direction = 's';
char pDirection = 's';

void loop() {
  // put your main code here, to run repeatedly:
  lightValue = analogRead(LDR_PIN);

  if(nec_ok) {                                     // If a good NEC message is received
    nec_ok = false;                             // Reset decoding process

    oled.clear();
    Serial.println("NEC IR Received:");
    
    Serial.print("Addr: ");
    address = nec_code >> 16;
    command = (nec_code & 0xFFFF) >> 8;         // Remove inverted Bits
    sprintf(text, "%04X", address);
    Serial.println(text);                          // Display address in hex format

    command = decodeIr(command);

    if('0' <= command && '9' >= command){
      mode = command - '0';
      oled.clear();
      oled.print("Mode: ");
      oled.print(mode);
      motorMoveControl(0, 1, 0);
      motorMoveControl(1, 1, 0);
    }else{
      direction = command;
    }
  } 

  if(mode != 0 && mode != 2){
    if(lightValue < 900){
      mode = 3;
    }else{
      mode = 1;
    }
  }

  Serial.println(lightValue);

  if(mode == 1){
    int distance = ping_mm();

    int i = (millis() / 100) % colLength;
    strip.setPixelColor(0, strip.Color(colors[(i) % colLength][0], colors[(i) % colLength][1], colors[(i) % colLength][2]));
    strip.setPixelColor(1, strip.Color(colors[(i + 1) % colLength][0], colors[(i + 1) % colLength][1], colors[(i + 1) % colLength][2]));
    strip.setPixelColor(2, strip.Color(colors[(i + 2) % colLength][0], colors[(i + 2) % colLength][1], colors[(i + 2) % colLength][2]));
    strip.show();

    if(distance < 150){
      motorMoveControl(0, 0, 50);
      motorMoveControl(1, 0, 50);
    }else if(distance < 250){
      motorMoveControl(0, invertTurn ? 1 : 0, 50);
      motorMoveControl(1, invertTurn ? 0 : 1, 50);
    }else{
      invertTurn = random(2) % 2 == 0;
      motorMoveControl(0, 1, 50);
      motorMoveControl(1, 1, 50);
    }
  
  }else if(mode == 3 || mode == 2){
    motorMoveControl(0, 0, 0);
    motorMoveControl(1, 0, 0);
    
    int i = (millis() / 500) % colLength;
    strip.setPixelColor(0, strip.Color(colors[(i) % colLength][0], colors[(i) % colLength][1], colors[(i) % colLength][2]));
    strip.setPixelColor(1, strip.Color(colors[(i + 1) % colLength][0], colors[(i + 1) % colLength][1], colors[(i + 1) % colLength][2]));
    strip.setPixelColor(2, strip.Color(colors[(i + 2) % colLength][0], colors[(i + 2) % colLength][1], colors[(i + 2) % colLength][2]));
    strip.show();

    if(millis() - 200 > turnTimer && random(50) % 50 == 0){
      motorMoveControl(0, invertTurn ? 1 : 0, 10);
      motorMoveControl(1, invertTurn ? 0 : 1, 10);
      playTone(NOTE_A7, 10);
      playTone(NOTE_A6, 10);
      playTone(NOTE_A7, 10);
      playTone(NOTE_A4, 10);
      playTone(NOTE_A7, 10);
      playTone(NOTE_A6, 10);
      playTone(NOTE_A7, 10);
      playTone(NOTE_A4, 10);
      playTone(NOTE_A4, 20);
      noTone(SPK_OUT);
      delay(random(10, 30));
      motorMoveControl(0, invertTurn ? 1 : 0, 0);
      motorMoveControl(1, invertTurn ? 0 : 1, 0);
      turnTimer = millis();
    }else if(millis() - 100 > turnTimer && random(200) % 200 == 0){
      motorMoveControl(0, 1, 10);
      motorMoveControl(1, 1, 10);
      delay(random(100, 200));
      motorMoveControl(0, 1, 10);
      motorMoveControl(1, 1, 10);
      turnTimer = millis();
    }else{
      invertTurn = random(2) % 2 == 0;
    }
  }else if(mode == 0){
    switch(direction){
      case 'o':
        motorMoveControl(0, 1, 0);
        motorMoveControl(1, 1, 0);
        break;
      case 'u':
        motorMoveControl(0, 1, 20 + 10*2);
        motorMoveControl(1, 1, 20 + 10*2);
        break;
      case 'r':
        motorMoveControl(0, 0, 20 + 10*2);
        motorMoveControl(1, 1, 20 + 10*2);
        break;
      case 'l':
        motorMoveControl(0, 1, 20 + 10*2);
        motorMoveControl(1, 0, 20 + 10*2);
        break;
      case 'd':
        motorMoveControl(0, 0, 20 + 10*2);
        motorMoveControl(1, 0, 20 + 10*2);
        break;
    }
  }

  delay(10);
}
