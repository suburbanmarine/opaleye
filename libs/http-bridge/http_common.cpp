#include "http_common.hpp"

#include <cstring>

constexpr std::array<std::pair<char const * const, http_common::REQUEST_METHOD>, 9> http_common::REQUEST_METHOD_STR;

char const * http_common::get_status_code_str(const STATUS_CODES& status_code)
{
    char const* msg = "";
    switch(status_code)
    {
        default:
        {
            msg = "unknown";
            break;
        }
    }
    return msg;
}
http_common::REQUEST_METHOD http_common::parse_req_method(const char req_method[])
{
    if(strlen(req_method) != 0)
    {
        for(auto str : REQUEST_METHOD_STR)
        {
            if(strcasecmp(str.first, req_method) == 0)
            {
                return str.second;
            }
        }
    }

    return REQUEST_METHOD::INVALID;
}