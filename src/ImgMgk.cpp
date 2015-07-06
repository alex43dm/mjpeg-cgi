#include <list>

#include "ImgMgk.h"
#include "Log.h"
#include "Config.h"

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
    Magick::Blob blob1;
    Magick::Blob blob(data,length);

    try
    {
        Magick::Image image;
        image.read(blob);

        image.annotate(text, Magick::SouthEastGravity);

        image.magick("JPEG"); // Set JPEG output format
        image.write(&blob1);
    }
    catch( std::exception &error_ )
    {
        Log::err("Caught exception: %s", error_.what());
        return blob;
    }

    return blob1;
}

Magick::Blob ImgMgk::def(const std::string &text)
{
    Magick::Blob blob1;
    try
    {
        Magick::Image image(Magick::Geometry(cfg->camWidth,cfg->camHeight), Magick::Color("white"));
        image.magick("JPEG" );
        //yge.annotate(text, Magick::SouthEastGravity);

        image.write(&blob1);
    }
    catch( std::exception &error_ )
    {
        Log::err("Caught exception: %s", error_.what());
    }
    return blob1;
}

Magick::Blob ImgMgk::gray(const void *data, size_t length, const std::string &text)
{
    Magick::Blob blob(data,length);

    try
    {
        Magick::Image image;
        image.read(blob);

        image.type( Magick::GrayscaleType );
        image.annotate(text, Magick::SouthEastGravity);

        Magick::Blob blob1;
        image.magick("JPEG"); // Set JPEG output format
        image.write(&blob1);
        return blob;
    }
    catch( std::exception &error_ )
    {
        Log::err("Caught exception: %s", error_.what());
    }

    return blob;
}
