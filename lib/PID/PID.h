//
// Created by lakshith on 8/14/22.
//

#ifndef ROBOFEST2022_PID_H
#define ROBOFEST2022_PID_H

#endif //ROBOFEST2022_PID_H

const double P = 0.01275;
const double I = 0;
const double D = 0.03;

int totalError = 0;
int prevError = 0;

int pid(int error) {
    totalError += error;

    double p = error * P;
    double i = totalError * I;
    double d = (error - prevError) * D;

    prevError = error;

    int correction = (int)(p + i + d);

    return correction;
}