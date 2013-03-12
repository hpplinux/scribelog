#include "CProcCenter.h"

CProcCenter*  CProcCenter::m_pInstance = NULL;

CProcCenter::CProcCenter(void)
{

}

CProcCenter::~CProcCenter(void)
{

}

int CProcCenter::init()
{
    m_stConfig = CConfigMgr::getInstance().getConfig();

    if(CTask::init(m_stConfig.wThreadNum,m_stConfig.dwTaskQueueSize) < 0)
    {
        return -1;
    }


	CCommMgr::getInstance().addTimer(WRITE_TIMER_ID,m_stConfig.dwWriteInterval,this);

    return 0;
}


void CProcCenter::onRead(SSession &stSession,const char * pszData, const int iSize)
{
	try
	{
		string sModule;
		string sMsg;

		PkgLog oPkg;
		oPkg.setPkgData(pszData,iSize);


		if(oPkg.head().getFieldNum() > 2)
		{
			LOG_WARN("field num invalid");
			return;
		}

		for(uint16_t i= 0;i<oPkg.head().getFieldNum();i++)
		{
			uint8_t cType = 0;
			oPkg>>cType;

			switch(cType)
			{
			case TYPE_MODULE:
				{
					oPkg.readString2(sModule);
				}			
			break;
			case TYPE_MSG:
				{
					oPkg.readString2(sMsg);
				}
			break;
			default:
				uint16_t wLen = 0;
				oPkg>> wLen;
				oPkg.seek(wLen);
			}
		}

		LOG_DEBUG("log msg module:%s,msg:%s",sModule.c_str(),sMsg.c_str());


		sMsg.append("\n");

		MAP_LOG_BUFFER::iterator it = m_mapLogBuffer.find(sModule);
		if(it != m_mapLogBuffer.end())
		{
			it->second->addData(sMsg.data(),sMsg.size());
		}
		else
		{
			if(m_mapLogBuffer.size() > MAX_MODULE_NUM)
			{
				LOG_ERROR("too many module");
				return;
			}
			CSocketBuf *pSocketBuf = new CSocketBuf(m_stConfig.dwBufSize,m_stConfig.dwMaxBufSize);
			m_mapLogBuffer[sModule]=pSocketBuf;
			pSocketBuf->addData(sMsg.data(),sMsg.size());
		}

	}
	catch(const CPackage<SHead>::Error e)
	{
		LOG_ERROR("package decode error: %s",e.what());
	}
}

void CProcCenter::onClose(SSession &stSession)
{
	LOG_INFO("Ip:%s,SessionId:%d close",stSession.getStrIp().c_str(),stSession.iFd);
	
}


void CProcCenter::onConnect(SSession &stSession,bool bOk)
{

}

void CProcCenter::onError(SSession &stSession,const char * szErrMsg,int iError)
{
	LOG_ERROR("onError %s",szErrMsg);
}

void CProcCenter::onTimer(uint32_t dwTimerId,void *pData)
{
	LOG_DEBUG("writing data...");
	dispatch(WRITE_MSG_ID,this);

	CCommMgr::getInstance().addTimer(WRITE_TIMER_ID,m_stConfig.dwWriteInterval,this);
}


void CProcCenter::onMessage(int dwMsgType,void *pData)
{
	if((int)dwMsgType == WRITE_MSG_ID)
	{
		SWriteInfo *pstInfo =(SWriteInfo*)pData;
		pstInfo->pSocketBuf->removeData(pstInfo->iSize);
		LOG_DEBUG("module:%s,size:%d write complete",pstInfo->sModule.c_str(),pstInfo->iSize);
		delete pstInfo;
	}
}

//线程函数 对于共享资源需加锁
void CProcCenter::onWork(int iTaskType,void *pData,int iIndex)
{
	for(MAP_LOG_BUFFER::iterator it=m_mapLogBuffer.begin();it!=m_mapLogBuffer.end();++it)
	{
		fstream oOutFile;
		string sLogFilePath= m_stConfig.sScribeLogPath;
		if(it->first.empty())
		{
			sLogFilePath+=getDateTimeStr()+".log";
		}
		else
		{
			sLogFilePath+=it->first+"-"+getDateTimeStr()+".log";
		}


		SWriteInfo *pstInfo = new SWriteInfo;
		oOutFile.open(sLogFilePath.c_str(),std::ios::out|std::ios::app);

		if(oOutFile.fail())
		{
			LOG_ERROR("open file %s error",sLogFilePath.c_str());
			continue;
		}
		if(it->second->getSize() == 0)
		{
			continue;
		}
		oOutFile.write(it->second->getData(),it->second->getSize());

		if(oOutFile.fail())
		{
			LOG_ERROR("write file %s error",sLogFilePath.c_str());
			continue;
		}

		pstInfo->iSize = it->second->getSize();


		pstInfo->sModule = it->first;
		pstInfo->pSocketBuf = it->second;
	
		CCommMgr::getInstance().sendMessage(iTaskType,this,pstInfo);
	}
}
