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

bool Serial::cmd(int8_t speedX, uint32_t x, int8_t speedY, uint32_t y)
{
    bool ret = true;

    Log::debug("serial cmd: speedX: %d x:%d speedY: %d y:%d", speedX, x, speedY, y);

    pthread_mutex_lock(&Mtx);

    char pre = 0xaa;
    if( write(fd,(void*)&pre,1) != 1 ) ret = false;

    if( write(fd,(void*)&speedX,1) != 1 )  ret = false;
    if( write(fd,(void*)&x,4) != 4) ret = false;

    if( write(fd,(void*)&speedY,1) != 1 )  ret = false;
    if( write(fd,(void*)&y,4) != 4) ret = false;

    pthread_mutex_unlock(&Mtx);

    if(!ret)
    {
        Log::err("write to device: %s",portName.c_str());
    }

    return ret;
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
