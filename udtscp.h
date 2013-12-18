#ifndef HSCP_VERSION
#include "hscp.h"
#endif

#ifndef INCLUDED_UDTSCP
#define INCLUDED_UDTSCP
#include <udt.h>

#define	UDTSBS "UDTSendBufSize"
#define	UDTRBS "UDTSecvBufSize"
#define	UDPSBS "UDPSendBufSize"
#define	UDPRBS "UDPSecvBufSize"

#define UDPSTP 18001
#define UDPEDP 18032
#define UDPCSTP 32768
#define UDPCEDP 32885

class UDTScp
{
public: 
   UDTSOCKET serv;
   UDTSOCKET handle;
   int assigned_port;
   int assigned_port_client;

   char *cpath;
   int svflag;
   const char *nodemode;
   int verbose_mode;
   int showprogress;
   int ipvx;
   long limitbw;

   UDTScp():	serv(0),
		handle(0),
		assigned_port(0),
		assigned_port_client(0),
		cpath(0),
		svflag(0),
		nodemode(NODEC),
		verbose_mode(0),
		showprogress(0),
		ipvx(4),
		limitbw(-1) {}

//   int getargument(char *, char*);
   void read_config();
   int setoption(int);
   int listen();
   int get_assigned_port();
   int set_assigned_port(int);
   int set_hscp_protocol(int);
   bool accept();

   UDTSOCKET connect(char*, char*);

//   int sendfile(char*);
//   int recvfile(char*);
   int sendfile(char*, size_t);
   int recvfile(char*, size_t);

   int disconnect();

   int set_verbose_mode(int);
   int set_server_flag(int, char *);
   int set_config_path(char *);
   int set_showprogress(int);
   int get_showprogress();
   int set_show_unit(int);
   int set_ipv6();
   int set_limit_rate(long);
};
#endif
