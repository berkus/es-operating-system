/*
 * Copyright 2008, 2009 Google Inc.
 * Copyright 2007 Nintendo Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NINTENDO_ESIDL_CPLUSPLUS_H_INCLUDED
#define NINTENDO_ESIDL_CPLUSPLUS_H_INCLUDED

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "esidl.h"

class CPlusPlus : public Visitor
{
protected:
    static const int TabWidth = 4;

    std::string indentString;
    std::string prefix;
    FILE* file;
    bool interfaceMode;
    bool constructorMode;

    void indent()
    {
        indentString += std::string(TabWidth, ' ');
    }

    void unindent()
    {
        indentString.erase(indentString.length() - TabWidth);
    }

    void write(const char* format, ...)
    {
        va_list ap;
        va_start(ap, format);
        vfprintf(file, format, ap);
        va_end(ap);
    }

    void writetab()
    {
        write("%s", indentString.c_str());
    }

    void writeln(const char* format, ...)
    {
        if (*format)
        {
            writetab();
        }
        va_list ap;
        va_start(ap, format);
        vfprintf(file, format, ap);
        va_end(ap);
        write("\n");
    }

    void printChildren(const Node* node)
    {
        if (node->isLeaf())
        {
            return;
        }

        std::string separator;
        bool br;
        int count = 0;
        for (NodeList::iterator i = node->begin(); i != node->end(); ++i)
        {
            if (1 < (*i)->getRank())
            {
                continue;
            }
            if ((*i)->isNative(node->getParent()))
            {
                continue;
            }
            if (0 < count)
            {
                write("%s", separator.c_str());
            }
            separator = (*i)->getSeparator();
            br  = (separator[separator.size() - 1] == '\n') ? true : false;
            if (br)
            {
                write("%s", indentString.c_str());
            }
            if (0 < prefix.size())
            {
                write("%s", prefix.c_str());
            }
            (*i)->accept(this);
            ++count;
        }
        if (br && 0 < count)
        {
            write("%s", separator.c_str());
        }
    }

public:
    CPlusPlus(FILE* file) :
        file(file),
        interfaceMode(false),
        constructorMode(false)
    {
    }

    virtual void at(const Node* node)
    {
        if (0 < node->getName().size())
        {
            if (!interfaceMode)
            {
                write("%s", node->getName().c_str());
            }
            else
            {
                std::string name = getInterfaceName(node->getName());
                write("%s", name.c_str());
            }
        }
        else
        {
            printChildren(node);
        }
    }

    virtual void at(const Type* node)
    {
        if (node->getName() == "boolean")
        {
            write("bool");
        }
        else if (node->getName() == "octet")
        {
            write("unsigned char");
        }
        else if (node->getName() == "long")
        {
            write("int");
        }
        else if (node->getName() == "unsigned long")
        {
            write("unsigned int");
        }
        else if (node->getName() == "any")
        {
            write("Any");
        }
        else if (node->getName() == "wchar")
        {
            write("wchar_t");
        }
        else if (node->getName() == "string")
        {
            write("char*");
        }
        else if (node->getName() == "wstring")
        {
            write("wchar_t*");
        }
        else if (node->getName() == "Object")
        {
            if (const char* base = Node::getBaseObjectName())
            {
                write("I%s", base);
            }
            else
            {
                write("void");
            }
        }
        else if (node->getName() == "uuid")
        {
            write("Guid&");
        }
        else
        {
            write("%s", node->getName().c_str());
        }
    }

    virtual void at(const SequenceType* node)
    {
        Node* spec = node->getSpec();
        if (spec->isOctet(node->getParent()))
        {
            write("void*");
        }
        else
        {
            spec->accept(this);
            write("*");
        }
    }

    void getter(const Attribute* node)
    {
        std::string cap = node->getName().c_str();
        cap[0] = toupper(cap[0]);   // XXX
        Node* spec = node->getSpec();
        SequenceType* seq = const_cast<SequenceType*>(spec->isSequence(node->getParent()));
        std::string name = node->getName();
        size_t pos = name.rfind("::");
        if (pos != std::string::npos)
        {
            name = name.substr(pos + 2);
        }
        name[0] = tolower(name[0]); // XXX

        write("virtual ");
        if (seq ||
            spec->isString(node->getParent()) ||
            spec->isWString(node->getParent()))
        {
            write("int get%s(", cap.c_str());
            if (seq)
            {
                seq->accept(this);
            }
            else
            {
                spec->accept(this);
            }
            write(" %s, int %sLength)", name.c_str(), name.c_str());
        }
        else if (spec->isStruct(node->getParent()))
        {
            write("void get%s(", cap.c_str());
            spec->accept(this);
            write("* %s)", name.c_str());
        }
        else if (spec->isArray(node->getParent()))
        {
            write("void get%s(", cap.c_str());
            spec->accept(this);
            write(" %s)", name.c_str());
        }
        else if (spec->isAny(node->getParent()))
        {
            spec->accept(this);
            write(" get%s(", cap.c_str());
            write("void* %s, int %sLength)", name.c_str(), name.c_str());
        }
        else
        {
            if (spec->isInterface(node->getParent()))
            {
                interfaceMode = true;
                spec->accept(this);
                write("*");
                interfaceMode = false;
            }
            else if (NativeType* nativeType = spec->isNative(node->getParent()))
            {
                nativeType->accept(this);
            }
            else
            {
                spec->accept(this);
            }
            write(" %s%s()", spec->isBoolean(node->getParent()) ? "is" : "get", cap.c_str());
        }
    }

    bool setter(const Attribute* node)
    {
        if (node->isReadonly())
        {
            return false;
        }

        std::string cap = node->getName().c_str();
        cap[0] = toupper(cap[0]);   // XXX
        Node* spec = node->getSpec();
        SequenceType* seq = const_cast<SequenceType*>(spec->isSequence(node->getParent()));
        std::string name = node->getName();
        size_t pos = name.rfind("::");
        if (pos != std::string::npos)
        {
            name = name.substr(pos + 2);
        }
        name[0] = tolower(name[0]); // XXX

        // setter
        write("virtual ");
        if (seq)
        {
            write("int set%s(const ", cap.c_str());
            seq->accept(this);
            write(" %s, int %sLength)", name.c_str(), name.c_str());
        }
        else if (spec->isString(node->getParent()) || spec->isWString(node->getParent()))
        {
            write("int set%s(const ", cap.c_str());
            spec->accept(this);
            write(" %s)", name.c_str());
        }
        else if (spec->isStruct(node->getParent()))
        {
            write("void set%s(const ", cap.c_str());
            spec->accept(this);
            write("* %s)", name.c_str());
        }
        else if (spec->isArray(node->getParent()) ||
                 spec->isAny(node->getParent()))
        {
            write("void set%s(const ", cap.c_str());
            spec->accept(this);
            write(" %s)", name.c_str());
        }
        else
        {
            write("void set%s(", cap.c_str());
            if (spec->isInterface(node->getParent()))
            {
                interfaceMode = true;
                spec->accept(this);
                write("*");
                interfaceMode = false;
            }
            else if (NativeType* nativeType = spec->isNative(node->getParent()))
            {
                nativeType->accept(this);
            }
            else
            {
                spec->accept(this);
            }
            write(" %s)", name.c_str());
        }
        return true;
    }

    virtual void at(const OpDcl* node)
    {
        if (!constructorMode)
        {
            write("virtual ");
        }
        else
        {
            write("static ");
        }

        Node* spec = node->getSpec();
        SequenceType* seq = const_cast<SequenceType*>(spec->isSequence(node->getParent()));
        if (seq ||
            spec->isString(node->getParent()) ||
            spec->isWString(node->getParent()))
        {
            std::string name = spec->getName();
            size_t pos = name.rfind("::");
            if (pos != std::string::npos)
            {
                name = name.substr(pos + 2);
            }
            name[0] = tolower(name[0]); // XXX

            write("int %s(", node->getName().c_str());
            if (seq)
            {
                seq->accept(this);
            }
            else
            {
                spec->accept(this);
            }
            write(" %s, int %sLength", name.c_str(), name.c_str());

            if (node->begin() != node->end())
            {
                write(", ");
            }
        }
        else if (spec->isStruct(node->getParent()))
        {
            std::string name = spec->getName();
            size_t pos = name.rfind("::");
            if (pos != std::string::npos)
            {
                name = name.substr(pos + 2);
            }
            name[0] = tolower(name[0]); // XXX

            write("void %s(", node->getName().c_str());
            spec->accept(this);
            write("* %s", name.c_str());

            if (node->begin() != node->end())
            {
                write(", ");
            }
        }
        else if (spec->isArray(node->getParent()))
        {
            std::string name = spec->getName();
            size_t pos = name.rfind("::");
            if (pos != std::string::npos)
            {
                name = name.substr(pos + 2);
            }
            name[0] = tolower(name[0]); // XXX

            write("void %s(", node->getName().c_str());
            spec->accept(this);
            write(" %s", name.c_str());

            if (node->begin() != node->end())
            {
                write(", ");
            }
        }
        else if (spec->isAny(node->getParent()))
        {
            std::string name = spec->getName();
            size_t pos = name.rfind("::");
            if (pos != std::string::npos)
            {
                name = name.substr(pos + 2);
            }
            name[0] = tolower(name[0]); // XXX

            spec->accept(this);
            write(" %s(", node->getName().c_str());
            write("void* %s, int %sLength", name.c_str(), name.c_str());

            if (node->begin() != node->end())
            {
                write(", ");
            }
        }
        else
        {
            if (spec->isInterface(node->getParent()))
            {
                interfaceMode = true;
                spec->accept(this);
                write("*");
                interfaceMode = false;
            }
            else if (NativeType* nativeType = spec->isNative(node->getParent()))
            {
                nativeType->accept(this);
            }
            else
            {
                spec->accept(this);
            }
            write(" %s", node->getName().c_str());
            write("(");
        }

        for (NodeList::iterator i = node->begin(); i != node->end(); ++i)
        {
            if (i != node->begin())
            {
                write(", ");
            }
            (*i)->accept(this);
        }

        write(")");
        if (node->getRaises())
        {
            write(" throw(");
            node->getRaises()->accept(this);
            write(")");
        }
    }

    virtual void at(const ParamDcl* node)
    {
        Node* spec = node->getSpec();
        SequenceType* seq = const_cast<SequenceType*>(spec->isSequence(node->getParent()));

        switch (node->getAttr())
        {
        case ParamDcl::In:
            if (seq ||
                spec->isGuid(node->getParent()) ||
                spec->isString(node->getParent()) ||
                spec->isWString(node->getParent()) ||
                spec->isStruct(node->getParent()) ||
                spec->isArray(node->getParent()))
            {
                write("const ");
            }
            break;
        }

        if (seq)
        {
            seq->accept(this);
            write(" %s, int %sLength", node->getName().c_str() , node->getName().c_str());
        }
        else if (spec->isStruct(node->getParent()))
        {
            spec->accept(this);
            write("* %s", node->getName().c_str());
        }
        else if (spec->isArray(node->getParent()))
        {
            spec->accept(this);
            write(" %s", node->getName().c_str());
        }
        else
        {
            if (spec->isInterface(node->getParent()))
            {
                interfaceMode = true;
                spec->accept(this);
                write("*");
                interfaceMode = false;
            }
            else if (NativeType* nativeType = spec->isNative(node->getParent()))
            {
                nativeType->accept(this);
            }
            else
            {
                spec->accept(this);
            }
            if (!spec->isString(node->getParent()) &&
                !spec->isWString(node->getParent()))
            {
                switch (node->getAttr())
                {
                case ParamDcl::Out:
                case ParamDcl::InOut:
                    write("*");
                    break;
                }
                write(" %s", node->getName().c_str());
            }
            else
            {
                write(" %s", node->getName().c_str());
                switch (node->getAttr())
                {
                case ParamDcl::Out:
                case ParamDcl::InOut:
                    write(", int %sLength", node->getName().c_str());
                    break;
                }
            }
        }
    }

    virtual void at(const Include* node)
    {
    }

    static std::string getInterfaceName(std::string qualifiedName)
    {
        size_t pos = qualifiedName.rfind("::");
        if (pos != std::string::npos)
        {
            qualifiedName.insert(pos + 2, "I");
        }
        else
        {
            qualifiedName.insert(0, "I");
        }
        return qualifiedName;
    }
};

#endif  // NINTENDO_ESIDL_CPLUSPLUS_H_INCLUDED
