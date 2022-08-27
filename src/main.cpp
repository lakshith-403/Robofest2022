#include <Arduino.h>
#include "../lib/MotorDriver/MotorDriver.h"
#include "util.h"

SensorPanel qtr(const_cast<uint8_t *>((const uint8_t[]) {24, 22, 23, 25, 27, 29, 31,
                                                         33, 35, 37, 39, 41, 43, 45, 47, 49}));

MotorDriver driver;

const int leftPins[] = {6, 9, 8};
const int rightPins[] = {7, 11, 10};

const int lightPins[] = {19, 50, 48};

const int switchPin = 18;
const int buzzerPin = 12;

const int turnSpeed = 80;
const int forwardSpeed = 60;

const int FORWARD_DELAY = 150;

inline void turnDelay() {
    delay(300);
}

inline void forwardDelay() {
    delay(150);
}

inline void waitTillButton() {
    int reading = digitalRead(switchPin);
    while (digitalRead(switchPin) == reading) {}
}

inline void waitTillMiddle() {
    qtr.read();
    while (!qtr.isMiddle) {
        qtr.read();
    }
    driver.stop();
}

inline void beepBeep() {
    digitalWrite(buzzerPin, HIGH);
}

inline void light(int freq[]) {
    for (int i = 0; i < 3; i++) {
        analogWrite(lightPins[i], freq[i]);
    }
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
    int lightFreq[] = {0, 100, 200};
    light(lightFreq);
}

[[noreturn]] void BotLoop() {
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
            for (int i = 0; i < 70; i++) {
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

            switch (pattern) {
                case 'L':
                    showLight('R');
                    driver.turnLeft(turnSpeed);
                    turnDelay();
                    waitTillMiddle();
                    break;

                case 'R':
                    showLight('G');
                    if (newPattern == 1) {
                        driver.forward(forwardSpeed);
                    } else {
                        driver.turnRight(turnSpeed);
                        turnDelay();
                        waitTillMiddle();
                    }
                    break;

                case 'T':
                    showLight('B');
                    driver.turnLeft(turnSpeed);
                    turnDelay();
                    waitTillMiddle();
                    break;

                default:
                    driver.turnRight(turnSpeed);
                    turnDelay();
                    waitTillMiddle();
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
}