//
// Created by lakshith on 8/12/22.
//

#ifndef ROBOFEST2022_MOTORDRIVER_H
#define ROBOFEST2022_MOTORDRIVER_H

#endif //ROBOFEST2022_MOTORDRIVER_H

class MotorDriver {
public:
    void init(int *leftPins, int *rightPins);

    void forward(int speed);

    void backward(int speed);

    void turnLeft(int speed);

    void turnRight(int speed);

    void forward(int leftSpeed, int rightSpeed);

    void applyPID(int correction);

private:
    int leftPWM;
    int rightPWM;
    int leftDirection[2];
    int rightDirection[2];
};
