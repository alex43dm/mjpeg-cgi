#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <assert.h>

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
    TiXmlElement *mRoot, *mElem, *mel, *mels;

    std::clog<<"open config file:"<<mFileName;

    mIsInited = false;
/*
    if((cfgFilePath = BoostHelpers::getConfigDir(mFileName)).empty())
    {
        return false;
    }

    std::clog<<" config dir:"<<cfgFilePath<<std::endl;

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

    if( (mels = mRoot->FirstChildElement("mongo")) )
    {
        if( (mel = mels->FirstChildElement("main")) )
        {
            for(mElem = mel->FirstChildElement("host"); mElem; mElem = mElem->NextSiblingElement("host"))
            {
                mongo_main_host_.push_back(mElem->GetText());
            }

            if( (mElem = mel->FirstChildElement("db")) && (mElem->GetText()) )
                mongo_main_db_ = mElem->GetText();

            if( (mElem = mel->FirstChildElement("set")) && (mElem->GetText()) )
                mongo_main_set_ = mElem->GetText();

            if( (mElem = mel->FirstChildElement("slave")) && (mElem->GetText()) )
                mongo_main_slave_ok_ = strncmp(mElem->GetText(),"false", 5) > 0 ? false : true;

            if( (mElem = mel->FirstChildElement("login")) && (mElem->GetText()) )
                mongo_main_login_ = mElem->GetText();

            if( (mElem = mel->FirstChildElement("passwd")) && (mElem->GetText()) )
                mongo_main_passwd_ = mElem->GetText();
        }
        else
        {
            exit("no main section in mongo in config file. exit");
        }

        if( (mel = mels->FirstChildElement("log")) )
        {
            for(mElem = mel->FirstChildElement("host"); mElem; mElem = mElem->NextSiblingElement("host"))
            {
                mongo_log_host_.push_back(mElem->GetText());
            }

            if( (mElem = mel->FirstChildElement("db")) && (mElem->GetText()) )
            {
                mongo_log_db_ = mElem->GetText();
            }

            if( (mElem = mel->FirstChildElement("set")) && (mElem->GetText()) )
                mongo_log_set_ = mElem->GetText();

            if( (mElem = mel->FirstChildElement("slave")) && (mElem->GetText()) )
                mongo_log_slave_ok_ = strncmp(mElem->GetText(),"false", 5) > 0 ? false : true;

            if( (mElem = mel->FirstChildElement("login")) && (mElem->GetText()) )
                mongo_log_login_ = mElem->GetText();

            if( (mElem = mel->FirstChildElement("passwd")) && (mElem->GetText()) )
                mongo_log_passwd_ = mElem->GetText();

            if( (mElem = mel->FirstChildElement("collection")) && (mElem->GetText()) )
                mongo_log_collection_ = mElem->GetText();
        }
        else
        {
            exit("no log section in mongo in config file. exit");
        }
    }
    else
    {
        exit("no mongo section in config file. exit");
    }


    //main config
    if( (mElem = mRoot->FirstChildElement("server")) )
    {
        if( (mel = mElem->FirstChildElement("ip")) && (mel->GetText()) )
        {
            server_ip_ = mel->GetText();
        }

        if( (mel = mElem->FirstChildElement("redirect_script")) && (mel->GetText()) )
        {
            redirect_script_ = mel->GetText();
        }

        if( (mel = mElem->FirstChildElement("geocity_path")) && (mel->GetText()) )
        {
            geocity_path_ = mel->GetText();

            if(!BoostHelpers::checkPath(geocity_path_, false, true))
            {
                ::exit(1);
            }
        }

        if( (mel = mElem->FirstChildElement("socket_path")) && (mel->GetText()) )
        {
            server_socket_path_ = mel->GetText();

            if(BoostHelpers::checkPath(server_socket_path_, true, true))
            {
                std::clog<<"server socket path: "<<server_socket_path_<<" exists"<<std::endl;
                unlink(server_socket_path_.c_str());
            }
        }

        if( (mel = mElem->FirstChildElement("children")) && (mel->GetText()) )
        {
            server_children_ = atoi(mel->GetText());
        }

        if( (mel = mElem->FirstChildElement("sqlite")) )
        {
            if( (mels = mel->FirstChildElement("db")) && (mels->GetText()) )
            {
                dbpath_ = mels->GetText();

                if(dbpath_!=":memory:" && !BoostHelpers::checkPath(dbpath_,true, true))
                {
                    ::exit(1);
                }
            }
            else
            {
                std::clog<<"sqlite database mode: in memory"<<std::endl;
                dbpath_ = ":memory:";
            }

            if( (mels = mel->FirstChildElement("schema")) && (mels->GetText()) )
            {
                db_dump_path_ = cfgFilePath + mels->GetText();

                if(!BoostHelpers::checkPath(db_dump_path_,false, false))
                {
                    ::exit(1);
                }
            }

            if( (mels = mel->FirstChildElement("geo_csv")) && (mels->GetText()) )
            {
                db_geo_csv_ = cfgFilePath + mels->GetText();

                if(!BoostHelpers::checkPath(db_geo_csv_, false, true))
                {
                    ::exit(1);
                }
            }
        }

        if( (mel = mElem->FirstChildElement("lock_file")) && (mel->GetText()) )
        {
            lock_file_ = mel->GetText();

            if(!BoostHelpers::checkPath(lock_file_,true, true))
            {
                ::exit(1);
            }
        }

        if( (mel = mElem->FirstChildElement("pid_file")) && (mel->GetText()) )
        {
            pid_file_ = mel->GetText();

            if(!BoostHelpers::checkPath(pid_file_,true, true))
            {
                ::exit(1);
            }
        }

        if( (mel = mElem->FirstChildElement("mq_path")) && (mel->GetText()) )
        {
            mq_path_ = mel->GetText();
        }

        if( (mel = mElem->FirstChildElement("user")) && (mel->GetText()) )
        {
            user_ = mel->GetText();
        }

        if( (mel = mElem->FirstChildElement("group")) && (mel->GetText()) )
        {
            group_ = mel->GetText();
        }

        if( (mel = mElem->FirstChildElement("time_update")) && (mel->GetText()) )
        {
            if((time_update_ = BoostHelpers::getSeconds(mel->GetText())) == -1)
            {
                exit("Config::Load: no time match in config.xml element: time_update");
            }
        }


        if( (mel = mElem->FirstChildElement("templates")) )
        {
            if( (mels = mel->FirstChildElement("teaser")) && (mels->GetText()) )
            {
                if(!BoostHelpers::checkPath(cfgFilePath + mels->GetText(),false, true))
                {
                    ::exit(1);
                }

                template_teaser_ = getFileContents(cfgFilePath + mels->GetText());
            }
            else
            {
                exit("element template_teaser is not inited");
            }

            if( (mels = mel->FirstChildElement("banner")) && (mels->GetText()) )
            {
                if(!BoostHelpers::checkPath(cfgFilePath + mels->GetText(),false, true))
                {
                    ::exit(1);
                }

                template_banner_ = getFileContents(cfgFilePath + mels->GetText());
            }
            else
            {
                exit("element template_banner is not inited");
            }

            if( (mels = mel->FirstChildElement("error")) && (mels->GetText()) )
            {
                if(!BoostHelpers::checkPath(cfgFilePath + mels->GetText(),false, true))
                {
                    ::exit(1);
                }

                template_error_ = getFileContents(cfgFilePath + mels->GetText());
            }
            else
            {
                exit("element template_error is not inited");
            }


            if( (mels = mel->FirstChildElement("swfobject")) && (mels->GetText()) )
            {
                if(!BoostHelpers::checkPath(cfgFilePath + mels->GetText(),false, true))
                {
                    ::exit(1);
                }

                swfobject_ = getFileContents(cfgFilePath + mels->GetText());
            }
            else
            {
                exit("element swfobject is not inited");
            }
        }

        if( (mel = mElem->FirstChildElement("cookie")) )
        {
            if( (mels = mel->FirstChildElement("name")) && (mels->GetText()) )
                cookie_name_ = mels->GetText();
            else
            {
                exit("element cookie_name is not inited");
            }
            if( (mels = mel->FirstChildElement("domain")) && (mels->GetText()) )
                cookie_domain_ = mels->GetText();
            else
            {
                exit("element cookie_domain is not inited");
            }

            if( (mels = mel->FirstChildElement("path")) && (mels->GetText()) )
                cookie_path_ = mels->GetText();
            else
            {
                exit("element cookie_path is not inited");
            }
        }
    }
    else
    {
        exit("no server section in config file. exit");
    }
*/
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
