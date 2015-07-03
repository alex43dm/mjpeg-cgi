#include <fcgiapp.h>

#include <sstream>

#include "Log.h"
#include "Request.h"

Request::Request(FCGX_Request *req) : status(200)
{
    char *tmp_str = nullptr;

    if (!(tmp_str = FCGX_GetParam("QUERY_STRING", req->envp)))
    {
        Log::warn("query string is not set");
        status = 404;
    }
    else
    {
        _pUrl = std::string(tmp_str);
    }

    tmp_str = nullptr;
    if( !(tmp_str = FCGX_GetParam("REMOTE_ADDR", req->envp)) )
    {
        Log::warn("remote address is not set");
        status = 404;
    }
    else
    {
        ip = std::string(tmp_str);
    }

    tmp_str = nullptr;
    if (!(tmp_str = FCGX_GetParam("SCRIPT_NAME", req->envp)))
    {
        Log::warn("script name is not set");
        status = 404;
    }
    else
    {
        scriptName = std::string(tmp_str);
    }

    std::string::size_type pos = _pUrl.find_first_of('?');
    if (pos == std::string::npos)
        pos = -1;

    while (pos < _pUrl.size() || pos == (std::string::size_type)-1)
    {
        std::string::size_type end = _pUrl.find_first_of('=', ++pos);
        if (end == std::string::npos) return;
        std::string param_name = _pUrl.substr(pos, end - pos);
        pos = end;

        end = _pUrl.find_first_of('&', ++pos);
        if (end == std::string::npos)
            end = _pUrl.size();
        std::string param_value = _pUrl.substr(pos, end - pos);
        pos = end;
        _pParams[param_name] = percent_decode(param_value);
    }
}

Request::~Request()
{
    //dtor
}

std::string Request::param(const std::string &par) const
{
    std::map<std::string,std::string>::const_iterator it = _pParams.find(par);
    if (it != _pParams.end())
        return it->second;
    else
        return std::string();
}

std::string Request::percent_decode(const std::string &str) const
{
    enum State
    {
        General,
        FirstPercentEncodedDigit,
        SecondPercentEncodedDigit
    } state = General;

    char first_char = '\0',
         second_char = '\0';
    std::stringstream result;

    for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
    {
        switch (state)
        {

        case General:
            if (*it != '%')
                result << *it;
            else
            {
                state = FirstPercentEncodedDigit;
                first_char = second_char = '\0';
            }
            break;

        case FirstPercentEncodedDigit:
            first_char = *it;
            if (is_hex_digit(first_char))
            {
                state = SecondPercentEncodedDigit;
            }
            else
            {
                result << "%" << first_char;
                state = General;
            }
            break;

        case SecondPercentEncodedDigit:
            second_char = *it;
            if (is_hex_digit(second_char))
            {

                result << char(
                           (hex_digit_to_int(first_char) << 4) |
                           hex_digit_to_int(second_char));
                state = General;
            }
            else
            {
                result << "%" << first_char << second_char;
                state = General;
            }
            break;
        }
    }
    return result.str();
}

bool Request::is_hex_digit(char hex_digit) const
{
    switch (hex_digit)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a':
    case 'A':
    case 'b':
    case 'B':
    case 'c':
    case 'C':
    case 'd':
    case 'D':
    case 'e':
    case 'E':
    case 'f':
    case 'F':
        return true;
    default:
        return false;
    }
}

int Request::hex_digit_to_int(char hex_digit) const
{
    switch (hex_digit)
    {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'a':
    case 'A':
        return 10;
    case 'b':
    case 'B':
        return 11;
    case 'c':
    case 'C':
        return 12;
    case 'd':
    case 'D':
        return 13;
    case 'e':
    case 'E':
        return 14;
    case 'f':
    case 'F':
        return 15;
    default:
        return 0;
    }
}
