#pragma once

#include "http-bridge/http_common.hpp"

#include <boost/filesystem/path.hpp>

#define NO_FCGI_DEFINES 1
#include <fcgi_config.h>
#include <fcgiapp.h>

class http_req_util
{
public:
	http_req_util();
	~http_req_util();

	void load(FCGX_Request* const request);

	void log_request_env();

	boost::filesystem::path doc_uri_path;
	http_common::REQUEST_METHOD request_method_enum;

	char const * QUERY_STRING;
	char const * REQUEST_METHOD;
	char const * CONTENT_TYPE;
	char const * CONTENT_LENGTH;
	char const * SCRIPT_NAME;
	char const * REQUEST_URI;
	char const * DOCUMENT_URI;
	char const * DOCUMENT_ROOT;
	char const * SERVER_PROTOCOL;
	char const * REQUEST_SCHEME;
	char const * HTTPS;
	char const * GATEWAY_INTERFACE;
	char const * SERVER_SOFTWARE;
	char const * REMOTE_ADDR;
	char const * REMOTE_PORT;
	char const * SERVER_ADDR;
	char const * SERVER_PORT;
	char const * SERVER_NAME;
	char const * REDIRECT_STATUS;
};
