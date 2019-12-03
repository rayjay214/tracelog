#ifndef  _GOOCAR_COMMON_ERROR_CODE_H_
#define  _GOOCAR_COMMON_ERROR_CODE_H_

#include <string>

enum error_code_e
{
    ERR_OK = 0,
    //////////// System error /////////////////
    // 系统内部错误，需要尽快修复
    ERR_SYSTEM_ERR_BEGIN = 1000,
    ERR_SYSTEM_ERROR,           // 含糊其辞的说法，一般不建议使用
    ERR_CONNECT_FAIL,           // 连接目标服务失败
    ERR_CALL_CGI_FAIL,
    ERR_CALL_PROXY_FAIL,
    ERR_CALL_SERVANT_FAIL,
    ERR_LOG_USER_OPERATE,       //用户操作日志添加失败
    ERR_PROTOBUF_SERIALIZE_ERR, //序列化Protobuf数据出错
    ERR_PROTOBUF_PARSE_ERR,     // 解析Protobuf数据出错
    ERR_PASS_OTHER_SHARD_DEVICE,  // 不处理其他分片的设备
    ERR_TIMEOUT,               // 调用超时
    ERR_ICE_EXCEPTION,         // ICE调用异常
    ERR_CGI_PROCESSOR_NOT_EXIST, //processor不存在
    ERR_CGI_CONFIG_ERR,          //CGI配置错误
    
    //ICE PARALL 模块错误
    ERR_ICE_PARALL_PARAM_EMPTY = 1100,
    ERR_ICE_PARALL_MAP_FUNC_ERROR,
    ERR_ICE_PARALL_ENGINE_BUSY,
    ERR_ICE_PARALL_MAP_FUNC_TIME_OUT,
    ERR_ICE_PARALL_MAP_FUNC_RETURN_ERROR,
    ERR_ICE_PARALL_REDUCE_ERROR,
    ERR_ICE_PARALL_THREAD_POOL_NOT_INIT,
    ERR_ICE_PARALL_THREAD_LOCK_MUTEX_ERROR,
    ERR_SERVANT_PARALL_BAD_ANY_CAST = 1120,
    ERR_SERVANT_PARALL_ICE_CALL_EXCEPTION,
    ERR_SERVANT_PARALL_CALL_TYPE_UNKNOWN,
    ERR_SERVANT_PARALL_CALL_PARAM_VALUE_EMPTY,
    ERR_SERVANT_PARALL_MAP_REQUEST_EMPTY,
    ERR_PART_GNS_ICE_FAIL = 1140,     // 获取多个GNS的ICE句柄时，部分获取有失败
    ERR_ALL_GNS_ICE_FAIL,               // 获取多个GNS的ICE句柄时，全部获取失败

    //调用系统模块错误
    ERR_CALL_SYS_MOD_BEGIN = 1200,
    ERR_CALL_GUD_FAIL,
    ERR_CALL_REDIS_FAIL,
    ERR_CALL_MYSQL_FAIL,
    ERR_CALL_GTC_FAIL,
    ERR_CALL_GMC_FAIL,
    ERR_CALL_GFS_FAIL,
    ERR_CALL_GMQ_FAIL,
    ERR_CALL_GSE_FAIL,
    ERR_CALL_GGB_FAIL,
    ERR_CALL_ACC_FAIL,
    ERR_CALL_DB_PROXY_FAIL,
	ERR_CALL_GOOMEPAY_FAIL,
	ERR_CALL_GOOMEIM_FAIL,
    
    //调用外部模块错误
    ERR_CALL_OTHER_MOD_BEGIN = 1300,
    ERR_CALL_GTREE_FAIL,
    ERR_CALL_GIMEI_FAIL,
    ERR_CALL_GLNAME_FAIL,
    ERR_CALL_GENID_FAIL,
    ERR_CALL_GALARM_SERVANT_FAIL,
    ERR_CALL_TBO_SERVANT_FAIL,
    ERR_CALL_BUS_PROXY_FAIL,
    ERR_CALL_GCID_FAIL,
    ERR_CALL_GIMAGE_FAIL,
    ERR_CALL_IOT_FAIL,  //调用IOT_BO（物联网）失败
    ERR_CALL_IOT_ISPGW_FAIL,  
    ERR_CALL_IOT_CMPPGW_FAIL,
    ERR_CALL_IOT_CARDPOOL_FAIL,
    
