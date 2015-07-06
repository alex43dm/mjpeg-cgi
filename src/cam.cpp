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
    filterGray(false),
    exit(false),
    zoomChanged(false),
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
        return;
    }
    if (::bind(devSock, (sockaddr*)&saLocal, sizeof(saLocal)) == -1)
    {
        Log::err("Bind failed with error code : ");
        return;
    }
}


cam::~cam()
{
    delete httpClient;
    pthread_cond_destroy(&CondVar);
    pthread_join(_pThread,0);
}

std::string cam::request(const std::string &cmd)
{
    std::string temp = httpClient->get("http://"+address+cmd);

    if(temp.empty())
    {
        return temp;
    }

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
        std::string ret = request(CAM_STREAM+std::to_string(localPort));
        if ( ret == "ok")
        {
            Log::info("cam: state: stream");
        }
        else
        {
            witeImage("cam stream request error :"+ret);
            Log::err("cam: state: stream error: %s",ret.c_str());
            return false;
        }
    }
    else
    {
        if (request(CAM_STOPSTREAM) == "ok")
        {
            Log::info("cam: state: stream");
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
    getState();

    std::string ret = request(CAM_CAPABILITY_REQUEST);
    if( ret == "ok")
    {
        Log::info("cam: state: connected");

        getState();
        if(!state.livestream)
        {
            Log::info("cam: livestream off");
            return false;
        }
    }
    else
    {
        Log::err("cam: state: connection error: %s",ret.c_str());
        witeImage("cam: state: connection error: "+ret);
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
        c1->getState();
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

    //init image
    pthread_mutex_lock(&Mtx);

    Magick::Blob b1 = ImgMgk::def();
    Len = b1.length();
    memcpy(Buffer,b1.data(),Len);

    pthread_cond_broadcast(&CondVar);
    pthread_mutex_unlock(&Mtx);

    while(!init() && !exit)
    {
        sleep(3);
    }

    while(!exit)
    {
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

                        Magick::Blob b1;

                        if(filterGray)
                        {
                            b1 = ImgMgk::gray(tempOutputStorage.c_str(),tempOutputStorage.size(),
                                             Config::currentDateTime());
                        }
                        else
                        {
                            b1 = ImgMgk::conv(tempOutputStorage.c_str(),tempOutputStorage.size(),
                                             Config::currentDateTime());
                        }
                        /*
                        if(filterMarkName)
                        {
                            b1 = ImgMgk::conv(tempOutputStorage.c_str(),tempOutputStorage.size(), "oNe");

                        }
                        else
                        {
                            Len = tempOutputStorage.size();
                            memcpy(Buffer,tempOutputStorage.c_str(),Len);
                        }*/

                        //copy image for output
                        pthread_mutex_lock(&Mtx);

                        Len = b1.length();
                        memcpy(Buffer,b1.data(),Len);

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


void cam::getState()
{
    std::string temp = httpClient->get("http://"+address+CAM_GETSTATE);

    if(temp.empty())
    {
        return;
    }

    TiXmlDocument mDoc;
    TiXmlElement *camrply, *mElem, *mel, *xel;

    const char* pTest = mDoc.Parse(temp.c_str(), 0 , TIXML_ENCODING_UTF8);
    if(pTest == NULL && mDoc.Error())
    {
        Log::err("parse: "+temp+
                 " error: "+ mDoc.ErrorDesc()+
                 " row: "+std::to_string(mDoc.ErrorRow())+
                 " col: "+std::to_string(mDoc.ErrorCol()));

        return;
    }

    camrply = mDoc.FirstChildElement("camrply");

    if(!camrply)
    {
        Log::err("does not found camrply section in file: "+temp);
        return;
    }

    if( (mElem = camrply->FirstChildElement("result")) )
    {
        std::string result = mElem->GetText();
        if( result == "ok")
        {
            if( (mElem = camrply->FirstChildElement("state")) )
            {
                if( (mel = mElem->FirstChildElement("batt")) && (mel->GetText()) )
                {
                    state.batt = mel->GetText();
                }
                if( (mel = mElem->FirstChildElement("batttype")) && (mel->GetText()) )
                {
                    state.batttype = mel->GetText();
                }
                if( (mel = mElem->FirstChildElement("livestream")) && (mel->GetText()) )
                {
                    state.livestream = std::string(mel->GetText()) == "on" ? true : false;
                }
                if( (mel = mElem->FirstChildElement("rec")) && (mel->GetText()) )
                {
                    state.rec = std::string(mel->GetText()) == "on" ? true : false;
                }
                if( (mel = mElem->FirstChildElement("mode")) && (mel->GetText()) )
                {
                    Mode(std::string(mel->GetText()));
                }
                if( (mel = mElem->FirstChildElement("recremaincapacity")) && (mel->GetText()) )
                {
                    bzero(&state.recremaincapacity,sizeof(state.recremaincapacity));
                    if( (xel = mElem->FirstChildElement("hour")) && (xel->GetText()) )
                    {
                        state.recremaincapacity.tm_hour = static_cast<unsigned>(atoi(mel->GetText()));
                    }
                    if( (xel = mElem->FirstChildElement("min")) && (xel->GetText()) )
                    {
                        state.recremaincapacity.tm_min = static_cast<unsigned>(atoi(mel->GetText()));
                    }
                }
                if( (mel = mElem->FirstChildElement("remaincapacity")) && (mel->GetText()) )
                {
                    state.remaincapacity = static_cast<unsigned>(atoi(mel->GetText()));
                }
                if( (mel = mElem->FirstChildElement("sdcardstatus")) && (mel->GetText()) )
                {
                    state.sdcardstatus = mel->GetText();
                }
                if( (mel = mElem->FirstChildElement("operation")) && (mel->GetText()) )
                {
                    state.operation = mel->GetText();
                }
                if( (mel = mElem->FirstChildElement("version")) && (mel->GetText()) )
                {
                    state.version = mel->GetText();
                }
                if( (mel = mElem->FirstChildElement("rectime")) && (mel->GetText()) )
                {
                    bzero(&state.rectime,sizeof(state.rectime));
                    if( (xel = mElem->FirstChildElement("hour")) && (xel->GetText()) )
                    {
                        state.rectime.tm_hour = static_cast<unsigned>(atoi(mel->GetText()));
                    }
                    if( (xel = mElem->FirstChildElement("min")) && (xel->GetText()) )
                    {
                        state.rectime.tm_min = static_cast<unsigned>(atoi(mel->GetText()));
                    }
                    if( (xel = mElem->FirstChildElement("sec")) && (xel->GetText()) )
                    {
                        state.rectime.tm_sec = static_cast<unsigned>(atoi(mel->GetText()));
                    }
                }
                if( (mel = mElem->FirstChildElement("temperature")) && (mel->GetText()) )
                {
                    state.temperature = mel->GetText();
                }
                if( (mel = mElem->FirstChildElement("pantiltmode")) && (mel->GetText()) )
                {
                    state.pantiltmode = mel->GetText();
                }
            }
        }
        else
        {
            Log::err("cannot get cam status: %s",result.c_str());
        }
    }
    else
    {
        Log::err("does not found result section in file: "+temp);
    }
}

void cam::witeImage(const std::string &mes)
{
    pthread_mutex_lock(&Mtx);

    Magick::Blob b1 = ImgMgk::def(mes);
    Len = b1.length();
    memcpy(Buffer,b1.data(),Len);

    pthread_cond_broadcast(&CondVar);
    pthread_mutex_unlock(&Mtx);
}

camMode cam::Mode(const std::string &mode)
{
    if( mode =="playmode" )
        state.mode = camMode::play;
    else if( mode =="recmode" )
        state.mode = camMode::rec;
    else if( mode =="pivmode" )
        state.mode = camMode::pict;
    else if( mode =="3boxplaymode" )
        state.mode = camMode::xboxplay;

    return state.mode;
}
