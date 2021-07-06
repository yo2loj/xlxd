//
//  cip.cpp
//  xlxd
//
//  Created by Jean-Luc Deltombe (LX3JL) on 31/10/2015.
//  Copyright © 2015 Jean-Luc Deltombe (LX3JL). All rights reserved.
//
// ----------------------------------------------------------------------------
//    This file is part of xlxd.
//
//    xlxd is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    xlxd is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Foobar.  If not, see <http://www.gnu.org/licenses/>. 
// ----------------------------------------------------------------------------

#include "main.h"

#include "cip.h"
#include <netdb.h>
#include <cstring>

#define CIP_CONVERT_IPV4_IN_IPV6

////////////////////////////////////////////////////////////////////////////////////////
// constructors

CIp::CIp()
{
    ::memset(&m_Addr, 0, sizeof(m_Addr));
}

CIp::CIp(const char *sz, int family, int port)
{
    ::memset(&m_Addr, 0, sizeof(m_Addr));

    struct addrinfo hints;
    struct addrinfo *result;

    bzero(&hints, sizeof(hints));
#ifdef IPV6_SUPPORT
    hints.ai_family = family;
#else
    hints.ai_family = AF_INET;
#endif

    if (0 == getaddrinfo(sz, NULL, &hints, &result))
    {
        ::memcpy(&m_Addr, result->ai_addr, result->ai_addrlen);
        freeaddrinfo(result);
    }

#ifdef IPV6_SUPPORT
    if (m_Addr.ss_family == AF_INET)
    {
        struct sockaddr_in6 sin6;
        bzero(&sin6, sizeof(sin6));
        uint32_t *a = (uint32_t *)&(((struct sockaddr_in6 *)&sin6)->sin6_addr);
        sin6.sin6_family = AF_INET6;
        a[0] = 0; a[1] = 0; a[2] = htonl(0xFFFF);
        a[3] = ((struct sockaddr_in *)&m_Addr)->sin_addr.s_addr;
        sin6.sin6_port = ((struct sockaddr_in *)&m_Addr)->sin_port;
        ::memcpy(&m_Addr, &sin6, sizeof(sin6));
    }
#endif

    if (port)
    {
        SetPort(port);
    }
}

CIp::CIp(const struct sockaddr_storage *ss)
{
    ::memcpy(&m_Addr, ss, sizeof(m_Addr));
}


CIp::CIp(const CIp &ip)
{
    ::memcpy(&m_Addr, &ip.m_Addr, sizeof(m_Addr));
}

////////////////////////////////////////////////////////////////////////////////////////
// set

#ifdef IPV6_SUPPORT
void CIp::SetSockAddr(struct sockaddr_storage *ss)
{
    ::memcpy(&m_Addr, ss, sizeof(m_Addr));
}
#endif

void CIp::SetSockAddr(struct sockaddr_in *ss)
{
#ifdef IPV6_SUPPORT
    struct sockaddr_in6 sin6;
    bzero(&sin6, sizeof(sin6));
    uint32_t *a = (uint32_t *)&(((struct sockaddr_in6 *)&sin6)->sin6_addr);
    sin6.sin6_family = AF_INET6;
    a[0] = 0; a[1] = 0; a[2] = htonl(0xFFFF);
    a[3] = ss->sin_addr.s_addr;
    sin6.sin6_port = ss->sin_port;
    ::memcpy(&m_Addr, &sin6, sizeof(sin6));
#else
    ::memcpy(&m_Addr, ss, sizeof(m_Addr));
#endif
}

////////////////////////////////////////////////////////////////////////////////////////
// operator

bool CIp::operator ==(const CIp &ip) const
{
    if (m_Addr.ss_family == ip.m_Addr.ss_family)
    {
#ifdef IPV6_SUPPORT
        if (m_Addr.ss_family == AF_INET)
        {
#endif
            struct sockaddr_in *s = (struct sockaddr_in *)&m_Addr;
            struct sockaddr_in *p = (struct sockaddr_in *)&ip.m_Addr;

            return ((s->sin_addr.s_addr == p->sin_addr.s_addr) &&
                 (s->sin_port == p->sin_port)) ;
#ifdef IPV6_SUPPORT
        }
        else
        {
            struct sockaddr_in6 *s = (struct sockaddr_in6 *)&m_Addr;
            struct sockaddr_in6 *p = (struct sockaddr_in6 *)&ip.m_Addr;

            return ((memcmp(&(s->sin6_addr), &(p->sin6_addr), sizeof(p->sin6_addr)) == 0) &&
                 (s->sin6_port == p->sin6_port)) ;
        }
#endif
    }

    return false;
}