    // 新增加的系统内部错误类型，只能添加到这下面，不可以打乱上面的顺序，也不能随便删除（错误码必须是固定的）

    //////////// Data error /////////////////
    // 系统内数据有错误
    ERR_DATA_ERR_BEGIN = 2000,
    ERR_JSON_FORMAT,
    ERR_DATA_SIZE,
    ERR_EID_NOT_EXIST,      // 指定的客户ID不存在
    ERR_IMEI_NOT_EXIST,
    ERR_HTTP_QUERY_FAIL,    // HTTP请求失败
    ERR_METHOD_NOT_SUPPORTED,
    // 新增加的系统内数据错误类型，只能添加到这下面，不可以打乱上面的顺序，也不能随便删除（错误码必须是固定的）

    //////////// User error /////////////////
    // 由用户提交的操作不合规范引起的错误
    ERR_USER_ERR_BEGIN = 3000,
    ERR_INVALIDATE_PARAM,   // 参数有错误
    ERR_LACK_PARAM,         // 缺少参数
    ERR_PARSE_USER_TICKET,  // 解析用户身份失败
    ERR_NOT_YET_LOGIN,      // 用户还未登陆，无操作权限
    ERR_OP_RATE_LIMIT,      // 操作频率限制
    ERR_METHOD_NOT_EXIST,   // 调用方法不存在
    ERR_PASSWORD,           // 用户密码错误
    ERR_USER_NOT_EXIST,
    ERR_DATA_NOT_FOUND,     // 未查找到符合条件的数据
    ERR_QUERY_NUMBER_LIMIT, // 查询数量限制
    ERR_CHECK_FAIL,         // 数据检查不通过
    ERR_NOT_ENOUGH_PERMISSION, // 请求权限不足
    ERR_INVALID_OPERATION,   // 非法操作
    ERR_CHECK_SIGN_FAIL,     // SIGN校验失败
    ERR_LOGIN_NAME_ALREADY_EXIST,  //注册登录名已存在
    ERR_RESULT_SET_TOO_LARGE,  //结果集太大
    ERR_WL_CARD_OUT_TIME,		//用户物联卡过期
    ERR_WL_MSISDN_EMPTY,	//物联卡专网号为空，不能下发指令
    ERR_SIGN_HAS_EXPIRED,       // SIGN已过期
    ERR_NOT_ENOUGH_HISTORY_PERMISSION, // 查询历史轨迹权限不足
    ERR_PLACE_FOR_DEPT,       //该父节点不能插入部门
    ERR_CANT_MODIFY_DEPT,     //部门属性不能被修改
    ERR_DEPT_ONLY_HAS_DEPT,   //部门下面只能是部门
    ERR_WL_CARD_NONEED_RENEW,  //2018年9月1号以后的物联卡不用续费

