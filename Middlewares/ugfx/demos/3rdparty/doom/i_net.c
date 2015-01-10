// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#if 0
	static const char
	rcsid[] = "$Id: m_bbox.c,v 1.1 1997/02/03 22:45:10 b1 Exp $";

	#include <stdlib.h>
	#include <string.h>
	#include <stdio.h>

	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <errno.h>
	#include <unistd.h>
	#include <netdb.h>
	#include <sys/ioctl.h>
#else
	#include "gfx.h"
#endif

#include "i_system.h"
#include "d_event.h"
#include "d_net.h"
#include "m_argv.h"

#include "doomstat.h"

#if 0
	#ifdef __GNUG__
	#pragma implementation "i_net.h"
	#endif
#endif
#include "i_net.h"




#if 0
	// For some odd reason...
	#define ntohl(x) \
			((unsigned long int)((((unsigned long int)(x) & 0x000000ffU) << 24) | \
								 (((unsigned long int)(x) & 0x0000ff00U) <<  8) | \
								 (((unsigned long int)(x) & 0x00ff0000U) >>  8) | \
								 (((unsigned long int)(x) & 0xff000000U) >> 24)))

	#define ntohs(x) \
			((unsigned short int)((((unsigned short int)(x) & 0x00ff) << 8) | \
								  (((unsigned short int)(x) & 0xff00) >> 8))) \

	#define htonl(x) ntohl(x)
	#define htons(x) ntohs(x)
#endif

void	NetSend (void);
boolean NetListen (void);


//
// NETWORKING
//

#if 0
int	DOOMPORT =	(IPPORT_USERRESERVED +0x1d );
#endif

int			sendsocket;
int			insocket;

#if 0
	struct	sockaddr_in	sendaddress[MAXNETNODES];
#endif

void	(*netget) (void);
void	(*netsend) (void);


//
// UDPsocket
//
int UDPsocket (void)
{
#if 0
    int	s;
	
    // allocate a socket
    s = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s<0)
	I_Error ("can't create socket: %s",strerror(errno));
		
    return s;
#else
    return -1;
#endif
}

