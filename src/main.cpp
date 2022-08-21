#include <Arduino.h>
#include "../lib/MotorDriver/MotorDriver.h"
#include "../lib/PID/PID.h"
#include "../lib/SensorPanel/SensorPanel.h"

SensorPanel qtr(const_cast<uint8_t *>((const uint8_t[]) {24, 22, 23, 25, 27, 29, 31,
                                                         33, 35, 37, 39, 41, 43, 45, 47, 49}));

MotorDriver driver;

const int leftPins[] = {6, 9, 8};
const int rightPins[] = {7, 11, 10};

const int lightPins[] = {19, 50, 48};

const int switchPin = 18;

const int turnSpeed = 100;
const int forwardSpeed = 100;

const int epsilon = 100;

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
    qtr.calibrate(5);
    int lightFreq[] = {0, 100, 200};
    light(lightFreq);

    waitTillButton();
}

[[noreturn]] void BotLoop() {
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
                    driver.turnLeft(100);
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
                    driver.turnRight(turnSpeed);
                    waitTillMiddle();
                    break;
            }
        }
    }
}

void setup() {
    BotSetup();
    driver.forward(150);
    delay(500);
}

void loop() {
    BotLoop();
}