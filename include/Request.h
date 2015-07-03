#ifndef REQUEST_H
#define REQUEST_H

#include <fcgiapp.h>

#include <map>
#include <string>

class Request
{
    public:
        unsigned status;
        std::string query, ip, scriptName, cookieValue;

        Request(FCGX_Request *req);
        virtual ~Request();

        std::map<std::string, std::string> params() const
        {
            return _pParams;
        }

        std::string param(const std::string &par) const;

    protected:
    private:
        std::string _pUrl;
        std::map<std::string, std::string> _pParams;

        std::string percent_decode(const std::string &str) const;
        bool is_hex_digit(char hex_digit) const;
        int hex_digit_to_int(char hex_digit) const;
};

#endif // REQUEST_H
