//
// Created by lakshith on 8/18/22.
//

#ifndef ROBOFEST2022_UTIL_H
#define ROBOFEST2022_UTIL_H

#include "Arduino.h"
#include "SensorPanel.h"
#include "PID.h"

#endif //ROBOFEST2022_UTIL_H

const int  redPin =19;
const int  bluePin =50;
const int  greenPin =48;


const String mappings[2][6] = {{"LBL", "LBS", "RBL", "SBL", "SBS", "LBR"},
                               {"S",   "R",   "B",   "R",   "B",   "B"}};

String mazeShort(String maze) {
    while (maze.indexOf("B") != -1) {
        for (int i = 0; i < 6; i++) {
            while (maze.indexOf(mappings[0][i]) != -1) {
                maze.replace(mappings[0][i], mappings[1][i]);
            }
        }
    }
    return maze;
}

void printReadings(const SensorPanel& qtr) {
    for (unsigned int i : qtr.panelReading) {
        Serial.print(i);
        Serial.print('\t');
    }
    Serial.println(qtr.pattern);
    Serial.print(qtr.error);
    Serial.print('\t');
    int correction = pid(qtr.error);
    Serial.println(correction);

    delay(250);
}

void showLight(char color){
    switch (color) {
        case 'R':
            digitalWrite(redPin,1);
            digitalWrite(greenPin,0);
            digitalWrite(bluePin,0);
            break;
        case 'G':
            digitalWrite(redPin,0);
            digitalWrite(greenPin,1);
            digitalWrite(bluePin,0);
            break;
        case 'B':
            digitalWrite(redPin,0);
            digitalWrite(greenPin,0);
            digitalWrite(bluePin,1);
            break;
        default:
            digitalWrite(redPin,0);
            digitalWrite(greenPin,0);
            digitalWrite(bluePin,0);
            break;
    }
}