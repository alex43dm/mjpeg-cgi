#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <list>

#include <tinyxml.h>

#define NANO 1000000000L

class Config
{
public:
    std::string SocketPath;
    unsigned ServerThreads;
    float FPS;
    size_t BufferSize;

    int         instanceId;
    std::string LockFile;
    std::string PidFile;
    std::string User;
    std::string Group;
    std::string indexFile;
    std::string imagesDir;

    std::string camIp;
    unsigned camPort;
    unsigned camJpgLocalPort;
    unsigned camAliveInterval;
    unsigned camWidth;
    unsigned camHeight;

     std::string upnpListernIp;
     std::string upnpMediaServerName;
     std::string upnpSearchDir;


    static Config* Instance();
    bool LoadConfig(const std::string fName);
    virtual ~Config();

    bool to_bool(std::string const& s)
    {
        return s != "false";
    }
    float to_float(std::string const& s)
    {
        return atof(s.c_str());
    }
    int to_int(std::string const& s)
    {
        return atoi(s.c_str());
    }

    static const std::string currentDateTime();
    static std::string getFileContents(const std::string &fileName);
    static struct timespec getThreadWait();

protected:
    bool Load();
private:
    static Config* mInstance;
    Config();
    bool mIsInited;
    std::string mes;
    std::string mFileName;
    std::string cfgFilePath;

    int getTime(const char *p);
    void exit(const std::string &mes);
    bool checkPath(const std::string &path_, bool checkWrite, bool isFile, std::string &mes);
};

extern Config *cfg;

#endif // CONFIG_H
