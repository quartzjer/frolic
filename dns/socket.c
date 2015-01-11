/* --------------------------------------------------------------------------
 *
 * License
 *
 * The contents of this file are subject to the Jabber Open Source License
 * Version 1.0 (the "License").  You may not copy or use this file, in either
 * source code or executable form, except in compliance with the License.  You
 * may obtain a copy of the License at http://www.jabber.com/license/ or at
 * http://www.opensource.org/.  
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Copyrights
 * 
 * Portions created by or assigned to Jabber.com, Inc. are 
 * Copyright (c) 1999-2000 Jabber.com, Inc.  All Rights Reserved.  Contact
 * information for Jabber.com, Inc. is available at http://www.jabber.com/.
 *
 * Portions Copyright (c) 1998-1999 Jeremie Miller.
 * 
 * Acknowledgements
 * 
 * Special thanks to the Jabber Open Source Contributors for their
 * suggestions and support of Jabber.
 * 
 * --------------------------------------------------------------------------*/

#include "lib.h"

/* socket.c
 *
 * Simple wrapper to make socket creation easy.
 * type = NETSOCKET_SERVER is local listening socket
 * type = NETSOCKET_CLIENT is connection socket
 * type = NETSOCKET_UDP is a UDP connection socket
 */

int make_netsocket(u_short port, char *host, int type)
{
    int s, flag = 1;
    struct sockaddr_in sa;
    struct in_addr *saddr;
    int socket_type;

    /* is this a UDP socket or a TCP socket? */
    socket_type = (type == NETSOCKET_UDP)?SOCK_DGRAM:SOCK_STREAM;

    bzero((void *)&sa,sizeof(struct sockaddr_in));

    if((s = socket(AF_INET,socket_type,0)) < 0)
        return(-1);
    if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag)) < 0)
        return(-1);

    saddr = make_addr(host);
    if(saddr == NULL && type != NETSOCKET_UDP)
        return(-1);
    sa.sin_family = AF_INET;

    if(type == NETSOCKET_SERVER)
    {
        /* bind to specific address if specified */
        if(host != NULL)
            sa.sin_addr.s_addr = saddr->s_addr;

        if(bind(s,(struct sockaddr*)&sa,sizeof sa) < 0)
        {
            close(s);
            return(-1);
        }
    }
    if(type == NETSOCKET_CLIENT)
    {
        sa.sin_addr.s_addr = saddr->s_addr;
        if(connect(s,(struct sockaddr*)&sa,sizeof sa) < 0)
        {
            close(s);
            return(-1);
        }
    }
    if(type == NETSOCKET_UDP)
    {
        /* bind to all addresses for now */
        if(bind(s,(struct sockaddr*)&sa,sizeof sa) < 0)
        {
            close(s);
            return(-1);
        }

        /* if specified, use a default recipient for read/write */
        if(host != NULL && saddr != NULL)
        {
            sa.sin_addr.s_addr = saddr->s_addr;
	    sa.sin_port = htons(port);
            if(connect(s,(struct sockaddr*)&sa,sizeof sa) < 0)
            {
                close(s);
                return(-1);
            }
        }
    }


    return(s);
}


struct in_addr *make_addr(char *host)
{
    struct hostent *hp;
    static struct in_addr addr;
    char myname[MAXHOSTNAMELEN + 1];

    if(host == NULL || strlen(host) == 0)
    {
        gethostname(myname,MAXHOSTNAMELEN);
        hp = gethostbyname(myname);
        if(hp != NULL)
        {
            return (struct in_addr *) *hp->h_addr_list;
        }
    }else{
        addr.s_addr = inet_addr(host);
        if(addr.s_addr != -1)
        {
            return &addr;
        }
        hp = gethostbyname(host);
        if(hp != NULL)
        {
            return (struct in_addr *) *hp->h_addr_list;
        }
    }
    return NULL;
}

/* Sets a file descriptor to close on exec.  "flag" is 1 to close on exec, 0 to
 * leave open across exec.
 * -- EJB 7/31/2000
 */
int set_fd_close_on_exec(int fd, int flag)
{
    int oldflags = fcntl(fd,F_GETFL);
    int newflags;

    if(flag)
        newflags = oldflags | FD_CLOEXEC;
    else
        newflags = oldflags & (~FD_CLOEXEC);

    if(newflags==oldflags)
        return 0;
    return fcntl(fd,F_SETFL,(long)newflags);
}

