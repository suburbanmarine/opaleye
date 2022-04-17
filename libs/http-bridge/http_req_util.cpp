#include "http-bridge/http_req_util.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <spdlog/spdlog.h>

http_req_util::http_req_util()
{
	QUERY_STRING      = "";
	REQUEST_METHOD    = "";
	CONTENT_TYPE      = "";
	CONTENT_LENGTH    = "";
	SCRIPT_NAME       = "";
	REQUEST_URI       = "";
	DOCUMENT_URI      = "";
	DOCUMENT_ROOT     = "";
	SERVER_PROTOCOL   = "";
	REQUEST_SCHEME    = "";
	HTTPS             = "";
	GATEWAY_INTERFACE = "";
	SERVER_SOFTWARE   = "";
	REMOTE_ADDR       = "";
	REMOTE_PORT       = "";
	SERVER_ADDR       = "";
	SERVER_PORT       = "";
	SERVER_NAME       = "";
	REDIRECT_STATUS   = "";
}
http_req_util::~http_req_util()
{

}

void http_req_util::load(FCGX_Request* const request)
{
	QUERY_STRING      = FCGX_GetParam("QUERY_STRING",    request->envp);
	REQUEST_METHOD    = FCGX_GetParam("REQUEST_METHOD",  request->envp);
	CONTENT_TYPE      = FCGX_GetParam("CONTENT_TYPE",    request->envp);
	CONTENT_LENGTH    = FCGX_GetParam("CONTENT_LENGTH",  request->envp);
	SCRIPT_NAME       = FCGX_GetParam("SCRIPT_NAME",     request->envp);
	REQUEST_URI       = FCGX_GetParam("REQUEST_URI",     request->envp);
	DOCUMENT_URI      = FCGX_GetParam("DOCUMENT_URI",    request->envp);
	DOCUMENT_ROOT     = FCGX_GetParam("DOCUMENT_ROOT",   request->envp);
	SERVER_PROTOCOL   = FCGX_GetParam("SERVER_PROTOCOL", request->envp);
	REQUEST_SCHEME    = FCGX_GetParam("REQUEST_SCHEME",  request->envp);
	HTTPS             = FCGX_GetParam("HTTPS",           request->envp);
	HTTPS             = (HTTPS) ? (HTTPS) : ("<null>");
	GATEWAY_INTERFACE = FCGX_GetParam("GATEWAY_INTERFACE", request->envp);
	SERVER_SOFTWARE   = FCGX_GetParam("SERVER_SOFTWARE",   request->envp);
	REMOTE_ADDR       = FCGX_GetParam("REMOTE_ADDR",       request->envp);
	REMOTE_PORT       = FCGX_GetParam("REMOTE_PORT",       request->envp);
	SERVER_ADDR       = FCGX_GetParam("SERVER_ADDR",       request->envp);
	SERVER_PORT       = FCGX_GetParam("SERVER_PORT",       request->envp);
	SERVER_NAME       = FCGX_GetParam("SERVER_NAME",       request->envp);
	REDIRECT_STATUS   = FCGX_GetParam("REDIRECT_STATUS",   request->envp);

	doc_uri_path = DOCUMENT_URI;

	request_method_enum = http_common::parse_req_method(REQUEST_METHOD);
}

void http_req_util::log_request_env()
{
    boost::property_tree::ptree request_env;
    request_env.put<std::string>("QUERY_STRING",      QUERY_STRING);
    request_env.put<std::string>("REQUEST_METHOD",    REQUEST_METHOD);
    request_env.put<std::string>("CONTENT_TYPE",      CONTENT_TYPE);
    request_env.put<std::string>("CONTENT_LENGTH",    CONTENT_LENGTH);
    request_env.put<std::string>("SCRIPT_NAME",       SCRIPT_NAME);
    request_env.put<std::string>("REQUEST_URI",       REQUEST_URI);
    request_env.put<std::string>("DOCUMENT_URI",      DOCUMENT_URI);
    request_env.put<std::string>("DOCUMENT_ROOT",     DOCUMENT_ROOT);
    request_env.put<std::string>("SERVER_PROTOCOL",   SERVER_PROTOCOL);
    request_env.put<std::string>("REQUEST_SCHEME",    REQUEST_SCHEME);
    request_env.put<std::string>("HTTPS",             HTTPS);
    request_env.put<std::string>("GATEWAY_INTERFACE", GATEWAY_INTERFACE);
    request_env.put<std::string>("SERVER_SOFTWARE",   SERVER_SOFTWARE);
    request_env.put<std::string>("REMOTE_ADDR",       REMOTE_ADDR);
    request_env.put<std::string>("REMOTE_PORT",       REMOTE_PORT);
    request_env.put<std::string>("SERVER_ADDR",       SERVER_ADDR);
    request_env.put<std::string>("SERVER_PORT",       SERVER_PORT);
    request_env.put<std::string>("SERVER_NAME",       SERVER_NAME);
    request_env.put<std::string>("REDIRECT_STATUS",   REDIRECT_STATUS);

    boost::property_tree::ptree temp;
	temp.put_child("env", request_env);

    std::stringstream ss;
    boost::property_tree::write_json(ss, temp);
    SPDLOG_DEBUG("request env: {:s}", ss.str());
}
