#include <Arduino.h>
#include "../lib/MotorDriver/MotorDriver.h"
#include "../lib/Gyro/Gyro.h"
#include "util.h"

SensorPanel qtr(const_cast<uint8_t *>((const uint8_t[]) {24, 22, 23, 25, 27, 29, 31,
                                                         33, 35, 37, 39, 41, 43, 45, 47, 49}));

MotorDriver driver;

const int leftPins[] = {6, 9, 8};
const int rightPins[] = {7, 11, 10};

const int lightPins[] = {19, 50, 48};

const int switchPin = 18;
const int buzzerPin = 12;

const int turnSpeed = 120;
const int forwardSpeed = 60;


inline void waitTillButton() {
    int reading = digitalRead(switchPin);
    while (digitalRead(switchPin) == reading) {}
}

inline void waitTill90(){
    double a = getAngle();
    double b = a;

    while (abs(a - b) <= 90) {
        b = getAngle();
    }
    driver.stop();
}

inline void waitTill180() {
    double a = getAngle();
    double b = a;

    while (abs(a - b) <= 180) {
        b = getAngle();
    }
    driver.stop();
}

void BotSetup() {
    pinMode(switchPin, OUTPUT);

    for (int lightPin: lightPins) {
        pinMode(lightPin, OUTPUT);
    }

    Serial.begin(9600);
    driver.init(const_cast<int *>(leftPins), const_cast<int *>(rightPins));

    Serial.println("Calibrating");
    qtr.calibrate(10);
    showLight('R');
    setupGyro();
    showLight('B');
    driver.forward(80);
    delay(500);
}

void BotLoop() {
    while (true) {
        qtr.read();

        if (qtr.pattern == 1) { //pid
            showLight('a');

            int correction = pid(qtr.error);
            driver.applyPID(correction);
        } else {
            char pattern = qtr.pattern;
            bool left = pattern == 'L';
            bool right = pattern == 'R';
            bool t = pattern == 'T';
            driver.forward(forwardSpeed);
            for (int i = 0; i < 30; i++) {
                qtr.read();
                if (qtr.pattern == 'L') {
                    left = true;
                } else if (qtr.pattern == 'R') {
                    right = true;
                } else if (qtr.pattern == 'T') {
                    t = true;
                }
            }
            if (t || (left && right)) {
                pattern = 'T';
            } else if (left) {
                pattern = 'L';
            } else if (right) {
                pattern = 'R';
            } else {
                pattern = 0;
            }
            driver.stop();

            qtr.read();
            char newPattern = qtr.pattern;

            if (qtr.isEnd) {
                driver.stop();
                showLight('R');
                return;
            }

            switch (pattern) {
                case 'L':
                    path += 'L';
                    showLight('R');
                    driver.turnLeft(turnSpeed);
                    waitTill90();
                    break;

                case 'R':
                    showLight('G');
                    if (newPattern == 1) {
                        path += 'S';
                        driver.forward(forwardSpeed);
                    } else {
                        path += 'R';
                        driver.turnRight(turnSpeed);
                        waitTill90();
                    }
                    break;

                case 'T':
                    path += 'L';
                    showLight('B');
                    driver.turnLeft(turnSpeed);
                    waitTill90();
                    break;

                default:
                    path += 'B';
                    showLight('B');
                    driver.turnLeft(turnSpeed);
                    waitTill180();
                    break;
            }
            driver.stop();
        }
    }
}

void setup() {
    BotSetup();
}

void loop() {
    BotLoop();
    waitTillButton();
}