    // 新增加的用户操作错误类型，只能添加到这下面，不可以打乱上面的顺序，也不能随便删除（错误码必须是固定的）
    ERR_CHARGE_MODULE_BEGIN = 4000, // 充值卡模块 + 短信充值错误码
    ERR_CHARGE_REPEAT_COMMIT,        // 5minute not repeat summit order
    //ERR_CHARGE_INVALIDATE_EID,      // 请使用ERR_INVALIDATE_PARAM替代
    // ERR_CHARGE_WITHOUT_PERMISSION,  // 请使用ERR_NOT_ENOUGH_PERMISSION替代
    // ERR_CHARGE_NOT_DEALER,         // 请使用ERR_NOT_ENOUGH_PERMISSION替代
    // ERR_CHARGE_NOT_CHILD,        // 请使用ERR_NOT_ENOUGH_PERMISSION替代
    // ERR_CHARGE_INVALIDATE_ORDERID,      // 请使用ERR_DATA_NOT_FOUND替代
    ERR_CHARGE_ORDER_STATUS_ERROR,       //订单状态错误，不能取消
    //ERR_CHARGE_LACK_REMARK,          // 请使用ERR_LACK_PARAM替代
    //ERR_CHARGE_PAYMONEY_ERROR,       //请使用ERR_INVALIDATE_PARAM替代
    ERR_CHARGE_WIOUT_CHARGECARD,     // 客户名下无充值卡
    ERR_CHARGE_OPE_DOMAIN_ERROR,      // 操作的referer 错误
    ERR_CHARGE_WRONG_PWD,       // 密码错误(售后接口)
    ERR_CHARGE_WITHOUT_PWD,     // 未设置密码(售后接口)
    ERR_CHARGE_EXIST_LIFELONG_IMIE,      // 充值存在终身设备
    ERR_CHARGE_CARD_NOT_SUFFICIENT,      // 充值卡不足
    ERR_CHARGE_TRANSFER_PARTIAL_SUCCESS,  // 充值卡转移部分成功
    ERR_CHARGE_WL_FORBIDDEN,            // 谷米卡禁止充值

	//UserGroup Errorcode
	ERR_USERGROUP_BEGIN = 4400,
	ERR_USERGROUP_CREATE_GROUP,
	ERR_USERGROUP_NAME_EXIST,
	ERR_USERGROUP_NOT_EMPTY,
	ERR_USERGROUP_CLEAR_DEFAULT_GROUP,
	ERR_USERGROUP_GET_GROUP_INFO,
	ERR_USERGROUP_GET_GROUP_USER,
	ERR_USERGROUP_GET_USER_INFO,
	ERR_USERGROUP_MODIFY_GROUP_INFO,
	ERR_USERGROUP_MOVE_GROUP_USER,
	ERR_USERGROUP_CLEAR_GROUP_USER,
	ERR_USERGROUP_DELETE_GROUP,

	//RentIncident
	ERR_RENT_BEGIN = 4500,
	ERR_RENT_DEVICENUM_NOT_ENOUGH,
	ERR_RENT_INCIDENTNUM_NOT_ENOUGH,
	ERR_RENT_EXIST,
	ERR_RENT_NOT_EXIST,

    //ShortMessage
	ERR_SHORT_MESSAGE_BEGIN = 4600,
	ERR_SHORT_MESSAGE_NOT_ENOUGH,

	//Account
	ERR_ACCOUNT_BEGIN = 4700,
	ERR_ACCOUNT_NAME_EXIST,
	ERR_ACCOUNT_NOT_EXIST,

	//GLname & GIMEI & GCID
    ERR_GLNAME_NAME_NOT_FOUND = 4800,	//GLNAME找不到对应的登录名
	ERR_GIMEI_IMEI_NOT_FOUND, 	//GIMEI中找不到对应的imei号
	ERR_GCID_CID_NOT_FOUND,	//GCID中找不到对应的CID信息

	//Search
	ERR_SEARCH_BEGIN = 4900,
	ERR_SEARCH_CACHE_ABNORMAL = 4901,
	ERR_SEARCH_FILE_ABNORMAL = 4902,
	
    //Delete
    ERR_DEL_ERROR = 5000,
    ERR_DEL_SUBEID_EXIST,        // 底下有下级客户
    ERR_DEL_SUBDEV_EXIST,        // 底下有子设备
    ERR_DEL_SUBACCOUNT_EXIST,    // 有虚拟账户
    ERR_DEL_CARD_EXIST,          // 有充值卡

    //Location
    ERR_LOCATION_ERROR = 6000,
    ERR_LOCATION_TOO_MANY_DEVICE = 6001,

