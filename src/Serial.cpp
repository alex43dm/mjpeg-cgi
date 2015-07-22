#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "Serial.h"
#include "Log.h"

Serial::Serial(const std::string &portName, int8_t speedX, uint32_t shiftX, int8_t speedY, uint32_t shiftY) :
    portName(portName),
    speedX(speedX),
    speedY(speedY),
    shiftX(shiftX),
    shiftY(shiftY),
    Mtx(PTHREAD_MUTEX_INITIALIZER)
{
    struct termios options;

    fd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    fcntl(fd, F_SETFL, 0);

    tcgetattr(fd, &options);

    cfsetispeed(&options, B115200);

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    tcsetattr(fd, TCSANOW, &options);
}

Serial::~Serial()
{
    close(fd);
}

void *Serial::internalThreadFunc(void *data)
{
    bool ret = true;
    char pre = 0xaa;
    Serial *s = (Serial*)data;

    while( !s->stop || ret)
    {
        pthread_mutex_lock(&s->Mtx);

        if( write(s->fd,(void*)&pre,1) != 1 ) ret = false;

        if( write(s->fd,(void*)&s->speedX,1) != 1 )  ret = false;
        if( write(s->fd,(void*)&s->shiftX,4) != 4) ret = false;

        if( write(s->fd,(void*)&s->speedY,1) != 1 )  ret = false;
        if( write(s->fd,(void*)&s->shiftY,4) != 4) ret = false;

        pthread_mutex_unlock(&s->Mtx);

        sleep(1);
    }

    if(!ret)
    {
        Log::err("write to device: %s",s->portName.c_str());
    }

    Log::debug("serial thread exit");

    return 0;
}

bool Serial::cmd(int8_t speedX, uint32_t x, int8_t speedY, uint32_t y)
{
    Log::debug("serial cmd: speedX: %d x:%d speedY: %d y:%d", speedX, x, speedY, y);

    if(stop)
    {
        pthread_create(&m_threadId, NULL, internalThreadFunc, this);
    }
    else
    {
        Log::err("serial run");
        stop = false;
    }

    return true;
}

bool Serial::moveX(bool direction)
{
    return cmd( direction ? speedX : -speedX, shiftX, 0, 0);
}

bool Serial::moveY(bool direction)
{
    return cmd( 0, 0, direction ? speedY : -speedY, shiftY);
}

bool Serial::moveXY(bool directionX, bool directionY)
{
     return cmd( directionX ? speedX : -speedX, shiftX, directionY ? speedY : -speedY, shiftY);
}
