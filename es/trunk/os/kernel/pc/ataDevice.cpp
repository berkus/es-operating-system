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

#include <string.h>
#include <es.h>
#include <es/clsid.h>
#include <es/handle.h>
#include <es/device/IPartition.h>
#include "io.h"
#include "ataController.h"

#define VERBOSE

using namespace ATAttachment;
using namespace Register;

AtaDevice::
AtaDevice(AtaController* ctlr, u8 device, u8* signature) :
    ctlr(ctlr),
    device(device),
    size(0),
    sectorSize(512),
    multiple(0),
    packetSize(0),
    dma(0),
    removal(false),
    partition(0)
{
    using namespace Command;
    using namespace DeviceIdentification;
    using namespace Status;

    esCreateInstance(CLSID_Monitor,
                     IID_IMonitor,
                     reinterpret_cast<void**>(&monitor));

    if (!identify(signature))
    {
        return;
    }

    // Select DMA mode
    if (ctlr->dma && (id[CAPABILITIES] & 0x100))
    {
        dma = id[MULTIWORD_DMA_TRANSFER] & 0x0707;
        dma = (dma >> 8) & dma;
        if (dma == 0 && (id[FIELD_VALIDITY] & 0x04))
        {
            dma = id[ULTRA_DMA_MODES] & 0x7f7f;
            dma = (dma >> 8) & dma;
            dma |= 0x80;
        }
    }

#ifdef VERBOSE
    esReport("CHS=%d/%d/%d\n", id[1], id[3], id[6]);
#endif

    if (AtaController::isAtapiDevice(signature))
    {
        return;
    }

    long long sectors;
    if (id[FEATURES_COMMAND_SETS_SUPPORTED + 1] & 0x0400)
    {
        sectors = id[MAXIMUM_USER_LBA_FOR_48_BIT_ADDRESS_FEATURE_SET] |
                  (id[MAXIMUM_USER_LBA_FOR_48_BIT_ADDRESS_FEATURE_SET + 1] << 16) |
                  ((long long) id[MAXIMUM_USER_LBA_FOR_48_BIT_ADDRESS_FEATURE_SET + 2] << 32);
    }
    else
    {
        sectors = id[TOTAL_NUMBER_OF_USER_ADDRESSABLE_SECTORS] |
                  (id[TOTAL_NUMBER_OF_USER_ADDRESSABLE_SECTORS + 1] << 16);
    }
    size = sectors * sectorSize;

    // Select read/write multiple
    u16 maxrwm = (id[READ_WRITE_MULTIPLE_SUPPORT] & 0xff);
    if (0 < maxrwm)
    {
        if (id[MULTIPLE_SECTOR_SETTING] & 0x100)
        {
            multiple = (id[MULTIPLE_SECTOR_SETTING] & 0xff);
        }
        if (multiple == 0)
        {
            multiple = maxrwm;
        }
        if (16 < multiple)
        {
            multiple = 16;
        }
        ctlr->select(device);
        outpb(ctlr->cmdPort + SECTOR_COUNT, multiple);
        outpb(ctlr->cmdPort + COMMAND, Command::SET_MULTIPLE_MODE);
        ctlr->sync(DF | ERR);
        u8 state = inpb(ctlr->cmdPort + STATUS);
        if (state & (DF | ERR))
        {
            multiple = 0;
        }
    }

    if (id[FEATURES_COMMAND_SETS_SUPPORTED + 1] & 0x0400)
    {
        if (dma)
        {
            readCmd = READ_DMA_EXT;
            writeCmd = WRITE_DMA_EXT;
        }
        else if (multiple)
        {
            readCmd = READ_MULTIPLE_EXT;
            writeCmd = WRITE_MULTIPLE_EXT;
        }
        else
        {
            readCmd = READ_SECTOR_EXT;
            writeCmd = WRITE_SECTOR_EXT;
        }
    }
    else
    {
        if (dma)
        {
            readCmd = READ_DMA;
            writeCmd = WRITE_DMA;
        }
        else if (multiple)
        {
            readCmd = READ_MULTIPLE;
            writeCmd = WRITE_MULTIPLE;
        }
        else
        {
            readCmd = READ_SECTOR;
            writeCmd = WRITE_SECTOR;
        }
    }
}

AtaDevice::
~AtaDevice()
{
    if (partition)
    {
        Handle<IPartition> handle(partition);
        handle->unmount();
        partition->release();
    }
    monitor->release();
}

