/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  (c) 1995-98 Artsoft Entertainment                       *
*              Holger Schemel                              *
*              Oststrasse 11a                              *
*              33604 Bielefeld                             *
*              phone: ++49 +521 290471                     *
*              email: aeglos@valinor.owl.de                *
*----------------------------------------------------------*
*  netserv.h                                               *
***********************************************************/

#ifndef NETSERV_H
#define NETSERV_H

#define DEFAULTPORT		19504

#define PROT_VERS_1		1
#define PROT_VERS_2		2
#define PROT_VERS_3		0

#define OP_PROTOCOL_VERSION	1
#define OP_BAD_PROTOCOL_VERSION	2
#define OP_PLAYER_CONNECTED	3
#define OP_PLAYER_DISCONNECTED	4
#define OP_YOUR_NUMBER		5
#define OP_NUMBER_WANTED	6
#define OP_NICKNAME		7
#define OP_START_PLAYING	8
#define OP_MOVE_FIGURE		9

#define OP_LOST			10
#define OP_GONE			11
#define OP_CLEAR		12
#define OP_LINES		13
#define OP_GROW			14
#define OP_MODE			15
#define OP_LEVEL		16
#define OP_BOT			17
#define OP_KILL			18
#define OP_PAUSE		19
#define OP_CONT			20
#define OP_BADVERS		21
#define OP_MSG			22
#define OP_YOUARE		23
#define OP_LINESTO		24
#define OP_WON			25
#define OP_ZERO			26

#define MAX_BUFFER_SIZE		4096

void NetworkServer(int, int);

#endif
