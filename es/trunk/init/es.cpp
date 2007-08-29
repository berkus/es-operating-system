/*
 * Copyright (c) 2006, 2007
 * Nintendo Co., Ltd.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Nintendo makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */

#include <stdlib.h>
#include <es.h>
#include <es/clsid.h>
#include <es/handle.h>
#include <es/base/IProcess.h>
#include <es/device/IFileSystem.h>

#include <string.h>
#include <es.h>
#include <es/dateTime.h>
#include <es/endian.h>
#include <es/handle.h>
#include <es/list.h>
#include <es/ref.h>
#include <es/base/IService.h>
#include <es/base/IStream.h>
#include <es/base/IThread.h>
#include <es/device/INetworkInterface.h>
#include <es/naming/IContext.h>
#include <es/net/ISocket.h>
#include <es/net/IInternetConfig.h>
#include <es/net/IResolver.h>
#include <es/net/arp.h>
#include <es/net/dhcp.h>
#include <es/net/dns.h>
#include <es/net/udp.h>

int esInit(IInterface** nameSpace);
extern void esRegisterInternetProtocol(IContext* context);
extern void esRegisterDHCPClient(IContext* context);

IStream* esReportStream();

void startProcess(Handle<IContext> root, Handle<IProcess> process, Handle<IFile> file)
{
    ASSERT(root);
    ASSERT(process);
    ASSERT(file);

    long long size = 0;

    size = file->getSize();
    esReport("size: %lld\n", size);

    process->setRoot(root);
    process->setIn(esReportStream());
    process->setOut(esReportStream());
    process->setError(esReportStream());
    process->start(file);
}

void init(Handle<IContext> root)
{
    Handle<IIterator>   iter;
    Handle<IFile>       file;
    long long           size = 0;

    // get console.
    Handle<IStream> console = 0;
    while (!console)
    {
        console = root->lookup("device/console");
        esSleep(10000000 / 60);
    }

    file = root->lookup("file/esjs.elf");
    if (!file)
    {
        esReport("Could not open \"esjs.elf\"\n");
        return;
    }
    size = file->getSize();
    esReport("main size: %lld\n", size);

    Handle<IProcess> process;
    esCreateInstance(CLSID_Process, IID_IProcess,
                     reinterpret_cast<void**>(&process));
    ASSERT(process);
    process->setRoot(root);
    process->setIn(console);
    process->setOut(console);
    process->setError(console);
    process->start(file, "esjs file/shell.js");
    process->wait();
    esReport("esjs exited.\n");
}

int initNetwork(Handle<IContext> context)
{
    // Get DIX interface
    Handle<INetworkInterface> ethernetInterface = context->lookup("device/ethernet");
    if (!ethernetInterface)
    {
        return -1;
    }

    esRegisterInternetProtocol(context);

    // Lookup resolver object
    Handle<IResolver> resolver = context->lookup("network/resolver");

    // Lookup internet config object
    Handle<IInternetConfig> config = context->lookup("network/config");

    // Setup DIX interface
    ethernetInterface->start();
    int dixID = config->addInterface(ethernetInterface);
    esReport("dixID: %d\n", dixID);

    esRegisterDHCPClient(context);

    Handle<IService> service = context->lookup("network/interface/2/dhcp");
    if (service)
    {
        service->start();
    }

#if 0
    esSleep(120000000);

    Handle<IInternetAddress> host = config->getAddress(dixID);
    if (host)
    {
        InAddr addr;

        host->getAddress(&addr, sizeof(InAddr));
        u32 h = ntohl(addr.addr);
        esReport("host: %d.%d.%d.%d\n", (u8) (h >> 24), (u8) (h >> 16), (u8) (h >> 8), (u8) h);
    }
#endif

    if (service)
    {
        // service->stop();
    }
    // ethernetInterface->stop();
    return 0;
}

int main(int argc, char* argv[])
{
    IInterface* ns = 0;
    esInit(&ns);
    Handle<IContext> nameSpace(ns);

    // initNetwork(nameSpace);

    Handle<IClassStore> classStore(nameSpace->lookup("class"));
    esRegisterFatFileSystemClass(classStore);

    Handle<IStream> disk = nameSpace->lookup("device/ata/channel0/device0");
    long long diskSize;
    diskSize = disk->getSize();
    esReport("diskSize: %lld\n", diskSize);

    Handle<IFileSystem> fatFileSystem;
    long long freeSpace;
    long long totalSpace;

    esCreateInstance(CLSID_FatFileSystem, IID_IFileSystem,
                     reinterpret_cast<void**>(&fatFileSystem));
    fatFileSystem->mount(disk);
    {
        Handle<IContext> root;

        fatFileSystem->getRoot(reinterpret_cast<IContext**>(&root));

        nameSpace->bind("file", root);

        // start event manager process.
        Handle<IProcess> eventProcess;
        esCreateInstance(CLSID_Process, IID_IProcess,
                         reinterpret_cast<void**>(&eventProcess));
        Handle<IFile> eventElf = nameSpace->lookup("file/eventManager.elf");
        startProcess(nameSpace, eventProcess, eventElf);

        // start console process.
        Handle<IProcess> consoleProcess;
        esCreateInstance(CLSID_Process, IID_IProcess,
                         reinterpret_cast<void**>(&consoleProcess));
        Handle<IFile> consoleElf = nameSpace->lookup("file/console.elf");
        startProcess(nameSpace, consoleProcess, consoleElf);

        init(nameSpace);

        consoleProcess->kill();
        eventProcess->kill();

        fatFileSystem->getFreeSpace(freeSpace);
        fatFileSystem->getTotalSpace(totalSpace);
        esReport("Free space %lld, Total space %lld\n", freeSpace, totalSpace);
    }
    fatFileSystem->dismount();
    fatFileSystem = 0;

    esSleep(10000000);
}
