#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <assert.h>
#include <libgen.h>

#include "Log.h"
#include "Config.h"

Config* Config::mInstance = NULL;

Config* Config::Instance()
{
    if (!mInstance)
        mInstance = new Config();

    return mInstance;
}

Config::Config()
{
    mIsInited = false;
}

bool Config::LoadConfig(const std::string fName)
{
    mFileName = fName;
    return Load();
}

void Config::exit(const std::string &mes)
{
    std::cerr<<mes<<std::endl;
    std::clog<<mes<<std::endl;
    ::exit(1);
}

bool Config::Load()
{
    TiXmlDocument *mDoc;
    TiXmlElement *mRoot, *mElem, *mel, *mel1;

    std::clog<<"open config file:"<<mFileName;

    mIsInited = false;
/*
    char * _bn = dirname(static_cast<char*>((char*)mFileName.c_str()));
    if(_bn)
    {
       cfgFilePath = std::string(_bn);
    }
    std::clog<<" config dir:"<<cfgFilePath<<std::endl;
*/

    if(!(mDoc = new TiXmlDocument(mFileName)))
    {
        exit("error create TiXmlDocument object");
    }


    if(!mDoc->LoadFile())
    {
        exit("load file: "+mFileName+
             " error: "+ mDoc->ErrorDesc()+
             " row: "+std::to_string(mDoc->ErrorRow())+
             " col: "+std::to_string(mDoc->ErrorCol()));
    }

    mRoot = mDoc->FirstChildElement("root");

    if(!mRoot)
    {
        exit("does not found root section in file: "+mFileName);
    }

    instanceId = atoi(mRoot->Attribute("id"));

    //main config
    if( (mElem = mRoot->FirstChildElement("server")) )
    {
        if( (mel = mElem->FirstChildElement("socket_path")) && (mel->GetText()) )
        {
            SocketPath = mel->GetText();
        }

        if( (mel = mElem->FirstChildElement("children")) && (mel->GetText()) )
        {
            ServerThreads = atoi(mel->GetText());
        }

        if( (mel = mElem->FirstChildElement("lock_file")) && (mel->GetText()) )
        {
            LockFile = mel->GetText();
        }

        if( (mel = mElem->FirstChildElement("pid_file")) && (mel->GetText()) )
        {
            PidFile = mel->GetText();
        }

        if( (mel = mElem->FirstChildElement("user")) && (mel->GetText()) )
        {
            User = mel->GetText();
        }

        if( (mel = mElem->FirstChildElement("group")) && (mel->GetText()) )
        {
            Group = mel->GetText();
        }

        if( (mel = mElem->FirstChildElement("buffersize")) && (mel->GetText()) )
        {
            BufferSize = atoi(mel->GetText());
        }

        if( (mel = mElem->FirstChildElement("indexFile")) && (mel->GetText()) )
        {
            indexFile = mel->GetText();
        }

        if( (mel = mElem->FirstChildElement("imagesDir")) && (mel->GetText()) )
        {
            imagesDir = mel->GetText();
        }

        if( (mel = mElem->FirstChildElement("fps")) && (mel->GetText()) )
        {
            FPS = strtof(mel->GetText(),NULL);
        }
    }
    else
    {
        exit("no server section in config file. exit");
    }

    if( (mElem = mRoot->FirstChildElement("camera")) )
    {
        if( (mel = mElem->FirstChildElement("stream_port")) && (mel->GetText()) )
        {
            camJpgLocalPort = static_cast<unsigned>(atoi(mel->GetText()));
        }
        if( (mel = mElem->FirstChildElement("ip")) && (mel->GetText()) )
        {
            camIp = mel->GetText();
        }
        if( (mel = mElem->FirstChildElement("port")) && (mel->GetText()) )
        {
            camPort = static_cast<unsigned>(atoi(mel->GetText()));
        }
        if( (mel = mElem->FirstChildElement("alive_inteval")) && (mel->GetText()) )
        {
            camAliveInterval = static_cast<unsigned>(atoi(mel->GetText()));
        }
        if( (mel = mElem->FirstChildElement("width")) && (mel->GetText()) )
        {
            camWidth = static_cast<unsigned>(atoi(mel->GetText()));
        }
        if( (mel = mElem->FirstChildElement("height")) && (mel->GetText()) )
        {
            camHeight = static_cast<unsigned>(atoi(mel->GetText()));
        }
    }
    else
    {
        exit("no camera section in config file. exit");
    }

    if( (mElem = mRoot->FirstChildElement("upnp")) )
    {
        if( (mel = mElem->FirstChildElement("listernIp")) && (mel->GetText()) )
        {
            upnpListernIp = mel->GetText();
        }
        if( (mel = mElem->FirstChildElement("mediaServerName")) && (mel->GetText()) )
        {
            upnpMediaServerName = mel->GetText();
        }
        if( (mel = mElem->FirstChildElement("searchDir")) && (mel->GetText()) )
        {
            upnpSearchDir = mel->GetText();
        }
    }
    else
    {
        exit("no upnp section in config file. exit");
    }

    if( (mElem = mRoot->FirstChildElement("rotator")) )
    {
        if( (mel = mElem->FirstChildElement("tty")) && (mel->GetText()) )
        {
            rotatorDev = mel->GetText();
        }

        if( (mel1 = mElem->FirstChildElement("x")) )
        {
            if( (mel = mElem->FirstChildElement("speed")) && (mel->GetText()) )
            {
                rotatorSpeedX = strtol(mel->GetText(),NULL,10);
            }

            if( (mel = mElem->FirstChildElement("shift")) && (mel->GetText()) )
            {
                rotatorShiftX = strtol(mel->GetText(),NULL,10);
            }
        }
    }
    else
    {
        exit("no rotator section in config file. exit");
    }

    mIsInited = true;
    return mIsInited;
}

//---------------------------------------------------------------------------------------------------------------
Config::~Config()
{
    mInstance = NULL;
}
//---------------------------------------------------------------------------------------------------------------
int Config::getTime(const char *p)
{
    struct tm t;
    int ret;
    strptime(p, "%H:%M:%S", &t);
    ret = t.tm_hour * 3600;
    ret = ret + t.tm_min * 60;
    return ret + t.tm_sec;
}
//---------------------------------------------------------------------------------------------------------------
std::string Config::getFileContents(const std::string &fileName)
{
    std::ifstream in(fileName, std::ios::in | std::ios::binary);

    if(in)
    {
        std::string cnt;
        in.seekg(0, std::ios::end);
        cnt.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&cnt[0], cnt.size());
        in.close();
        return(cnt);
    }

    std::clog<<"error open file: "<<fileName<<" error number: "<<errno<<std::endl;
    return std::string();
}

const std::string Config::currentDateTime()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%d-%m-%Y %H:%M:%S", &tstruct);
    return buf;
}

struct timespec Config::getThreadWait()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    if(cfg->FPS < 1)
    {
        ts.tv_sec += 1/cfg->FPS;
    }
    else
    {
        if(ts.tv_nsec + NANO / cfg->FPS > NANO)
        {
            ts.tv_sec++;
            ts.tv_nsec =  ts.tv_nsec + NANO / cfg->FPS - NANO;
        }
        else
        {
            ts.tv_nsec += NANO / cfg->FPS;
        }
    }
    return ts;
}