	//APPPAY
	ERR_APPPAY_ERROR = 6100,
	ERR_APPPAY_MONEY_NOT_ENOUGH = 6101,
	ERR_APPPAY_CARD_NOT_NOUGH,
	ERR_APPPAY_NOT_COMPLETED,
	ERR_APPPAY_BIND_CARD_ERROR,
	ERR_APPPAY_NOT_TOP_DEALER,
	ERR_APPPAY_PLATEFEE_INVALID,
	ERR_APPPAY_FORBID_RENNEW_DEV,
	ERR_APPPAY_RENNEW_FEE_ERROR,
	ERR_APPPAY_RENNEW_FEE_NOT_MATCH,
	
	
	//GW01计划表模式相关错误码
	ERR_SCHEDULE_ERROR = 6400,
	ERR_SCHEDULE_PRODUCT_TYPE_NOT_SUPPORTED = 6401,
	ERR_SCHEDULE_MODE_INFO_NOT_EXIST,
    
    //公交系统错误码
    ERR_BUS_BEGIN = 10000,
    ERR_BUS_EID_NOT_EXIST,
    ERR_BUS_UID_NOT_EXIST,
	ERR_BUS_CITYCODE_NOT_EXIST,
	ERR_BUS_AD_ID_NOT_EXIST,
    ERR_BUS_GET_SHARDID_FAIL,
    ERR_BUS_GET_KDTREE_FAIL,
    ERR_BUS_STATION_NOT_IN_ORDER,
    ERR_BUS_TRACE_IDX_NOT_CORRECT,
    ERR_BUS_TOO_MANY_MASTERLINE_BY_CUSTOMER_ID,
    ERR_BUS_TOO_MANY_SUBLINE_BY_CUSTOMER_ID_AND_DIRECTION,
    ERR_BUS_EMPTY_MASTER_INFO_BEFORE_SET_SUBLINE_INFO,
    ERR_BUS_SUBLINEID_EMPTY_AFTER_INSERT_T_BUS_SUBLINE,
    ERR_BUS_QUERY_CITYCODE_BY_CUSTOMER_ID,
    ERR_BUS_GET_LINE_KD_TREE_DB,
    ERR_BUS_GET_NEAREST_PINT_FROM_KD_TREE,
    ERR_BUS_GET_NEAREST_IDX_POINT_FROM_KD_TREE,
    ERR_BUS_STATION_INFO_VEC_SIZE_NOT_EQUAL,
    ERR_BUS_EXIST_JW_STATION_TOO_SMALL,
	
	
	//金融系统错误码
	ERR_FINANCE_BEGIN = 11000,
	ERR_CAR_NOT_EXIST,                        // 车辆不存在
	ERR_CAR_PLATE_ALREADY_EXIST,              // 车牌号重复
	ERR_DEVICE_ALREADY_INSTALLED,             // 设备已装车
	ERR_DEVICE_NOT_INSTALLED,                 // 设备未装车
	ERR_NOT_BELONG_TO_CUSTOMER,               // 设备或车辆不属于客户或其子孙
	ERR_CAR_HAS_DEVICES_INSIDE,               // 车辆上有设备
	ERR_EID_OF_CAR_DEV_NOT_MATCH,             // 设备和车辆的所属客户不一致
	ERR_IDCARD_EXIST,                         // 身份证号重复
	
    
	ERR_END
};



