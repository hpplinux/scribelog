#include "CConfigMgr.h"

CConfigMgr* CConfigMgr::m_pInstance = NULL;

CConfigMgr::CConfigMgr()
{

}

CConfigMgr::~CConfigMgr()
{

}

bool CConfigMgr::init( const string &sProgramName )
{
	m_stConfig.sProgramName  = sProgramName;
	m_stConfig.sCfgFile = m_stConfig.sProgramName + ".ini";

 	m_stConfig.SVR_VERSION = sProgramName + " version: " + __DATE__ + " " +  __TIME__;

	lce::CConfig oCfg;

	if(!oCfg.loadConfig(m_stConfig.sCfgFile))
	{
	    snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s",oCfg.getErrMsg());
		return false;
	}

	oCfg.getValue("sys", "daemon", m_stConfig.bDaemon,true);

	oCfg.getValue("sys", "open_core_file", m_stConfig.bUsingCore, false);
	oCfg.getValue("sys", "open_file_num", m_stConfig.dwMaxOpenFile,MAX_OPEN_FILE);
	oCfg.getValue("sys", "core_file_size", m_stConfig.dwMaxCoreFile,MAX_CORE_FILE);
	oCfg.getValue("sys", "thread_num", m_stConfig.wThreadNum,8);
	oCfg.getValue("sys", "task_queue_size", m_stConfig.dwTaskQueueSize,5000);

	oCfg.getValue("sys", "max_clients", m_stConfig.dwMaxClients,5000);

	oCfg.getValue("sys", "max_requests", m_stConfig.dwMaxRequests,5000);

	oCfg.getValue("log", "path",m_stConfig.sLogPath, "/tmp/");
	oCfg.getValue("log", "file_size", m_stConfig.dwLogFileSize, 5*1024*1024);
	oCfg.getValue("log", "file_count",m_stConfig.dwLogFileCount, 10);
	oCfg.getValue("log", "show_stdout", m_stConfig.bShowCmd, false);
	oCfg.getValue("log", "log_level", m_stConfig.cLogLevel, 0);
	oCfg.getValue("log", "show_line", m_stConfig.bShowLine, false);

	oCfg.getValue("comm", "udp_listen_ip", m_stConfig.sUdpIp,"127.0.0.1");
	oCfg.getValue("comm", "udp_port", m_stConfig.wUdpPort, 8080);
	oCfg.getValue("comm", "udp_recv_buffer", m_stConfig.dwUdpRecvBufSize, 10*1024);
	oCfg.getValue("comm", "udp_send_buffer",m_stConfig.dwUdpSendBufSize,  1024*1024);
	oCfg.getValue("comm", "udp_max_recv_buffer", m_stConfig.dwMaxUdpRecvBufSize, 100*1024);
	oCfg.getValue("comm", "udp_max_send_buffer",m_stConfig.dwMaxUdpSendBufSize,  10240*1024);

	oCfg.getValue("scribe","write_interval",m_stConfig.dwWriteInterval,5);
	oCfg.getValue("scribe","log_path",m_stConfig.sScribeLogPath,"/tmp/");
	oCfg.getValue("scribe","max_log_buffer_size",m_stConfig.dwMaxBufSize,1024*1024);
  	oCfg.getValue("scribe","log_buffer_size",m_stConfig.dwBufSize,1024*1024);

    CLog::init(m_stConfig.sLogPath+"/"+m_stConfig.sProgramName,m_stConfig.dwLogFileSize,m_stConfig.dwLogFileCount,m_stConfig.bShowCmd,m_stConfig.cLogLevel,m_stConfig.bShowLine);

	LOG_INFO("%s", m_stConfig.SVR_VERSION.c_str());

	LOG_INFO("****************************日志配置信息************************************");
	LOG_INFO("日志保存目录:%s",m_stConfig.sLogPath.c_str());
	LOG_INFO("日志文件大小:%lu, 文件个数:%u", m_stConfig.dwLogFileSize, m_stConfig.dwLogFileCount);


	//设置limit
	LOG_INFO("****************************系统配置信息************************************");
	if(!lce::setFileLimit(m_stConfig.dwMaxOpenFile))
	{
		LOG_ERROR("打开文件<count=%u>描述符限制出错:%s", m_stConfig.dwMaxOpenFile, strerror(errno));
	}
	else
	{
		LOG_INFO("程序可以使用文件描述符的个数: %u", m_stConfig.dwMaxOpenFile);
	}

	if (m_stConfig.bUsingCore)
	{
		if(!lce::setCoreLimit(m_stConfig.dwMaxCoreFile))
		{
			LOG_ERROR("打开core文件<size=%u>限制出错:%s", m_stConfig.dwMaxCoreFile, strerror(errno));
		}
		else
		{
			LOG_INFO("程序打开core文件大小: %u", m_stConfig.dwMaxCoreFile);
		}
	}

	//服务配置
	LOG_INFO("****************************服务配置信息************************************");

	LOG_INFO("udp server config:" );
	LOG_INFO("服务绑定IP:%s" , m_stConfig.sUdpIp.c_str());
	LOG_INFO("服务监听端口:%d" ,m_stConfig.wUdpPort);
	LOG_INFO("服务通信数据缓冲大小: recv=%lu; send=%lu", m_stConfig.dwUdpRecvBufSize, m_stConfig.dwUdpSendBufSize);
	LOG_INFO("服务通信数据最大缓冲大小: recv=%lu; send=%lu", m_stConfig.dwMaxUdpRecvBufSize, m_stConfig.dwMaxUdpSendBufSize );

	LOG_INFO("***************************************************************************");

	LOG_INFO("写入磁盘间隔时间:%d",m_stConfig.dwWriteInterval);
	LOG_INFO("写入路径:%s",m_stConfig.sScribeLogPath.c_str());
	LOG_INFO("最大缓冲大小:%d",m_stConfig.dwMaxBufSize);
	LOG_INFO("初始化缓冲大小:%d",m_stConfig.dwBufSize);

	LOG_INFO("***************************************************************************");

	if (m_stConfig.bDaemon)
	{
        LOG_INFO("设置为后台运行...");
		lce::initDaemon();
	}

	return true;
}



