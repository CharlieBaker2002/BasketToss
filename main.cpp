#include "Classes.h"
#include "N5110.h"
#include "PinNamesTypes.h"
#include "ThisThread.h"
#include <cstdio>
#include <ostream>
#include <string>

//Pin assignment format:  lcd(IO, Ser_TX, Ser_RX, MOSI, SCLK, PWM)  
N5110 lcd(PC_7, PA_9, PA_10, PB_5, PB_3, PC_8);
DigitalIn button(PA_13);
Joystick joy(PC_3, PC_2); 
DigitalOut green(PA_14); //CHECK PIN
DigitalOut red(PA_15); //CHECK PIN
DigitalOut blue(PA_11); //CHECK PIN

State state;
int score = 0;
int counter = 0;
int level = 1;
std::vector<Hoop*> hoops;
Ball currentBall;

float Random(float max = 1)
{
    float random = rand();
    random = random/(float)RAND_MAX;
    return random * max;
}

void Initialise(){
    lcd.init(LPH7366_1);     
    lcd.setContrast(0.5);  
    lcd.setBrightness(0.5);
    joy.init();
    button.mode(PullUp);
    state = State::intro;
    hoops = {};
}

void SetState(State s){
    state = s;
}

void DrawAll(bool refresh = true){
    lcd.clear();
    for(auto h : hoops){
        h->Draw();
    }
    std::cout << " X: " + std::to_string(currentBall.x) + " Y: " + std::to_string(currentBall.y) << std::endl;
    currentBall.Draw();
    if(refresh){
        lcd.refresh();
    }
}

void ClearHoops(){
    for(int i = 0; i < hoops.size(); i++)
    {
        delete hoops[i];
    }
    hoops.clear();
}

void Intro(){
    ClearHoops();
    counter = 0;
    score = 0;
    level = 1;
    std::cout << "Intro" << std::endl;
    lcd.clear();
    lcd.printString("Basket Toss", 10, 0);
    std::cout << "Title" << std::endl;
    lcd.refresh();
    ThisThread::sleep_for(2s);
    lcd.printString("Press Joystick", 0, 5);
    std::cout << "Cont" << std::endl;
    lcd.refresh();
    while(button.read() == 0){
        ThisThread::sleep_for(10ms);
    }
    SetState(State::waiting);
    lcd.clear();
}

void MakeHoops(){
    for(int i = 0; i < 3; i++){
        if(i == 0 || Random() < 0.2){
            int x = 10 + std::round(Random(44));
            int y = 9 + Random(30);
            hoops.emplace_back(new Hoop(x,y));
        }
    }
}

//Screen moving up to the next challenge and adding hoops
void Moving(){
    if(counter == level){
        SetState(State::waiting);
        return;
    }
    green.write(1);
    MakeHoops();
    int up = Random(40);
    for(auto h : hoops){
        h->x += up;
    }
    for(int i = 0; i < up; i++){
        for(auto h : hoops){
            h->x -= 1;
        }
        currentBall.x -= 1;
        DrawAll();
        ThisThread::sleep_for(50ms);
    }
    green.write(0);
    SetState(State::waiting);
}

//Ball has reached bottom of the screen, lose UI and restart
void Losing(){
    red.write(1);
    for(int i = 0; i < 3; i ++){
        for(float t = 0; t < 1; t+= 0.05){
            lcd.setBrightness(t);
            ThisThread::sleep_for(5ms);
        }
        for(float t = 1; t > 0; t-= 0.05){
            lcd.setBrightness(t);
            ThisThread::sleep_for(5ms);
        }
    }
    lcd.clear();
    lcd.refresh();
    ThisThread::sleep_for(1500ms);
    lcd.printString("You Lose", 16, 0);
    std::string s = std::to_string(score);
    lcd.printString("Score: ",0,5); 
    lcd.printString(s.c_str(), 50, 5);
    lcd.refresh();
    ThisThread::sleep_for(2s);
    red.write(0);
    lcd.setBrightness(0.5);
    SetState(State::intro);
}

//Start of each level, display level.
void Waiting(){
    if(counter == level){
        level+= 1;
        counter = 0;
        ThisThread::sleep_for(1s);
        lcd.clear();
        hoops.clear();
        lcd.printString("Level Complete!",0,0);
        lcd.refresh();
        ThisThread::sleep_for(1s);
    }
    if(counter == 0)
    {
        MakeHoops();
    }
    lcd.clear();
    std::string s = std::to_string(level);
    lcd.printString("Level: ",0,0); 
    lcd.printString(s.c_str(), 50, 0);
    s = std::to_string(score);
    lcd.printString("Score: ",0,5); 
    lcd.printString(s.c_str(), 50, 5);
    lcd.refresh();
    ThisThread::sleep_for(1s);
    lcd.printString("Press Joystick...", 0, 3);
    lcd.refresh();
    while(button.read() == 0){
        ThisThread::sleep_for(100ms);
    }
    SetState(State::aim);
}

//About to shoot, recording joystick data to determine strength of throw
void Aiming(){
    currentBall = Ball();
    std::vector<Vector2D> dirs = {};
    float angle;
    do
    {
        dirs.insert(dirs.begin(),joy.get_mapped_coord());
        if(dirs.size() > 100){
            dirs.pop_back();
        }
        angle = joy.get_angle();
        DrawAll(false);
        for(int i = 0; i < 20; i++){
            if(i%2 == 0){
                continue;
            }
            lcd.setPixel(currentBall.x + sin(angle * DEG2RAD) * i,currentBall.y + -cosf(angle * DEG2RAD) * i);
        }
        lcd.refresh();
        ThisThread::sleep_for(10ms);
    }
    while(joy.get_mag() < 0.975);
    int start = 0;
    float sqrMag = 1;
    float buffer;
    for(int i = 0; i < dirs.size(); i++){
        buffer = dirs[i].x * dirs[i].x + dirs[i].y + dirs[i].y;
        if(buffer < sqrMag){
            sqrMag = buffer;
            start = i;
        }
    }
    float iterationsToMax = dirs.size() - start;
    float magnitude = 1 + 3 * ((100-iterationsToMax)/(float)100);
    currentBall.Shoot(angle, magnitude);
    SetState(State::shoot);
}

//Ball moving, waiting for result.
void Shooting(){
    if(currentBall.Move(hoops)){ //returns true for succesful collision
        score += 1;
        counter +=1;
        SetState(State::move);
    }
    if(currentBall.x <= 0){ 
        SetState(State::lose);
    }
    DrawAll(true);
}


int main(){
    std::cout << "Hello world" << std::endl;
    Initialise();
    while (1) {
        ThisThread::sleep_for(10ms);
        //switch statements not possible for enum / enum class, hence if statements
        if(state == State::intro){
            std::cout << "Intro SM" << std::endl;
            Intro();
            continue;
        }
        else if(state == State::waiting){
            std::cout << "Waiting SM" << std::endl;
            Waiting();
            continue;
        }
        else if(state == State::aim){
            std::cout << "Aim SM" << std::endl;
            Aiming();
            continue;
        }
        else if(state == State::shoot){
            std::cout << "Shoot SM" << std::endl;
            Shooting();
            continue;
        }
        else if(state == State::move){
            std::cout << "Move SM" << std::endl;
            Moving();
            continue;
        }
        else if(state == State::lose){
            std::cout << "Lose SM" << std::endl;
            Losing();
            continue;
        }
        else{
            std::cout << "Non-Implemented State" << std::endl;
        }
    }
}
