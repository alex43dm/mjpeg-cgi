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
        static Magick::Blob def(const std::string &text = "under reconstration");
        static Magick::Blob gray(const void *data, size_t length);
    protected:
    private:
};

#endif // IMGMGK_H
