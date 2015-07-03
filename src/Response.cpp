#include <algorithm>

#include "Response.h"
#include "Log.h"

#include <fcgi_stdio.h>

Response::Response(FCGX_Request *req):
    req(req)
{

}

void Response::ok(const std::string &content, const std::string &cookie)
{
    if(content.empty())
    {
        return;
    }

    FCGX_FPrintF(req->out,"Status: 200 OK\r\n");
    FCGX_FPrintF(req->out,"Content-type: text/html\r\n");
    FCGX_FPrintF(req->out,"Set-Cookie: %s\r\n", cookie.c_str());
    FCGX_FPrintF(req->out,"Pragma: no-cache\r\n");
    FCGX_FPrintF(req->out,"Expires: Fri, 01 Jan 1990 00:00:00 GMT\r\n");
    FCGX_FPrintF(req->out,"Cache-Control: no-cache, must-revalidate, no-cache=\"Set-Cookie\", private\r\n");
    FCGX_FPrintF(req->out,"\r\n");
    FCGX_FPrintF(req->out,"%s\r\n", content.c_str());
    FCGX_FFlush(req->out);
    FCGX_Finish_r(req);
}

void Response::status(unsigned status)
{
    if(req && req->out)
    {
        FCGX_FPrintF(req->out,"Status: %d OK\r\n",status);
        FCGX_FPrintF(req->out,"Content-type: text/html\r\n");
        FCGX_FPrintF(req->out,"\r\n\r\n");
        FCGX_FFlush(req->out);
        FCGX_Finish_r(req);
    }
}

Response::~Response()
{
    //dtor
}

void Response::startMJPG()
{
    _pBoundary = randomString(16);

    FCGX_FPrintF(req->out,"Cache-Control: no-cache\r\n");
    FCGX_FPrintF(req->out,"Pragma: no-cache\r\n");
    FCGX_FPrintF(req->out,"Expires: Thu, 01 Dec 1994 16:00:00 GMT\r\n");
    FCGX_FPrintF(req->out,"Connection: close\r\n");
    FCGX_FPrintF(req->out,"Content-Type: multipart/x-mixed-replace; boundary=--%s\r\n",_pBoundary.c_str());
    FCGX_FPrintF(req->out,"\r\n");
    FCGX_FFlush(req->out);

}

bool Response::nextMJPG(const unsigned char *img, int len)
{
    bool ret = true;

    FCGX_FPrintF(req->out,"--%s\r\n",_pBoundary.c_str());
    //FCGX_FPrintF(req->out,"Content-Disposition: form-data; name=\"mjpg\"\r\n");
    //FCGX_FPrintF(req->out,"Content-Transfer-Encoding: binary\r\n");
    FCGX_FPrintF(req->out,"Content-type: image/jpeg\r\n");
    FCGX_FPrintF(req->out,"Content-length: %d\r\n",len);
    FCGX_FPrintF(req->out,"\r\n");
    int wlen = FCGX_PutStr((const char *)img, len, req->out);
    if(wlen != len)
    {
        //Log::err("FCGX_PutStr");
        ret = false;
    }
    else
    {
        FCGX_FFlush(req->out);
    }

    return ret;
}

void Response::endMJPG()
{
    FCGX_FPrintF(req->out,"--%s--\r\n",_pBoundary.c_str());
    FCGX_FPrintF(req->out,"\r\n");
    FCGX_FFlush(req->out);
}

std::string Response::randomString( size_t length )
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
//        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}
