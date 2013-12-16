// Generated by esidl 0.2.1.
// This file is expected to be modified for the Web IDL interface
// implementation.  Permission to use, copy, modify and distribute
// this file in any software license is hereby granted.

#ifndef ORG_W3C_DOM_BOOTSTRAP_CSSPERCENTAGECOMPONENTVALUEIMP_H_INCLUDED
#define ORG_W3C_DOM_BOOTSTRAP_CSSPERCENTAGECOMPONENTVALUEIMP_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <org/w3c/dom/css/CSSPercentageComponentValue.h>

namespace org
{
namespace w3c
{
namespace dom
{
namespace bootstrap
{
class CSSPercentageComponentValueImp : public ObjectMixin<CSSPercentageComponentValueImp>
{
public:
    // CSSPercentageComponentValue
    float getPercent();
    void setPercent(float percent);
    // Object
    virtual Any message_(uint32_t selector, const char* id, int argc, Any* argv)
    {
        return css::CSSPercentageComponentValue::dispatch(this, selector, id, argc, argv);
    }
    static const char* const getMetaData()
    {
        return css::CSSPercentageComponentValue::getMetaData();
    }
};

}
}
}
}

#endif  // ORG_W3C_DOM_BOOTSTRAP_CSSPERCENTAGECOMPONENTVALUEIMP_H_INCLUDED