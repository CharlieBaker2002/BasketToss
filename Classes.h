
#ifndef CLASSES_H
#define CLASSES_H
#include "DigitalIn.h"
#include "PinNamesTypes.h"
#include "mbed.h"
#include "Joystick.h" 
#include "N5110.h"
#include <vector>
#include <iostream>
#include <cstdlib>
#define DEG2RAD 0.0174533

extern N5110 lcd; 
extern DigitalOut blue;
enum class State {intro, waiting, shoot, aim, move, lose};
extern State state;

class ScreenObject {
     public:
        int x;
        int y;
        int* sprite;
};

class Hoop : public ScreenObject {
    public:
        bool hasBall;
        Hoop(int _x, int _y);
        void Draw();
};

class Ball : public ScreenObject {
    public:
        bool succesful;
        Ball();
        void Draw();
        void Shoot(float deg, float speed);
        bool CheckContact(Hoop* H); //Returns true if collision has occured
        bool Move(std::vector<Hoop*>); //returns true if succesful collision
    private:
        float xVel;
        float yVel;
        float xExtra;
        float yExtra;
        
};

#endif