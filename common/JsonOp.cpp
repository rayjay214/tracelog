#include "JsonOp.h"
#include "StringUtility.h"

void modifyDouble(double &data)
{
    char buffer[100] = {0};
    snprintf(buffer, sizeof(buffer), "%.6f", data);
    data = atof(buffer);
    return;
}

std::string retJson(rapidjson::Document & doc, int errCode, const std::string &msg, rapidjson::Value & data, bool IsInnerAccess)
{
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    rapidjson::Value val;
    bool success = (0 == errCode);

    if (IsInnerAccess)
    {
        OBJ_ADD_BOOL_MEMBER(doc, "success", success);
        OBJ_ADD_INT_MEMBER(doc,"errcode", errCode);
    }
    else
    {
        OBJ_ADD_INT_MEMBER(doc,"ret", errCode);
    }
    OBJ_ADD_STR_MEMBER(doc,"msg", msg);
    //OBJ_ADD_OBJ_MEMBER(doc, "data", data);
    if (!success && !IsInnerAccess)
    {
        //外部接口 如果接口调用失败 不返回data字段
    }
    else
    {
        OBJ_ADD_OBJ_MEMBER(doc, "data", data);
    }

    rapidjson::StringBuffer strbuf;       
    MyWriter<rapidjson::StringBuffer> writer(strbuf);       
    doc.Accept(writer);        
    return strbuf.GetString();
}

std::string retJson(error_code_e errCode, rapidjson::Value & data, bool IsInnerAccess)
{
    Document root;
    Document::AllocatorType& allocator = root.GetAllocator();
    root.SetObject();
    rapidjson::Value val;
    
    std::string msg = ToErrString(errCode);
    bool success = (0 == errCode);
    if (IsInnerAccess)
    {
        OBJ_ADD_BOOL_MEMBER(root, "success", success);
        OBJ_ADD_INT_MEMBER(root,"errcode", errCode);
    }
    else
    {
        OBJ_ADD_INT_MEMBER(root,"ret", errCode);
    }
    OBJ_ADD_STR_MEMBER(root,"msg", msg);

    if (!success && !IsInnerAccess)
    {
        //外部接口 如果接口调用失败 不返回data字段
    }
    else
    {
        OBJ_ADD_OBJ_MEMBER(root, "data", data);
    }

    rapidjson::StringBuffer strbuf;       
    MyWriter<rapidjson::StringBuffer> writer(strbuf);       
    root.Accept(writer);        
    return strbuf.GetString();
}

std::string retJson(error_code_e errCode, rapidjson::Value & pos, int resEndTime, bool IsInnerAccess)
{
	Document root;
    Document::AllocatorType& allocator = root.GetAllocator();
    root.SetObject();
    rapidjson::Value val;
	rapidjson::Value data(kObjectType);

	OBJ_ADD_OBJ_MEMBER(data, "pos", pos);
	OBJ_ADD_INT_MEMBER(data, "resEndTime", resEndTime);
    
    std::string msg = ToErrString(errCode);
    bool success = (0 == errCode);
    if (IsInnerAccess)
    {
        OBJ_ADD_BOOL_MEMBER(root, "success", success);
        OBJ_ADD_INT_MEMBER(root,"errcode", errCode);
    }
    else
    {
        OBJ_ADD_INT_MEMBER(root,"ret", errCode);
    }
    OBJ_ADD_STR_MEMBER(root,"msg", msg);
    //OBJ_ADD_OBJ_MEMBER(root, "data", data);
    if (!success && !IsInnerAccess)
    {
        //外部接口 如果接口调用失败 不返回data字段
    }
    else
    {
        OBJ_ADD_OBJ_MEMBER(root, "data", data);
    }

    rapidjson::StringBuffer strbuf;       
    MyWriter<rapidjson::StringBuffer> writer(strbuf);       
    root.Accept(writer);        
    return strbuf.GetString();
}



std::string retJson(error_code_e errcode, bool IsInnerAccess)
{
    std::string msg = ToErrString(errcode);

    return retJson(errcode, msg, IsInnerAccess);
}

std::string retJson(int errCode,const std::string &msg, bool IsInnerAccess)
{
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Value data(rapidjson::kObjectType);
    return retJson(doc, errCode, msg, data, IsInnerAccess);
}

bool getJsonVal(rapidjson::Document &doc, const std::string &key, rapidjson::Value &val)
{
    rapidjson::Value dir;

    std::vector<std::string> vecString;
    Goome::SplitString(key, "/", vecString);

    std::vector<std::string>::iterator iter = vecString.begin();
    rapidjson::Value::MemberIterator member = doc.FindMember(*iter);
    
    if (member == doc.MemberEnd())
    {
        return false;
    }

    if (iter != vecString.end())
    {
        for (++iter; iter != vecString.end(); ++iter)
        {
            dir.CopyFrom(member->value, doc.GetAllocator());
            if (!dir.IsObject())
            {
                break;
            }
            member = dir.FindMember(*iter);
            if (member == dir.MemberEnd())
            {
                break;
            }
        }
    }
    
    if (iter == vecString.end())
    {
        val.CopyFrom(member->value, doc.GetAllocator());
    }
    return !(val.IsNull());
}

bool getArraryVal(rapidjson::Document &doc, const std::string &key, rapidjson::Value &arrVal)
{
    rapidjson::Value val;
    getJsonVal(doc, key, val);
    if (val.IsArray())
    {
        arrVal.SetArray();
        rapidjson::Value::ValueIterator iter = val.Begin();
        for (; iter != val.End(); ++iter)
        {
            arrVal.PushBack(*iter, doc.GetAllocator());
        }
    }

    return arrVal.IsArray();
}

bool getArraryVal(rapidjson::Value &pval, Document::AllocatorType& allocator, const std::string &key, rapidjson::Value &arrVal)
{
    rapidjson::Value val;
    rapidjson::Value::MemberIterator member = pval.FindMember(key);
    if (member == pval.MemberEnd())
    {
        return false;
    }
    
    if (member->value.IsArray())
    {
        arrVal.SetArray();
        rapidjson::Value::ValueIterator iter = member->value.Begin();
        for (; iter != member->value.End(); ++iter)
        {
            arrVal.PushBack(*iter, allocator);
        }
    }

    return arrVal.IsArray();
}



bool  getArraryRowVal(rapidjson::Document &doc, const std::string &key, uint32_t uIdx, rapidjson::Value &arrVal)
{
    rapidjson::Value val;
    getJsonVal(doc, key, val);
    if (val.IsArray())
    {
        if (uIdx < val.Size())
        {
            arrVal.CopyFrom(val[uIdx], doc.GetAllocator());
        }
    }
    return !(arrVal.IsNull());
}

