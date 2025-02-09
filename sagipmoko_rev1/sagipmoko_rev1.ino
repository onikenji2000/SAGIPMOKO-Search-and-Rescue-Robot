//Arduino Bluetooth Controlled Car//
//Before uploading the code you have to install the necessary library//
//AFMotor Library https://learn.adafruit.com/adafruit-motor-shield/library-install //

#include <AFMotor.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); //The LCD address and size. You can change according you yours

// Data wire is plugged into pin 38 on the Arduino
#define ONE_WIRE_BUS 38 //pin for sensor

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Declare four motors to control the four wheels.
AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

//Camera controls servo's
int posx = 0;
int posy = 0;
//Servo motor pointer.
Servo servo1, servo2;

void setup() {
  Serial1.begin(9600); //Set the baud rate to your Bluetooth module.
  Serial.begin(9600);
  //Attach servo's to the designated pins.
  servo1.attach(10);
  servo2.attach(9);
// start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");

  // Start up the library
  sensors.begin();
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("SAGIPMOKO");
  lcd.setCursor(0, 1);
  lcd.print("   by KCNHS   ");
  delay(2000);
  lcd.clear();
}


void loop() {
  temperature();
  char command; // 8 bit character sent by the bluetooth to communicate with the board.
  if(Serial1.available() > 0) {
    command = Serial1.read(); //Takes the command value sent by the bluetooth device.
    Serial.println(command);
    //Read the bluetooth command and decide motor actions.
    runMotor(0, 0, 0, 0); // Full stop motor.
    switch(command) {
      case 'F':
      Serial.println("Forward!");
      runMotor(250, 250, 250, 250); // Run all the motors forward full power.
      break;
      case 'B':
      Serial.println("Backward!");
      runMotor(-250, -250, -250, -250); // RUn all the motors backward full power.
      break;
      case 'L':
      Serial.println("Left!");
      runMotor(-250, -250, 250, 250); // Run both left motors backward and right motors forward.
      break;
      case 'R':
      Serial.println("Right!");
      runMotor(250, 250, -250, -250); // Run both left motors forward and right motors backward.
      break;
      case 'C':
      Serial.println("Cam Left");
      cameraControl(10, 0);
      break;
      case 'U':
      Serial.println("Cam Up");
      cameraControl(0, 10);
      break;
      case 'D':
      Serial.println("Cam Down");
      cameraControl(0, -10);
      break;
      case 'A':
      Serial.println("Cam Right");
      cameraControl(-10, 0);
      break;
    }
  }
}

// Function for controlling the motors individually.
void runMotor(int mot1, int mot2, int mot3, int mot4) {
  if(mot1 < 0) {
    mot1 *= -1;
    motor1.setSpeed(mot1);
    motor1.run(BACKWARD);
  }
  else if(mot1 = 0) {
    motor1.setSpeed(mot1);
    motor1.run(RELEASE);
  }
  else {
    motor1.setSpeed(mot1);
    motor1.run(FORWARD);
  }
  if(mot2 < 0) {
    mot2 *= -1;
    motor2.setSpeed(mot2);
    motor2.run(BACKWARD);
  }
  else if(mot2 = 0) {
    motor2.setSpeed(mot2);
    motor2.run(RELEASE);
  }
  else {
    motor2.setSpeed(mot2);
    motor2.run(FORWARD);
  }
  if(mot3 < 0) {
    mot3 *= -1;
    motor3.setSpeed(mot3);
    motor3.run(BACKWARD);
  }
  else if(mot3 = 0) {
    motor3.setSpeed(mot3);
    motor3.run(RELEASE);
  }
  else {
    motor3.setSpeed(mot3);
    motor3.run(FORWARD);
  }
  if(mot4 < 0) {
    mot4 *= -1;
    motor4.setSpeed(mot4);
    motor4.run(BACKWARD);
  }
  else if(mot4 = 0) {
    motor4.setSpeed(mot4);
    motor4.run(RELEASE);
  }
  else {
    motor4.setSpeed(mot4);
    motor4.run(FORWARD);
  }
}

void cameraControl(int rotx, int roty) {
  posx += rotx;
  posy += roty;
  if(posx < 0) posx = 0;
  if(posx > 180) posx = 180;
  if(posy < 0) posy = 0;
  if(posy > 180) posy = 180;
  servo1.write(posx);
  servo2.write(posy);
  Serial.print("(");
  Serial.print(posx);
  Serial.print(", ");
  Serial.print(posy);
  Serial.println(") ");
}
void temperature()
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  float tempC = sensors.getTempCByIndex(0);

  // Check if reading was successful
  if (tempC != DEVICE_DISCONNECTED_C)
  {
    Serial.print("Temperature for the device 1 (index 0) is: ");
    Serial.println(tempC);
    lcd.setCursor(0, 0);
    lcd.print("Temperature:");
    lcd.setCursor(0, 1);
    lcd.print(tempC);
    lcd.print((char)223);
    lcd.print("C");
    lcd.print("|");
    lcd.print(DallasTemperature::toFahrenheit(tempC));
    lcd.print(" F");
  }
  else
  {
    Serial.println("Error: Could not read temperature data");
  }
}