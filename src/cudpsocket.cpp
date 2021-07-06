//
//  cudpsocket.cpp
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
#include <string.h>
#include "creflector.h"
#include "cudpsocket.h"
#include <iostream>


////////////////////////////////////////////////////////////////////////////////////////
// constructor

CUdpSocket::CUdpSocket()
{
    m_Socket = -1;
}

////////////////////////////////////////////////////////////////////////////////////////
// destructor

CUdpSocket::~CUdpSocket()
{
    if ( m_Socket != -1 )
    {
        Close();
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// open & close

bool CUdpSocket::Open(uint16 uiPort)
{
    bool open = false;
    
    // create socket
#ifdef IPV6_SUPPORT
    m_Socket = socket(PF_INET6,SOCK_DGRAM,0);
#else
    m_Socket = socket(PF_INET,SOCK_DGRAM,0);
#endif
    if ( m_Socket != -1 )
    {
        // initialize sockaddr struct
#ifdef IPV6_SUPPORT
        ::memset(&m_SocketAddr, 0, sizeof(struct sockaddr_storage));
        CIp lip = g_Reflector.GetListenIp();
        lip.SetPort(uiPort);
        ::memcpy(&m_SocketAddr, lip.GetSockAddr(), sizeof(struct sockaddr_storage));
        if ( bind(m_Socket, (struct sockaddr *)&m_SocketAddr, sizeof(struct sockaddr_storage)) == 0 )
#else
        ::memset(&m_SocketAddr, 0, sizeof(struct sockaddr_in));
        m_SocketAddr.sin_family = AF_INET;
        m_SocketAddr.sin_port = htons(uiPort);
        m_SocketAddr.sin_addr.s_addr = inet_addr(g_Reflector.GetListenIp());
        if ( bind(m_Socket, (struct sockaddr *)&m_SocketAddr, sizeof(struct sockaddr_in)) == 0 )
#endif
        {
            fcntl(m_Socket, F_SETFL, O_NONBLOCK);
            open = true;
        }
        else
        {
            close(m_Socket);
            m_Socket = -1;
        }
    }
    
    // done
    return open;
}

void CUdpSocket::Close(void)
{
    if ( m_Socket != -1 )
    {
        close(m_Socket);
        m_Socket = -1;
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// read

int CUdpSocket::Receive(CBuffer *Buffer, CIp *Ip, int timeout)
{
#ifdef IPV6_SUPPORT
    struct sockaddr_storage Sin;
    unsigned int uiFromLen = sizeof(struct sockaddr_storage);
#else
    struct sockaddr_in Sin;
    unsigned int uiFromLen = sizeof(struct sockaddr_in);
#endif
    fd_set FdSet;
    int iRecvLen = -1;
    struct timeval tv;

    // socket valid ?
    if ( m_Socket != -1 )
    {
        // control socket
        FD_ZERO(&FdSet);
        FD_SET(m_Socket, &FdSet);
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        select(m_Socket + 1, &FdSet, 0, 0, &tv);

        // allocate buffer
        Buffer->resize(UDP_BUFFER_LENMAX);

        // read
        iRecvLen = (int)recvfrom(m_Socket,
            (void *)Buffer->data(), UDP_BUFFER_LENMAX,
            0, (struct sockaddr *)&Sin, &uiFromLen);

        // handle
        if ( iRecvLen != -1 )
        {
            // adjust buffer size
            Buffer->resize(iRecvLen);

            // get IP
#ifdef IPV6_SUPPORT
            Ip->SetSockAddr((struct sockaddr_storage *)&Sin);
#else
            Ip->SetSockAddr(&Sin);
#endif
        }
    }

    // done
    return iRecvLen;
}

////////////////////////////////////////////////////////////////////////////////////////
// write

int CUdpSocket::Send(const CBuffer &Buffer, const CIp &Ip)
{
    CIp temp(Ip);
#ifdef IPV6_SUPPORT
    return (int)::sendto(m_Socket,
           (void *)Buffer.data(), Buffer.size(),
           0, (struct sockaddr *)temp.GetSockAddr(), sizeof(struct sockaddr_storage));
#else
    return (int)::sendto(m_Socket,
           (void *)Buffer.data(), Buffer.size(),
           0, (struct sockaddr *)temp.GetSockAddr(), sizeof(struct sockaddr_in));
#endif
}

int CUdpSocket::Send(const char *Buffer, const CIp &Ip)
{
    CIp temp(Ip);
#ifdef IPV6_SUPPORT
    return (int)::sendto(m_Socket,
           (void *)Buffer, ::strlen(Buffer),
           0, (struct sockaddr *)temp.GetSockAddr(), sizeof(struct sockaddr_storage));
#else
    return (int)::sendto(m_Socket,
           (void *)Buffer, ::strlen(Buffer),
           0, (struct sockaddr *)temp.GetSockAddr(), sizeof(struct sockaddr_in));
#endif
}

int CUdpSocket::Send(const CBuffer &Buffer, const CIp &Ip, uint16 destport)
{
    CIp temp(Ip);
    temp.SetPort(destport);
#ifdef IPV6_SUPPORT
    return (int)::sendto(m_Socket,
                         (void *)Buffer.data(), Buffer.size(),
                         0, (struct sockaddr *)temp.GetSockAddr(), sizeof(struct sockaddr_storage));
#else
    return (int)::sendto(m_Socket,
                         (void *)Buffer.data(), Buffer.size(),
                         0, (struct sockaddr *)temp.GetSockAddr(), sizeof(struct sockaddr_in));
#endif
}

int CUdpSocket::Send(const char *Buffer, const CIp &Ip, uint16 destport)
{
    CIp temp(Ip);
    temp.SetPort(destport);
#ifdef IPV6_SUPPORT
    return (int)::sendto(m_Socket,
                         (void *)Buffer, ::strlen(Buffer),
                         0, (struct sockaddr *)temp.GetSockAddr(), sizeof(struct sockaddr_storage));
#else
    return (int)::sendto(m_Socket,
                         (void *)Buffer, ::strlen(Buffer),
                         0, (struct sockaddr *)temp.GetSockAddr(), sizeof(struct sockaddr_in));
#endif
}


