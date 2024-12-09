#include <Stepper.h>
#include <math.h>

// Baord size
#define BOARD_SIZE 15.0

// laser 1cm from ground
#define LASER_HEIGHT 1.0 

// diagonal = 28cm
#define DIAGONAL_DISTANCE 28.0

// button -> reset -> go to 0
#define RESET_BUTTON_PIN 2  

// motor -> 2048 = 360
const int STEPS_PER_REV = 2048;

// motor setting
// down right
#define MOTOR1_PIN1 4
#define MOTOR1_PIN2 6
#define MOTOR1_PIN3 5
#define MOTOR1_PIN4 7

// up right
#define MOTOR2_PIN1 8
#define MOTOR2_PIN2 10
#define MOTOR2_PIN3 9
#define MOTOR2_PIN4 11

// up left
#define MOTOR3_PIN1 12
#define MOTOR3_PIN2 A0
#define MOTOR3_PIN3 13
#define MOTOR3_PIN4 A1

// down left
#define MOTOR4_PIN1 A2
#define MOTOR4_PIN2 A4
#define MOTOR4_PIN3 A3
#define MOTOR4_PIN4 A5


Stepper motor1(STEPS_PER_REV, MOTOR1_PIN1, MOTOR1_PIN3, MOTOR1_PIN2, MOTOR1_PIN4);
Stepper motor2(STEPS_PER_REV, MOTOR2_PIN1, MOTOR2_PIN3, MOTOR2_PIN2, MOTOR2_PIN4);
Stepper motor3(STEPS_PER_REV, MOTOR3_PIN4, MOTOR3_PIN2, MOTOR3_PIN3, MOTOR3_PIN1);
Stepper motor4(STEPS_PER_REV, MOTOR4_PIN4, MOTOR4_PIN2, MOTOR4_PIN3, MOTOR4_PIN1);

// track current angle
float currentAngle1 = 0;
float currentAngle2 = 0;
float currentAngle3 = 0;
float currentAngle4 = 0;

// button setting
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;    // 디바운스 시간 50ms
int lastButtonState = HIGH;
int buttonState = HIGH;

void setup() {
  Serial.begin(9600);
  
  // speed
  motor1.setSpeed(5);
  motor2.setSpeed(5);
  motor3.setSpeed(5);
  motor4.setSpeed(5);
  

  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  

  Serial.println("START");
  Serial.println("Enter measurements in format: height,length,width");
}

void loop() {
  // check!!!!
  int buttonReading = digitalRead(RESET_BUTTON_PIN);
  if (buttonReading == LOW) {
    Serial.println("Button pressed!");
  }
  checkResetButton();
  
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
      
      // button -> pressed (LOW)
      if (buttonState == LOW) {
        resetPosition();
      }
    }
  }
  
  lastButtonState = reading;
}

