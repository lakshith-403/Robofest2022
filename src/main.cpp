#include <Arduino.h>
#include "../lib/MotorDriver/MotorDriver.h"
#include "../lib/PID/PID.h"
#include "../lib/SensorPanel/SensorPanel.h"

SensorPanel qtr(const_cast<uint8_t *>((const uint8_t[])
        {37, 39, 41, 43, 45, 47, 49, 51,
         35, 33, 31, 29, 27, 25, 23, 22}));

MotorDriver driver;

const int leftPins[] = {0, 1, 2};
const int rightPins[] = {0, 1, 2};

const int switchPin = 0;

const int turnSpeed = 100;
const int forwardSpeed = 100;

const int epsilon = 100;

const String mappings [6][6] = { { "LBL", "LBS", "RBL" ,"SBL" ,"SBS", "LBR"}, { "S", "R", "B","R", "B", "B" } };
String mazeShort(String maze);

inline void waitTillButton() {
    int reading = digitalRead(switchPin);
    while (digitalRead(switchPin) == reading) {}
}

inline void waitTillMiddle() {
    delay(500);
    qtr.read();
    while (abs(qtr.error) > epsilon) {
        qtr.read();
    }
}

void setup() {
    pinMode(switchPin, OUTPUT);

    Serial.begin(9600);
    driver.init(const_cast<int *>(leftPins), const_cast<int *>(rightPins));

    waitTillButton();
    qtr.calibrate(10);
}

[[noreturn]] void loop() {
    waitTillButton();

    driver.forward(150);
    delay(300);

    while (true) {
        qtr.read();

        if (qtr.pattern == 1) { //pid
            int correction = pid(qtr.error);
            driver.applyPID(correction);
        } else {
            char pattern = qtr.pattern;

            driver.forward(150);
            delay(200);

            qtr.read();
            char newPattern = qtr.pattern;

            switch (pattern) {
                case 'L':
                    if (pattern == 'L') {
                        driver.turnLeft(100);
                    } else if (pattern ==)
                        driver.turnLeft(turnSpeed);
                    waitTillMiddle();
                    break;
                case 'R':
                    if (newPattern == 1) {
                        driver.forward(forwardSpeed);
                        delay(200);
                    } else {
                        driver.turnRight(turnSpeed);
                        waitTillMiddle();
                    }
                    break;
                case 'T':
                    driver.turnLeft(turnSpeed);
                    waitTillMiddle();
                default:
                    driver.turnLeft(turnSpeed);
                    waitTillMiddle();
            }
        }
    }
}
String mazeShort(String maze){                          // sumthin like "SSBLLBLLLBLBLBLS"
  while(maze.indexOf("B")!=-1){
    for(int i=0;i<6;i++){
      while(maze.indexOf(mappings[0][i])!=-1){
        maze.replace(mappings[0][i],mappings[1][i]);
      }
    }
  }
  return maze;
}
