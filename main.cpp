#include "CgiService.h"

int main()
{
    CgiService(3,"/tmp/mjpeg-cgi.socket").run();

    return 0;
}
