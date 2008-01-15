/*
 * Copyright 2008 Google Inc.
 * Copyright 2006, 2007 Nintendo Co., Ltd.
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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <es.h>
#include <es/classFactory.h>
#include <es/clsid.h>
#include <es/exception.h>
#include <es/handle.h>
#include <es/ref.h>
#include <es/base/IClassStore.h>
#include <es/base/IInterfaceStore.h>
#include <es/base/IProcess.h>
#include "location.h"

#define TEST(exp)                           \
    (void) ((exp) ||                        \
            (esPanic(__FILE__, __LINE__, "\nFailed test " #exp), 0))

using namespace es;

ICurrentProcess* System();

class Location : public ILocation
{
    static int  id;

    Ref         ref;
    Point       point;
    char        name[14];

public:
    Location()
    {
        point.x = point.y = 0;
        sprintf(name, "id%d", ++id);
        name[13] = '\0';
    }

    ~Location()
    {
    }

    void set(const Point* point)
    {
        this->point = *point;
    }

    void get(Point* point)
    {
        *point = this->point;
    }

    void move(const Point* direction)
    {
        point.x += direction->x;
        point.y += direction->y;
    }

    int setName(const char* name)
    {
        strncpy(this->name, name, 13);
    }

    int getName(char* name, int len)
    {
        unsigned count(strlen(this->name) + 1);
        if (len < count)
        {
            count = len;
        }
        strncpy(name, this->name, count);
        return count;
    }

    void* queryInterface(const Guid& riid)
    {
        void* objectPtr;
        if (riid == IInterface::iid())
        {
            objectPtr = static_cast<ILocation*>(this);
        }
        else if (riid == ILocation::iid())
        {
            objectPtr = static_cast<ILocation*>(this);
        }
        else
        {
            return NULL;
        }
        static_cast<IInterface*>(objectPtr)->addRef();
        return objectPtr;
    }

    unsigned int addRef()
    {
        return ref.addRef();
    }

    unsigned int release()
    {
        unsigned int count = ref.release();
        if (count == 0)
        {
            delete this;
            return 0;
        }
        return count;
    }
};

int Location::id = 0;

int main(int argc, char* argv[])
{
    esReport("This is the Location server process.\n");
    System()->trace(true);

    Handle<IContext> nameSpace = System()->getRoot();

    // Register ILocation interface.
    Handle<IInterfaceStore> interfaceStore = nameSpace->lookup("interface");
    TEST(interfaceStore);
    interfaceStore->add(ILocationInfo, ILocationInfoSize);

    // Register Location factory.
    Handle<IClassStore> classStore = nameSpace->lookup("class");
    TEST(classStore);
    Handle<IClassFactory> binderFactory(new(ClassFactory<Location>));
    classStore->add(CLSID_Location, binderFactory);

    // Create a client process.
    Handle<IProcess> client;
    client = reinterpret_cast<IProcess*>(
        classStore->createInstance(CLSID_Process,
                                   client->iid()));
    TEST(client);

    // Start the client process.
    Handle<IFile> file = nameSpace->lookup("file/locationClient.elf");
    TEST(file);
    client->start(file);

    // Wait for the client to exit
    client->wait();

    // Unregister Location factory.
    classStore->remove(CLSID_Location);

    System()->trace(false);
}
