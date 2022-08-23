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

const int turnSpeed = 100;
const int forwardSpeed = 100;

inline void turnDelay() {
    delay(200);
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

            int lightFreq[] = {0, 0, 0};
            light(lightFreq);

            int correction = pid(qtr.error);
            driver.applyPID(correction);
        } else {

            int lightFreq[] = {0, 100, 200};
            light(lightFreq);

            driver.stop();
            char pattern = qtr.pattern;

            driver.forward(forwardSpeed);
            forwardDelay();

            qtr.read();
            char newPattern = qtr.pattern;

            switch (pattern) {
                case 'L':
                    driver.turnLeft(turnSpeed);
                    turnDelay();
                    waitTillMiddle();
                    break;
                case 'R':
                    if (newPattern == 1) {
                        driver.forward(forwardSpeed);
                    } else {
                        driver.turnRight(turnSpeed);
                        turnDelay();
                        waitTillMiddle();
                    }
                    break;
                case 'T':
                    driver.turnLeft(turnSpeed);
                    turnDelay();
                    waitTillMiddle();
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
    qtr.read();
    int correction = pid(qtr.error);
    driver.applyPID(correction);
    if (qtr.pattern!=1) {
        driver.stop();
        waitTillButton();
    }
}