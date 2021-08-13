/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

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
  BadRequest() : HTTPException(400, "Bad Request")
  {

  }
  BadRequest(const char msg[]) : HTTPException(400, msg)
  {

  }
};

class InternalServerError : public HTTPException
{
  public:
  InternalServerError() : HTTPException(500, "Internal Error")
  {

  }
  InternalServerError(const char msg[]) : HTTPException(500, msg)
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
