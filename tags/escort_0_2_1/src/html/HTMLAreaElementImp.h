// Generated by esidl (r1752).
// This file is expected to be modified for the Web IDL interface
// implementation.  Permission to use, copy, modify and distribute
// this file in any software license is hereby granted.

#ifndef ORG_W3C_DOM_BOOTSTRAP_HTMLAREAELEMENTIMP_H_INCLUDED
#define ORG_W3C_DOM_BOOTSTRAP_HTMLAREAELEMENTIMP_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <org/w3c/dom/html/HTMLAreaElement.h>
#include "HTMLElementImp.h"

#include <org/w3c/dom/html/HTMLElement.h>
#include <org/w3c/dom/DOMTokenList.h>

namespace org
{
namespace w3c
{
namespace dom
{
namespace bootstrap
{
class HTMLAreaElementImp : public ObjectMixin<HTMLAreaElementImp, HTMLElementImp>
{
public:
    HTMLAreaElementImp(DocumentImp* ownerDocument) :
        ObjectMixin(ownerDocument, u"area") {
    }
    HTMLAreaElementImp(HTMLAreaElementImp* org, bool deep) :
        ObjectMixin(org, deep) {
    }

    // HTMLAreaElement
    std::u16string getAlt();
    void setAlt(std::u16string alt);
    std::u16string getCoords();
    void setCoords(std::u16string coords);
    std::u16string getShape();
    void setShape(std::u16string shape);
    std::u16string getHref();
    void setHref(std::u16string href);
    std::u16string getTarget();
    void setTarget(std::u16string target);
    std::u16string getPing();
    void setPing(std::u16string ping);
    std::u16string getRel();
    void setRel(std::u16string rel);
    DOMTokenList getRelList();
    std::u16string getMedia();
    void setMedia(std::u16string media);
    std::u16string getHreflang();
    void setHreflang(std::u16string hreflang);
    std::u16string getType();
    void setType(std::u16string type);
    std::u16string getProtocol();
    void setProtocol(std::u16string protocol);
    std::u16string getHost();
    void setHost(std::u16string host);
    std::u16string getHostname();
    void setHostname(std::u16string hostname);
    std::u16string getPort();
    void setPort(std::u16string port);
    std::u16string getPathname();
    void setPathname(std::u16string pathname);
    std::u16string getSearch();
    void setSearch(std::u16string search);
    std::u16string getHash();
    void setHash(std::u16string hash);
    // HTMLAreaElement-8
    bool getNoHref();
    void setNoHref(bool noHref);
    // Object
    virtual Any message_(uint32_t selector, const char* id, int argc, Any* argv)
    {
        return html::HTMLAreaElement::dispatch(this, selector, id, argc, argv);
    }
    static const char* const getMetaData()
    {
        return html::HTMLAreaElement::getMetaData();
    }
};

}
}
}
}

#endif  // ORG_W3C_DOM_BOOTSTRAP_HTMLAREAELEMENTIMP_H_INCLUDED