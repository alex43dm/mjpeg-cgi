#include "ImgMgk.h"

ImgMgk::ImgMgk(char *argv[])
{
    //ctor
    Magick::InitializeMagick(*argv);
}

ImgMgk::~ImgMgk()
{
    //dtor
}

void ImgMgk::conv(void *data, size_t length)
{
    Magick::Blob blob( data, length );
    Magick::Image image;
    image.read(blob);

    Magick::Blob blob1;
    image.magick( "JPEG" ); // Set JPEG output format
    image.write( &blob1 );
}
