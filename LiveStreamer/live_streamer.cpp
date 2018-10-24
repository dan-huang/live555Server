#include "live_streamer.h"


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
//    Medium::close(sms);
//
//    Medium::close(_rtspServer);
//
//    _env->reclaim();
//    delete _scheduler;
    
    LOGI("deinit live streamer");
}

bool LiveStreamer::init(CustomTaskFunc *onRRReceived){
    
    _scheduler         = BasicTaskScheduler::createNew();
    _env             = BasicUsageEnvironment::createNew(*_scheduler);
    _rtspServer     = RTSPServer::createNew(*_env, rtspPort, _authDB, timeout);
    
    if(_rtspServer == NULL)
    {
        
        LOGI("init rtsp server failed: %s", _env->getResultMsg());
        
        return false;
    }
    
    
    UsageEnvironment& env(_rtspServer->envir());
    sms = ServerMediaSession::createNew(env, url.c_str());
    
    _rtspServer->addServerMediaSession(sms);
    
    LOGI("init rtsp server success");
    
    _displaySource = H264_DisplayDeviceSource::createNew(*_env, queueSize, useThread, repeatConfig);
    
    _audioSource = DisplayDeviceSource::createNew(*_env, queueSize, useThread);
    
    
    if(_displaySource == NULL || _audioSource == NULL)
    {
        LOGI("unable to create source for device");
        return false;
    }
    
    OutPacketBuffer::maxSize = 600000;//DisplayDeviceSource::bufferedSize;
    video_replicator = StreamReplicator::createNew(*_env, _displaySource, false);
    
    audio_replicator = StreamReplicator::createNew(*_env, _audioSource, false);
    
    UnicastServerMediaSubsession *session = UnicastServerMediaSubsession::createNew(*_env, video_replicator, onRRReceived, True);
    
    UnicastServerMediaSubsession *audio_session = UnicastServerMediaSubsession::createNew(*_env,audio_replicator, NULL, False);
    
    sms->addSubsession(session);
    sms->addSubsession(audio_session);
    
    //            addSession(_rtspServer, url.c_str(), session, audio_session);
    
    LOGI("create rtsp server success");
    
    return true;
}


// This function will block current thread
void LiveStreamer::loop(CustomTaskFunc *onRRReceived)
{
    quit = 0;
    LOGI("START LOOP");
    _displaySource->startThread();
    _audioSource->startThread();
    
    _env->taskScheduler().doEventLoop(&quit);
    LOGI("END LOOP");
    
    
//    Medium::close(_displaySource);
//    Medium::close(_audioSource);
//
////    Medium::close(video_replicator);
////    Medium::close(audio_replicator);
//
//
//    Medium::close(session);
//    Medium::close(audio_session);
//
//    sms->deleteAllSubsessions();

//    Medium::close(sms);
//    Medium::close(_rtspServer);
//
//    if (_env->reclaim()) {
//        //            delete _env;
//        LOGI("delete env success");
//    } else {
//        LOGI("delete env failed");
//    }
//    delete _scheduler;

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
