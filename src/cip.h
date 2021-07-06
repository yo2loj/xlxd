//
//  cip.h
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

#ifndef cip_h
#define cip_h

#include <arpa/inet.h>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////////////////////////
// class

class CIp
{
public:
    // constructors
    CIp();

    CIp(const struct sockaddr_storage *);
#ifdef IPV6_SUPPORT
    CIp(const char *, int = AF_UNSPEC, int = 0);
#else
    CIp(const char *, int = AF_INET, int = 0);
#endif
    CIp(const CIp &);
    
    // destructor
    virtual ~CIp() {};
    
    // sockaddr
    void SetSockAddr(struct sockaddr_in *);

#ifdef IPV6_SUPPORT
    void SetSockAddr(struct sockaddr_storage *);
    struct sockaddr_storage *GetSockAddr(void) const { return (sockaddr_storage *)&m_Addr; }
#else
    struct sockaddr_in *GetSockAddr(void) const      { return (sockaddr_in *)&m_Addr; }
#endif
    
    // operator
    bool operator ==(const CIp &) const;
    operator const char *() const;
#ifdef IPV6_SUPPORT
    operator const struct in6_addr () const          { return ((struct sockaddr_in6 *)&m_Addr)->sin6_addr;  }
    const int GetPort(void) const                    { return ntohs((m_Addr.ss_family == AF_INET)?(((struct sockaddr_in *)&m_Addr)->sin_port):(((struct sockaddr_in6 *)&m_Addr)->sin6_port));  }
    const int GetAf(void) const                      { return m_Addr.ss_family; }
    void SetPort(int p)                              { (m_Addr.ss_family == AF_INET)?(((struct sockaddr_in *)&m_Addr)->sin_port = htons(p)):(((struct sockaddr_in6 *)&m_Addr)->sin6_port = htons(p)); }
#else
    operator const struct in_addr () const           { return ((struct sockaddr_in *)&m_Addr)->sin_addr;  }
    const int GetPort(void) const                    { return ntohs(((struct sockaddr_in *)&m_Addr)->sin_port);  }
    const int GetAf(void) const                      { return AF_INET; }
    void SetPort(int p)                              { ((struct sockaddr_in *)&m_Addr)->sin_port = htons(p); }
#endif
    friend std::ostream& operator <<(std::ostream& stream, const CIp& Ip);

    bool HasSameAddr(const CIp &);

    // helper
    unsigned long int StreamId(void) const;

protected:
    // data
    struct sockaddr_storage  m_Addr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ostream operator

std::ostream& operator <<(std::ostream& stream, const CIp& Ip);

#endif // cip_h

