// Generated by esidl 0.2.1.
// This file is expected to be modified for the Web IDL interface
// implementation.  Permission to use, copy, modify and distribute
// this file in any software license is hereby granted.

#ifndef ORG_W3C_DOM_BOOTSTRAP_XMLHTTPREQUESTIMP_H_INCLUDED
#define ORG_W3C_DOM_BOOTSTRAP_XMLHTTPREQUESTIMP_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <org/w3c/dom/XMLHttpRequest.h>
#include "XMLHttpRequestEventTargetImp.h"

#include <org/w3c/dom/Document.h>
#include <org/w3c/dom/file/Blob.h>
#include <org/w3c/dom/html/Function.h>
#include <org/w3c/dom/typedarray/ArrayBuffer.h>
#include <org/w3c/dom/XMLHttpRequestEventTarget.h>
#include <org/w3c/dom/XMLHttpRequestUpload.h>
#include <org/w3c/dom/XMLHttpRequest.h>
#include <org/w3c/dom/FormData.h>

namespace org
{
namespace w3c
{
namespace dom
{
namespace bootstrap
{
class XMLHttpRequestImp : public ObjectMixin<XMLHttpRequestImp, XMLHttpRequestEventTargetImp>
{
public:
    // XMLHttpRequest
    html::Function getOnreadystatechange();
    void setOnreadystatechange(html::Function onreadystatechange);
    unsigned short getReadyState();
    void open(const std::u16string& method, const std::u16string& url);
    void open(const std::u16string& method, const std::u16string& url, bool async);
    void open(const std::u16string& method, const std::u16string& url, bool async, const Nullable<std::u16string>& user);
    void open(const std::u16string& method, const std::u16string& url, bool async, const Nullable<std::u16string>& user, const Nullable<std::u16string>& password);
    void setRequestHeader(const std::u16string& header, const std::u16string& value);
    unsigned int getTimeout();
    void setTimeout(unsigned int timeout);
    bool getWithCredentials();
    void setWithCredentials(bool withCredentials);
    XMLHttpRequestUpload getUpload();
    void send();
    void send(typedarray::ArrayBuffer data);
    void send(file::Blob data);
    void send(Document data);
    void send(FormData data);
    void send(const Nullable<std::u16string>& data);
    void abort();
    unsigned short getStatus();
    std::u16string getStatusText();
    std::u16string getResponseHeader(const std::u16string& header);
    std::u16string getAllResponseHeaders();
    void overrideMimeType(const std::u16string& mime);
    std::u16string getResponseType();
    void setResponseType(const std::u16string& responseType);
    Any getResponse();
    std::u16string getResponseText();
    Document getResponseXML();
    // Object
    virtual Any message_(uint32_t selector, const char* id, int argc, Any* argv)
    {
        return XMLHttpRequest::dispatch(this, selector, id, argc, argv);
    }
    static const char* const getMetaData()
    {
        return XMLHttpRequest::getMetaData();
    }
};

}
}
}
}

#endif  // ORG_W3C_DOM_BOOTSTRAP_XMLHTTPREQUESTIMP_H_INCLUDED