#include <Stepper.h>
#include <math.h>

// board size
#define BOARD_SIZE 15.0
#define LASER_HEIGHT 1.0  // laser from bottom
#define DIAGONAL_DISTANCE 28.0  // diagnol


#define RESET_BUTTON_PIN 2

const int STEPS_PER_REV = 2048;  // 2048 = 360


// nano Left
#define MOTOR1_PIN1 4
#define MOTOR1_PIN2 6
#define MOTOR1_PIN3 5
#define MOTOR1_PIN4 7

// nano right
#define MOTOR2_PIN1 8
#define MOTOR2_PIN2 10
#define MOTOR2_PIN3 9
#define MOTOR2_PIN4 11

// far Left
#define MOTOR3_PIN1 12
#define MOTOR3_PIN2 A0
#define MOTOR3_PIN3 13
#define MOTOR3_PIN4 A1

// far Right
#define MOTOR4_PIN1 A2
#define MOTOR4_PIN2 A4
#define MOTOR4_PIN3 A3
#define MOTOR4_PIN4 A5



Stepper motor1(STEPS_PER_REV, MOTOR1_PIN1, MOTOR1_PIN3, MOTOR1_PIN2, MOTOR1_PIN4);
Stepper motor2(STEPS_PER_REV, MOTOR2_PIN1, MOTOR2_PIN3, MOTOR2_PIN2, MOTOR2_PIN4);
// oposite
Stepper motor3(STEPS_PER_REV, MOTOR3_PIN4, MOTOR3_PIN2, MOTOR3_PIN3, MOTOR3_PIN1);
Stepper motor4(STEPS_PER_REV, MOTOR4_PIN4, MOTOR4_PIN2, MOTOR4_PIN3, MOTOR4_PIN1);

// track angle
float currentAngle1 = 0;
float currentAngle2 = 0;
float currentAngle3 = 0;
float currentAngle4 = 0;

// push button -> go to 0
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
int lastButtonState = HIGH;
int buttonState = HIGH;

void setup() {
  Serial.begin(9600);
  
  // sppeed
  motor1.setSpeed(10);
  motor2.setSpeed(10);
  motor3.setSpeed(10);
  motor4.setSpeed(10);
  
  // reset
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  
  Serial.println("Start");
  Serial.println("Enter h,l,w");
}

void loop() {
  checkResetButton();
  
  // Serial
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    processData(data);
  }
}

void checkResetButton() {
  int reading = digitalRead(RESET_BUTTON_PIN);
  

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      // button pushed (LOW)
      if (buttonState == LOW) {
        resetPosition();
      }
    }
  }
  
  lastButtonState = reading;
}

void resetPosition() {
  Serial.println("Resetting to initial position...");
  
  // move to first setting
  int steps1 = (-currentAngle1) * (STEPS_PER_REV/360.0);
  int steps2 = (-currentAngle2) * (STEPS_PER_REV/360.0);
  int steps3 = (-currentAngle3) * (STEPS_PER_REV/360.0);
  int steps4 = (-currentAngle4) * (STEPS_PER_REV/360.0);
  
  motor1.step(steps1);
  delay(100);
  motor2.step(steps2);
  delay(100);
  motor3.step(steps3);
  delay(100);
  motor4.step(steps4);
  
  // change angle to 0
  currentAngle1 = 0;
  currentAngle2 = 0;
  currentAngle3 = 0;
  currentAngle4 = 0;
  
  Serial.println("Reset complete! All lasers at 0 degrees");
}

void processData(String data) {
  int firstComma = data.indexOf(',');
  int secondComma = data.indexOf(',', firstComma + 1);
  
  if (firstComma != -1 && secondComma != -1) {
    float height = data.substring(0, firstComma).toFloat();
    float length = data.substring(firstComma + 1, secondComma).toFloat();
    float width = data.substring(secondComma + 1).toFloat();
    
    Serial.print("Received - Height: ");
    Serial.print(height);
    Serial.print(" Length: ");
    Serial.print(length);
    Serial.print(" Width: ");
    Serial.println(width);
    
    // calculate from laser
    float halfLength = length / 2.0;
    float halfWidth = width / 2.0;
    float actualHeight = height - LASER_HEIGHT;  // from laser height
    
    // calculate from dialgnol
    float diagonalDistanceXY = DIAGONAL_DISTANCE * cos(45.0 * M_PI / 180.0);
    
    // calculate angle
    // nano lef +length가 반시계 방향
    float targetAngle1 = atan2(halfLength - diagonalDistanceXY, actualHeight) * 180.0/M_PI;
    
    // nano right +width가 반시계 방향
    float targetAngle2 = atan2(halfWidth - diagonalDistanceXY, actualHeight) * 180.0/M_PI;
    
    // far left: +length가 시계 방향
    float targetAngle3 = atan2(halfLength - diagonalDistanceXY, actualHeight) * 180.0/M_PI;
    
    // far right: +width가 시계 방향
    float targetAngle4 = atan2(halfWidth - diagonalDistanceXY, actualHeight) * 180.0/M_PI;
    
    // limit angle 
    targetAngle1 = constrain(targetAngle1, -60, 60);
    targetAngle2 = constrain(targetAngle2, -60, 60);
    targetAngle3 = constrain(targetAngle3, -60, 60);
    targetAngle4 = constrain(targetAngle4, -60, 60);
    
    // CAlculate step
    int steps1 = (targetAngle1 - currentAngle1) * (STEPS_PER_REV/360.0);
    int steps2 = (targetAngle2 - currentAngle2) * (STEPS_PER_REV/360.0);
    int steps3 = (targetAngle3 - currentAngle3) * (STEPS_PER_REV/360.0);
    int steps4 = (targetAngle4 - currentAngle4) * (STEPS_PER_REV/360.0);
    
    // print when motors are moving
    Serial.println("Moving motors...");
    
    motor1.step(steps1);
    currentAngle1 = targetAngle1;
    delay(100);
    
    motor2.step(steps2);
    currentAngle2 = targetAngle2;
    delay(100);
    
    motor3.step(steps3);
    currentAngle3 = targetAngle3;
    delay(100);
    
    motor4.step(steps4);
    currentAngle4 = targetAngle4;
    
    Serial.println("complete");
    Serial.print("Current angles: ");
    Serial.print(currentAngle1);
    Serial.print(", ");
    Serial.print(currentAngle2);
    Serial.print(", ");
    Serial.print(currentAngle3);
    Serial.print(", ");
    Serial.println(currentAngle4);
  } else {
    Serial.println("Please use height,length,width format");
  }
}
