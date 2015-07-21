#ifndef SERIAL_H
#define SERIAL_H

#include <string>

#include <pthread.h>

class Serial
{
    public:
        Serial(const std::string &portName, int8_t speedX, uint32_t shiftX, int8_t speedY, uint32_t shiftY);
        virtual ~Serial();

        bool Up(){ return moveX(true);};
        bool Down(){ return moveX(false);};
        bool Left(){ return moveY(true);};
        bool Right(){ return moveY(false);};

        bool moveX(bool direction = true);
        bool moveY(bool direction = true);
        bool moveXY(bool directionX = true, bool directionY = true);
    protected:
    private:
        std::string portName;
        int8_t speedX, speedY;
        uint32_t shiftX, shiftY;
        int fd;
        pthread_mutex_t Mtx;

        bool cmd(int8_t speedX, uint32_t x, int8_t speedY, uint32_t y);

};

#endif // SERIAL_H
