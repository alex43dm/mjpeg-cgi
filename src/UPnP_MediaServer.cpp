#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>
#include <fstream>

#include "UPnP_MediaServer.h"
#include "xmltools.h"
#include "HttpClient.h"
#include "Log.h"

extern bool verbose;

UPnP_MediaServer::UPnP_MediaServer() : Device(), verbose(false)
{
}

UPnP_MediaServer::UPnP_MediaServer(Device *d) : Device(d)
{
}

void UPnP_MediaServer::upnpEventReceived( Upnp_Event* e)
{
    std::cerr<<"upnpEventReceived( Upnp_Event* e)"<<std::endl;
    std::cerr<<"Sid: "<< e->Sid << " EventKey: "<< e->EventKey <<" ChangedVariables "<<  ixmlDocumenttoString(e->ChangedVariables) <<std::endl;
}

UPnP_MediaServer::~UPnP_MediaServer()
{
    //dtor
}

void UPnP_MediaServer::GetSearchCapabilities()
{
    IXML_Document * doc = SendAction( ContentDirectory,"GetSearchCapabilities", 0);
    DOMString str = ixmlDocumenttoString(doc);
    std::cerr<<str<<std::endl;
    return;
}

void UPnP_MediaServer::GetSystemUpdateID()
{
    IXML_Document * doc = SendAction( ContentDirectory, "GetSystemUpdateID", 0);
    DOMString str = ixmlDocumenttoString(doc);
    std::cerr<<str<<std::endl;
    return;
}

void UPnP_MediaServer::GetSortCapabilities()
{
    IXML_Document * doc = SendAction( ContentDirectory,"GetSortCapabilities", 0);
    DOMString str = ixmlDocumenttoString(doc);
    std::cerr<<str<<std::endl;
    return;
}

bool UPnP_MediaServer::Search(std::string str)
{
    IXML_Document * getInfo = SendAction( ContentDirectory,
                              "Search", 6,
                              "ContainerID", "0",
                              "SearchCriteria", str.c_str(),
                              "Filter",  "*",
                              "StartingIndex", "0",
                              "RequestedCount", "0",
                              "SortCriteria", "");

    if(!getInfo) return false;

    std::string ii =  ixmlDocumenttoString(getInfo);
    std::cerr<<ii<<std::endl;
//    IXML_Document* result = parseBrowseResult( getInfo );
    if (getInfo)   ixmlDocument_free(getInfo);

    return false;
}

void UPnP_MediaServer::startRoot()
{
    root = new Container( 0, "0", "root" );
    pointer = root;

    //getUrls();
    getMedia(root);
    //mv 1
    pointer = root->getContainer(0);
    getMedia(pointer);

    if(verbose)
    {
        //std::cout<<pointer->getTitle()<<"."<<std::endl;
        printCurDir();
    }
}

void UPnP_MediaServer::printCurDir()
{
    if(verbose){std::cout<<pointer->getTitle()<<":"<<std::endl;}
    for ( unsigned int i = 0; i < pointer->getNumContainers(); i++ )
    {
        Container* container = pointer->getContainer( i );
        std::cout<<container->getTitle()<<std::endl;
        for ( unsigned int j = 0; j < container->getNumItems(); j++ )
        {
            Item* item = container->getItem( j );
            if(verbose){std::cout<<item->getTitle()<<std::endl;}
        }
    }

    for ( unsigned int j = 0; j < pointer->getNumItems(); j++ )
    {
        Item* item = pointer->getItem( j );
        if(verbose){std::cout<<item->getTitle()<<std::endl;}
    }
}

bool UPnP_MediaServer::wgetCurDir(const std::string &dirName)
{
    mkdir(dirName.c_str(),S_IRWXU);

    for ( unsigned int j = 0; j < pointer->getNumItems(); j++ )
    {
        Item* item = pointer->getItem( j );

        if(verbose)
        {
            std::cout<<item->getTitle()<<" : "<<item->getResource()<<std::endl;
        }

        HttpClient ch = HttpClient();
        std::string ret = ch.get(std::string(item->getResource()),atoi(item->size.c_str()));
        if(!ret.empty())
        {
            std::ofstream imageFp;
            imageFp.open(dirName+"/"+std::string(item->getTitle())+".jpg");
            imageFp << ret;
            imageFp.close();
        }
    }

    return true;
}

std::string UPnP_MediaServer::wgetLastImage(const std::string &dirName)
{
    Item* item;

    for ( unsigned int j = 0; j < pointer->getNumItems(); j++ )
    {
        item = pointer->getItem(j);

        if(verbose)
        {
            std::cout<<item->getTitle()<<" : "<<item->getResource()<<std::endl;
        }
    }

    HttpClient ch = HttpClient();
    return ch.get(std::string(item->getResource()),atoi(item->size.c_str()));
}

bool UPnP_MediaServer::rm(const std::string &id)
{
    IXML_Document * doc = SendAction( ContentDirectory,"DestroyObject", 1, "ObjectID", id.c_str());
    DOMString str = ixmlDocumenttoString(doc);
    std::cerr<<str<<std::endl;


    if(doc)ixmlDocument_free(doc);

    return true;
}