void resetPosition() {
  Serial.println("Resetting to initial position...");
  
  const int STEPS_PER_MOVE = 100;  // movement size
  
  int remaining1 = (-currentAngle1) * (STEPS_PER_REV/360.0);
  int remaining2 = (-currentAngle2) * (STEPS_PER_REV/360.0);
  int remaining3 = (-currentAngle3) * (STEPS_PER_REV/360.0);
  int remaining4 = (-currentAngle4) * (STEPS_PER_REV/360.0);
  
  while(remaining1 != 0 || remaining2 != 0 || remaining3 != 0 || remaining4 != 0) {
    if(remaining1 > 0) {
      motor1.step(min(STEPS_PER_MOVE, remaining1));
      remaining1 -= min(STEPS_PER_MOVE, remaining1);
    } else if(remaining1 < 0) {
      motor1.step(max(-STEPS_PER_MOVE, remaining1));
      remaining1 -= max(-STEPS_PER_MOVE, remaining1);
    }
    
    if(remaining2 > 0) {
      motor2.step(min(STEPS_PER_MOVE, remaining2));
      remaining2 -= min(STEPS_PER_MOVE, remaining2);
    } else if(remaining2 < 0) {
      motor2.step(max(-STEPS_PER_MOVE, remaining2));
      remaining2 -= max(-STEPS_PER_MOVE, remaining2);
    }
    
    if(remaining3 > 0) {
      motor3.step(min(STEPS_PER_MOVE, remaining3));
      remaining3 -= min(STEPS_PER_MOVE, remaining3);
    } else if(remaining3 < 0) {
      motor3.step(max(-STEPS_PER_MOVE, remaining3));
      remaining3 -= max(-STEPS_PER_MOVE, remaining3);
    }
    
    if(remaining4 > 0) {
      motor4.step(min(STEPS_PER_MOVE, remaining4));
      remaining4 -= min(STEPS_PER_MOVE, remaining4);
    } else if(remaining4 < 0) {
      motor4.step(max(-STEPS_PER_MOVE, remaining4));
      remaining4 -= max(-STEPS_PER_MOVE, remaining4);
    }
  }
  
  // current angle = 0
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
    
    // calculate
    float halfLength = length / 2.0;
    float halfWidth = width / 2.0;
    float actualHeight = height - LASER_HEIGHT;
    
    // down right
    float targetAngle1 = atan2(halfLength, actualHeight) * 180.0/M_PI;
    
    // up right
    float targetAngle2 = atan2(halfWidth, actualHeight) * 180.0/M_PI;
    
    // up left
    float targetAngle3 = -atan2(halfLength, actualHeight) * 180.0/M_PI;
    
    // down left
    float targetAngle4 = -atan2(halfWidth, actualHeight) * 180.0/M_PI;
    

    Serial.println("Calculation:");
    Serial.print("Height: "); Serial.print(actualHeight);
    Serial.print(", Length: "); Serial.print(halfLength);
    Serial.print(", Width: "); Serial.println(halfWidth);
    Serial.print("Right Bottom (D4): "); Serial.println(targetAngle1);
    Serial.print("Right Top (D8): "); Serial.println(targetAngle2);
    Serial.print("Left Top (D12): "); Serial.println(targetAngle3);
    Serial.print("Left Bottom (A2): "); Serial.println(targetAngle4);

    // limit angle
    targetAngle1 = constrain(targetAngle1, -45, 45);
    targetAngle2 = constrain(targetAngle2, -45, 45);
    targetAngle3 = constrain(targetAngle3, -45, 45);
    targetAngle4 = constrain(targetAngle4, -45, 45);

    const int STEPS_PER_MOVE = 100;
    
    int steps1 = (targetAngle1 - currentAngle1) * (STEPS_PER_REV/360.0);
    int steps2 = (targetAngle2 - currentAngle2) * (STEPS_PER_REV/360.0);
    int steps3 = (targetAngle3 - currentAngle3) * (STEPS_PER_REV/360.0);
    int steps4 = (targetAngle4 - currentAngle4) * (STEPS_PER_REV/360.0);
    
    Serial.println("Moving motors...");
    
    while(steps1 != 0 || steps2 != 0 || steps3 != 0 || steps4 != 0) {
      if(steps1 > 0) {
        motor1.step(min(STEPS_PER_MOVE, steps1));
        steps1 -= min(STEPS_PER_MOVE, steps1);
      } else if(steps1 < 0) {
        motor1.step(max(-STEPS_PER_MOVE, steps1));
        steps1 -= max(-STEPS_PER_MOVE, steps1);
      }
      
      if(steps2 > 0) {
        motor2.step(min(STEPS_PER_MOVE, steps2));
        steps2 -= min(STEPS_PER_MOVE, steps2);
      } else if(steps2 < 0) {
        motor2.step(max(-STEPS_PER_MOVE, steps2));
        steps2 -= max(-STEPS_PER_MOVE, steps2);
      }
      
      if(steps3 > 0) {
        motor3.step(min(STEPS_PER_MOVE, steps3));
        steps3 -= min(STEPS_PER_MOVE, steps3);
      } else if(steps3 < 0) {
        motor3.step(max(-STEPS_PER_MOVE, steps3));
        steps3 -= max(-STEPS_PER_MOVE, steps3);
      }
      
      if(steps4 > 0) {
        motor4.step(min(STEPS_PER_MOVE, steps4));
        steps4 -= min(STEPS_PER_MOVE, steps4);
      } else if(steps4 < 0) {
        motor4.step(max(-STEPS_PER_MOVE, steps4));
        steps4 -= max(-STEPS_PER_MOVE, steps4);
      }
    }
    
    currentAngle1 = targetAngle1;
    currentAngle2 = targetAngle2;
    currentAngle3 = targetAngle3;
    currentAngle4 = targetAngle4;
    
    Serial.println("Movement complete!");
    Serial.print("Current angles: ");
    Serial.print(currentAngle1);
    Serial.print(", ");
    Serial.print(currentAngle2);
    Serial.print(", ");
    Serial.print(currentAngle3);
    Serial.print(", ");
    Serial.println(currentAngle4);
  } else {
    Serial.println("Invalid data format. Please use: height,length,width");
  }
}
