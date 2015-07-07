#ifndef _UPnP_MediaServer_H_
#define _UPnP_MediaServer_H_

#include <fstream>

#include "UPnP.h"

class UPnP_MediaServer : public Device
{
public:
    std::string *id;
    Item *found;
    Container* pointer;
    bool verbose;

    UPnP_MediaServer();
    UPnP_MediaServer(Device *d);
    void startRoot();
    Container *nextFolder(const std::string &id);
    bool isRoot(){return pointer == root;};

    Item* FindById(std::string Id);
    Item* FindById(Container *c, std::string Id);
    bool Search(std::string str);
    void GetSearchCapabilities();
    void GetSystemUpdateID();
    void GetSortCapabilities();
    void upnpEventReceived( Upnp_Event* );
    Container *getMediaByID(std::string id);
    Container *nextFolderById(const std::string &id);
    bool subscribe();
    bool unSubscribe();
    void printCurDir();
    bool wgetCurDir(const std::string &);
    std::string wgetLastImage(const std::string &dirName);
    bool GetProtocolInfo();
    bool rm(const std::string &id);
    ~UPnP_MediaServer();
protected:
private:
    Container* root;

    bool getItem( Container* parent);
    void buildPlaylist( Container* parent );
    bool getMedia(Container* parent);
    IXML_Document* parseBrowseResult( IXML_Document* doc );
};

#endif // _UPnP_MediaServer_H_
