#include <sstream>
// live555
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <Base64.hh>
// project
#include "ServerMediaSubsession.h"
#include "DisplayDeviceSource.h"

// ---------------------------------
//   BaseServerMediaSubsession
// ---------------------------------
FramedSource* BaseServerMediaSubsession::createSource(UsageEnvironment& env, FramedSource * videoES, Boolean isVideo)
{
    if (isVideo) {
        FramedSource*  source = H264VideoStreamDiscreteFramer::createNew(env, videoES);
        return source;
    } else {
        return videoES;
    }
}

RTPSink*  BaseServerMediaSubsession::createSink(UsageEnvironment& env, Groupsock * rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, Boolean isVideo)
{
    if (isVideo) {
        RTPSink* videoSink = H264VideoRTPSink::createNew(env, rtpGroupsock,rtpPayloadTypeIfDynamic);
        return videoSink;
    } else {
        RTPSink* audioSink = MPEG4GenericRTPSink::createNew(env, rtpGroupsock, rtpPayloadTypeIfDynamic,
                                       8000, "audio", "AAC-hbr", "aac_audio_sink", 1);
        // return audio
        return audioSink;
    }
}

char const* BaseServerMediaSubsession::getAuxLine(DisplayDeviceSource* source,unsigned char rtpPayloadType)
{
    const char* auxLine = NULL;
    if (source)
    {
        std::ostringstream os;
        os << "a=fmtp:" << int(rtpPayloadType) << " ";
        os << source->getAuxLine();
        os << "\r\n";
        auxLine = strdup(os.str().c_str());
    }
    return auxLine;
}

// -----------------------------------------
//    ServerMediaSubsession for Unicast
// -----------------------------------------
UnicastServerMediaSubsession* UnicastServerMediaSubsession::createNew(UsageEnvironment& env, StreamReplicator* replicator,TaskFunc* handlerTask, Boolean isVideo)
{
    return new UnicastServerMediaSubsession(env,replicator, handlerTask, isVideo);
}

FramedSource* UnicastServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
//    estBitrate = 50000; // kbps, estimate
    FramedSource* source = m_replicator->createStreamReplica();
    return createSource(envir(), source, fIsVideo);
}

RTPSink* UnicastServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,  unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
    return createSink(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, fIsVideo);
}

RTCPInstance* UnicastServerMediaSubsession
::createRTCP(Groupsock* RTCPgs, unsigned totSessionBW, /* in kbps */
             unsigned char const* cname, RTPSink* sink) {
    // Default implementation; may be redefined by subclasses:
    
    RTCPInstance* rtcpInstance = RTCPInstance::createNew(envir(), RTCPgs, totSessionBW, cname, sink, NULL/*we're a server*/);
    if (fTaskFuncOnRRReceived != NULL) {
        rtcpInstance->setRRHandler(fTaskFuncOnRRReceived, sink);
    }
    return rtcpInstance;
}

char const* UnicastServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink,FramedSource* inputSource)
{
    return this->getAuxLine((DisplayDeviceSource*)(m_replicator->inputSource()), rtpSink->rtpPayloadType());
}











// -----------------------------------------
//    ServerMediaSubsession for Multicast
// -----------------------------------------
MulticastServerMediaSubsession* MulticastServerMediaSubsession::createNew(UsageEnvironment& env
        , struct in_addr destinationAddress
        , Port rtpPortNum, Port rtcpPortNum
        , int ttl
        , StreamReplicator* replicator,TaskFunc* handlerTask
)
{
    // Create a source
    FramedSource* source = replicator->createStreamReplica();
    FramedSource* videoSource = createSource(env, source, True);

    // Create RTP/RTCP groupsock
    Groupsock* rtpGroupsock = new Groupsock(env, destinationAddress, rtpPortNum, ttl);
    Groupsock* rtcpGroupsock = new Groupsock(env, destinationAddress, rtcpPortNum, ttl);

    // Create a RTP sink  
    RTPSink* videoSink = createSink(env, rtpGroupsock, 96, True);

    // Create 'RTCP instance'
    const unsigned maxCNAMElen = 100;
    unsigned char CNAME[maxCNAMElen+1];
    gethostname((char*)CNAME, maxCNAMElen);
    CNAME[maxCNAMElen] = '\0';
    RTCPInstance* rtcpInstance = RTCPInstance::createNew(env, rtcpGroupsock,  500, CNAME, videoSink, NULL);

//    rtcpInstance->setRRHandler(onRRReceived, videoSink);
//    rtcpInstance->setRRHandler(handlerTask, videoSink);

    // Start Playing the Sink
    videoSink->startPlaying(*videoSource, NULL, NULL);

    return new MulticastServerMediaSubsession(replicator, videoSink, rtcpInstance);
}

char const* MulticastServerMediaSubsession::sdpLines()
{
    if (m_SDPLines.empty())
    {
        // Ugly workaround to give SPS/PPS that are get from the RTPSink
        m_SDPLines.assign(PassiveServerMediaSubsession::sdpLines());
        m_SDPLines.append(getAuxSDPLine(m_rtpSink,NULL));
    }
    return m_SDPLines.c_str();
}

char const* MulticastServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink,FramedSource* inputSource)
{
    return this->getAuxLine((DisplayDeviceSource*)(m_replicator->inputSource()), rtpSink->rtpPayloadType());
}



