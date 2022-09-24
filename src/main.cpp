#include <Arduino.h>
#include "../lib/MotorDriver/MotorDriver.h"
#include "../lib/Gyro/Gyro.h"
#include "util.h"
#include <Servo.h>

SensorPanel qtr(const_cast<uint8_t *>((const uint8_t[]) {24, 22, 23, 25, 27, 29, 31,
                                                         33, 35, 37, 39, 41, 43, 45, 47, 49}));

MotorDriver driver;

const int leftPins[] = {6, 9, 8};
const int rightPins[] = {7, 11, 10};

const int lightPins[] = {19, 50, 48};

const int switchPin = 18;
const int buzzerPin = 12;

const int turnSpeed = 120;
const int forwardSpeed = 80;

Servo xServo;
Servo yServo;

bool foundBox = false;

const int forwardReadings = 40;

inline void waitTillButton() {
    int reading = digitalRead(switchPin);
    while (digitalRead(switchPin) == reading) {}
}

inline void waitTill90() {
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

    xServo.attach(15);
    yServo.attach(14);

    xServo.write(85);
    yServo.write(180);

    xServo.detach();
    yServo.detach();

    for (int lightPin: lightPins) {
        pinMode(lightPin, OUTPUT);
    }

    Serial.begin(9600);
    driver.init(const_cast<int *>(leftPins), const_cast<int *>(rightPins));

    Serial.println("Calibrating");
    qtr.calibrate(10);
    showLight('R');
    waitTillButton();
    setupGyro();
    showLight('B');
    driver.forward(80);
    delay(500);
}

void straightenEnd() {
    int reverseSpeed = 70;
    int leftSensor = 0, rightSensor = 15;
    qtr.read();
    const int limit = 100;
    while (!qtr.panelReading[leftSensor] || !qtr.panelReading[rightSensor]) {
        int count = 0;
        while (!qtr.panelReading[leftSensor]) {
            driver.reverseLeft(reverseSpeed);
            qtr.read();
            if (count++ >= limit) {
                break;
            }
        }
        count = 0;
        driver.stop();
        qtr.read();
        while (!qtr.panelReading[rightSensor]) {
            driver.reverseRight(reverseSpeed);
            qtr.read();
            if (count++ >= limit) {
                break;
            }
        }
        driver.stop();
    }
}

void straightenStart() {
    int reverseSpeed = 70;
    int leftSensor = 0, rightSensor = 15;
    qtr.read();
    const int limit = 100;
    while (qtr.panelReading[leftSensor] || qtr.panelReading[rightSensor]) {
        int count = 0;
        qtr.read();
        while (qtr.panelReading[leftSensor]) {
            driver.reverseLeft(reverseSpeed);
            qtr.read();
            if (count++ >= limit) {
                break;
            }
        }
        driver.stop();

        count = 0;
        qtr.read();
        while (qtr.panelReading[rightSensor]) {
            driver.reverseRight(reverseSpeed);
            qtr.read();
            if (count++ >= limit) {
                break;
            }
        }
        driver.stop();
    }
}

void goThroughBox() {
    double initTheta = getAngle();
    qtr.read();
    int count = 0;
    int limit = 50;
    while (qtr.panelReading[0] || qtr.panelReading[15] || count++ < limit) {
        qtr.read();
        double theta = getAngle();
        double error = initTheta - theta;

        int correction = gyroPid(error);
        driver.applyGyroPID(correction * -1);
        delay(1);
    }
}

void rotateServo(Servo &s, int start, int end) {
    if (start < end) {
        for (int i = start; i <= end; i++) {
            s.write(i);
            delay(5);
        }
        delay(500);
    } else {
        for (int i = start; i >= end; i--) {
            s.write(i);
            delay(15);
        }
        delay(500);
    }
}

void liftBox() {
    // x axis 175 <-> 85 <-> 0
    //y axis 90 <-> 180
    xServo.attach(15);
    yServo.attach(14);

    rotateServo(xServo, 85, 175);
    rotateServo(yServo, 180, 50);

    rotateServo(yServo, 50, 180);
    rotateServo(xServo, 175, 85);

    xServo.detach();
    yServo.detach();
}

void placeBox() {
    xServo.attach(15);
    yServo.attach(14);
    // x axis 175 <-> 85 <-> 0
    //y axis 90 <-> 180
    rotateServo(xServo, 85, 0);
    rotateServo(yServo, 180, 50);
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

            int tCount = 0;
            for (int i = 0; i < forwardReadings; i++) {
                qtr.read();
                if (qtr.pattern == 'L') {
                    left = true;
                } else if (qtr.pattern == 'R') {
                    right = true;
                } else if (qtr.pattern == 'T') {
                    t = true;
                    tCount++;
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

            if (tCount >= forwardReadings / 3) { //black
                driver.stop();
                qtr.read();
                if (qtr.pattern == 'T') {
                    if (foundBox) {
                        driver.stop();
                        showLight('R');
                        straightenStart();
                        driver.stop();
                        goThroughBox();
                        driver.stop();
                        straightenEnd();
                        driver.stop();
                        placeBox();
                        return;
                    }

                    foundBox = true;

                    delay(100);
                    straightenStart();
                    driver.stop();
                    goThroughBox();
                    driver.stop();
                    straightenEnd();
                    driver.stop();
                    liftBox();
                    driver.forward(forwardSpeed);
                    delay(forwardReadings * 10);
                    continue;
                }
            }

            switch (pattern) {
                case 'L':
                    showLight('R');
                    driver.turnLeft(turnSpeed);
                    waitTill90();
                    break;

                case 'R':
                    showLight('G');
                    if (newPattern == 1) {
                        driver.forward(forwardSpeed);
                    } else {
                        driver.turnRight(turnSpeed);
                        waitTill90();
                    }
                    break;

                case 'T':
                    showLight('B');
                    driver.turnLeft(turnSpeed);
                    waitTill90();
                    break;

                default:
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