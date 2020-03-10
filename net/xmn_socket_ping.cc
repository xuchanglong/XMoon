#include "comm/xmn_socket.h"
#include "xmn_memory.h"
#include "xmn_lockmutex.hpp"
#include "xmn_global.h"
#include "xmn_func.h"

int XMNSocket::PutInConnSockInfo2PingMultiMap(std::shared_ptr<XMNConnSockInfo>  connsockinfo)
{
    XMNMemory &memory = SingletonBase<XMNMemory>::GetInstance();
    time_t nexttime = time(nullptr) + pingwaittime_;

    XMNLockMutex pinglock(&ping_multimap_mutex_);
    XMNMsgHeader *pmsgheader = (XMNMsgHeader *)memory.AllocMemory(kMsgHeaderLen_, false);
    pmsgheader->currsequence = connsockinfo->currsequence;
    pmsgheader->connsockinfo = connsockinfo;

    ping_multimap_.insert(std::make_pair(nexttime, pmsgheader));
    ++ping_multimap_count_;

    ping_multimap_headtime_ = GetEarliestTime();
    return 0;
}

time_t XMNSocket::GetEarliestTime()
{
    std::multimap<time_t, XMNMsgHeader *>::iterator it;
    it = ping_multimap_.begin();
    return it->first;
}

void *XMNSocket::PingThread(void *pthreadinfo)
{
    std::shared_ptr<ThreadInfo> threadinfo_new((ThreadInfo *)pthreadinfo);
    XMNSocket *psocket = threadinfo_new->pthis_;

    time_t current_time;
    int err = 0;
    while (!g_isquit)
    {
        if (psocket->ping_multimap_count_ > 0)
        {
            current_time = time(nullptr);
            if (psocket->ping_multimap_headtime_ < current_time)
            {
                /**
                 * 初步判断出存在某个连接有心跳通信超时的情况。
                */
                std::list<XMNMsgHeader *> timeoutpinglist;
                XMNMsgHeader *pmsgheadertmp = nullptr;

                err = pthread_mutex_lock(&psocket->ping_multimap_mutex_);
                if (err != 0)
                {
                    XMNLogStdErr(err, "XMNSocket::PingThread()中pthread_mutex_lock()执行失败，返回值是 %d 。", err);
                }
                /**
                 * 通过互斥量，将所有的心跳通信超时的连接揪出来。
                */
                while ((pmsgheadertmp = psocket->GetOverTimeMsgHeader(current_time)) != nullptr)
                {
                    timeoutpinglist.push_back(pmsgheadertmp);
                }
                err = pthread_mutex_unlock(&psocket->ping_multimap_mutex_);
                if (err != 0)
                {
                    XMNLogStdErr(err, "XMNSocket::PingThread()pthread_mutex_unlock()执行失败，返回值是 %d 。", err);
                }
                while (!timeoutpinglist.empty())
                {
                    pmsgheadertmp = timeoutpinglist.front();
                    timeoutpinglist.pop_front();
                    /**
                     * 具体的判断一下该连接是否通信超时并超过指定的时间，若是如此，则断开该连接。
                    */
                    psocket->PingTimeOutChecking(pmsgheadertmp, current_time);
                }
            }
        }
    }

    return nullptr;
}

XMNMsgHeader *XMNSocket::GetOverTimeMsgHeader(const time_t &currenttime)
{
    if (ping_multimap_count_ == 0)
    {
        return nullptr;
    }
    XMNMemory &memory = SingletonBase<XMNMemory>::GetInstance();
    time_t earliesttime = GetEarliestTime();
    XMNMsgHeader *pmsgheadertmp = nullptr;

    if (earliesttime < currenttime)
    {
        /**
         * 确实存在超时的连接，即：没有在指定的时间间隔内
         * 完成心跳包的收发。
        */
        pmsgheadertmp = RemoveFirstMsgHeader();

        /**
         * 下次超时的时间依然需要判断，故需要更新时间并
         * 重新加入到监控 multimap 中。
        */
        time_t newputinmultimaptime = currenttime + pingwaittime_;
        XMNMsgHeader *pmsgheadertmp_new = (XMNMsgHeader *)memory.AllocMemory(kMsgHeaderLen_, false);
        pmsgheadertmp_new->currsequence = pmsgheadertmp->currsequence;
        pmsgheadertmp_new->connsockinfo = pmsgheadertmp->connsockinfo;
        ping_multimap_.insert(std::make_pair(newputinmultimaptime, pmsgheadertmp_new));
        ++ping_multimap_count_;

        if (ping_multimap_count_ > 0)
        {
            ping_multimap_headtime_ = GetEarliestTime();
        }
        return pmsgheadertmp;
    }
    return nullptr;
}

XMNMsgHeader *XMNSocket::RemoveFirstMsgHeader()
{
    if (ping_multimap_count_ == 0)
    {
        return nullptr;
    }
    XMNMsgHeader *pmsgheadertmp = nullptr;
    std::multimap<time_t, XMNMsgHeader *>::iterator it;

    it = ping_multimap_.begin();
    pmsgheadertmp = it->second;
    it = ping_multimap_.erase(it);
    --ping_multimap_count_;
    return pmsgheadertmp;
}

int XMNSocket::PingTimeOutChecking(XMNMsgHeader *pmsgheader, time_t currenttime)
{
    // XMNMemory *pmemory = SingletonBase<XMNMemory>::GetInstance();
    // pmemory->FreeMemory(pmemory);
    return 0;
}

int XMNSocket::PutOutMsgHeaderFromMultiMap(std::shared_ptr<XMNConnSockInfo>  connsockinfo)
{
    XMNMemory &memory = SingletonBase<XMNMemory>::GetInstance();
    XMNLockMutex pinglock(&ping_multimap_mutex_);

    std::multimap<time_t, XMNMsgHeader *>::iterator it;
    for (it = ping_multimap_.begin(); it != ping_multimap_.end();)
    {
        if (it->second->connsockinfo == connsockinfo)
        {
            memory.FreeMemory(it->second);
            it = ping_multimap_.erase(it);
            --ping_multimap_count_;
        }
        ++it;
    }

    if (ping_multimap_count_ > 0)
    {
        ping_multimap_headtime_ = GetEarliestTime();
    }

    return 0;
}

void XMNSocket::FreePingMultiMap()
{
    XMNMemory &memory = SingletonBase<XMNMemory>::GetInstance();
    for (auto &x : ping_multimap_)
    {
        memory.FreeMemory(x.second);
        --ping_multimap_count_;
    }
    ping_multimap_.clear();
}