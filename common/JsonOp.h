#ifndef _JSON_COMMON_OP_H_
#define _JSON_COMMON_OP_H_



#include <string>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/stringbuffer.h"
#include "errorcode.h"

using namespace rapidjson;



std::string retJson(rapidjson::Document & doc, int errCode, const std::string &msg, rapidjson::Value & data, bool IsInnerAccess = true);
std::string retJson(error_code_e errCode, rapidjson::Value & data, bool IsInnerAccess = true);
std::string retJson(error_code_e errCode, rapidjson::Value & pos, int resEndTime, bool IsInnerAccess = true);
std::string retJson(error_code_e errcode, bool IsInnerAccess = true);
std::string retJson(int errCode,const std::string &msg, bool IsInnerAccess = true);


#define OBJ_ADD_ARRAY_MEMBER(obj, member)    do {obj.PushBack(member, allocator);} while (0)

#define ARRAY_ADD_STR_MEMBER(array, member)    do {array.PushBack(val.SetString(member.c_str(),member.length(),allocator),allocator);} while (0)
#define ARRAY_ADD_UINT64_MEMBER(array, member)    do {array.PushBack(val.SetUint64(member),allocator);} while (0)
#define ARRAY_ADD_INT_MEMBER(array, member)    do {array.PushBack(val.SetInt(member),allocator);} while (0)
#define ARRAY_ADD_UINT_MEMBER(array, member)    do {array.PushBack(val.SetUint(member),allocator);} while (0)
#define ARRAY_ADD_DOUBLE_MEMBER(array, member)    do {array.PushBack(val.SetDouble(member),allocator);} while (0)




#define OBJ_ADD_STR_MEMBER(obj,k, v)    do {obj.AddMember(k, v, allocator);} while (0)  // this won't work for std::string, unless you define "RAPIDJSON_HAS_STDSTRING" in your Makefile
#define OBJ_ADD_OBJ_MEMBER(obj,k, v)   do {obj.AddMember(k, v, allocator);} while (0)
#define OBJ_ADD_INT_MEMBER(obj,k, v)    do {obj.AddMember(k, val.SetInt(v), allocator);} while (0)
#define OBJ_ADD_UINT_MEMBER(obj,k, v)   do {obj.AddMember(k, val.SetUint(v), allocator);} while (0)
#define OBJ_ADD_DOUBLE_MEMBER(obj,k, v) do {obj.AddMember(k, val.SetDouble(v), allocator);} while (0)
#define OBJ_ADD_UINT64_MEMBER(obj,k, v) do {obj.AddMember(k, val.SetUint64(v), allocator);} while (0)
#define OBJ_ADD_BOOL_MEMBER(obj,k, v) 	do {obj.AddMember(k, val.SetBool(v), allocator);} while (0)

#define GetJsonString(obj, k, v)     do {if (obj.HasMember(k) && obj[k].IsString()) v = obj[k].GetString(); else v = "";} while (0)
#define GetJsonInt(obj, k, v)        do {if (obj.HasMember(k) && obj[k].IsInt()) v = obj[k].GetInt(); else v = 0xffffffff;} while (0)
#define GetJsonUint(obj, k, v)       do {if (obj.HasMember(k) && obj[k].IsUint()) v = obj[k].GetUint(); else v = 0xffffffff;} while (0)
#define GetJsonInt64(obj, k, v)      do {if (obj.HasMember(k) && obj[k].IsInt64()) v = obj[k].GetInt64(); else v = 0xffffffffffffffff;} while (0)
#define GetJsonUint64(obj, k, v)     do {if (obj.HasMember(k) && obj[k].IsUint64()) v = obj[k].GetUint64(); else v = 0xffffffffffffffff;} while (0)
#define GetJsonDouble(obj, k, v)     do {if (obj.HasMember(k) && obj[k].IsDouble()) v = obj[k].GetDouble(); else v = 0xffffffffffffffff;} while (0)
#define GetJsonBool(obj, k, v)       do {if (obj.HasMember(k) && obj[k].IsBool()) v = obj[k].GetBool(); else v = false;} while (0)

#define GET_JSON_STR_VAL(val, v)        do {if ((val.IsString()) v = val.GetString(); else v = "";} while (0)
#define GET_JSON_INT_VAL(val, v)        do {if ((val.IsInt()) v = val.GetInt(); else v = 0xffffffff;} while (0)
#define GET_JSON_UINT_VAL(val, v)       do {if ((val.IsUint()) v = val.GetUint(); else v = 0xffffffff;} while (0)
#define GET_JSON_INT64_VAL(val, v)      do {if ((val.IsInt64()) v = val.GetInt64(); else v = 0xffffffffffffffff;} while (0)
#define GET_JSON_UINT64_VAL(val, v)     do {if ((val.IsUint64()) v = val.GetUint64(); else v = 0xffffffffffffffff;} while (0)
#define GET_JSON_DOUBLE_VAL(val, v)     do {if ((val.IsDouble()) v = val.GetDouble(); else v = 0xffffffffffffffff;} while (0)
#define GET_JSON_BOOL_VAL(val, v)       do {if ((val.IsBool()) v = val.GetBool(); else v = false;} while (0)


bool getJsonVal(rapidjson::Document &doc, const std::string &key, rapidjson::Value &val);
bool getArraryVal(rapidjson::Document &doc, const std::string &key, rapidjson::Value &arrVal);
bool getArraryVal(rapidjson::Value &pval, Document::AllocatorType& allocator, const std::string &key, rapidjson::Value &arrVal);
bool getArraryRowVal(rapidjson::Document &doc, const std::string &key, uint32_t uIdx, rapidjson::Value &arrVal);


void modifyDouble(double &data);

template<typename Stream>
class MyWriter : public rapidjson::Writer<Stream>
{
public:
   MyWriter(Stream& stream) : rapidjson::Writer<Stream>(stream)
   {
   }

   bool Double(double d)
   {
      this->Prefix(rapidjson::kNumberType);
      char buffer[100];
      int ret = snprintf(buffer, sizeof(buffer), "%.6f", d);
      RAPIDJSON_ASSERT(ret >= 1);
      for (int i = 0; i < ret; ++i)
         PutN(*this->os_, buffer[i], 1);
      return true;
   }
};

#endif

