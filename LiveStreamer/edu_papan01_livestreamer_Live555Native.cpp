#include "edu_papan01_livestreamer_Live555Native.h"
#include "live_streamer.h"
#include <liveMedia.hh>
#include "LMRTSPClient.h"

extern "C"
{
JNIEXPORT jboolean JNICALL Java_com_segway_robot_module_stream_Live555Native_initialize
  (JNIEnv *, jobject, jint, jint);
JNIEXPORT void JNICALL Java_com_segway_robot_module_stream_Live555Native_loopNative
  (JNIEnv *, jobject);
JNIEXPORT void JNICALL Java_com_segway_robot_module_stream_Live555Native_stopNative
  (JNIEnv *, jobject);
JNIEXPORT void JNICALL Java_com_segway_robot_module_stream_Live555Native_feedH264Data
  (JNIEnv *, jobject, jbyteArray);
JNIEXPORT void JNICALL Java_com_segway_robot_module_stream_Live555Native_destroy
  (JNIEnv *, jobject);
    
JNIEXPORT void JNICALL Java_com_segway_robot_module_stream_Live555Native_addListener
  (JNIEnv *, jobject, jobject);
    
    
JNIEXPORT void JNICALL Java_com_segway_robot_mobile_network_stream_Live555Client_addBufferListener
    (JNIEnv *, jobject, jobject);
    
JNIEXPORT void JNICALL Java_com_segway_robot_mobile_network_stream_Live555Client_playRTSP
    (JNIEnv *, jobject, jstring);
    
JNIEXPORT void JNICALL Java_com_segway_robot_mobile_network_stream_Live555Client_stopRTSP
    (JNIEnv *, jobject);
}

static JavaVM * savedVM = NULL;
static jobject saved_listener_instance = NULL;
static jint jniVersion = 0;

JNIEXPORT void JNICALL Java_com_segway_robot_module_stream_Live555Native_addListener
(JNIEnv *env, jobject jthiz, jobject listener_instance)
{
    env->GetJavaVM( &savedVM );
    //save listener_instance for use later
    
    //Remember to free it after you're done using it otherwise garbage collector won't collect it and you'll get memory leaks:
    if (saved_listener_instance != NULL) {
        LOGI("LiveStreamer_Native 2, DeleteGlobalRef listener_instance");
        env->DeleteGlobalRef(saved_listener_instance);
    }
    
    
    saved_listener_instance = env->NewGlobalRef(listener_instance);
    
    jniVersion = env->GetVersion();
}

void sendClientNetworkInfo(jdouble packetLossPercent, jdouble delayInSeconds)
{
    //Get current thread JNIEnv
    
    JNIEnv * ENV;

    if (savedVM == NULL) {
        LOGI("LiveStreamer_Native 2, java vm is null");
        return;
    }

    if (saved_listener_instance == NULL) {
        LOGI("LiveStreamer_Native 2, saved_listener_instance is null");
        return;
    }

    int stat = savedVM->GetEnv((void **)&ENV, jniVersion);

    if( ENV == NULL ) {
        LOGI("LiveStreamer_Native 2, java env is null");
        return;  //Cant attach to java, bail
    }

    if (stat == JNI_EDETACHED) {  //We are on a different thread, attach

//        LOGI("LiveStreamer_Native 2, We are on a different thread, attach");
        if (savedVM->AttachCurrentThread(&ENV, NULL) != 0) {
            LOGI("LiveStreamer_Native 2, failed to attach current thread");
            return;
        } else {
//            LOGI("LiveStreamer_Native 2, We are on a different thread, attach, success");
        }
    } else if (stat == JNI_OK) {
        //
//        LOGI("LiveStreamer_Native 2, env ok!");
    } else if (stat == JNI_EVERSION) {
        LOGI("LiveStreamer_Native 2, jni version not supported");
        return;
    }

//    jclass randomClass = ENV->FindClass("com/segway/robot/module/stream/VideoNetworkManager");
    jclass randomClass = ENV->GetObjectClass(saved_listener_instance);



    if (randomClass==0) {
        LOGI("LiveStreamer_Native 2, com/segway/robot/module/stream/VideoNetworkManager not exist");

        return;
    }

    jmethodID listenerEventOccured = ENV->GetMethodID(randomClass, "clientNetworkInfo", "(DD)V");

    if (listenerEventOccured == 0 || listenerEventOccured == NULL) {
        LOGI("LiveStreamer_Native 2, method is null");
        return;
    }

    ENV->CallVoidMethod(saved_listener_instance, listenerEventOccured, packetLossPercent, delayInSeconds);

//    ENV->DeleteLocalRef(listenerEventOccured);
    ENV->DeleteLocalRef(randomClass);

    
//    jobject obj = env->GetObjectArrayElement(array, i);
    //your code here
    
//    if (ENV->ExceptionCheck()) {
//        LOGI("LiveStreamer_Native 2, exception checked");
//        ENV->ExceptionDescribe();
//    }

//    savedVM->DetachCurrentThread();
}