Container *UPnP_MediaServer::nextFolder(const std::string &folderName)
{
	for(unsigned int i = 0; i < pointer->getNumContainers(); i++)
	{
		Container* container = pointer->getContainer(i);

		if(container->getTitle() == folderName)
		{
			pointer = container;
			getMedia(pointer);

			printCurDir();

			return pointer;
		}
	}
	return NULL;
}

Container *UPnP_MediaServer::nextFolderById(const std::string &id)
{
    if(id == "-1")
    {
        return pointer = pointer->getParent();
    }

	for(unsigned int i = 0; i < pointer->getNumContainers(); i++)
	{
		Container* container = pointer->getContainer(i);

		if(container->getObjectID() == id)
		{
			pointer = container;
			getMedia(pointer);

			printCurDir();

			return pointer;
		}
	}
	return NULL;
}


Item* UPnP_MediaServer::FindById(std::string Id)
{
    return UPnP_MediaServer::FindById(pointer, Id);
}

Item* UPnP_MediaServer::FindById(Container *c, std::string Id)
{
    if(!c)return NULL;

    id = new std::string(Id);
    found = 0;
    getItem(c);
    if( found == NULL )
    {
        std::cerr<< "Not found id: " << id <<std::endl;
        return 0;
    }
    return found;
}

bool UPnP_MediaServer::getMedia(Container* parent)
{
    if(parent == NULL)
        return false;

    if(parent->isLoaded())
        return true;

    IXML_Document * getInfo =
        SendAction( ContentDirectory,
                       "Browse", 6, "ObjectID", parent->getObjectID(),
                       "BrowseFlag", "BrowseDirectChildren",
                       "Filter", "*",
                       "StartingIndex", "0",
                       "RequestedCount", "0",
                       "SortCriteria", "");

    std::cout<<ixmlDocumenttoString(getInfo)<<std::endl;

    IXML_Document* result = parseBrowseResult( getInfo );
    if (getInfo)   ixmlDocument_free(getInfo);

    IXML_NodeList* containerNodeList = ixmlDocument_getElementsByTagName( result, "container" );
    if ( containerNodeList )
    {
        for ( unsigned int i = 0; i < ixmlNodeList_length( containerNodeList ); i++ )
        {
            IXML_Element* containerElement = ( IXML_Element* )ixmlNodeList_item( containerNodeList, i );

            const char* objectID = ixmlElement_getAttribute( containerElement, "id" );
            if ( !objectID )
            {
                Log::debug("Error item in container list parse(objectID): %s",ixmlDocumenttoString(getInfo));
                continue;
            }

            const char* title = xml_getChildElementValue( containerElement, "dc:title" );
            if ( !title )
            {
                Log::debug("Error item in container list parse(title): %s",ixmlDocumenttoString(getInfo));
                continue;
            }

            const char* childCountStr = ixmlElement_getAttribute( containerElement, "childCount" );
            int childCount = 0;
            if ( childCountStr ) childCount = atoi( childCountStr );

            const char* resource = xml_getChildElementValue( containerElement, "res" );

            if ( resource && childCount < 1 )
            {
                Item* item = new Item( parent, objectID, title, resource );
                parent->addItem( item );
            }
            else
            {
                Container* container = new Container( parent, objectID, title );
                container->artist = xmlGetChildElementValue( containerElement, "upnp:artist" );
                container->album = xmlGetChildElementValue( containerElement, "upnp:album" );
                parent->addContainer( container );

                //recursion
                //if ( childCount > 0 ) getMedia( container );
            }
        }
        ixmlNodeList_free( containerNodeList );
    }


    IXML_NodeList* itemNodeList = ixmlDocument_getElementsByTagName( result, "item" );
    if ( itemNodeList )
    {
        for ( unsigned int i = 0; i < ixmlNodeList_length( itemNodeList ); i++ )
        {
            IXML_Element* itemElement = ( IXML_Element* )ixmlNodeList_item( itemNodeList, i );

            const char* objectID = ixmlElement_getAttribute( itemElement, "id" );
            if ( !objectID )
            {
                Log::debug("Error item in node list parse(objectID): %s",ixmlDocumenttoString(getInfo));
                continue;
            }

            const char* title = xml_getChildElementValue( itemElement, "dc:title" );
            if ( !title )
            {
                Log::debug("Error item in node list parse(title): %s",ixmlDocumenttoString(getInfo));
                continue;
            }

            const char* resource = xml_getChildElementValue( itemElement, "res" );
            if ( !resource )
            {
                Log::debug("Error item in node list parse(resource): %s",ixmlDocumenttoString(getInfo));
                continue;
            }

            //fprintf(stderr,"objectID: %s title: %s resource: %s\n", objectID, title, resource);

            Item* item = new Item( parent, objectID, title, resource );
            //*outfile<<"Item: id: "<< item->getObjectID()<<" title: "<<item->getTitle()<< " url: "<< item->getResource();
            item->upnpClass =xmlGetChildElementValue( itemElement, "upnp:class" );
            item->artist = xmlGetChildElementValue( itemElement, "upnp:artist" );
            item->album = xmlGetChildElementValue( itemElement, "upnp:album" );
            item->originalTrackNumber = xmlGetChildElementValue( itemElement, "upnp:originalTrackNumber" );
            item->date = xmlGetChildElementValue( itemElement, "dc:date" );
            item->description = xmlGetChildElementValue( itemElement, "dc:description" );
            item->res = xmlGetChildElement( itemElement, "res" );
            item->size = xmlGetChildElementAttr( itemElement, "res", "size" );
            item->protocolInfo = xmlGetChildElementAttr( itemElement, "res", "protocolInfo" );

            if( item->protocolInfo.find("audio") != std::string::npos ||
               item->protocolInfo.find("video") != std::string::npos)
            {
                item->duration = xmlGetChildElementAttr( itemElement, "res", "duration" );
                item->bitrate = xmlGetChildElementAttr( itemElement, "res", "bitrate" );
                item->sampleFrequency = xmlGetChildElementAttr( itemElement, "res", "sampleFrequency" );
                item->nrAudioChannels = xmlGetChildElementAttr( itemElement, "res", "nrAudioChannels" );
            }

//            item->all = xmlGetChildElement( result, "item" );
            //std::cerr<<"Item: id: "<< item->getObjectID()<<" title: "<<item->getTitle()<<std::endl;
//            *outfile<<" duration: "<< item->duration <<" res: "<<item->res<<std::endl;
            //*outfile<<" duration: "<< item->duration <<std::endl;

            parent->addItem( item );
        }

        ixmlNodeList_free( itemNodeList );
    }

    ixmlDocument_free( result );


    return true;
}

