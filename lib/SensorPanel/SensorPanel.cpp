//
// Created by lakshith on 8/8/22.
//

#include "SensorPanel.h"

inline uint16_t abs(uint16_t a) {
    return a > 0 ? a : a * -1;
}

SensorPanel::SensorPanel(uint8_t *sensorPins) {
    SensorPanel::qtr.setTypeRC();
    SensorPanel::qtr.setSensorPins(sensorPins, SensorCount);
}

bool SensorPanel::calibrate(int seconds) {
    for (int i = 0; i < seconds * 40; i++) {
        SensorPanel::qtr.calibrate();
    }
    return true;
}

uint16_t SensorPanel::readLine(uint16_t *sensorValues) {
    return SensorPanel::qtr.readLineWhite(sensorValues);
}

void SensorPanel::read() {
    position = SensorPanel::readLine(panelReading);

    error = (int)position - SensorCount / 2;

    for (int i = 0; i < SensorPanel::SensorCount; i++) {
        panelReading[i] = panelReading[i] > 700 ? 1 : 0;
    }

    SensorPanel::updatePattern();
}

void SensorPanel::updatePattern() { //todo update hyper params
    const uint16_t THRESHOLD = 35;

    uint16_t leftSum = 0;
    uint16_t rightSum = 0;

    for (int i = 0; i < SensorPanel::SensorCount / 2; i++) {
        leftSum += ((SensorCount / 2) - i) * 2 * SensorPanel::panelReading[i];
    }

    for (int i = 0; i < SensorPanel::SensorCount / 2; i++) {
        rightSum += (i + 1) * 2 * SensorPanel::panelReading[i + SensorPanel::SensorCount / 2];
    }

    if (leftSum >= THRESHOLD && rightSum >= THRESHOLD) {
        SensorPanel::pattern = 'T';
    } else if (leftSum >= THRESHOLD) {
        SensorPanel::pattern = 'L';
    } else if (rightSum >= THRESHOLD) {
        SensorPanel::pattern = 'R';
    } else {
        SensorPanel::pattern = 0;
    }
}