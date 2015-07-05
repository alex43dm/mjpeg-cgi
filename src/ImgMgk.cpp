#include <list>

#include "ImgMgk.h"

ImgMgk::ImgMgk()
{
    //ctor
}

ImgMgk::~ImgMgk()
{
    //dtor
}

void ImgMgk::init(char *argv[])
{
    Magick::InitializeMagick(*argv);
}

Magick::Blob ImgMgk::conv(const void *data, size_t length, const std::string &text)
{
    Magick::Blob blob(data,length);
    Magick::Image image;
    image.read(blob);

    image.annotate(text, Magick::SouthEastGravity);

    Magick::Blob blob1;
    image.magick("JPEG"); // Set JPEG output format
    image.write(&blob1);

    return blob1;
}

Magick::Blob ImgMgk::def(const std::string &text)
{
    Magick::Image image(Magick::Geometry(640,480), Magick::Color("white"));
    image.magick("JPEG" );
    //image.font("-*-bitstream charter-medium-r-normal-*-*-*-*-*-*-*-iso8859-1");
    image.annotate(text, Magick::SouthEastGravity);

    Magick::Blob blob1;
    image.write(&blob1);

    return blob1;
}

Magick::Blob ImgMgk::gray(const void *data, size_t length)
{
    Magick::Blob blob(data,length);
    Magick::Image image;
    image.read(blob);

    image.type( Magick::GrayscaleType );

    Magick::Blob blob1;
    image.magick("JPEG"); // Set JPEG output format
    image.write(&blob1);

    return blob1;
}
