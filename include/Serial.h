#ifndef SERIAL_H
#define SERIAL_H

#include <string>

#include <pthread.h>

class Serial
{
    public:
        std::string portName;
        int8_t speedX, speedY;
        uint32_t shiftX, shiftY;
        bool stop;
        pthread_mutex_t Mtx;
        int fd;

        Serial(const std::string &portName, int8_t speedX, uint32_t shiftX, int8_t speedY, uint32_t shiftY);
        virtual ~Serial();

        bool Up(){ return moveY(false);};
        bool Down(){ return moveY(true);};
        bool Left(){ return moveX(false);};
        bool Right(){ return moveX(true);};
        bool Stop(){ return stop = true;}

        bool moveX(bool direction = true);
        bool moveY(bool direction = true);
        bool moveXY(bool directionX = true, bool directionY = true);
    protected:
    private:
        pthread_t m_threadId;

        bool cmd(int8_t speedX, uint32_t x, int8_t speedY, uint32_t y);
        static void *internalThreadFunc(void *data);
};

#endif // SERIAL_H
