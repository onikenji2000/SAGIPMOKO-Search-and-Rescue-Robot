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
int posx = 90;
int posy = 90;
int leftmot = 0;
int rightmot = 0;
int sx = 0;
int sy = 0;
int led1 = 46;
int led2 = 48;
//Servo motor pointer.
Servo servo1, servo2;

void setup() {
  Serial1.begin(115200); //Set the baud rate to your Bluetooth module.
  Serial.begin(9600);
  Serial1.setTimeout(100);
  //Attach servo's to the designated pins.
  servo1.attach(10);
  servo2.attach(9);
// start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

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
  cameraControl(0, 0);
  delay(500);
  cameraControl(-90, 0);
  delay(500);
  cameraControl(180, 0);
  delay(500);
  cameraControl(-90, 0);
  delay(500);
  cameraControl(0, 90);
  delay(500);
  cameraControl(0, -90);
  lcd.clear();
  lcd.noBacklight();
}

//Values for the hardware variables
void setValue(String identifier, int value) {
  if(identifier == "left") leftmot = value;
  else if(identifier == "right") rightmot = value;
  else if(identifier == "pan") sx = value;
  else if(identifier == "tilt") sy = value;
  else if(identifier == "lights") turnLights(value);
}

void loop() {
  temperature();
  String command; // 8 bit character sent by the bluetooth to communicate with the board.
  if(Serial1.available() > 0) {
    command = Serial1.readString(); //Takes the command value sent by the bluetooth device.
    //Serial.println(command);
    //Read the bluetooth command and decide motor actions
    bool goon = true;
    while(goon) {
      int index = command.indexOf(':');
      if(index > 0) {
        String com = command.substring(0, index);
        int indx = com.indexOf('=');
        String ct = com.substring(0, indx);
        int val = com.substring(indx + 1, com.length()).toInt();
        setValue(ct, val);
        command = command.substring(index + 1, command.length());
      }
      else if(command.length() > 0) {
        int indx = command.indexOf('=');
        String ct = command.substring(0, indx);
        int val = command.substring(indx + 1, command.length()).toInt();
        setValue(ct, val);
        command = "";
      }
      else {
        goon = false;
      }
    }
    Serial.print("Motor:(");
    Serial.print(leftmot);
    Serial.print(", ");
    Serial.print(rightmot);
    Serial.println(")");
    runMotor(leftmot, leftmot, rightmot, rightmot);
    cameraControl(sx, sy);
    Serial.print("Servo Motion(");
    Serial.print(posx);
    Serial.print(", ");
    Serial.print(posy);
    Serial.println(")");
    sx = sy = 0;
  }
}

// Function for controlling the motors individually.
void runMotor(int mot1, int mot2, int mot3, int mot4) {
  if(mot1 < 0) { //FRONT-LEFT MOTOR
    mot1 *= -1;
    motor1.setSpeed(mot1);
    motor1.run(BACKWARD);
  }
  else if(mot1 == 0) {
    motor1.setSpeed(mot1);
    motor1.run(RELEASE);
  }
  else {
    motor1.setSpeed(mot1);
    motor1.run(FORWARD);
  }
  if(mot2 < 0) { // REAR-LEFT MOTOR
    mot2 *= -1;
    motor2.setSpeed(mot2);
    motor2.run(BACKWARD);
  }
  else if(mot2 == 0) {
    motor2.setSpeed(mot2);
    motor2.run(RELEASE);
  }
  else {
    motor2.setSpeed(mot2);
    motor2.run(FORWARD);
  }
  if(mot3 < 0) { //FRONT-RIGHT MOTOR
    mot3 *= -1;
    motor3.setSpeed(mot3);
    motor3.run(BACKWARD);
  }
  else if(mot3 == 0) {
    motor3.setSpeed(mot3);
    motor3.run(RELEASE);
  }
  else {
    motor3.setSpeed(mot3);
    motor3.run(FORWARD);
  }
  if(mot4 < 0) { //REAR-RIGHT MOTOR
    mot4 *= -1;
    motor4.setSpeed(mot4);
    motor4.run(BACKWARD);
  }
  else if(mot4 == 0) {
    motor4.setSpeed(mot4);
    motor4.run(RELEASE);
  }
  else {
    motor4.setSpeed(mot4);
    motor4.run(FORWARD);
  }
}

void turnLights(int value) {
  int ledStatus = LOW;
  if(value > 0) ledStatus = HIGH;
  digitalWrite(led1, ledStatus);
  digitalWrite(led2, ledStatus);
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
    Serial1.print("Temperature:");
    Serial1.println(tempC);
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
