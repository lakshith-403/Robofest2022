//
// Created by lakshith on 8/8/22.
//

#include "SensorPanel.h"

inline uint16_t abs(uint16_t a) {
    return a > 0 ? a : a * -1;
}

SensorPanel::SensorPanel(uint8_t *sensorPins) {
    SensorPanel::qtr.setTypeRC();
    SensorPanel::qtr.setSensorPins(sensorPins, 16);
}

bool SensorPanel::calibrate(int seconds) {
    for (int i = 0; i < seconds * 40; i++) {
        SensorPanel::qtr.calibrate();
    }
//    uint16_t maxSensorValues[] = {2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500,
//                                  2500, 2500};
//    uint16_t minSensorValues[] = {280, 280, 188, 276, 188, 276, 192, 276, 276, 280, 184, 184, 184, 92, 92, 92};
//    qtr.virtualCalibrate(maxSensorValues, minSensorValues);
    return true;
}

uint16_t SensorPanel::readLine(uint16_t *sensorValues) {
    return SensorPanel::qtr.readLineBlack(sensorValues);
}

void SensorPanel::read() {
    SensorPanel::position = SensorPanel::readLine(panelReading);

    error = (int) position - 8000;

    for (int i = 0; i < SensorPanel::SensorCount; i++) {
        rawReadings[i] = panelReading[i];
        panelReading[i] = panelReading[i] > 700 ? 1 : 0;
    }

    SensorPanel::updatePattern();
}

void SensorPanel::updatePattern() {
    SensorPanel::isMiddle = false;
    for (int i = 6; i <= 9; i++) {
        if (SensorPanel::panelReading[i] == 1) {
            SensorPanel::isMiddle = true;
        }
    }

    int sensorCount = 0;

    for (int i = 0; i < 16; i++) {
        sensorCount += panelReading[i];
    }

    left = false;
    right = false;
    for (int i = 0; i < 2; i++) {
        if (SensorPanel::panelReading[i] == 1) {
            left = true;
        }
        if (SensorPanel::panelReading[15 - i] == 1) {
            right = true;
        }
    }

    if (sensorCount == 0) {
        SensorPanel::pattern = 0;
    } else if (left && right && SensorPanel::isMiddle) {
        SensorPanel::pattern = 'T';
    } else if (left && SensorPanel::isMiddle) {
        SensorPanel::pattern = 'L';
    } else if (right && SensorPanel::isMiddle) {
        SensorPanel::pattern = 'R';
    } else {
        SensorPanel::pattern = 1;
    }

    isEnd = true;
    for (int i = 0; i < 3; i++) {
        isEnd &= (panelReading[i] == 1) && (panelReading[15 - i] == 1);
    }
    int midWhiteCount = 0;
    for (int i = 5; i <= 10; i++) {
        midWhiteCount += panelReading[i] == 0 ? 1 : 0;
    }
    isEnd &= (midWhiteCount >= 1);
}