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

    std::list<Magick::Drawable> draw_list;

    draw_list.push_back(Magick::DrawableText(10, 10, text));
    draw_list.push_back(Magick::DrawableFont("Dejavu", Magick::NormalStyle, 12, Magick::NormalStretch));
    // set the text color (the fill color must be set to transparent)
    //draw_list.push_back(Magick::DrawableStrokeColor(Magick::Color("black")));
    //draw_list.push_back(Magick::DrawableFillColor(Magick::Color("black")));

    image.draw(draw_list);

    Magick::Blob blob1;
    image.magick("JPEG"); // Set JPEG output format
    image.write(&blob1);

    return blob1;
}