//
// BindToLocalPort
//
void
BindToLocalPort
( int	s,
  int	port )
{
#if 0
    int			v;
    struct sockaddr_in	address;
	
    memset (&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = port;
			
    v = bind (s, (void *)&address, sizeof(address));
    if (v == -1)
	I_Error ("BindToPort: bind: %s", strerror(errno));
#endif
}


//
// PacketSend
//
void PacketSend (void)
{
#if 0
    int		c;
    doomdata_t	sw;
				
    // byte swap
    sw.checksum = htonl(netbuffer->checksum);
    sw.player = netbuffer->player;
    sw.retransmitfrom = netbuffer->retransmitfrom;
    sw.starttic = netbuffer->starttic;
    sw.numtics = netbuffer->numtics;
    for (c=0 ; c< netbuffer->numtics ; c++)
    {
	sw.cmds[c].forwardmove = netbuffer->cmds[c].forwardmove;
	sw.cmds[c].sidemove = netbuffer->cmds[c].sidemove;
	sw.cmds[c].angleturn = htons(netbuffer->cmds[c].angleturn);
	sw.cmds[c].consistancy = htons(netbuffer->cmds[c].consistancy);
	sw.cmds[c].chatchar = netbuffer->cmds[c].chatchar;
	sw.cmds[c].buttons = netbuffer->cmds[c].buttons;
    }
		
    //I_DBGprintf("sending %i\n",gametic);
    c = sendto (sendsocket , &sw, doomcom->datalength
		,0,(void *)&sendaddress[doomcom->remotenode]
		,sizeof(sendaddress[doomcom->remotenode]));
	
    //	if (c == -1)
    //		I_Error ("SendPacket error: %s",strerror(errno));
#endif
}


//
// PacketGet
//
void PacketGet (void)
{
#if 0
    int			i;
    int			c;
    struct sockaddr_in	fromaddress;
    int			fromlen;
    doomdata_t		sw;
				
    fromlen = sizeof(fromaddress);
    c = recvfrom (insocket, &sw, sizeof(sw), 0
		  , (struct sockaddr *)&fromaddress, &fromlen );
    if (c == -1 )
    {
	if (errno != EWOULDBLOCK)
	    I_Error ("GetPacket: %s",strerror(errno));
	doomcom->remotenode = -1;		// no packet
	return;
    }

    {
	static int first=1;
	if (first)
	    I_printf("len=%d:p=[0x%x 0x%x] \n", c, *(int*)&sw, *((int*)&sw+1));
	first = 0;
    }

    // find remote node number
    for (i=0 ; i<doomcom->numnodes ; i++)
	if ( fromaddress.sin_addr.s_addr == sendaddress[i].sin_addr.s_addr )
	    break;

    if (i == doomcom->numnodes)
    {
	// packet is not from one of the players (new game broadcast)
	doomcom->remotenode = -1;		// no packet
	return;
    }
	
    doomcom->remotenode = i;			// good packet from a game player
    doomcom->datalength = c;
	
    // byte swap
    netbuffer->checksum = ntohl(sw.checksum);
    netbuffer->player = sw.player;
    netbuffer->retransmitfrom = sw.retransmitfrom;
    netbuffer->starttic = sw.starttic;
    netbuffer->numtics = sw.numtics;

    for (c=0 ; c< netbuffer->numtics ; c++)
    {
	netbuffer->cmds[c].forwardmove = sw.cmds[c].forwardmove;
	netbuffer->cmds[c].sidemove = sw.cmds[c].sidemove;
	netbuffer->cmds[c].angleturn = ntohs(sw.cmds[c].angleturn);
	netbuffer->cmds[c].consistancy = ntohs(sw.cmds[c].consistancy);
	netbuffer->cmds[c].chatchar = sw.cmds[c].chatchar;
	netbuffer->cmds[c].buttons = sw.cmds[c].buttons;
    }
#else
	doomcom->remotenode = -1;		// no packet
#endif
}



int GetLocalAddress (void)
{
#if 0
    char		hostname[1024];
    struct hostent*	hostentry;	// host information entry
    int			v;

    // get local address
    v = gethostname (hostname, sizeof(hostname));
    if (v == -1)
	I_Error ("GetLocalAddress : gethostname: errno %d",errno);
	
    hostentry = gethostbyname (hostname);
    if (!hostentry)
	I_Error ("GetLocalAddress : gethostbyname: couldn't get local host");
		
    return *(int *)hostentry->h_addr_list[0];
#else
    return 0;
#endif
}


//
// I_InitNetwork
//
void I_InitNetwork (void)
{
#if 0
    boolean		trueval = true;
    int			i;
    int			p;
    struct hostent*	hostentry;	// host information entry
	
    doomcom = I_malloc (sizeof (*doomcom) );
    memset (doomcom, 0, sizeof(*doomcom) );
    
    // set up for network
    i = M_CheckParm ("-dup");
    if (i && i< myargc-1)
    {
	doomcom->ticdup = myargv[i+1][0]-'0';
	if (doomcom->ticdup < 1)
	    doomcom->ticdup = 1;
	if (doomcom->ticdup > 9)
	    doomcom->ticdup = 9;
    }
    else
	doomcom-> ticdup = 1;
	
    if (M_CheckParm ("-extratic"))
	doomcom-> extratics = 1;
    else
	doomcom-> extratics = 0;
		
    p = M_CheckParm ("-port");
    if (p && p<myargc-1)
    {
	DOOMPORT = atoi (myargv[p+1]);
	I_printf ("using alternate port %i\n",DOOMPORT);
    }
    
    // parse network game options,
    //  -net <consoleplayer> <host> <host> ...
    i = M_CheckParm ("-net");
    if (!i)
    {
	// single player game
	netgame = false;
	doomcom->id = DOOMCOM_ID;
	doomcom->numplayers = doomcom->numnodes = 1;
	doomcom->deathmatch = false;
	doomcom->consoleplayer = 0;
	return;
    }

    netsend = PacketSend;
    netget = PacketGet;
    netgame = true;

    // parse player number and host list
    doomcom->consoleplayer = myargv[i+1][0]-'1';

    doomcom->numnodes = 1;	// this node for sure
	
    i++;
    while (++i < myargc && myargv[i][0] != '-')
    {
	sendaddress[doomcom->numnodes].sin_family = AF_INET;
	sendaddress[doomcom->numnodes].sin_port = htons(DOOMPORT);
	if (myargv[i][0] == '.')
	{
	    sendaddress[doomcom->numnodes].sin_addr.s_addr 
		= inet_addr (myargv[i]+1);
	}
	else
	{
	    hostentry = gethostbyname (myargv[i]);
	    if (!hostentry)
		I_Error ("gethostbyname: couldn't find %s", myargv[i]);
	    sendaddress[doomcom->numnodes].sin_addr.s_addr 
		= *(int *)hostentry->h_addr_list[0];
	}
	doomcom->numnodes++;
    }
	
    doomcom->id = DOOMCOM_ID;
    doomcom->numplayers = doomcom->numnodes;
    
    // build message to receive
    insocket = UDPsocket ();
    BindToLocalPort (insocket,htons(DOOMPORT));
    ioctl (insocket, FIONBIO, &trueval);

    sendsocket = UDPsocket ();
#else
    doomcom = gfxAlloc (sizeof (*doomcom) );
    memset (doomcom, 0, sizeof(*doomcom) );

	// single player game
	netgame = false;
	doomcom-> ticdup = 1;
	doomcom-> extratics = 0;
	doomcom->id = DOOMCOM_ID;
	doomcom->numplayers = doomcom->numnodes = 1;
	doomcom->deathmatch = false;
	doomcom->consoleplayer = 0;
#endif
}


void I_NetCmd (void)
{
    if (doomcom->command == CMD_SEND)
    {
	netsend ();
    }
    else if (doomcom->command == CMD_GET)
    {
	netget ();
    }
    else
	I_Error ("Bad net cmd: %i\n",doomcom->command);
}

