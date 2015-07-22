#include <iostream>

#include <getopt.h>
#include <unistd.h>

#include "config.h"
#include "Log.h"
#include "Config.h"
#include "Server.h"
#include "CgiService.h"
#ifdef HAVE_IMAGEMAGICK
#include "ImgMgk.h"
#endif // HAVE_IMAGEMAGICK

Config *cfg;

int main(int argc, char *argv[])
{
    std::clog.rdbuf(new Log(LOG_LOCAL0));

    std::string config = "config.xml";
    std::string sock_path;
    int ret;
    bool fPrintPidFile = false;

    while ( (ret = getopt(argc,argv,"pc:s:")) != -1)
    {
        switch (ret)
        {
        case 'c':
            config = optarg;
            break;
        case 's':
            sock_path = optarg;
            break;
        case 'p':
            fPrintPidFile = true;
            break;
        default:
            printf("Error found! %s -c config_file -s socket_path\n",argv[0]);
            ::exit(1);
        };
    };

    cfg = Config::Instance();
    cfg->LoadConfig(config);

    if(fPrintPidFile)
    {
        std::cout<<cfg->PidFile<<std::endl;
        ::exit(0);
    }

#ifndef DEBUG
    new Server(cfg->LockFile, cfg->PidFile);
#endif

    if( sock_path.size() > 8 )
    {
        cfg->SocketPath = sock_path;
    }
#ifdef HAVE_IMAGEMAGICK
    ImgMgk::init(argv);
#endif // HAVE_IMAGEMAGICK
    CgiService(cfg->ServerThreads,cfg->SocketPath).run();

    return 0;
}
