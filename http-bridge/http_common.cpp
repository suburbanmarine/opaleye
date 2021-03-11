#include "http_common.hpp"

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
