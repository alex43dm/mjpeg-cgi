#include <iostream>
#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <upnp/ixml.h>

std::string xmlGetChildElementValue( IXML_Element* parent,  const std::string tagName )
{
    if ( !parent ) return "";
    if ( tagName.empty() ) return "";


    IXML_NodeList* nodeList = ixmlElement_getElementsByTagName( parent, tagName.c_str());
    if ( !nodeList ) return "";

    IXML_Node* element = ixmlNodeList_item( nodeList, 0 );
    ixmlNodeList_free( nodeList );
    if ( !element ) return "";

    IXML_Node* textNode = ixmlNode_getFirstChild( element );
    if ( !textNode ) return "";
    const char* ret = ixmlNode_getNodeValue( textNode );
    if( ret )
    {
        return ret;
    }

    return "";
}

std::string xmlGetChildElement( IXML_Element* parent, const char* tagName )
{
    if ( !parent ) return "";
    if ( !tagName ) return "";

    char* s = strdup( tagName );
    IXML_NodeList* nodeList = ixmlElement_getElementsByTagName( parent, s );
    free( s );

    IXML_Node* element = ixmlNodeList_item( nodeList, 0 );
    ixmlNodeList_free( nodeList );
    if ( !element ) return "";

    const char* ret = ixmlNodetoString( element );
    if( ret )
    {
        return ret;
    }

    return "";
}

std::string xmlGetChildElementAttr( IXML_Element* parent, const char* tagName, const char* attrName )
{
    if ( !parent ) return "";
    if ( !tagName ) return "";
    if ( !attrName ) return "";

    char* s = strdup( tagName );
    IXML_NodeList* nodeList = ixmlElement_getElementsByTagName( parent, s );
    free( s );

    IXML_Node* element = ixmlNodeList_item( nodeList, 0 );
    ixmlNodeList_free( nodeList );
    if ( !element ) return "";

    const char* ret = ixmlElement_getAttribute( (IXML_Element* )element,  attrName);
    if( ret )
    {
        return ret;
    }

    return "";
}
/*
IXML_Node* xmlGetChildElementAttr(std::string name, std::string val, IXML_Document* doc)
{
    IXML_Element *el = ixmlDocument_createElement(doc, name.c_str());
    IXML_Element *v = ixmlDocument_createElement(doc, val.c_str());
    (&v->n)->nodeType = eTEXT_NODE;
    ixmlNode_setNodeValue(&v->n,val.c_str());
    ixmlNode_appendChild( &el->n, &v->n );
    return &el->n;
}
*/
void Replace(std::string& str, const std::string& oldStr, const std::string& newStr)
{
    size_t pos = 0;
    while((pos = str.find(oldStr, pos)) != std::string::npos)
    {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}

const char* xml_getChildElementValue( IXML_Element* parent, const char* tagName )
{
    if ( !parent ) return 0;
    if ( !tagName ) return 0;

    char* s = strdup( tagName );
    IXML_NodeList* nodeList = ixmlElement_getElementsByTagName( parent, s );
    free( s );
    if ( !nodeList ) return 0;

    IXML_Node* element = ixmlNodeList_item( nodeList, 0 );
    ixmlNodeList_free( nodeList );
    if ( !element ) return 0;

    IXML_Node* textNode = ixmlNode_getFirstChild( element );
    if ( !textNode ) return 0;

    return ixmlNode_getNodeValue( textNode );
}

const char* xml_getChildElement( IXML_Element* parent, const char* tagName )
{
    if ( !parent ) return 0;
    if ( !tagName ) return 0;

    char* s = strdup( tagName );
    IXML_NodeList* nodeList = ixmlElement_getElementsByTagName( parent, s );
    free( s );

    IXML_Node* element = ixmlNodeList_item( nodeList, 0 );
    ixmlNodeList_free( nodeList );
    if ( !element ) return 0;

    IXML_Node* textNode = ixmlNode_getFirstChild( element );
    if ( !textNode ) return 0;

    return ixmlNodetoString( textNode );
}

IXML_Document* clearXmlDoc(IXML_Document* doc)
{
    std::string ss = ixmlDocumenttoString(doc);
    Replace(ss,"&quot;","\"");
    Replace(ss,"&lt;","<");
    Replace(ss,"&gt;",">");

   return ixmlParseBuffer( ss.c_str() );
}
void clearXmlString(std::string &str)
{
    Replace(str,"&quot;","\"");
    Replace(str,"&lt;","<");
    Replace(str,"&gt;",">");
}