LiveStreamer* getInstance(JNIEnv *env, jobject jthiz)
{
    const char* fieldName = "NativeInstance";
    jclass cls = env->GetObjectClass(jthiz);
    jfieldID instanceFieldId = env->GetFieldID(cls, fieldName, "J");
    if(instanceFieldId == NULL){
        LOGI("LiveStreamer_Native has no long field named with: NativeInstance");
        return NULL;
    }
    jlong instanceValue = env->GetLongField(jthiz,instanceFieldId);
    if(instanceValue == 0)
    {
        LOGI("instanceValue NULL ");
        return NULL;
    }
    else{
//        LOGI("instanceValue NOT NULL ");
    }
    return (LiveStreamer*)instanceValue;
}

void storeInstance(JNIEnv *env, jobject jthiz,LiveStreamer* instance)
{
    const char* fieldName = "NativeInstance";
    jclass cls = env->GetObjectClass(jthiz);
    jfieldID instanceFieldId = env->GetFieldID(cls, fieldName, "J");
    if(instanceFieldId == NULL){
        LOGI("LiveStreamer_Native has no long field named with: NativeInstance");
        return;
    }
    jlong value = (instance == NULL) ? 0 : (jlong)instance;
    env->SetLongField(jthiz,instanceFieldId,value);
}



void onRRReceived(void* clientData) {
    RTPSink *sink = (RTPSink *)clientData;
    
    RTPTransmissionStatsDB& db = sink->transmissionStatsDB();
    
    RTPTransmissionStatsDB::Iterator itr(db);
    
    RTPTransmissionStats* stats = itr.next();
    
    //TODO: check db numReceivers count?
    
//    LOGI("LiveStreamer_Native 2, onRRReceived numReceivers: %d", db.numReceivers());
    
    if (stats == NULL) {
        LOGI("LiveStreamer_Native 2, stats is NULL");
        return;
    }
    sendClientNetworkInfo(stats->packetLossPercent(), stats->delayInSeconds());
}


JNIEXPORT jboolean JNICALL Java_com_segway_robot_module_stream_Live555Native_initialize
        (JNIEnv *env, jobject jthiz, jint fps, jint port)
{
    LiveStreamer* streamer = new LiveStreamer(fps,port);
    storeInstance(env,jthiz,streamer);
    return streamer->init(onRRReceived);
}

JNIEXPORT void JNICALL Java_com_segway_robot_module_stream_Live555Native_loopNative
        (JNIEnv *env, jobject jthiz)
{
    LiveStreamer* streamer = getInstance(env,jthiz);
    if(streamer != NULL) streamer->loop();
}


JNIEXPORT void JNICALL Java_com_segway_robot_module_stream_Live555Native_stopNative
        (JNIEnv *env, jobject jthiz)
{
    LiveStreamer* streamer = getInstance(env,jthiz);
    if(streamer != NULL) streamer->stop();
}

JNIEXPORT void JNICALL Java_com_segway_robot_module_stream_Live555Native_feedH264Data
        (JNIEnv *env, jobject jthiz, jbyteArray dataArray)
{
    LiveStreamer* streamer = getInstance(env,jthiz);
    if(streamer == NULL) return;
    int len = env->GetArrayLength(dataArray);
    char* buf = new char[len];
    env->GetByteArrayRegion (dataArray, 0, len, reinterpret_cast<jbyte*>(buf));
    streamer->dataPushed(buf,len);
}

