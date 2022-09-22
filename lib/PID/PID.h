//
// Created by lakshith on 8/14/22.
//

#ifndef ROBOFEST2022_PID_H
#define ROBOFEST2022_PID_H

#endif //ROBOFEST2022_PID_H

const double P = 0.087;
const double I = 0;
const double D = 1.7;

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

const double GP = 3;
const double GI = 0;
const double GD = 1;

double totalGyroError = 0.0;
double prevGyroError = 0.0;

int gyroPid(double error) {
    totalGyroError += error;

    double p = error * GP;
    double i = totalGyroError * GI;
    double d = (error - prevGyroError) * GD;

    prevGyroError = error;

    double correction = p + i + d;

    return (int)correction;
}