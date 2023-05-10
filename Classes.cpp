#include "Classes.h"
#include "ThisThread.h"
#include <cmath>
#include <string>

//sideways orientation prioritises height over width
Hoop::Hoop(int _x, int _y) : ScreenObject(){
    x = _x;
    y = _y;
    hasBall = false;
    sprite = (int*)((const int [8][6])
    {
        {0,0,0,0,1,1},
        {0,0,1,1,0,0},
        {0,1,1,0,0,0},
        {1,0,0,0,0,0},
        {1,0,0,0,0,0},
        {0,1,1,0,0,0},
        {0,0,1,1,0,0},
        {0,0,0,0,1,1},
    });
}

Ball::Ball() : ScreenObject(){
    x = 6;
    y = 24;
    succesful = false;
    xVel = 0;
    yVel = 0;
    yExtra = 0;
    xExtra = 0;
    sprite = (int*)((const int[4][4]){
        {0, 1, 1, 0},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {0, 1, 1, 0}
    });
}

//offset to make x,y position central
void Hoop::Draw(){
    lcd.drawSprite(x - 3, y - 4, 8, 6, (int*)sprite);
}

void Ball::Draw(){
    lcd.drawSprite(x- 2,y - 2, 4, 4,sprite); 
}

float Distance(int x1, int x2, int y1, int y2){
    return(sqrt(pow(x1 - x2,2) + pow(y1 - y2,2)));
}

bool Ball::CheckContact(Hoop* H){
    if(Distance(x,H->x,y,H->y)<=5)
    {
        if(x > H->x)
        {
            if (abs(H->y - y) <= 3)
            {
                if(H->hasBall) //mega jump
                {
                    xVel += 3;
                    return true;
                }
                else //success
                {
                    succesful = true;
                    xVel = 0;
                    yVel = 0;
                    H->hasBall = true;
                    return true;
                }
            }
            else //mini jump
            {
                xVel += 1;
            }
        }
        else //bounce off bottom
        { 
            float xdir = x - H->x;
            float ydir = y - H->x;
            float normalizeFactor = sqrt(pow(xdir,2) + pow(ydir,2));
            xdir /= normalizeFactor;
            ydir /= normalizeFactor;
            normalizeFactor = sqrt(pow(xVel,2) + pow(yVel,2));
            xVel /= normalizeFactor;
            yVel /= normalizeFactor;
            float dot = xVel * xdir + ydir * yVel;
            xVel = (xVel - 2 * dot * xdir) * normalizeFactor;
            yVel = (yVel - 2 * dot * ydir) * normalizeFactor;
            return true;
        }
    }
    return false;
}

bool Ball::Move(std::vector<Hoop*> hoops){
    if(y >= 48) //wall collision
    {
        if(yVel > 0)
        {
            blue.write(1);
            ThisThread::sleep_for(100ms);
            blue.write(0);
            yVel *= -1;
            yExtra = -abs(yExtra);
        }
    }
    if(y < 0)
    {
        if(yVel < 0)
        {
            blue.write(1);
            ThisThread::sleep_for(100ms);
            blue.write(0);
            yVel *= -1;
            yExtra = abs(yExtra);
        }
    }
    if(x >= 84){
        if(xVel > 0){
            blue.write(1);
            ThisThread::sleep_for(100ms);
            blue.write(0);
            xVel *= -1;
            xExtra = -abs(xExtra);
        }
    }
    float temp = xVel + xExtra;
    x += std::round(temp);
    xExtra += temp - std::round(temp);
    temp = yVel + yExtra;
    y += std::round(temp);
    yExtra += temp - std::round(temp);
    xVel -= 0.006; //'gravity'
    for(auto h : hoops){
        if(CheckContact(h)){
            break;
        }
    }
    if(succesful){
        return true;
    }
    return false;
}

void Ball::Shoot(float deg, float speed){
    ThisThread::sleep_for(500ms);
    xVel = sinf(deg * DEG2RAD) * speed * 0.5;
    yVel = -cosf(deg * DEG2RAD) * speed * 0.5;
}

