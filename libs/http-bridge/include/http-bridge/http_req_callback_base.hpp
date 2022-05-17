/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include <http-bridge/http_common.hpp>

#define NO_FCGI_DEFINES 1
#include <fcgi_config.h>
#include <fcgiapp.h>

#include <exception>
#include <stdexcept>

class HTTPException : public std::runtime_error
{
public:
  HTTPException(const int code, const char msg[]) : std::runtime_error(msg), m_code(code)
  {

  }
  int get_code() const
  {
    return m_code;
  }
  protected:
  const int m_code;
};

class BadRequest : public HTTPException
{
  public:
  BadRequest() : HTTPException(http_common::STATUS_CODES::BAD_REQUEST, "Bad Request")
  {

  }
  BadRequest(const char msg[]) : HTTPException(http_common::STATUS_CODES::BAD_REQUEST, msg)
  {

  }
};

class NotFound : public HTTPException
{
  public:
  NotFound() : HTTPException(http_common::STATUS_CODES::NOT_FOUND, "Not Found")
  {

  }
  NotFound(const char msg[]) : HTTPException(http_common::STATUS_CODES::NOT_FOUND, msg)
  {

  }
};

class MethodNotAllowed : public HTTPException
{
  public:
  MethodNotAllowed() : HTTPException(http_common::STATUS_CODES::METHOD_NOT_ALLOWED, "Method Not Allowed")
  {

  }
  MethodNotAllowed(const char msg[]) : HTTPException(http_common::STATUS_CODES::METHOD_NOT_ALLOWED, msg)
  {

  }
};

class InternalServerError : public HTTPException
{
  public:
  InternalServerError() : HTTPException(http_common::STATUS_CODES::INTERNAL_SERVER_ERROR, "Internal Error")
  {

  }
  InternalServerError(const char msg[]) : HTTPException(http_common::STATUS_CODES::INTERNAL_SERVER_ERROR, msg)
  {

  }
};

class ServiceUnavailable : public HTTPException
{
  public:
  ServiceUnavailable() : HTTPException(http_common::STATUS_CODES::SERVICE_UNAVAILABLE, "Service Unavailable")
  {

  }
  ServiceUnavailable(const char msg[]) : HTTPException(http_common::STATUS_CODES::SERVICE_UNAVAILABLE, msg)
  {

  }
};

class http_req_callback_base
{
public:

  http_req_callback_base()
  {

  }

  virtual ~http_req_callback_base()
  {
    
  }

  virtual void handle(FCGX_Request* const request)
  {
    FCGX_PutS("Status: 500 Internal Error\r\n", request->out);
    FCGX_Finish_r(request);
  }

protected:

};
