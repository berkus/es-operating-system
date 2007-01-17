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

#include <new>
#include <es/handle.h>
#include "inet4.h"
#include "inet4address.h"

// StateNonMember

void Inet4Address::
StateNonMember::start(Inet4Address* a)
{
    ASSERT(a->inFamily);
    a->inFamily->joinGroup(a);
    a->setState(Inet4Address::stateDelayingMember);

    // Immediately send report
    int len = sizeof(IGMPHdr);
    int pos = 14 + 60;          // XXX Assume MAC, IPv4
    u8 chunk[pos + len];
    IGMPHdr* igmphdr = reinterpret_cast<IGMPHdr*>(chunk + pos);
    igmphdr->type = IGMPHdr::ReportVer2;    // XXX v1 case
    igmphdr->maxRespTime = 0;
    igmphdr->addr = a->getAddress();

    InetMessenger r(&InetReceiver::output, chunk, pos + len, pos);
    r.setRemote(a);
    Handle<Inet4Address> src = a->inFamily->selectSourceAddress(a);
    r.setLocal(src);

    Visitor v(&r);
    a->adapter->accept(&v);

    // Repeat once after a short delay.
    a->alarm(IGMPHdr::UnsolicitedReportInterval * 10000000LL);
}

bool Inet4Address::
StateNonMember::input(InetMessenger* m, Inet4Address* a)
{
    return true;
}

bool Inet4Address::
StateNonMember::output(InetMessenger* m, Inet4Address* a)
{
    return true;
}

bool Inet4Address::
StateNonMember::error(InetMessenger* m, Inet4Address* a)
{
}

// StateDelayingMember

void Inet4Address::
StateDelayingMember::stop(Inet4Address* a)
{
    // Send leave if flag set
    int len = sizeof(IGMPHdr);
    int pos = 14 + 60;          // XXX Assume MAC, IPv4
    u8 chunk[pos + len];
    IGMPHdr* igmphdr = reinterpret_cast<IGMPHdr*>(chunk + pos);
    igmphdr->type = IGMPHdr::Leave;
    igmphdr->maxRespTime = 0;
    igmphdr->addr = a->getAddress();

    InetMessenger r(&InetReceiver::output, chunk, pos + len, pos);
    r.setRemote(&a->inFamily->addressAllRouters);
    Handle<Inet4Address> src = a->inFamily->selectSourceAddress(a);
    r.setLocal(src);

    Visitor v(&r);
    a->adapter->accept(&v);

    ASSERT(a->inFamily);
    a->inFamily->leaveGroup(a);
    a->setState(Inet4Address::stateNonMember);
}

void Inet4Address::
StateDelayingMember::expired(Inet4Address* a)
{
    esReport("StateDelayingMember::expired()\n");

    // Send report
    int len = sizeof(IGMPHdr);
    int pos = 14 + 60;          // XXX Assume MAC, IPv4
    u8 chunk[pos + len];
    IGMPHdr* igmphdr = reinterpret_cast<IGMPHdr*>(chunk + pos);
    igmphdr->type = IGMPHdr::ReportVer2;    // XXX v1 case
    igmphdr->maxRespTime = 0;
    igmphdr->addr = a->getAddress();

    InetMessenger r(&InetReceiver::output, chunk, pos + len, pos);
    r.setRemote(a);
    Handle<Inet4Address> src = a->inFamily->selectSourceAddress(a);
    r.setLocal(src);

    Visitor v(&r);
    a->adapter->accept(&v);

    a->setState(Inet4Address::stateIdleMember);
}

bool Inet4Address::
StateDelayingMember::input(InetMessenger* m, Inet4Address* a)
{
    IGMPHdr* igmphdr = static_cast<IGMPHdr*>(m->fix(sizeof(IGMPHdr)));
    switch (igmphdr->type)
    {
    case IGMPHdr::ReportVer2:
        a->setState(Inet4Address::stateIdleMember);
        break;
    }
    return true;
}

bool Inet4Address::
StateDelayingMember::output(InetMessenger* m, Inet4Address* a)
{
    return true;
}

bool Inet4Address::
StateDelayingMember::error(InetMessenger* m, Inet4Address* a)
{
}

// StateIdleMember

void Inet4Address::
StateIdleMember::stop(Inet4Address* a)
{
    // Send leave if flag set
    int len = sizeof(IGMPHdr);
    int pos = 14 + 60;          // XXX Assume MAC, IPv4
    u8 chunk[pos + len];
    IGMPHdr* igmphdr = reinterpret_cast<IGMPHdr*>(chunk + pos);
    igmphdr->type = IGMPHdr::Leave;
    igmphdr->maxRespTime = 0;
    igmphdr->addr = a->getAddress();

    InetMessenger r(&InetReceiver::output, chunk, pos + len, pos);
    r.setRemote(&a->inFamily->addressAllRouters);
    Handle<Inet4Address> src = a->inFamily->selectSourceAddress(a);
    r.setLocal(src);

    Visitor v(&r);
    a->adapter->accept(&v);

    ASSERT(a->inFamily);
    a->inFamily->leaveGroup(a);
    a->setState(Inet4Address::stateNonMember);
}

bool Inet4Address::
StateIdleMember::input(InetMessenger* m, Inet4Address* a)
{
    IGMPHdr* igmphdr = static_cast<IGMPHdr*>(m->fix(sizeof(IGMPHdr)));
    switch (igmphdr->type)
    {
    case IGMPHdr::Query:
        u8 maxRespTime = igmphdr->maxRespTime;
        if (maxRespTime == 0)
        {
            maxRespTime = IGMPHdr::QueryResponseInterval;
        }
        a->setState(Inet4Address::stateDelayingMember);
        a->alarm(maxRespTime * 1000000LL);
        break;
    }
    return true;
}

bool Inet4Address::
StateIdleMember::output(InetMessenger* m, Inet4Address* a)
{
    return true;
}

bool Inet4Address::
StateIdleMember::error(InetMessenger* m, Inet4Address* a)
{
}

s16 IGMPReceiver::
checksum(InetMessenger* m)
{
    int len = m->getLength();
    IGMPHdr* igmphdr = static_cast<IGMPHdr*>(m->fix(len));
    s32 sum = m->sumUp(len);
    while (sum >> 16)
    {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    return ~sum;
}

bool IGMPReceiver::
input(InetMessenger* m)
{
    ASSERT(m);

    IGMPHdr* igmphdr = static_cast<IGMPHdr*>(m->fix(sizeof(IGMPHdr)));
    if (!igmphdr || checksum(m) != 0)
    {
        return false;
    }

    if (!IN_IS_ADDR_MULTICAST(igmphdr->addr) &&
        !IN_IS_ADDR_UNSPECIFIED(igmphdr->addr))
    {
        return false;
    }

    Handle<Inet4Address> address;
    address = inFamily->getAddress(igmphdr->addr, m->getScopeID());
    if (!address)
    {
        return false;
    }
    m->setLocal(address);

    return true;
}

bool IGMPReceiver::
output(InetMessenger* m)
{
    IGMPHdr* igmphdr = static_cast<IGMPHdr*>(m->fix(sizeof(IGMPHdr)));
    igmphdr->sum = 0;
    igmphdr->sum = checksum(m);
    m->setType(IPPROTO_IGMP);
    return true;
}
