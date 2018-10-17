#include "live_streamer.h"
#include "DisplayDeviceSource.h"

#include <GroupsockHelper.hh>
#include <BasicUsageEnvironment.hh>

#include "H264_DisplayDeviceSource.h"
#include "ServerMediaSubsession.h"

LiveStreamer::LiveStreamer(unsigned int fps, unsigned int port) {
    fps = 30;
    rtspPort = 8554;
    multicast = false;
    url = "camera";
    murl = "camera";
    queueSize = 40;
    rtpPortNum = 20000;
    rtcpPortNum = rtpPortNum+1;
    ttl = 5;
    rtspOverHTTPPort = 0;
    useMmap = true;
    useThread = true;
    maddr = INADDR_NONE;
    repeatConfig = true;
    timeout = 65;
    _authDB = NULL;
    quit = 0;
}

#define DEBUG_RR 1

LiveStreamer::~LiveStreamer(){
    Medium::close(_displaySource);
    Medium::close(_audioSource);
    Medium::close(_rtspServer);
    
    _env->reclaim();
    delete _scheduler;
    
    LOGI("deinit live streamer");
}

void LiveStreamer::addSession(RTSPServer* rtspServer, const char* sessionName, ServerMediaSubsession *subSession, ServerMediaSubsession *audio_subSession)
{
    UsageEnvironment& env(rtspServer->envir());
    ServerMediaSession* sms = ServerMediaSession::createNew(env, sessionName);
    sms->addSubsession(subSession);
    sms->addSubsession(audio_subSession);
    rtspServer->addServerMediaSession(sms);

    char* url = rtspServer->rtspURL(sms);
    LOGI("Play this stream using the URL : %s",url);
    delete[] url;
}


bool LiveStreamer::init(CustomTaskFunc *onRRReceived){
    _scheduler 		= BasicTaskScheduler::createNew();
    _env 			= BasicUsageEnvironment::createNew(*_scheduler);
    _rtspServer 	= RTSPServer::createNew(*_env, rtspPort, _authDB, timeout);

    if(_rtspServer == NULL)
    {
        LOGI("create rtsp server failed");
        return false;
    }
    else
    {
        // if (rtspOverHTTPPort)
        // {
        // 	_rtspServer->setUpTunnelingOverHTTP(rtspOverHTTPPort);
        // }
        _displaySource = H264_DisplayDeviceSource::createNew(*_env, queueSize, useThread, repeatConfig);
        
        _audioSource = DisplayDeviceSource::createNew(*_env, queueSize, useThread);

        
        if(_displaySource != NULL && _audioSource != NULL)
        {
            OutPacketBuffer::maxSize = 600000;//DisplayDeviceSource::bufferedSize;
            StreamReplicator* replicator = StreamReplicator::createNew(*_env, _displaySource, false);

            StreamReplicator* audio_replicator = StreamReplicator::createNew(*_env, _audioSource, false);

            UnicastServerMediaSubsession *session = UnicastServerMediaSubsession::createNew(*_env,replicator, onRRReceived, True);
            
            UnicastServerMediaSubsession *audio_session = UnicastServerMediaSubsession::createNew(*_env,audio_replicator, NULL, False);
            
            addSession(_rtspServer, url.c_str(), session, audio_session);
            
            //            if (multicast)
            //            {
            //                if (maddr == INADDR_NONE) maddr = chooseRandomIPv4SSMAddress(*_env);
            //                destinationAddress.s_addr = maddr;
            //                LOGI("RTP  address :%s:%d", inet_ntoa(destinationAddress),rtpPortNum);
            //                LOGI("RTCP address :%s:%d", inet_ntoa(destinationAddress),rtcpPortNum);
            //                addSession(_rtspServer, murl.c_str(), MulticastServerMediaSubsession::createNew(*_env,destinationAddress,
            //                                                                                                Port(rtpPortNum), Port(rtcpPortNum), ttl, replicator, onRRReceived));
            //            } else {
            //
            //            }
            
        }
        else
        {
            LOGI("unable to create source for device");
            return false;
        }
    }
    return true;
}

// This function will block current thread
void LiveStreamer::loop()
{
    quit = 0;
    LOGI("START LOOP");
    _displaySource->startThread();
    _audioSource->startThread();
    _env->taskScheduler().doEventLoop(&quit);
    LOGI("END LOOP");

    _displaySource->stopThread();
    _audioSource->stopThread();
}


void LiveStreamer::dataPushed(char* data,unsigned int dataSize)
{
//    LOGI("push  raw data\t dataSize:%d",dataSize);
    _displaySource->pushRawData(data,dataSize);
}

void LiveStreamer::audioDataPushed(char* data,unsigned int dataSize)
{
    //    LOGI("push  raw data\t dataSize:%d",dataSize);
    _audioSource->pushRawData(data,dataSize);
}

void LiveStreamer::stop()
{
    LOGI("STOP....");
    quit = 1;
}
