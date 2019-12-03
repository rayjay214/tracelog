#ifndef MUDUO_FASTCGI_FASTCGI_H
#define MUDUO_FASTCGI_FASTCGI_H

#include <map>
#include "Buffer.h"

#define FCGI_MAX_LENGTH 0xffff

typedef struct tagRecordHeader
{
    uint8_t version;
    uint8_t type;
    uint16_t id;
    uint16_t length;
    uint8_t padding;
    uint8_t unused;
}RecordHeader;

const unsigned kRecordHeader = static_cast<unsigned>(sizeof(RecordHeader));

enum FcgiType
{
    kFcgiInvalid = 0,
    kFcgiBeginRequest = 1,
    kFcgiAbortRequest = 2,
    kFcgiEndRequest = 3,
    kFcgiParams = 4,
    kFcgiStdin = 5,
    kFcgiStdout = 6,
    kFcgiStderr = 7,
    kFcgiData = 8,
    kFcgiGetValues = 9,
    kFcgiGetValuesResult = 10,
};

enum FcgiRole
{
    // kFcgiInvalid = 0,
    kFcgiResponder = 1,
    kFcgiAuthorizer = 2,
};

enum FcgiConstant
{
    kFcgiKeepConn = 1,
};

using namespace std;

// one FastCgiCodec per TcpConnection
// both lighttpd and nginx do not implement multiplexing,
// so there is no concurrent requests of one connection.
class FastCgiCodec
{
 public:
  typedef std::map<string, string> ParamMap;

  explicit FastCgiCodec()
    : gotRequest_(false),
      keepConn_(false)
  {
  }

  bool gotRequest_;
  bool keepConn_;
  muduo::net::Buffer stdin_;
  muduo::net::Buffer paramsStream_;
  ParamMap params_;
  time_t reqBeginTime_;
};

#endif  // MUDUO_FASTCGI_FASTCGI_H
