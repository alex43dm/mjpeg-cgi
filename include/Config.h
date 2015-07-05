#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <list>

#include <tinyxml.h>

class Config
{
public:
    //new params
    std::string SocketPath;
    unsigned ServerThreads;
    unsigned FPS;
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
    std::string getFileContents(const std::string &fileName);
    void exit(const std::string &mes);
    bool checkPath(const std::string &path_, bool checkWrite, bool isFile, std::string &mes);
};

extern Config *cfg;

#endif // CONFIG_H