CIp::operator const char *() const
{
#ifdef IPV6_SUPPORT
    static char sz[INET6_ADDRSTRLEN + 8];

    bzero(sz, sizeof(sz));
    if (m_Addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&m_Addr;
        inet_ntop(AF_INET6, &(s->sin6_addr), sz, sizeof(sz));
    }
    else
    {
        struct sockaddr_in *s = (struct sockaddr_in *)&m_Addr;
        inet_ntop(AF_INET, &(s->sin_addr), sz, sizeof(sz));
    }
#else
    static char sz[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(((struct sockaddr_in *)&m_Addr)->sin_addr), sz, sizeof(sz));
#endif
    return sz;
}

std::ostream& operator <<(std::ostream& stream, const CIp& Ip)
{
    if (Ip.GetAf() == AF_INET6)
    {
        const struct sockaddr_storage *ss = &Ip.m_Addr;
        uint32_t *a = (uint32_t *)&(((struct sockaddr_in6 *)ss)->sin6_addr);

#ifdef CIP_CONVERT_IPV4_IN_IPV6
        // show IPv4 in IPv6 as a simple IPv4 IP
        if (a[0] == 0 && a[1] == 0 && ntohl(a[2]) == 0xFFFF)
        {
            stream << inet_ntoa(*((in_addr *)&a[3]));
        }
        // rest of IPv6
        else
#endif
        {
            const char *sz = Ip;
            stream << "[";
            stream << sz;
            stream << "]";
        }
    }
    else
    {
        const char *sz = Ip;
        stream << sz;
    }

    //if (Ip.GetPort()) stream << ":" << Ip.GetPort();
    return stream;
}

bool CIp::HasSameAddr(const CIp &ip)
{
#if IPV6_SUPPORT
    uint32_t addr[4] = {0, 0, 0, 0};

    if (m_Addr.ss_family == AF_INET)
    {
        if (ip.GetAf() == AF_INET)
        {
            return (((struct sockaddr_in *)&m_Addr)->sin_addr.s_addr == ((struct sockaddr_in *)ip.GetSockAddr())->sin_addr.s_addr);
        }
        else
        {
            addr[0] = 0; addr[1] = 0; addr[2] = htonl(0xFFFF);
            addr[3] = ((struct sockaddr_in *)&m_Addr)->sin_addr.s_addr;
            return (::memcmp(&addr, &((struct sockaddr_in6 *)ip.GetSockAddr())->sin6_addr, sizeof(addr)) == 0);
        }
    }
    else 
    {
        if (ip.GetAf() == AF_INET)
        {
            addr[0] = 0; addr[1] = 0; addr[2] = htonl(0xFFFF);
            addr[3] = ((struct sockaddr_in *)ip.GetSockAddr())->sin_addr.s_addr;
            return (::memcmp(&addr, &((struct sockaddr_in6 *)&m_Addr)->sin6_addr, sizeof(addr)) == 0);
        }
        else
        {
            return (::memcmp(&((struct sockaddr_in6 *)ip.GetSockAddr())->sin6_addr, &((struct sockaddr_in6 *)&m_Addr)->sin6_addr, sizeof(addr)) == 0);
        }
    }
#else
    return (((struct sockaddr_in *)&m_Addr)->sin_addr.s_addr == ((struct sockaddr_in *)ip.GetSockAddr())->sin_addr.s_addr);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////
// helper
unsigned long int CIp::StreamId(void) const
{
    uint32_t stream_id;
#ifdef IPV6_SUPPORT
    if (m_Addr.ss_family == AF_INET)
    {
#endif
        stream_id = ((struct sockaddr_in *)&m_Addr)->sin_addr.s_addr;
        stream_id ^= (((uint32_t)((struct sockaddr_in *)&m_Addr)->sin_port) << 16) | (uint32_t)((struct sockaddr_in *)&m_Addr)->sin_port;
#ifdef IPV6_SUPPORT
    }
    else
    {
        stream_id  = *(uint32_t *)(&((struct sockaddr_in6 *)&m_Addr)->sin6_addr.s6_addr[0]);
        stream_id ^= *(uint32_t *)(&((struct sockaddr_in6 *)&m_Addr)->sin6_addr.s6_addr[4]);
        stream_id ^= *(uint32_t *)(&((struct sockaddr_in6 *)&m_Addr)->sin6_addr.s6_addr[8]);
        stream_id ^= *(uint32_t *)(&((struct sockaddr_in6 *)&m_Addr)->sin6_addr.s6_addr[12]);
        stream_id ^= (((uint32_t)((struct sockaddr_in6 *)&m_Addr)->sin6_port) << 16) | (uint32_t)((struct sockaddr_in6 *)&m_Addr)->sin6_port;
    }
#endif
    return stream_id;
}