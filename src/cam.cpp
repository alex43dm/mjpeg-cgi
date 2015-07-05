#include "cam.h"

#include <arpa/inet.h>
#include <unistd.h>

#include <chrono>

#include "Config.h"
#include "Log.h"
#include "ImgMgk.h"

using namespace panasonic;

cam::cam(const std::string &address, unsigned short remotePort, unsigned short localPort) :
    Mtx(PTHREAD_MUTEX_INITIALIZER),
    filterMarkName(false),
    exit(false),
    zoomChanged(false),
    state(state_t::initional),
    zoom(zoom_t::none),
    localPort(localPort),
    address(address)
{
    sockaddr_in saLocal;

    pthread_cond_init(&CondVar,NULL);

    Buffer = new unsigned char[cfg->BufferSize];

    httpClient = new HttpClient();

    //setup address structure for outgoing HTTP control messages
    memset(&sa, 0, sizeof(sa));
    if (inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) < 1)
    {
        Log::err("Invalid destination IP address: %s",address.c_str());
        state = state_t::dispose;
        return;
    }
    sa.sin_family = AF_INET;
    sa.sin_port = htons(remotePort);//0x5000;

    //setup address structure for incoming UDP image data
    memset(&saLocal, 0, sizeof(saLocal));
    saLocal.sin_family = AF_INET;
    saLocal.sin_port = htons(localPort);
    saLocal.sin_addr.s_addr = INADDR_ANY;
    if ((devSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        Log::err("socket() failed with error code : ");
        state = state_t::dispose;
        return;
    }
    if (::bind(devSock, (sockaddr*)&saLocal, sizeof(saLocal)) == -1)
    {
        Log::err("Bind failed with error code : ");
        state = state_t::dispose;
        return;
    }
}


cam::~cam()
{
    delete httpClient;
    state = state_t::dispose;
    pthread_cond_destroy(&CondVar);
    pthread_join(_pThread,0);
}

std::string cam::request(const std::string &cmd)
{
    Log::gdb("cam cmd: %s",cmd.c_str());

    std::string temp = httpClient->get("http://"+address+cmd);

    Log::gdb("get: %s",temp.c_str());

    TiXmlDocument mDoc;
    TiXmlElement *camrply, *mElem;

    const char* pTest = mDoc.Parse(temp.c_str(), 0 , TIXML_ENCODING_UTF8);
    if(pTest == NULL && mDoc.Error())
    {
        Log::err("parse: "+temp+
             " error: "+ mDoc.ErrorDesc()+
             " row: "+std::to_string(mDoc.ErrorRow())+
             " col: "+std::to_string(mDoc.ErrorCol()));

        return "";
    }

    camrply = mDoc.FirstChildElement("camrply");

    if(!camrply)
    {
        Log::err("does not found camrply section in file: "+temp);
        return "";
    }

    if( (mElem = camrply->FirstChildElement("result")) )
    {
        return mElem->GetText();
    }
    else
    {
        Log::err("does not found result section in file: "+temp);
    }

    return "";
}

bool cam::stream(bool OnOff = true)
{
    if(OnOff)
    {
        if (request(CAM_STREAM+std::to_string(localPort)) == "ok")
        {
            Log::info("cam: state: stream");
            state = state_t::stream;
        }
        else
        {
            Log::err("cam: state: stream error");
            return false;
        }
    }
    else
    {
        if (request(CAM_STOPSTREAM) == "ok")
        {
            Log::info("cam: state: stream");
            state = state_t::stream;
        }
        else
        {
            Log::err("cam: state: stream error");
            return false;
        }
    }

    return true;
}

bool cam::init()
{
    if (request(CAM_CAPABILITY_REQUEST) == "ok")
    {
        state = state_t::connected;
        Log::info("cam: state: connected");
    }
    else
    {
        Log::err("cam: state: connected error");
        return false;
    }
/*
    if (request(CAM_RECMODE) == "ok")
    {
        Log::info("cam: state: ready");
        state = state_t::ready;
    }
    else
    {
        Log::err("cam: state: ready error");
        return false;
    }
*/
    if(stream(true))
    {
        state = state_t::startstream;
        if(pthread_create(&_pThread, NULL, &this->streamUpdate, this))
        {
            Log::err("creating thread failed");
            ::exit(1);
        }

        Log::info("cam is inited");
        return true;
    }

    return false;
}

void *cam::streamUpdate(void *data)
{
    cam *c1 = (cam*)data;

    sleep(cfg->camAliveInterval);
    while(!c1->exit)
    {
        c1->stream(true);
        sleep(cfg->camAliveInterval);
    }

    return NULL;
}

int cam::applyZoom(zoom_t lzoom)
{
    std::string result;
    switch (lzoom)
    {
    case zoom_t::none:
        result = request(CAM_ZOOMSTOP);
        break;
    case zoom_t::in:
        result = request(CAM_ZOOMIN);
        break;
    case zoom_t::out:
        result = request(CAM_ZOOMOUT);
        break;
    case zoom_t::fastIn:
        result = request(CAM_ZOOMIN_FAST);
        break;
    case zoom_t::fastOut:
        result = request(CAM_ZOOMOUT_FAST);
        break;
    default:
        result = "";
    }
    if (result == "ok")
        return 0;
    else return -1;
}

void cam::reciever()
{
    const char JpegHeaderStart[3] = { static_cast<const char>(0xFF), static_cast<const char>(0xD8), 0 };
    const char JpegHeaderEnd[3] = { static_cast<const char>(0xFF), static_cast<const char>(0xD9), 0 };
    std::string parserStorage;
    std::string tempOutputStorage;
    char packetStorage[65536];
    enum { started, ended } jpegState = ended;
    int numRecieved;
    size_t pos;

    while(!init() && !exit)
    {
        sleep(3);
    }

    while(!exit)
    {
        if (state == state_t::dispose)
            break;
        numRecieved = recv(devSock, packetStorage, sizeof(packetStorage), 0);
        if (numRecieved > 0)
        {
            parserStorage.clear();
            parserStorage.append(packetStorage, numRecieved);
            while (!parserStorage.empty())
            {
                if (jpegState == ended)
                {
                    pos = parserStorage.find(JpegHeaderStart);
                    if (pos == std::string::npos)
                        break;
                    parserStorage = parserStorage.substr(pos);
                    jpegState = started;
                }
                else
                {
                    pos = parserStorage.find(JpegHeaderEnd);
                    if (pos == std::string::npos)
                    {
                        tempOutputStorage.append(parserStorage);
                        break;
                    }
                    else
                    {
                        tempOutputStorage.append(parserStorage, 0, pos + 2);

                        pthread_mutex_lock(&Mtx);


                        if(filterMarkName)
                        {
                            Magick::Blob b1 = ImgMgk::conv(tempOutputStorage.c_str(),tempOutputStorage.size(), "oNe");

                            Len = b1.length();
                            memcpy(Buffer,b1.data(),Len);
                        }
                        else
                        {
                            Len = tempOutputStorage.size();
                            memcpy(Buffer,tempOutputStorage.c_str(),Len);
                        }

                        pthread_cond_broadcast(&CondVar);
                        pthread_mutex_unlock(&Mtx);

                        tempOutputStorage.clear();
                        parserStorage = parserStorage.substr(pos + 2);
                        jpegState = ended;
                    }
                }
            }
        }

    }
}