inline std::string ToErrString(int errcode)
{
    std::string msg;
    // 注意：以下各枚举值出现的顺序，需与该枚举结构中定义的顺序一致，方便查找
    switch(errcode)
    {
    case ERR_OK:
        msg = "OK";
        break;
        //////////// System error /////////////////
        // 系统内部错误，需要尽快修复
    case ERR_SYSTEM_ERROR:
        msg = "system error";
        break;
    case ERR_CALL_GUD_FAIL:
		msg = "call gud error";
        break;
    case ERR_CALL_REDIS_FAIL:
    	msg = "call redis error";
        break;
    case ERR_CALL_MYSQL_FAIL:
		msg = "call mysql error";
        break;
    case ERR_CALL_GTC_FAIL:
		msg = "call GTC error";
        break;
    case ERR_CALL_GMC_FAIL:
		msg = "call GMC error";
        break;
    case ERR_CALL_GFS_FAIL:
		msg = "call GFS error";
        break;
    case ERR_CALL_GMQ_FAIL:
		msg = "call GMQ error";
        break;
    case ERR_CALL_GSE_FAIL:
		msg = "call GSE error";
        break;
    case ERR_CALL_GGB_FAIL:
		msg = "call GGB error";
        break;
    case ERR_CALL_ACC_FAIL:
		msg = "call ACC error";
        break;
    
    //调用外部模块错误
    case ERR_CALL_GTREE_FAIL:
		msg = "call Gtree error";
        break;
    case ERR_CALL_GIMEI_FAIL:
		msg = "call GIMEI error";
        break;
    case ERR_CALL_GLNAME_FAIL:
		msg = "call Glname error";
        break;
    case ERR_CALL_GENID_FAIL:
		msg = "call GenId error";
        break;
    case ERR_CALL_GALARM_SERVANT_FAIL:
		msg = "call GAlarm error";
        break;
    case ERR_CALL_TBO_SERVANT_FAIL:
		msg = "call TboServant error";
        break;
    
        //////////// Data error /////////////////
        // 系统内数据有错误
    case ERR_JSON_FORMAT:
        msg = "parse json data fail.";
        break;

        //////////// User error /////////////////
        // 由用户提交的操作不合规范引起的错误
    case ERR_INVALIDATE_PARAM:
        msg = "invalidate parameter";
        break;
    case ERR_LACK_PARAM:
        msg = "lack param";
        break;
    case ERR_PARSE_USER_TICKET:
        msg = "parse user ticket fail";
        break;
    case ERR_NOT_YET_LOGIN:
        msg = "please login first";
        break;
    case ERR_OP_RATE_LIMIT:
        msg = "operation too frequent";
        break;
    case ERR_METHOD_NOT_EXIST:
        msg = "call method not exist";
        break;
    case ERR_PASSWORD:
        msg = "password error";
        break;
    case ERR_USER_NOT_EXIST:
        msg = "user not exist,please check";
        break;
    case ERR_DATA_NOT_FOUND:
        msg = "data not found";
        break;
    case ERR_QUERY_NUMBER_LIMIT:
        msg = "over records number limit";
        break;
    case ERR_CHECK_FAIL:
        msg = "check data fail";
        break;
    case ERR_NOT_ENOUGH_PERMISSION:
        msg = "no permission";
        break;
    case ERR_INVALID_OPERATION:
		msg = "invalid operation";
		break;
    case ERR_CHECK_SIGN_FAIL:
		msg = "check sign fail";
		break;
    case ERR_LOGIN_NAME_ALREADY_EXIST:
		msg = "login_name exist";
		break;
    case ERR_RESULT_SET_TOO_LARGE:
		msg = "result is too large";
		break;
	case ERR_WL_CARD_OUT_TIME:
		msg = "wulian card is out of date";
		break;
	case ERR_WL_MSISDN_EMPTY:
		msg = "wulian card msisdn is empty";
		break;
    case ERR_NOT_ENOUGH_HISTORY_PERMISSION:
        msg = "no history permission";
        break;

	//charge error
    case ERR_CHARGE_REPEAT_COMMIT:
		msg = "repeat commit";
		break;
    case ERR_CHARGE_ORDER_STATUS_ERROR:
		msg= "order status error";
		break;
    case ERR_CHARGE_WIOUT_CHARGECARD:
		msg = "without charge card";
		break;
    case ERR_CHARGE_OPE_DOMAIN_ERROR:
		msg = "operation refer error";
		break;
    case ERR_CHARGE_WRONG_PWD:
		msg = "wrong passwd";
		break;
    case ERR_CHARGE_WITHOUT_PWD:
		msg = "without passwd";
		break;
    case ERR_CHARGE_EXIST_LIFELONG_IMIE:
		msg = "exist lifelong IMEI";
		break;
    case ERR_CHARGE_CARD_NOT_SUFFICIENT:
		msg = "charge card not enough";
		break;
    case ERR_CHARGE_TRANSFER_PARTIAL_SUCCESS:
		msg = "transfer partly success";
		break;

	//usergroup error
	case ERR_USERGROUP_CREATE_GROUP:
		msg = "create group failed";
		break;
	case ERR_USERGROUP_NAME_EXIST:
		msg = "usergroup name exist";
		break;
	case ERR_USERGROUP_NOT_EMPTY:
		msg = "usergroup not empty";
		break;
	case ERR_USERGROUP_CLEAR_DEFAULT_GROUP:
		msg = "cannot clear default group";
		break;
	case ERR_USERGROUP_GET_GROUP_INFO:
		msg = "get group info failed";
		break;
	case ERR_USERGROUP_GET_GROUP_USER:
		msg = "get group user failed";
		break;
	case ERR_USERGROUP_GET_USER_INFO:
		msg = "get user info failed";
		break;
	case ERR_USERGROUP_MODIFY_GROUP_INFO:
		msg = "modify group failed";
		break;
	case ERR_USERGROUP_MOVE_GROUP_USER:
		msg = "move group user failed";
		break;
	case ERR_USERGROUP_CLEAR_GROUP_USER:
		msg = "clear group user failed";
		break;
	case ERR_USERGROUP_DELETE_GROUP:
		msg = "delete group failed";
		break;
		
    case ERR_SHORT_MESSAGE_NOT_ENOUGH:
        msg = "please buy short message first";
        break;

	//account error
	case ERR_ACCOUNT_NAME_EXIST:
		msg = "account name exist";
		break;
	case ERR_ACCOUNT_NOT_EXIST:
		msg = "account not exist";
		break;

	//Glname error
	case ERR_GLNAME_NAME_NOT_FOUND:
		msg = "not found the name";
		break;
	case ERR_GIMEI_IMEI_NOT_FOUND:
		msg = "not found the IMEI";
		break;

	//search error
	case ERR_SEARCH_CACHE_ABNORMAL:
		msg = "search cache abnormal";
		break;
	case ERR_SEARCH_FILE_ABNORMAL:
		msg = "search file abnormal";
		break;
	
    //Delete
    case ERR_DEL_SUBEID_EXIST:
		msg = "sub eid exist";
		break;
    case ERR_DEL_SUBDEV_EXIST:
		msg = "sub device exist";
		break;
    case ERR_DEL_CARD_EXIST:
		msg = "card exist";
		break;
		
		
	//Bus error
	case ERR_BUS_CITYCODE_NOT_EXIST:
		msg = "citycode not exist";
		break;
	case ERR_BUS_TOO_MANY_SUBLINE_BY_CUSTOMER_ID_AND_DIRECTION:
        msg = "more than one subline find by on customer id and direction";
        break;
    case ERR_BUS_EMPTY_MASTER_INFO_BEFORE_SET_SUBLINE_INFO:
        msg = "empty master info before set subline info";
        break;
		

	//Finance error
	case ERR_CAR_NOT_EXIST:
		msg = "car not exist";
		break;
	case ERR_CAR_PLATE_ALREADY_EXIST:
        msg = "car plate already exist";
        break;
    case ERR_DEVICE_ALREADY_INSTALLED:
        msg = "device already installed";
        break; 
	case ERR_DEVICE_NOT_INSTALLED:
        msg = "device not installed";
        break; 
	case ERR_NOT_BELONG_TO_CUSTOMER:
        msg = "car or device not belong to customer";
        break; 
	case ERR_CAR_HAS_DEVICES_INSIDE:
		msg = "car has devices on it";
        break;
	case ERR_EID_OF_CAR_DEV_NOT_MATCH:
		msg = "eid of car and eid of device not match";
		break;
			
		
    default:
        msg = "unknow error";
        break;
    }
    return msg;
}



#endif // _GOOCAR_COMMON_ERROR_CODE_H_