JNIEXPORT void JNICALL Java_com_segway_robot_module_stream_Live555Native_feedAACAudioData
(JNIEnv *env, jobject jthiz, jbyteArray dataArray)
{
    LiveStreamer* streamer = getInstance(env,jthiz);
    if(streamer == NULL) return;
    int len = env->GetArrayLength(dataArray);
    char* buf = new char[len];
    env->GetByteArrayRegion (dataArray, 0, len, reinterpret_cast<jbyte*>(buf));
    streamer->audioDataPushed(buf,len);
}

JNIEXPORT void JNICALL Java_com_segway_robot_module_stream_Live555Native_destroy
        (JNIEnv *env, jobject jthiz)
{
    LiveStreamer* streamer = getInstance(env,jthiz);
    if(streamer != NULL){
        delete streamer;
        streamer = 0;
        storeInstance(env,jthiz,NULL);
    }
}


static JavaVM * savedVM1 = NULL;
static jobject saved_listener_instance1 = NULL;
static jint jniVersion1 = 0;

JNIEXPORT void JNICALL Java_com_segway_robot_mobile_network_stream_Live555Client_addBufferListener
(JNIEnv *env, jobject jthiz, jobject listener_instance)
{
    env->GetJavaVM( &savedVM1 );
    //save listener_instance for use later
    
    //Remember to free it after you're done using it otherwise garbage collector won't collect it and you'll get memory leaks:
    if (saved_listener_instance1 != NULL) {
        LOGI("LiveStreamer_Native 3, DeleteGlobalRef listener_instance");
        env->DeleteGlobalRef(saved_listener_instance1);
    }
    
    
    saved_listener_instance1 = env->NewGlobalRef(listener_instance);
    
    jniVersion1 = env->GetVersion();
}

void sendBuffer(u_int8_t* buffer, unsigned size)
{
    
    if (savedVM1 == NULL) {
        LOGI("LiveStreamer_Native 3, java vm is null");
        return;
    }
    
    if (saved_listener_instance1 == NULL) {
        LOGI("LiveStreamer_Native 3, saved_listener_instance is null");
        return;
    }
    
    //Get current thread JNIEnv
    JNIEnv * ENV;
    int stat = savedVM1->GetEnv((void **)&ENV, jniVersion1);
    if( ENV == NULL ) {
        LOGI("LiveStreamer_Native 3, java env is null");
        return;  //Cant attach to java, bail
    }
    
    if (stat == JNI_EDETACHED) {  //We are on a different thread, attach
        
        if (savedVM1->AttachCurrentThread(&ENV, NULL) != 0) {
            LOGI("LiveStreamer_Native 3, failed to attach current thread");
            return;
        } else {
            LOGI("LiveStreamer_Native 3, We are on a different thread, attach, success");
        }
    } else if (stat == JNI_OK) {
        //
    } else if (stat == JNI_EVERSION) {
        LOGI("LiveStreamer_Native 3, jni version not supported");
        return;
    }
    
    //    jclass randomClass = ENV->FindClass("com/segway/robot/module/stream/VideoNetworkManager");
    jclass randomClass = ENV->GetObjectClass(saved_listener_instance1);
    
    if (randomClass==0) {
        LOGI("LiveStreamer_Native 3, no object class for send client buffer");
        return;
    }
    
    jmethodID listenerEventOccured = ENV->GetMethodID(randomClass, "bufferArrived", "([B)V");
    
    if (listenerEventOccured == 0 || listenerEventOccured == NULL) {
        LOGI("LiveStreamer_Native 2, method is null");
        return;
    }
    
    jbyteArray arr = ENV->NewByteArray(size);
    ENV->SetByteArrayRegion(arr, 0, size, reinterpret_cast<jbyte*>(buffer));
    
    ENV->CallVoidMethod(saved_listener_instance1, listenerEventOccured, arr);
    
    //    ENV->DeleteLocalRef(listenerEventOccured);
    ENV->DeleteLocalRef(randomClass);

    ENV->DeleteLocalRef(arr);
}

JNIEXPORT void JNICALL Java_com_segway_robot_mobile_network_stream_Live555Client_playRTSP
(JNIEnv *env, jobject jthiz, jstring url) {
    const char *nativeString = env->GetStringUTFChars(url, 0);
    playRTSP(nativeString, sendBuffer);
    env->ReleaseStringUTFChars(url, nativeString);
}

JNIEXPORT void JNICALL Java_com_segway_robot_mobile_network_stream_Live555Client_stopRTSP
(JNIEnv *env, jobject jthiz) {
    stopRTSP();
}
