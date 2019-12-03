#include "helper.h"
#include <sstream>
#include "JsonOp.h"


std::string GetHttpResponse(std::string res, int resType)
{
	std::ostringstream resp;
	switch(resType)
	{
	case RES_JS:
	    resp << HTML_HEAD_JS;
		break;
	case RES_EXCEL:
		resp << HTML_HEAD_EXCEL;
		break;
	case RES_TXT:
		resp << HTML_HEAD_TXT;
	   	break;
	default:
	   	resp << HTML_HEAD_JS;
	}

   	resp << res;
		
    return resp.str();
}

std::string GenLoginRespJson(::pb::LoginResp oPB)
{
	Document doc;
    Document::AllocatorType& allocator = doc.GetAllocator();
    doc.SetObject();
    rapidjson::Value data(kObjectType);
    rapidjson::Value val(kObjectType);

	OBJ_ADD_INT_MEMBER(data, "ret", oPB.ret());
	OBJ_ADD_STR_MEMBER(data, "msg", oPB.ret_msg());
	OBJ_ADD_STR_MEMBER(data, "session_id", oPB.session_id());
    OBJ_ADD_OBJ_MEMBER(doc, "data", data);

	rapidjson::StringBuffer strbuf;       
    MyWriter<rapidjson::StringBuffer> writer(strbuf);       
    doc.Accept(writer);        
    return strbuf.GetString();
}

std::string GenSetConfigRespJson(::pb::CfgInfoResp oPB)
{
	Document doc;
    Document::AllocatorType& allocator = doc.GetAllocator();
    doc.SetObject();
    rapidjson::Value data(kObjectType);
    rapidjson::Value val(kObjectType);

	OBJ_ADD_INT_MEMBER(data, "ret", oPB.ret());
	OBJ_ADD_STR_MEMBER(data, "msg", oPB.ret_msg());
    OBJ_ADD_OBJ_MEMBER(doc, "data", data);

	rapidjson::StringBuffer strbuf;       
    MyWriter<rapidjson::StringBuffer> writer(strbuf);       
    doc.Accept(writer);        
    return strbuf.GetString();
}

std::string GenGetLogRespJson(::pb::GetLogResp oPB)
{
	Document doc;
    Document::AllocatorType& allocator = doc.GetAllocator();
    doc.SetObject();
    rapidjson::Value data(kObjectType);
    rapidjson::Value val(kObjectType);

	rapidjson::Value arrLogText(rapidjson::kArrayType);
	for(int i = 0; i < oPB.log_text().size(); i++)
	{
		rapidjson::Value logText(kObjectType);
		const ::pb::LogText oLogText = oPB.log_text(i);
		OBJ_ADD_STR_MEMBER(logText, "ip_port", oLogText.ip_port());
		OBJ_ADD_STR_MEMBER(logText, "gns_name", oLogText.gns_name());
		rapidjson::Value arrText(rapidjson::kArrayType);
		for(int j = 0; j < oLogText.text().size(); j++)
		{
			rapidjson::Value textItem(kObjectType);
			OBJ_ADD_STR_MEMBER(textItem, "text", oLogText.text(j));	
			OBJ_ADD_ARRAY_MEMBER(arrText, textItem);	
		}
		OBJ_ADD_OBJ_MEMBER(logText, "text_array", arrText);
		OBJ_ADD_ARRAY_MEMBER(arrLogText, logText);
	}

	OBJ_ADD_INT_MEMBER(data, "ret", oPB.ret());
	OBJ_ADD_STR_MEMBER(data, "msg", oPB.ret_msg());
	OBJ_ADD_OBJ_MEMBER(data, "log_info", arrLogText);
    OBJ_ADD_OBJ_MEMBER(doc, "data", data);

	rapidjson::StringBuffer strbuf;       
    MyWriter<rapidjson::StringBuffer> writer(strbuf);       
    doc.Accept(writer);        
    return strbuf.GetString();
}

