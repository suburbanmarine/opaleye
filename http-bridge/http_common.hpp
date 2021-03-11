#pragma once

class http_common
{
public:
  enum STATUS_CODES : int
  {
    OK                            = 200,
    CREATED                       = 201,
    ACCEPTED                      = 202,
    NON_AUTHORITATIVE_INFORMATION = 203, // (since HTTP/1.1)
    NO_CONTENT                    = 204,
    RESET_CONTENT                 = 205,
    PARTIAL_CONTENT               = 206, // (RFC 7233)
    MULTI_STATUS                  = 207, // (WebDAV; RFC 4918)
    ALREADY_REPORTED              = 208, // (WebDAV; RFC 5842)
    IM_USED                       = 226, // (RFC 3229)

    // Client Errors
    BAD_REQUEST           = 400,
    UNAUTHORIZED          = 401, // (RFC 7235)
    PAYMENT_REQUIRED      = 402,
    FORBIDDEN             = 403,
    NOT_FOUND             = 404,
    METHOD_NOT_ALLOWED    = 405,
    NOT_ACCEPTABLE        = 406,
    PROXY_AUTHENTICATION  = 407, // REQUIRED (RFC 7235)
    REQUEST_TIMEOUT       = 408,
    CONFLICT              = 409,
    GONE                  = 410,
    LENGTH_REQUIRED       = 411,
    PRECONDITION_FAILED   = 412, // (RFC 7232)
    PAYLOAD_TOO_LARGE     = 413, // (RFC 7231)
    URI_TOO_LONG          = 414, // (RFC 7231)
    UNSUPPORTED_MEDIA     = 415, // TYPE (RFC 7231)
    RANGE_NOT_SATISFIABLE = 416, // (RFC 7233)
    EXPECTATION_FAILED    = 417,
    IM_A_TEAPOT           = 418, // (RFC 2324, RFC 7168)
    MISDIRECTED_REQUEST   = 421, // (RFC 7540)
    UNPROCESSABLE_ENTITY  = 422, // (WEBDAV; RFC 4918)
    LOCKED                = 423, // (WEBDAV; RFC 4918)
    FAILED_DEPENDENCY     = 424, // (WEBDAV; RFC 4918)
    TOO_EARLY             = 425, // (RFC 8470)
    UPGRADE_REQUIRED      = 426,
    PRECONDITION_REQUIRED = 428, // (RFC 6585)
    TOO_MANY_REQUESTS     = 429, // (RFC 6585)
    REQUEST_HEADER_FIELDS = 431, // TOO LARGE (RFC 6585)
    UNAVAIL_LEGAL_REASONS = 451, // (RFC 7725)

    // Server Errors
    INTERNAL_SERVER_ERROR      = 500,
    NOT_IMPLEMENTED            = 501,
    BAD_GATEWAY                = 502,
    SERVICE_UNAVAILABLE        = 503,
    GATEWAY_TIMEOUT            = 504,
    HTTP_VERSION_NOT_SUPPORTED = 505,
    VARIANT_ALSO_NEGOTIATES    = 506, //  (RFC 2295)
    INSUFFICIENT_STORAGE       = 507, //  (WEBDAV; RFC 4918)
    LOOP_DETECTED              = 508, //  (WEBDAV; RFC 5842)
    NOT_EXTENDED               = 510, //  (RFC 2774)
    NETWORK_AUTH_REQ           = 511 //  (RFC 6585)
  };
  static char const * get_status_code_str(const STATUS_CODES& status_code);
};
