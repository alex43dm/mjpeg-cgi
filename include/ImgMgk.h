#ifndef IMGMGK_H
#define IMGMGK_H

#include <Magick++.h>

class ImgMgk
{
    public:
        ImgMgk();
        virtual ~ImgMgk();
        static void init(char *argv[]);
        static Magick::Blob conv(const void *data, size_t length, const std::string &text);
    protected:
    private:
};

#endif // IMGMGK_H