IXML_Document* UPnP_MediaServer::parseBrowseResult( IXML_Document* doc )
{
    if ( !doc ) return 0;

    IXML_NodeList* resultList = ixmlDocument_getElementsByTagName( doc, "Result" );
    if ( !resultList ) return 0;

    IXML_Node* resultNode = ixmlNodeList_item( resultList, 0 );

    ixmlNodeList_free( resultList );

    if ( !resultNode ) return 0;

    IXML_Node* textNode = ixmlNode_getFirstChild( resultNode );
    if ( !textNode ) return 0;

    const char* resultString = ixmlNode_getNodeValue( textNode );
    char* resultXML = strdup( resultString );

    //resolve_xml_special_chars( resultXML );

    IXML_Document* browseDoc = ixmlParseBuffer( resultXML );

    free( resultXML );

    return browseDoc;
}

void UPnP_MediaServer::buildPlaylist( Container* parent )
{
    for ( unsigned int i = 0; i < parent->getNumContainers(); i++ )
    {
        Container* container = parent->getContainer( i );
        //recursion
        buildPlaylist( container );
    }
    /*
        for ( unsigned int i = 0; i < parent->getNumItems(); i++ )
        {
            Item* item = parent->getItem( i );
        }*/
}

bool UPnP_MediaServer::getItem( Container* parent)
{
    for ( unsigned int i = 0; i < parent->getNumContainers(); i++ )
    {
        Container* container = parent->getContainer( i );
        //recursion
        if( getItem( container) )
        {
            return true;
        }
    }

    for ( unsigned int i = 0; i < parent->getNumItems(); i++ )
    {
        Item* item = parent->getItem( i );
        if( item->getObjectID() == *id)
        {
            found = item;
            return true;
        }
    }
    return false;
}


bool UPnP_MediaServer::subscribe()
{
    return Subscribe(ContentDirectory, 20);
}

bool UPnP_MediaServer::unSubscribe()
{
    return UnSubscribe(ContentDirectory);
}

bool UPnP_MediaServer::GetProtocolInfo()
{
    startRoot();
    nextFolder("JPEG");
/*

    IXML_Document * getInfo =
        SendAction( ConnectionManager, "GetProtocolInfo", 0);

    std::cout<<ixmlDocumenttoString(getInfo)<<std::endl;

    getInfo =
        SendAction( ConnectionManager, "GetCurrentConnectionIDs", 0);

    std::cout<<ixmlDocumenttoString(getInfo)<<std::endl;

    getInfo =
        SendAction( ConnectionManager, "GetCurrentConnectionInfo", 1,
                   "ConnectionID","0");

    std::cout<<ixmlDocumenttoString(getInfo)<<std::endl;

    getInfo =
        SendAction( ConnectionManager, "GetRendererItemInfo", 0);

    std::cout<<ixmlDocumenttoString(getInfo)<<std::endl;
*/
  /*
   getInfo =
        SendAction( ConnectionManager, "DestroyObject", 1, "ObjectID",);

    std::cout<<ixmlDocumenttoString(getInfo)<<std::endl;
*/


/*
    IXML_Document* result = parseBrowseResult( getInfo );
    if (getInfo)   ixmlDocument_free(getInfo);

    IXML_NodeList* containerNodeList = ixmlDocument_getElementsByTagName( result, "container" );
    if ( containerNodeList )
*/
    return true;
}
