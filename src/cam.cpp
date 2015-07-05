#include "cam.h"

#include <arpa/inet.h>
#include <unistd.h>

#include <chrono>

#include "Config.h"
#include "Log.h"

using namespace panasonic;

cam::cam(const std::string &address, unsigned short remotePort, unsigned short localPort) :
    Mtx(PTHREAD_MUTEX_INITIALIZER),
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
    std::string result;

    Log::gdb("cam cmd: %s",cmd.c_str());

    std::string *temp = httpClient->get("http://"+address+cmd);
    /*
    if ((s1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        Log::err("socket() failed with error code : ");
        return "";
    }

    if (connect(s1,(struct sockaddr *)&sa, sizeof(sa)) != 0)
    {
        Log::err("Could not connect");
        ::close(s1);
        return "";
    }

    if (send(s1, cmd.c_str(), cmd.length(), 0) != (int)cmd.length())
    {
        Log::err("Could not send the request to the Server");
        close(s1);
        return "";
    }

    if (send(s1, httpReqEnd, strlen(httpReqEnd), 0) != (int)strlen(httpReqEnd))
    {
		Log::err("Could not send the end of request to the Server");
        close(s1);
        return "";
    }

    memset(buffer, 0, sizeof(buffer));
    while (recv(s1, buffer, sizeof(buffer), 0) > 0)
    {
        temp.append(buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    // Cleaning up Windows Socket Dependencies
    close(s1);
*/
    if(temp == NULL)
    {
        return "";
    }

    Log::gdb("get: %s",(*temp).c_str());

    std::size_t start = (*temp).find("<result>");
    if (start == std::string::npos)
    {
        Log::err("no result open tag");
        return "";
    }
    else start += 8;

    std::size_t end = (*temp).find("</result>");
    if (end == std::string::npos)
    {
        Log::err("no result close tag");
        delete temp;
        return "";
    }

    result = (*temp).substr(start, end - start);
    if (cmd != CAM_GETSTATE)
    {
        Log::gdb("result: %s",result.c_str());
        delete temp;
        return result;
    }


    start = (*temp).find("<livestream>");
    if (start == std::string::npos)
    {
        delete temp;
        return "";
    }
    else start += 12;

    end = (*temp).find("</livestream>");
    if (end == std::string::npos)
    {
        delete temp;
        return "";
    }

    result = (*temp).substr(start, end - start);
    delete temp;
    return result;
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

                        memcpy(Buffer,tempOutputStorage.c_str(),tempOutputStorage.size());
                        Len = tempOutputStorage.size();

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
