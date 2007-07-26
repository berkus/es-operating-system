/*
 * Copyright (c) 2006
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
#include <es/formatter.h>
#include "iso9660Stream.h"

Iso9660Stream::
Iso9660Stream(Iso9660FileSystem* fileSystem, Iso9660Stream* parent, u32 offset, u8* record) :
    fileSystem(fileSystem), ref(1), cache(0),
    parent(parent), offset(offset)
{
    if (parent)
    {
        parent->addRef();
        dirLocation = parent->location;
    }
    else
    {
        dirLocation = 0;
    }

    location = LittleEndian::dword(record + DR_Location) + LittleEndian::byte(record + DR_AttributeRecordLength);
    location *= fileSystem->bytsPerSec;
    size = LittleEndian::dword(record + DR_DataLength);
    flags = LittleEndian::byte(record + DR_FileFlags);
    dateTime = fileSystem->getTime(record + DR_RecordingDateAndTime);

    // XXX interleave

    cache = fileSystem->cacheFactory->create(this);
    fileSystem->add(this);
}

Iso9660Stream::
~Iso9660Stream()
{
    if (cache)
    {
        delete cache;
    }
    if (parent)
    {
        parent->release();
        dirLocation = parent->location;
    }
    else
    {
        dirLocation = 0;
    }
    fileSystem->remove(this);
}

bool Iso9660Stream::
isRoot()
{
    return parent ? false : true;
}

int Iso9660Stream::
hashCode() const
{
    return Iso9660FileSystem::hashCode(dirLocation, offset);
}

long long Iso9660Stream::
getPosition()
{
    return 0;
}

void Iso9660Stream::
setPosition(long long pos)
{
}

long long Iso9660Stream::
getSize()
{
    return this->size;
}

void Iso9660Stream::
setSize(long long newSize)
{
}

int Iso9660Stream::
read(void* dst, int count)
{
    return -1;
}

int Iso9660Stream::
read(void* dst, int count, long long offset)
{
    if (size <= offset || count <= 0)
    {
        return 0;
    }
    if (size - offset < count)
    {
        count = size - offset;
    }

    int len;
    int n;
    for (len = 0; len < count; len += n, offset += n)
    {
        n = fileSystem->disk->read((u8*) dst + len,
                                   ((count - len) + fileSystem->bytsPerSec - 1) & ~(fileSystem->bytsPerSec - 1),
                                   location + offset + len);
        if (n <= 0)
        {
            break;
        }
    }

    return len;
}

int Iso9660Stream::
write(const void* src, int count)
{
    return -1;
}

int Iso9660Stream::
write(const void* src, int count, long long offset)
{
    return -1;
}

void Iso9660Stream::
flush()
{
}

bool Iso9660Stream::
findNext(IStream* dir, u8* record)
{
    ASSERT(isDirectory());
    while (0 < dir->read(record, 1))
    {
        if (record[DR_Length] <= DR_FileIdentifier)
        {
            break;
        }
        if (dir->read(record + 1, record[DR_Length] - 1) < record[DR_Length] - 1)
        {
            break;
        }
        if (record[DR_FileIdentifierLength] == 0)
        {
            break;
        }
        if (record[DR_FileIdentifierLength] != 1 ||
            record[DR_FileIdentifier] != 0 && record[DR_FileIdentifier] != 1)
        {
            return true;
        }
    }
    return false;
}

bool Iso9660Stream::
queryInterface(const Guid& riid, void** objectPtr)
{
    if (isDirectory() && riid == IID_IContext)
    {
        *objectPtr = static_cast<IContext*>(this);
    }
    else if (riid == IID_IFile)
    {
        *objectPtr = static_cast<IFile*>(this);
    }
    else if (riid == IID_IBinding)
    {
        *objectPtr = static_cast<IBinding*>(this);
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

unsigned int Iso9660Stream::
addRef(void)
{
    return ref.addRef();
}

unsigned int Iso9660Stream::
release(void)
{
    unsigned int count = ref.release();
    if (count == 0)
    {
        delete this;
        return 0;
    }
    return count;
}
