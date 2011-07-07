// Generated by esidl (r1745).
// This file is expected to be modified for the Web IDL interface
// implementation.  Permission to use, copy, modify and distribute
// this file in any software license is hereby granted.

#ifndef ORG_W3C_DOM_BOOTSTRAP_BOOLEANCALLBACKIMP_H_INCLUDED
#define ORG_W3C_DOM_BOOTSTRAP_BOOLEANCALLBACKIMP_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <org/w3c/dom/html/BooleanCallback.h>

namespace org
{
namespace w3c
{
namespace dom
{
namespace bootstrap
{
class BooleanCallbackImp : public ObjectMixin<BooleanCallbackImp>
{
public:
    // BooleanCallback
    void handleEvent(bool arg);
    // Object
    virtual Any message_(uint32_t selector, const char* id, int argc, Any* argv)
    {
        return html::BooleanCallback::dispatch(this, selector, id, argc, argv);
    }
    static const char* const getMetaData()
    {
        return html::BooleanCallback::getMetaData();
    }
};

}
}
}
}

#endif  // ORG_W3C_DOM_BOOTSTRAP_BOOLEANCALLBACKIMP_H_INCLUDED
