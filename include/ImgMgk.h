#ifndef IMGMGK_H
#define IMGMGK_H

#include <Magick++.h>

class ImgMgk
{
    public:
        ImgMgk(char *argv[]);
        virtual ~ImgMgk();
        void conv(void *data, size_t length);
    protected:
    private:
};

#endif // IMGMGK_H
