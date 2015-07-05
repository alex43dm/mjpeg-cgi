#ifndef CAM_H
#define CAM_H

#include <string>

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "HttpClient.h"

namespace panasonic
{

std::string const CAM_CAPABILITY_REQUEST = "/cam.cgi?mode=getinfo&type=capability";
std::string const CAM_GETSTATE			 = "/cam.cgi?mode=getstate";
std::string const CAM_STREAM			 = "/cam.cgi?mode=startstream&value=";
std::string const CAM_STOPSTREAM		 = "/cam.cgi?mode=stopstream";
std::string const CAM_PLAYMODE			 = "/cam.cgi?mode=camcmd&value=playmode";
std::string const CAM_PICTMODE			 = "/cam.cgi?mode=camcmd&value=pictmode";
std::string const CAM_RECMODE			 = "/cam.cgi?mode=camcmd&value=recmode";
std::string const CAM_ZOOMOUT			 = "/cam.cgi?mode=camcmd&value=wide-normal";
std::string const CAM_ZOOMOUT_FAST		 = "/cam.cgi?mode=camcmd&value=wide-fast";
std::string const CAM_ZOOMIN			 = "/cam.cgi?mode=camcmd&value=tele-normal";
std::string const CAM_ZOOMIN_FAST		 = "/cam.cgi?mode=camcmd&value=tele-fast";
std::string const CAM_ZOOMSTOP			 = "/cam.cgi?mode=camcmd&value=zoomstop";
//take a picture
std::string const CAM_TAKESHOT			 = "/cam.cgi?mode=camcmd&value=capture";
//take a picture with focus on a given coordinate: 840/234 and value2 on/off
std::string const CAM_TAKESHOTWITHFOCUS	 = "/cam.cgi?mode=camctrl&type=touchcapt&value=%d/%d&value2=%s";
std::string const CAM_3BOXMODE       	 = "/cam.cgi?mode=camcmd&value=3boxplaymode";
std::string const CAM_VIDEO_RECSTART 	 = "/cam.cgi?mode=camcmd&value=video_recstart";
std::string const CAM_VIDEO_RECSTOP 	 = "/cam.cgi?mode=camcmd&value=video_recstop";
std::string const CAM_POWEROFF      	 = "/cam.cgi?mode=camcmd&value=poweroff";
std::string const CAM_HIGHTLIGHTCANCEL   = "/cam.cgi?mode=camcmd&value=highlightcancel";
std::string const CAM_RECSTART           = "/cam.cgi?mode=camcmd&value=recstart";
std::string const CAM_RECSTOP            = "/cam.cgi?mode=camcmd&value=recstop";


enum state_t
{
    initional,
    connected,
    ready,
    startstream,
    stream,
    dispose,
    error
};

enum zoom_t
{
    fastOut = -2,
    out,
    none,
    in,
    fastIn
};

class cam
{
public:
    pthread_cond_t  CondVar;
    pthread_mutex_t Mtx;
    unsigned char *Buffer;
    size_t Len;
    bool filterMarkName;
    bool filterGray;

    cam(const std::string &address, unsigned short remotePort, unsigned short localPort);
    ~cam();

    bool init();
    void run(const state_t &s);
    void reciever();
    bool stream(bool OnOff);
    int applyZoom(zoom_t lzoom);
    bool takeAshot(){ return request(CAM_TAKESHOT) == "ok" ? true : false;}
protected:
    bool exit;
private:
    int devSock;
    bool zoomChanged;
    std::string request(const std::string &cmd);
    state_t state;
    zoom_t zoom;
    sockaddr_in sa;
    pthread_t _pThread;
    HttpClient *httpClient;
    unsigned short localPort;
    std::string address;

    cam(const cam&);
    cam& operator=(const cam&);

    static const char JpegHeaderStart[3];
    static const char JpegHeaderEnd[3];

    static void *streamUpdate(void*);
    std::string State();
};
}
#endif