bool AtaDevice::
identify(u8* signature)
{
    using namespace Status;

    ctlr->select(device);
    if (AtaController::isAtapiDevice(signature))
    {
        outpb(ctlr->cmdPort + COMMAND, Command::IDENTIFY_PACKET_DEVICE);
    }
    else
    {
        outpb(ctlr->cmdPort + COMMAND, Command::IDENTIFY_DEVICE);
    }
    esSleep(4);
    ctlr->sync(DRQ | ERR);
    u8 state = inpb(ctlr->cmdPort + STATUS);
    if (state & ERR)
    {
        return false;
    }
    memset(id, 0, sectorSize);
    inpsw(ctlr->cmdPort + DATA, id, 256);
    inpb(ctlr->cmdPort + STATUS);
    return true;
}

long long AtaDevice::
getPosition()
{
    return 0;
}

void AtaDevice::
setPosition(long long pos)
{
}

long long AtaDevice::
getSize()
{
    return this->size;
}

void AtaDevice::
setSize(long long size)
{
}

int AtaDevice::
read(void* dst, int count)
{
    return 0;
}

int AtaDevice::
read(void* dst, int count, long long offset)
{
    // esReport("AtaDevice::%s(%p, %d, %lld)\n", __func__, dst, count, offset);
    count = ctlr->issue(this, readCmd, dst, count / sectorSize, offset / sectorSize);
    return (count <= 0) ? count : (count * sectorSize);
}

int AtaDevice::
write(const void* src, int count)
{
    return 0;
}

int AtaDevice::
write(const void* src, int count, long long offset)
{
    // esReport("AtaDevice::%s(%p, %d, %lld)\n", __func__, src, count, offset);
    count = ctlr->issue(this, writeCmd, const_cast<void*>(src), count / sectorSize, offset / sectorSize);
    return (count <= 0) ? count : (count * sectorSize);
}

void AtaDevice::
flush()
{
}

int AtaDevice::initialize()
{
    return 0;
}

int AtaDevice::
getGeometry(Geometry* geometry)
{
    geometry->cylinders = id[1];        // 0..1023
    geometry->heads = id[3];            // 0..255
    geometry->sectorsPerTrack = id[6];  // 1..63
    geometry->bytesPerSector = sectorSize;
    geometry->diskSize = size;
    return 0;
}

int AtaDevice::
getLayout(Partition* partition)
{
    return -1;
}

int AtaDevice::
setLayout(Partition* partition)
{
    return -1;
}

IContext* AtaDevice::
getPartition()
{
    if (!partition)
    {
        Synchronized<IMonitor*> method(monitor);

        esCreateInstance(CLSID_Partition,
                         IID_IContext,
                         reinterpret_cast<void**>(&partition));
        if (partition)
        {
            Handle<IPartition> handle(partition);
            if (handle)
            {
                handle->mount(this);
            }
        }
    }
    return partition;
}

IBinding* AtaDevice::
bind(const char* name, IInterface* object)
{
    if (!getPartition())
    {
        return 0;
    }
    return partition->bind(name, object);
}

IContext* AtaDevice::
createSubcontext(const char* name)
{
    if (!getPartition())
    {
        return 0;
    }
    return partition->createSubcontext(name);
}

int AtaDevice::
destroySubcontext(const char* name)
{
    if (!getPartition())
    {
        return 0;
    }
    return partition->destroySubcontext(name);
}

IInterface* AtaDevice::
lookup(const char* name)
{
    if (!getPartition())
    {
        return 0;
    }
    return partition->lookup(name);
}

int AtaDevice::
rename(const char* oldName, const char* newName)
{
    if (!getPartition())
    {
        return 0;
    }
    return partition->rename(oldName, newName);
}

int AtaDevice::
unbind(const char* name)
{
    if (!getPartition())
    {
        return 0;
    }
    return partition->unbind(name);
}

IIterator* AtaDevice::
list(const char* name)
{
    if (!getPartition())
    {
        return 0;
    }
    return partition->list(name);
}

bool AtaDevice::
queryInterface(const Guid& riid, void** objectPtr)
{
    if (riid == IID_IStream)
    {
        *objectPtr = static_cast<IStream*>(this);
    }
    else if (riid == IID_IDiskManagement)
    {
        *objectPtr = static_cast<IDiskManagement*>(this);
    }
    else if (riid == IID_IContext && getPartition())
    {
        *objectPtr = static_cast<IContext*>(this);
    }
    else if (riid == IID_IInterface)
    {
        *objectPtr = static_cast<IStream*>(this);
    }
    else
    {
        *objectPtr = NULL;
        return false;
    }
    static_cast<IInterface*>(*objectPtr)->addRef();
    return true;
}

unsigned int AtaDevice::
addRef()
{
    return ref.addRef();
}

unsigned int AtaDevice::
release()
{
    unsigned int count = ref.release();
    if (count == 0)
    {
        delete this;
        return 0;
    }
    return count;
}

bool AtaDevice::
detect()
{
    return true;
}
