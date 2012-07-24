// Generated by esidl (r1745).
// This file is expected to be modified for the Web IDL interface
// implementation.  Permission to use, copy, modify and distribute
// this file in any software license is hereby granted.

#ifndef ORG_W3C_DOM_BOOTSTRAP_HTMLOPTIONELEMENTIMP_H_INCLUDED
#define ORG_W3C_DOM_BOOTSTRAP_HTMLOPTIONELEMENTIMP_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <org/w3c/dom/html/HTMLOptionElement.h>
#include "HTMLElementImp.h"

#include <org/w3c/dom/html/HTMLElement.h>
#include <org/w3c/dom/html/HTMLFormElement.h>
#include <org/w3c/dom/html/HTMLOptionElement.h>

namespace org
{
namespace w3c
{
namespace dom
{
namespace bootstrap
{
class HTMLOptionElementImp : public ObjectMixin<HTMLOptionElementImp, HTMLElementImp>
{
public:
    // HTMLOptionElement
    bool getDisabled();
    void setDisabled(bool disabled);
    html::HTMLFormElement getForm();
    Nullable<std::u16string> getLabel();
    void setLabel(Nullable<std::u16string> label);
    bool getDefaultSelected();
    void setDefaultSelected(bool defaultSelected);
    bool getSelected();
    void setSelected(bool selected);
    std::u16string getValue();
    void setValue(std::u16string value);
    std::u16string getText();
    void setText(std::u16string text);
    int getIndex();
    // Object
    virtual Any message_(uint32_t selector, const char* id, int argc, Any* argv)
    {
        return html::HTMLOptionElement::dispatch(this, selector, id, argc, argv);
    }
    static const char* const getMetaData()
    {
        return html::HTMLOptionElement::getMetaData();
    }
    HTMLOptionElementImp(DocumentImp* ownerDocument) :
        ObjectMixin(ownerDocument, u"option") {
    }
    HTMLOptionElementImp(HTMLOptionElementImp* org, bool deep) :
        ObjectMixin(org, deep) {
    }
};

}
}
}
}

#endif  // ORG_W3C_DOM_BOOTSTRAP_HTMLOPTIONELEMENTIMP_H_INCLUDED
