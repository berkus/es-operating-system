// Generated by esidl (r1745).
// This file is expected to be modified for the Web IDL interface
// implementation.  Permission to use, copy, modify and distribute
// this file in any software license is hereby granted.

#ifndef ORG_W3C_DOM_BOOTSTRAP_HASHCHANGEEVENTIMP_H_INCLUDED
#define ORG_W3C_DOM_BOOTSTRAP_HASHCHANGEEVENTIMP_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <org/w3c/dom/html/HashChangeEvent.h>
#include "EventImp.h"

#include <org/w3c/dom/events/Event.h>

namespace org
{
namespace w3c
{
namespace dom
{
namespace bootstrap
{
class HashChangeEventImp : public ObjectMixin<HashChangeEventImp, EventImp>
{
public:
    // HashChangeEvent
    Any getOldURL();
    Any getNewURL();
    void initHashChangeEvent(std::u16string typeArg, bool canBubbleArg, bool cancelableArg, std::u16string oldURLArg, std::u16string newURLArg);
    // Object
    virtual Any message_(uint32_t selector, const char* id, int argc, Any* argv)
    {
        return html::HashChangeEvent::dispatch(this, selector, id, argc, argv);
    }
    static const char* const getMetaData()
    {
        return html::HashChangeEvent::getMetaData();
    }
};

}
}
}
}

#endif  // ORG_W3C_DOM_BOOTSTRAP_HASHCHANGEEVENTIMP_H_INCLUDED
