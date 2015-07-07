#ifndef RESPONSE_H
#define RESPONSE_H

#include <fcgiapp.h>

#include <string>

class Response
{
    public:
        Response(FCGX_Request *req);
        virtual ~Response();
        void startMJPG();
        bool nextMJPG(const unsigned char *img, int len);
        void status(unsigned status);
        void ok(const std::string &content, const std::string &cookie);
        void endMJPG();
        bool sendMJPG(const std::string &);
    protected:
    private:
        FCGX_Request *req;
        std::string _pBoundary;

        std::string randomString( size_t length );
};

#endif // RESPONSE_H
