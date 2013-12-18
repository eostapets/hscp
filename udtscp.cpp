/********************************************************************************
$Id: udtscp.cpp 102 2010-12-26 13:47:53Z bunpojpn $

udtscp.cpp for hscp.cpp v 0.9.20 2010-12-25
Copyright (c) 2009,2010 RCCS technical team of IMS,
   Fumiyasu Mizutani,
   Fumitsuna Teshima, Masataka Sawa,
   Shigeki Naitoh,    Jun-ichi Matsuo,
   Kensuke Iwahashi,  Takakazu Nagaya.
All rights reserved.
We special thanks to
   Hironori Kogawa (Hitachi, Ltd., for the first try to merge UDT into scp),
   UDT distributer (the board of trustees of the University of Illinois),
   OpenSSH distributers, and other open source distributers.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of 
    conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, 
    this list of conditions and the following disclaimer in the documentation and/or 
    other materials provided with the distribution.
  * Neither the name of the "NINS (National Institutes of Natural Sciences), 
    IMS (Institute for Molecular Science)" nor the names of its contributors may be 
    used to endorse or promote products derived from this software without specific 
    prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
MERCHANT ABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF 
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "hscp.h"
#include "udtscp.h"
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <udt.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <netdb.h>
using namespace std;

#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <iomanip>

#include <libgen.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#define PORTMX 256
#define SIN6_LEN

#ifndef UDP_RECV_BUF_SIZE
#define UDP_RECV_BUF_SIZE 0
#endif

int udp_start_port = 0;
int udp_end_port = 0;
int udp_client_start_port = 0;
int udp_client_end_port = 0;
int udp_sendbufsize = 0;
int udp_recvbufsize = UDP_RECV_BUF_SIZE;
int udt_sendbufsize = 0;
int udt_recvbufsize = 0;
int udt_mss = 0;
int udt_fc = 0;
int64_t udt_maxbw = -1;
bool udt_sndsyn = 1;
bool udt_rcvsyn = 1;
int show_progress_mode = 0;
int show_unit_mode = 1;
int hscp_protocol_mode = 1;
char file_name[1024];
int portnm[PORTMX];
int portrg = 0;
int portnm_client[PORTMX];
int portrg_client = 0;
int monitorfg = 1;
int tsec = 0;
int stalled_boundary = 0;
int stalled_continuously = 10;

void UDTScp::read_config()
{
   const char *ftitle = "[read_config] ";
   if (strlen(UDTScp::cpath) == 0) return;

   ifstream ifs(UDTScp::cpath);
   string line, para, num;
   while (getline(ifs, line)){
      if (UDTScp::svflag){
         para = "UDPStartPort";
         if (line.find(para) == 0) {
            num = line.substr(para.size());
            udp_start_port = atol(num.c_str());
            if (UDTScp::verbose_mode){
                cerr << UDTScp::nodemode << ftitle;
		cerr << "UDP start port: " << udp_start_port << endl;
 	    }
         }
         para = "UDPEndPort";
         if (line.find(para) == 0) {
            num = line.substr(para.size());
            udp_end_port = atol(num.c_str());
            if (UDTScp::verbose_mode){
                cerr << UDTScp::nodemode << ftitle;
		cerr << "UDP end port: " << udp_end_port << endl;
	    }
         }
      }
      else {
         para = "UDPClientStartPort";
         if (line.find(para) == 0) {
            num = line.substr(para.size());
            udp_client_start_port = atol(num.c_str());
            if (UDTScp::verbose_mode){
                cerr << UDTScp::nodemode << ftitle;
		cerr << "UDP client start port: " << udp_client_start_port << endl;
	    }
         }
         para = "UDPClientEndPort";
         if (line.find(para) == 0) {
            num = line.substr(para.size());
            udp_client_end_port = atol(num.c_str());
            if (UDTScp::verbose_mode){
                cerr << UDTScp::nodemode << ftitle;
		cerr << "UDP client end port: " << udp_client_end_port << endl;
	    }
         }
      }
      para = "UDPSendBufSize";
      if (line.find(para) == 0) {
         num = line.substr(para.size());
         udp_sendbufsize = atol(num.c_str());
         if (UDTScp::verbose_mode){
            cerr << UDTScp::nodemode << ftitle;
	    cerr << "UDPSBS: " << udp_sendbufsize << endl;
         }
      }
      para = "UDPRecvBufSize";
      if (line.find(para) == 0) {
         num = line.substr(para.size());
         udp_recvbufsize = atol(num.c_str());
         if (UDTScp::verbose_mode){
            cerr << UDTScp::nodemode << ftitle;
	    cerr << "UDPRBS: " << udp_recvbufsize << endl;
         }
      }
      para = "UDTSendBufSize";
      if (line.find(para) == 0) {
         num = line.substr(para.size());
         udt_sendbufsize = atol(num.c_str());
         if (UDTScp::verbose_mode){
            cerr << UDTScp::nodemode << ftitle;
            cerr << "UDTSBS: " << udt_sendbufsize << endl;
         }
      }
      para = "UDTRecvBufSize";
      if (line.find(para) == 0) {
         num = line.substr(para.size());
         udt_recvbufsize = atol(num.c_str());
         if (UDTScp::verbose_mode){
            cerr << UDTScp::nodemode << ftitle;
            cerr << "UDTRBS: " << udt_recvbufsize << endl;
         }
      }
      para = "UDTSendSynMode";
      if (line.find(para) == 0) {
         num = line.substr(para.size());
         if (atoi(num.c_str()) == 0) udt_sndsyn = 0;
         if (UDTScp::verbose_mode){
            cerr << UDTScp::nodemode << ftitle;
            cerr << "UDTRSY: " << udt_sndsyn << endl;
         }
      }
      para = "UDTRecvSynMode";
      if (line.find(para) == 0) {
         num = line.substr(para.size());
         if (atoi(num.c_str()) == 0) udt_rcvsyn = 0;
         if (UDTScp::verbose_mode){
            cerr << UDTScp::nodemode << ftitle;
            cerr << "UDTRSY: " << udt_rcvsyn << endl;
         }
      }
      para = "UDTMaxPktSize";
      if (line.find(para) == 0) {
         num = line.substr(para.size());
         udt_mss = atol(num.c_str());
         if (UDTScp::verbose_mode){
            cerr << UDTScp::nodemode << ftitle;
            cerr << "UDTMSS: " << udt_mss << endl;
         }
      }
      para = "UDTMaxWinSize";
      if (line.find(para) == 0) {
         num = line.substr(para.size());
         udt_fc = atol(num.c_str());
         if (UDTScp::verbose_mode){
            cerr << UDTScp::nodemode << ftitle;
            cerr << "UDTFC: " << udt_fc << endl;
         }
      }
      para = "UDTMaxBandWidth";
      if (line.find(para) == 0) {
         num = line.substr(para.size());
         udt_maxbw = atoll(num.c_str());
         if (UDTScp::verbose_mode){
            cerr << UDTScp::nodemode << ftitle;
            cerr << "UDTMBW: " << udt_maxbw << endl;
         }
      }
      para = "StalledBoundary";
      if (line.find(para) == 0) {
         num = line.substr(para.size());
         stalled_boundary = atoll(num.c_str());
         if (UDTScp::verbose_mode){
            cerr << UDTScp::nodemode << ftitle;
            cerr << "STLBND: " << stalled_boundary << endl;
         }
      }
      para = "StalledContinuously";
      if (line.find(para) == 0) {
         num = line.substr(para.size());
         stalled_continuously = atoll(num.c_str());
         if (UDTScp::verbose_mode){
            cerr << UDTScp::nodemode << ftitle;
            cerr << "STLCNT: " << stalled_continuously << endl;
         }
      }
   }
   ifs.close();
   if (UDTScp::svflag){
      udp_start_port == 0 ? udp_start_port = UDPSTP : udp_start_port;
      udp_end_port  == 0 ? udp_end_port = UDPEDP : udp_end_port ;
      if (udp_end_port < udp_start_port) udp_end_port = udp_start_port;
      if (udp_end_port - udp_start_port + 1 > PORTMX) udp_end_port = udp_start_port + PORTMX - 1;
      if (UDTScp::verbose_mode){
         cerr << UDTScp::nodemode << ftitle;
         cerr << "UDP server port range: " << udp_start_port << " - " << udp_end_port << endl;
      }
      portrg = udp_end_port - udp_start_port + 1;

      int i, r1, r2, st;
      srand(time(NULL));
      for (i = 0; i < portrg; i++){ portnm[i] = udp_start_port + i; }
      for (i = 0; i < portrg * 10; i++){
        r1 = (int)((double)rand()/RAND_MAX * portrg);
        r2 = (int)((double)rand()/RAND_MAX * portrg);
        st = portnm[r1]; portnm[r1] = portnm[r2]; portnm[r2] = st;
      }
   }
   else {
      if (udp_client_start_port > 0 && udp_client_end_port > 0){
        if (udp_client_end_port < udp_client_start_port) udp_client_end_port = udp_client_start_port;
        if (udp_client_end_port - udp_client_start_port + 1 > PORTMX) udp_client_end_port = udp_client_start_port + PORTMX - 1;
      }
      if (UDTScp::verbose_mode){
         cerr << UDTScp::nodemode << ftitle << "[read_config] UDP client port ";
         if (udp_client_start_port > 0){
            cerr << "range: " << udp_client_start_port << " - " << udp_client_end_port << endl;
         }
         else {
            cerr << " any" << endl;
         }
      }
      portrg_client = udp_client_end_port - udp_client_start_port + 1;

      int i, r1, r2, st;
      srand(time(NULL));
      for (i = 0; i < portrg_client; i++){ portnm_client[i] = udp_client_start_port + i; }
      for (i = 0; i < portrg_client * 10; i++){
        r1 = (int)((double)rand()/RAND_MAX * portrg_client);
        r2 = (int)((double)rand()/RAND_MAX * portrg_client);
        st = portnm_client[r1]; portnm_client[r1] = portnm_client[r2]; portnm_client[r2] = st;
      }
   }
}

double my_clock()
{
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

void *monitor_s(void *);
void *monitor_r(void *);

//int UDTScp::getargument(char *args, char *opts)
//{
//   long argc;
//
//   argc = strtol(opts);
//   if (argc > 0){
//     if (strstr(args, UDTSBS) != 0){ udt_sendbufsize = argc; }
//     if (strstr(args, UDTRBS) != 0){ udt_recvbufsize = argc; }
//     if (strstr(args, UDPSBS) != 0){ udp_sendbufsize = argc; }
//     if (strstr(args, UDPRBS) != 0){ udp_recvbufsize = argc; }
//   }
//}

int UDTScp::setoption(UDTSOCKET socket_handle)
{
   const char *ftitle = "[setoption] ";
   const char *fmode = "[getoption]";
   bool block = 0, parab;
   int para, size, sizeb, sizel;
   int64_t paral;

   size = sizeof(int);
   sizeb = sizeof(bool);
   sizel = sizeof(int64_t);
   if (UDTScp::verbose_mode){
     UDT::getsockopt(socket_handle, 0, UDP_SNDBUF, &para, &size);
     cerr << UDTScp::nodemode << ftitle << fmode;
     cerr << " UDP_SNDBUF: " << para << endl;
     UDT::getsockopt(socket_handle, 0, UDP_RCVBUF, &para, &size);
     cerr << UDTScp::nodemode << ftitle << fmode;
     cerr << " UDP_RCVBUF: " << para << endl;
     UDT::getsockopt(socket_handle, 0, UDT_SNDBUF, &para, &size);
     cerr << UDTScp::nodemode << ftitle << fmode;
     cerr << " UDT_SNDBUF: " << para << endl;
     UDT::getsockopt(socket_handle, 0, UDT_RCVBUF, &para, &size);
     cerr << UDTScp::nodemode << ftitle << fmode;
     cerr << " UDT_RCVBUF: " << para << endl;
     UDT::getsockopt(socket_handle, 0, UDT_MSS, &para, &size);
     cerr << UDTScp::nodemode << ftitle << fmode;
     cerr << " UDT_MSS:    " << para << endl;
     UDT::getsockopt(socket_handle, 0, UDT_FC, &para, &size);
     cerr << UDTScp::nodemode << ftitle << fmode;
     cerr << " UDT_FC:     " << para << endl;
     UDT::getsockopt(socket_handle, 0, UDT_SNDSYN, &parab, &sizeb);
     cerr << UDTScp::nodemode << ftitle << fmode;
     cerr << " UDT_SNDSYN: " << parab << endl;
     UDT::getsockopt(socket_handle, 0, UDT_RCVSYN, &parab, &sizeb);
     cerr << UDTScp::nodemode << ftitle << fmode;
     cerr << " UDT_RCVSYN: " << parab << endl;
     UDT::getsockopt(socket_handle, 0, UDT_MAXBW, &paral, &sizel);
     cerr << UDTScp::nodemode << ftitle << fmode;
     cerr << " UDT_MAXBW:  " << paral << endl;
   }

   if (udp_sendbufsize > 0){
     UDT::setsockopt(socket_handle, 0, UDP_SNDBUF, &udp_sendbufsize, size);
     UDT::getsockopt(socket_handle, 0, UDP_SNDBUF, &para, &size);
     if (UDTScp::verbose_mode){ 
       cerr << UDTScp::nodemode << ftitle;
       cerr << "UDP_SBF: " << udp_sendbufsize;
       cerr << " get_again " << para << endl;
     }
   }
   if (udp_recvbufsize > 0){
     UDT::setsockopt(socket_handle, 0, UDP_RCVBUF, &udp_recvbufsize, size);
     UDT::getsockopt(socket_handle, 0, UDP_RCVBUF, &para, &size);
     if (UDTScp::verbose_mode){ 
       cerr << UDTScp::nodemode << ftitle;
       cerr << "UDP_RBF: " << udp_recvbufsize;
       cerr << " get_again " << para << endl;
     }
   }
   if (udt_sendbufsize > 0){
     UDT::setsockopt(socket_handle, 0, UDT_SNDBUF, &udt_sendbufsize, size);
     UDT::getsockopt(socket_handle, 0, UDT_SNDBUF, &para, &size);
     if (UDTScp::verbose_mode){
       cerr << UDTScp::nodemode << ftitle;
       cerr << "UDT_SBF: " << udt_sendbufsize;
       cerr << " get_again " << para << endl;
     }
   }
   if (udt_recvbufsize > 0){
     UDT::setsockopt(socket_handle, 0, UDT_RCVBUF, &udt_recvbufsize, size);
     UDT::getsockopt(socket_handle, 0, UDT_RCVBUF, &para, &size);
     if (UDTScp::verbose_mode){ 
       cerr << UDTScp::nodemode << ftitle;
       cerr << "UDT_RBF: " << udt_recvbufsize;
       cerr << " get_again " << para << endl;
     }
   }
   if (udt_mss > 0){
     UDT::setsockopt(socket_handle, 0, UDT_MSS, &udt_mss, size);
     UDT::getsockopt(socket_handle, 0, UDT_MSS, &para, &size);
     if (UDTScp::verbose_mode){ 
       cerr << UDTScp::nodemode << ftitle;
       cerr << "UDT_MSS: " << udt_mss;
       cerr << " get_again " << para << endl;
     }
   }
   if (udt_fc > 0){
     UDT::setsockopt(socket_handle, 0, UDT_FC, &udt_fc, size);
     UDT::getsockopt(socket_handle, 0, UDT_FC, &para, &size);
     if (UDTScp::verbose_mode){ 
       cerr << UDTScp::nodemode << ftitle;
       cerr << "UDT_FC: " << udt_fc;
       cerr << " get_again " << para << endl;
     }
   }
   if (udt_sndsyn == 0){
     UDT::setsockopt(socket_handle, 0, UDT_SNDSYN, &udt_sndsyn, sizeb);
     UDT::getsockopt(socket_handle, 0, UDT_SNDSYN, &parab, &sizeb);
     if (UDTScp::verbose_mode){ 
       cerr << UDTScp::nodemode << ftitle;
       cerr << "UDT_SNDSYN: " << udt_sndsyn;
       cerr << " get_again " << parab << endl;
     }
   }
   if (udt_rcvsyn == 0){
     UDT::setsockopt(socket_handle, 0, UDT_RCVSYN, &udt_rcvsyn, sizeb);
     UDT::getsockopt(socket_handle, 0, UDT_RCVSYN, &parab, &sizeb);
     if (UDTScp::verbose_mode){ 
       cerr << UDTScp::nodemode << ftitle;
       cerr << "UDT_RCVSYN: " << udt_rcvsyn;
       cerr << " get_again " << parab << endl;
     }
   }
   if (udt_maxbw > 0 || limitbw > 0){
     if (udt_maxbw > 0 && limitbw > 0){
       if (udt_maxbw > limitbw) udt_maxbw = limitbw;
     }
     else udt_maxbw = limitbw;
     UDT::setsockopt(socket_handle, 0, UDT_MAXBW, &udt_maxbw, sizel);
     UDT::getsockopt(socket_handle, 0, UDT_MAXBW, &paral, &sizel);
     if (UDTScp::verbose_mode){ 
       cerr << UDTScp::nodemode << ftitle;
       cerr << "UDT_MAXBW: " << udt_maxbw;
       cerr << " get_again " << paral << endl;
     }
   }
   return 0;
}

UDTSOCKET UDTScp::listen()
{
   UDTScp::read_config();
   UDT::startup();
   if (UDTScp::ipvx == 4) UDTScp::serv = UDT::socket(AF_INET,  SOCK_STREAM, 0);
   if (UDTScp::ipvx == 6) UDTScp::serv = UDT::socket(AF_INET6, SOCK_STREAM, 0);
   UDTScp::setoption(UDTScp::serv);
   int p;
   for (p = 0; p < portrg; p++)
   {
      if (UDTScp::ipvx == 4){
         sockaddr_in my_addr;
         my_addr.sin_family = AF_INET;
         my_addr.sin_port = htons(portnm[p]);
         my_addr.sin_addr.s_addr = INADDR_ANY;
         memset(&(my_addr.sin_zero), '\0', 8);
         if (UDT::ERROR == UDT::bind(UDTScp::serv, (sockaddr*)&my_addr, sizeof(my_addr)))
         {
            if (UDTScp::verbose_mode){
               cerr << UDTScp::nodemode;
               cerr << "[listen v4] Port using: " << portnm[p] << endl;
               cerr << "bind: " << UDT::getlasterror().getErrorMessage() << endl;
            }
         }
         else {
            UDTScp::assigned_port = portnm[p];
            break;
         }
      }
      if (UDTScp::ipvx == 6){
         sockaddr_in6 my_addr;
         my_addr.sin6_family = AF_INET6;
         my_addr.sin6_port = htons(portnm[p]);
         my_addr.sin6_addr = in6addr_any;
         my_addr.sin6_flowinfo = 0;
         if (UDT::ERROR == UDT::bind(UDTScp::serv, (sockaddr*)&my_addr, sizeof(my_addr)))
         {
            if (UDTScp::verbose_mode){
               cerr << UDTScp::nodemode;
               cerr << "[listen v6] Port using: " << portnm[p] << endl;
               cerr << "bind: " << UDT::getlasterror().getErrorMessage() << endl;
            }
         }
         else {
            UDTScp::assigned_port = portnm[p];
            break;
         }
      }
   }
   if (p == portrg){
      if (UDTScp::verbose_mode){
          cerr << "ERROR bind: " << UDT::getlasterror().getErrorMessage() << endl;
      }
      return 0;
   }
   UDT::listen(UDTScp::serv, 1);
   if (UDTScp::verbose_mode){
      cerr << UDTScp::nodemode;
      if (UDTScp::ipvx == 4){ cerr << "[listen v4] success" << endl; }
      if (UDTScp::ipvx == 6){ cerr << "[listen v6] success" << endl; }
   }
   return 1;
}

int UDTScp::get_assigned_port()
{
   return(UDTScp::assigned_port);
}

int UDTScp::set_assigned_port(int port)
{
   UDTScp::assigned_port = port;
   return(0);
}

bool UDTScp::accept()
{
   sockaddr_in  their_addr;
   sockaddr_in6 their_addr6;
   int namelen;
   bool block = 0;
   double sec, st, ed;

   if (UDTScp::ipvx == 4) namelen = sizeof(their_addr);
   if (UDTScp::ipvx == 6) namelen = sizeof(their_addr6);
   UDT::setsockopt(UDTScp::serv, 0, UDT_RCVSYN, &block, sizeof(bool));
   st = my_clock();
   while(1){
      if (UDTScp::ipvx == 4){
         if (UDT::INVALID_SOCK != (UDTScp::handle = UDT::accept(UDTScp::serv, (sockaddr*)&their_addr, &namelen))){
            UDT::close(UDTScp::serv);
//            UDTScp::setoption(UDTScp::handle);
            if (UDTScp::verbose_mode){
              cerr << UDTScp::nodemode << "[accept v4] success" << endl;
            }
            return 1;
         }
      }
      if (UDTScp::ipvx == 6){
         if (UDT::INVALID_SOCK != (UDTScp::handle = UDT::accept(UDTScp::serv, (sockaddr*)&their_addr6, &namelen))){
            UDT::close(UDTScp::serv);
//            UDTScp::setoption(UDTScp::handle);
            if (UDTScp::verbose_mode){
              cerr << UDTScp::nodemode << "[accept v6] success" << endl;
            }
            return 1;
         }
      }
      ed = my_clock();
      sec = ed - st;
      if (sec >= 5.0) break;
   }
   if (UDTScp::verbose_mode){
     cerr << UDTScp::nodemode;
     cerr << "[accept] " << UDT::getlasterror().getErrorMessage() << endl;
   }
   return 0;
}

UDTSOCKET UDTScp::connect(char* host, char* port)
{
   const char *ftitle = "[connect] ";
   UDTScp::read_config();
   UDT::startup();
   if (UDTScp::ipvx == 4) UDTScp::handle = UDT::socket(AF_INET,  SOCK_STREAM, 0);
   if (UDTScp::ipvx == 6) UDTScp::handle = UDT::socket(AF_INET6, SOCK_STREAM, 0);
   UDTScp::setoption(UDTScp::handle);

   if (udp_client_start_port > 0 && udp_client_end_port > 0){
     int p;
     for (p = 0; p < portrg_client; p++){
       if (UDTScp::ipvx == 4){
         sockaddr_in my_addr;
         my_addr.sin_family = AF_INET;
         my_addr.sin_port = htons(portnm_client[p]);
         my_addr.sin_addr.s_addr = INADDR_ANY;
         memset(&(my_addr.sin_zero), '\0', 8);
         if (UDT::ERROR == UDT::bind(UDTScp::handle,(sockaddr*)&my_addr,sizeof(my_addr)))
         {
           if (UDTScp::verbose_mode){
            cerr << UDTScp::nodemode;
            cerr << "[connect bind] Port using: " << portnm_client[p] << endl;
            cerr << "connect bind: " << UDT::getlasterror().getErrorMessage() << endl;
           }
         }
         else {
           UDTScp::assigned_port_client = portnm_client[p];
           if (UDTScp::verbose_mode){
             cerr << UDTScp::nodemode;
             cerr << "[connect bind] Port assigned: " << assigned_port_client << endl;
           }
           break;
         }
       }
       if (UDTScp::ipvx == 6){
         sockaddr_in6 my_addr;
         my_addr.sin6_family = AF_INET6;
         my_addr.sin6_port = htons(portnm_client[p]);
         my_addr.sin6_addr = in6addr_any;
         my_addr.sin6_flowinfo = 0;
         if (UDT::ERROR == UDT::bind(UDTScp::handle,(sockaddr*)&my_addr,sizeof(my_addr)))
         {
           if (UDTScp::verbose_mode){
            cerr << UDTScp::nodemode;
            cerr << "[connect bind] Port using: " << portnm_client[p] << endl;
            cerr << "connect bind: " << UDT::getlasterror().getErrorMessage() << endl;
           }
         }
         else {
           UDTScp::assigned_port_client = portnm_client[p];
           if (UDTScp::verbose_mode){
             cerr << UDTScp::nodemode;
             cerr << "[connect bind] Port assigned: " << assigned_port_client << endl;
           }
           break;
         }
       }
     }
     if (p == portrg_client){
       cerr << "ERROR connect bind: " << UDT::getlasterror().getErrorMessage() << endl;
       return 0;
     }
   }

/*
   const hostent* host_info = 0;
   host_info = gethostbyname(host);
   const in_addr* address;
   if (host_info) {
     address = (in_addr*)host_info->h_addr_list[0];
   }
   if (UDTScp::ipvx == 4){
      sockaddr_in  serv_addr;
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_port   = htons(short(atoi(port)));
      if (inet_pton(AF_INET, inet_ntoa(*address), &serv_addr.sin_addr) <= 0){
         cerr << "incorrect network address.\n";
         return 0;
      }
      memset(&(serv_addr.sin_zero), '\0', 8);
   }
   if (UDTScp::ipvx == 6){
      sockaddr_in6 serv_addr;
      serv_addr.sin6_family = AF_INET6;
      serv_addr.sin6_port = htons(short(atoi(port)));
      if (inet_pton(AF_INET6, inet_ntoa(*address), &serv_addr.sin6_addr) <= 0){
         cerr << "incorrect network address.\n";
         return 0;
      }
   }
*/
   struct addrinfo hints;
   struct addrinfo *rp;
   int rt;
   memset(&hints, 0, sizeof(struct addrinfo));
   if (UDTScp::ipvx == 4) hints.ai_family = AF_INET;
   if (UDTScp::ipvx == 6) hints.ai_family = AF_INET6;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = 0;
   hints.ai_protocol = 0;
   rt = getaddrinfo(host, port, &hints, &rp);
   if (rt != 0){
     cerr << nodemode << ftitle << "getaddrinfo " << gai_strerror(rt) << endl;
     return 0;
   }

   if (UDT::ERROR == UDT::connect(UDTScp::handle, (sockaddr*)rp->ai_addr, rp->ai_addrlen))
//   if (UDT::ERROR == UDT::connect(UDTScp::handle, (sockaddr*)&serv_addr, sizeof(serv_addr)))
   {
      cerr << "connect: " << UDT::getlasterror().getErrorMessage() << endl;
      return 0;
/*
      cerr << "connect: " << UDT::getlasterror().getErrorMessage() << endl;

      char *ip;
      if (UDTScp::ipvx == 4)
         inet_ntop(AF_INET, (const void*)(serv_addr.sin_addr.s_addr), ip, sizeof(serv_addr));
      if (UDTScp::ipvx == 6)
         inet_ntop(AF_INET6, (const void*)(serv_addr.sin6_addr.s6_addr), ip, sizeof(serv_addr));
*/
   }
   freeaddrinfo(rp);
   return(UDTScp::handle);
}

void displayFileSize(char *file, int64_t sz, double sec)
{
   double dn;
   const char *ds;

   cerr.width(37);
   cerr.fill(' ');
   cerr << left;
   cerr << basename(file);

   dn = (double)sz;
   if (show_unit_mode){
     cerr.width(10);
     cerr << setiosflags(ios::fixed);
     cerr << setw(5) << right;
     cerr << setprecision(0);
     if (dn < 10000.0) ds = "B";
     else {
        cerr << setprecision(3);
        dn /= 1000;
        if (dn < 10000.0) ds = "kB";
        else {
           dn /= 1000;
           if (dn < 10000.0) ds = "MB";
           else {
              dn /= 1000;
              if (dn < 10000.0) ds = "GB";
              else {
                dn /= 1000;
                ds = "TB";
              }
           }
        }
     }
   }
   else {
     cerr.width(15);
     cerr << setiosflags(ios::fixed);
     cerr << setw(15) << right;
     cerr << setprecision(0);
     ds = "Byte";
   }
   cerr << dn << ds;

   dn = (double)sz/(double)sec;
   cerr.width(12);
   cerr.fixed;
   cerr.precision(3);
   if (dn < 10000) ds = "B/s";
   else {
      dn /= 1000;
      if (dn < 10000) ds = "kB/s";
      else {
         dn /= 1000;
         if (dn < 10000) ds = "MB/s";
         else {
            dn /= 1000;
            if (dn < 10000) ds = "GB/s";
            else {
               dn /= 1000;
               ds = "TB/s";
            }
         }
      }
   }
   cerr << dn << ds;
   cerr.width(10);
   cerr << (float)sec << "sec" << endl;
}

void displayProgressSCP(int64_t bsz, double rate, int hour, int min, int sec, int eta, char *file)
{
   double  sz;
   int64_t dn;
   const char   *ds;
   char fbuf[36];
   int i;

   for (i = 0; i < 34; i++){
      if (file[i] == 0x00) break;
      fbuf[i] = file[i];
   }
   fbuf[i] = 0x00;
   sz = (double)bsz / tsec;
   dn = bsz;
   fprintf(stderr, "%-34s", file);
   fprintf(stderr, "  %8.3f%%", rate);
   if (show_unit_mode){
     if (dn < 10000) ds = "   %4lldB";
     else {
        dn /= 1000;
        if (dn < 10000) ds = "  %4lldkB";
        else {
           dn /= 1000;
           if (dn < 10000) ds = "  %4lldMB";
           else {
              dn /= 1000;
              if (dn < 10000) ds = "  %4lldGB";
              else {
                 dn /= 1000;
                 ds = "  %4lldTB";
              }
           }
        }
     }
   }
   else {
     ds = " %lldByte";
   }
   fprintf(stderr, ds, dn);

   if (sz < 10000.0) ds = "  %6.1fB/s";
   else {
      sz /= 1000;
      if (sz < 1000.0) ds = "  %6.1fkB/s";
      else {
         sz /= 1000;
         if (sz < 1000.0) ds = "  %6.1fMB/s";
         else {
            sz /= 1000;
            if (sz < 1000.0) ds = "  %6.1fGB/s";
            else {
               sz /= 1000;
               ds = "  %6.1fTB/s";
            }
         }
      }
   } 
   fprintf(stderr, ds, sz);

   if (hour == 0){ fprintf(stderr, "    %02d:%02d", min, sec); }
   else          { fprintf(stderr, " %2d:%02d:%02d", hour,min,sec); }
   if (eta) fprintf(stderr, " ETA");
   else     fprintf(stderr, "    ");
   fflush(stderr);
}

void displayProgressFooter(double rate, double rtt, int cwd, double ppd, int ack, int nak, int buf, double xfd, int hour, int min, int sec)
{
   double dn;
   const char  *ds;

   fprintf(stderr, "%11.3f", rate);
   fprintf(stderr, "%9.3f",  rtt);
   fprintf(stderr, "%6d",    cwd);
   dn = ppd;
   if (dn < 1000.0) ds = "%9.2fus";
   else {
      dn /= 1000.0;
      if (dn < 1000.0) ds = "%9.2fms";
      else {
         dn /= 1000.0;
         if (dn < 1000.0) ds = "%9.2f s";
         else {
            dn /= 1000.0;
            if (dn < 1000.0) ds = "%9.2fks";
            else {
               dn /= 1000.0;
               ds = "%9.2fMs";
            }
         }
      }
   }
   fprintf(stderr, ds, dn);
   fprintf(stderr, "%6d",    ack);
   fprintf(stderr, "%6d",    nak);
   fprintf(stderr, " %10d", buf);
   fprintf(stderr, "%8.3f%%",xfd);
   if (hour == 0){ fprintf(stderr, "    %02d:%02d", min, sec); }
   else          { fprintf(stderr, " %2d:%02d:%02d", hour,min,sec); }
   fflush(stderr);
}

void displayProgressHeader(int fw)
{
   if (fw == 0){
      cerr << " SndR(Mb/s)";
      cerr << "  RTT(ms)";
      cerr << "  CWnd";
//      cerr << "  PSndP(us)";
      cerr << "  PktSendPd";
      cerr << "  RACK";
      cerr << "  RNAK";
//      cerr << "  RcvACKTotal";
      cerr << "  AvSndBfSz";
      cerr << "   Send(%)";
      cerr << "   Etime";
      cerr << endl;
   }
   else {
      cerr << " RcvR(Mb/s)";
      cerr << "  RTT(ms)";
      cerr << "  CWnd";
//      cerr << "  PSndP(us)";
      cerr << "  PktSendPd";
      cerr << "  SACK";
      cerr << "  SNAK";
//      cerr << "  SndACKTotal";
      cerr << "  AvRcvBfSz";
      cerr << "   Recv(%)";
      cerr << "   Etime";
      cerr << endl;
   }
}

int UDTScp::sendfile(char* file, size_t size)
{
   const char *ftitle = "[sendfile] ";
   double sec, st, ed, rt;
   int64_t sz, offset;

   fstream ifs(file, ios::in | ios::binary);
   strcpy(file_name, basename(file));
   int s = (int)UDTScp::handle;

   if (hscp_protocol_mode == 1){
     sz = size;
   }
   if (hscp_protocol_mode == 0){
     ifs.seekg(0, ios::end);
     sz = ifs.tellg();
     ifs.seekg(0, ios::beg);
     if (UDTScp::verbose_mode){
       cerr << UDTScp::nodemode << ftitle;
       cerr << "File size: " << sz << endl;
     }

     // send file size information
     char sizetext[32];
     sprintf(sizetext, "%lld", sz);
     if (strlen(sizetext) == 0){
        cerr << "send: can't convert size = " << sz << endl;
        return 0;
     }
     if (UDTScp::verbose_mode){
       cerr << UDTScp::nodemode << ftitle;
       cerr << "File size string: " << sizetext << endl;
     }
     if (UDT::ERROR == UDT::send(UDTScp::handle, sizetext, sizeof(sizetext), 0))
//     if (UDT::ERROR == UDT::send(UDTScp::handle, (char*)&sz, sizeof(int64_t), 0))
     {
        cerr << "send: " << UDT::getlasterror().getErrorMessage() << endl;
        return 0;
     }
   }

//   int m = UDTScp::showprogress;
//   if (UDTScp::svflag) m = 999;
   int m = show_progress_mode;
   if (sz == 0){
      ifs.close();
      tsec = 1;
      if (m < 3){
         if (m > 0){
            displayProgressHeader(0);
            displayProgressFooter(0.0,0.0,0,0.0,0,0,0,0.0,0,0,1);
            fprintf(stderr, "\n");
         }
         displayProgressSCP(0,0.0,0,0,1,0,file_name);
         fprintf(stderr, "\n");
      }
      return 0;
   }

   UDT::TRACEINFO trace;
   pthread_t th;
   tsec = 0;
   if (m < 999) {
//      pthread_create(&th, NULL, monitor_s, (void *)&(UDTScp::handle));
      monitorfg = 1;
      pthread_create(&th, NULL, monitor_s, (void *) &s);
   }

//   if (m == 2) {
//      UDT::perfmon(UDTScp::handle, &trace);
//   }

//   if (m < 3) {
      st = my_clock();
//   }

   // send the file
   offset = 0;
   if (UDT::ERROR == UDT::sendfile(UDTScp::handle, ifs, offset, sz))
   {
      cerr << endl << "sendfile: " << UDT::getlasterror().getErrorMessage() << endl;
      raise(SIGTERM);
      return 0;
   }

//   if (m > 0 && m < 3) {
      ed = my_clock();
      sec = ed - st;
//   }

   if (m < 999) {
//      sleep(1);
      pthread_cancel(th);
      pthread_detach(th);
      monitorfg = 0;
      if (tsec != -1){
         monitor_s((void *) &s);
         fprintf(stderr, "\n");
      }
   }

//   if (offset == -1){
//      raise(SIGTERM);
//      return 0;
//   }

//   if (m == 2) {
//      UDT::perfmon(UDTScp::handle, &trace);
//      //cerr << "speed = " << trace.mbpsSendRate << "Mbits/sec" << endl;
//
//      cerr.width(35);
//      cerr.fill(' ');
//      cerr << left;
//      cerr << basename(file);
//      cerr << "speed = " << trace.mbpsSendRate << "Mbits/sec" << endl;
//   }

   if (m > 0 && m < 3) displayFileSize(file, sz, sec);

   ifs.close();
   if (UDTScp::verbose_mode){
     cerr << UDTScp::nodemode << ftitle;
     cerr << "File send finished. " << endl;
   }
   return 0; //20090409
}

int UDTScp::recvfile(char* file, size_t size)
{
   const char *ftitle = "[recvfile] ";
   double sec, st, ed, rt;
   bool block = 1;
   int64_t sz, offset;
   char sizetext[32];

   UDT::setsockopt(UDTScp::handle, 0, UDT_RCVSYN, &block, sizeof(bool));
   if (hscp_protocol_mode == 1){
     sz = size;
   }
   if (hscp_protocol_mode == 0){
     // get size information
     if (UDT::ERROR == UDT::recv(UDTScp::handle, sizetext, sizeof(sizetext), 0))
//     if (UDT::ERROR == UDT::recv(UDTScp::handle, (char*)&sz, sizeof(int64_t), 0))
     {
        cerr << "recv: " << UDT::getlasterror().getErrorMessage() << endl;
        return 0;
     }
     if (UDTScp::verbose_mode){
       cerr << UDTScp::nodemode << ftitle;
       cerr << "File size string: " << sizetext << endl;
     }
     sz = (int64_t)strtoll(sizetext,NULL,10);
   }

   strcpy(file_name, basename(file));
//   int m = UDTScp::showprogress;
//   if (UDTScp::svflag) m = 999;
   int m = show_progress_mode;
   if (sz == 0){
//      cerr << "recv: size is zero" << endl;
      fstream ofs(file, ios::out | ios::binary | ios::trunc);
      ofs.close();
      tsec = 1;
      if (m < 3){
         if (m > 0){
            displayProgressHeader(0);
            displayProgressFooter(0.0,0.0,0,0.0,0,0,0,0.0,0,0,1);
            fprintf(stderr, "\n");
         }
         displayProgressSCP(0,0.0,0,0,1,0,file_name);
         fprintf(stderr, "\n");
      }
      return 0;
   }
   if (UDTScp::verbose_mode){
     cerr << UDTScp::nodemode << ftitle;
     cerr << "File size: " << sz << endl;
   }

   // receive the file
   fstream ofs(file, ios::out | ios::binary | ios::trunc);
   int64_t recvsize;

   UDT::TRACEINFO trace;

   int s = (int)UDTScp::handle;
   pthread_t th;
   tsec = 0;
   if (m < 999) {
//      pthread_create(&th, NULL, monitor_r, (void *)&(UDTScp::handle));
      monitorfg = 1;
      pthread_create(&th, NULL, monitor_r, (void *) &s);
   }

//   if (m == 2) {
//      UDT::perfmon(UDTScp::handle, &trace);
//   }

//   if (m < 3) {
      st = my_clock();
//   }

   offset = 0;
   if (UDT::ERROR == (recvsize = UDT::recvfile(UDTScp::handle, ofs, offset, sz)))
   {
      cerr << endl << "recvfile: " << UDT::getlasterror().getErrorMessage() << endl;
      raise(SIGTERM);
      return 0;
//      offset = -1;
   }

//   if (m > 0 && m < 3) {
      ed = my_clock();
      sec = ed - st;
//   }

   if (recvsize < sz){
      cerr << endl << "recvfile: interrupted." << endl;
      raise(SIGTERM);
      return 0;
   }

   if (m < 999) {
//      sleep(1);
      pthread_cancel(th);
      pthread_detach(th);
      monitorfg = 0;
      if (tsec != -1){
         monitor_r((void *) &s);
         fprintf(stderr, "\n");
      }
   }

//   if (offset == -1){
//      raise(SIGTERM);
//      return 0;
//   }

//   if (m == 2) {
//      UDT::perfmon(UDTScp::handle, &trace);
//
//      cerr.width(35);
//      cerr.fill(' ');
//      cerr << left;
//      cerr << basename(file);
//      cerr << "speed = " << trace.mbpsRecvRate << "Mbits/sec" << endl;
//   }

   if (m > 0 && m < 3) displayFileSize(file, sz, sec);

   ofs.close();
//   if (UDTScp::verbose_mode){
//     cerr << UDTScp::nodemode << ftitle;
//     cerr << "File receive finished. " << endl;
//   }
   return 0;
}

int UDTScp::disconnect()
{
   UDT::close(UDTScp::handle);
   // use this function to release the UDT library
   UDT::cleanup();
   return 0; //20090409
}

int UDTScp::set_limit_rate(long limit_r)
{
   UDTScp::limitbw = limit_r / 8;
   return 0;
}

int UDTScp::set_verbose_mode(int mode)
{
   UDTScp::verbose_mode = mode;
   return 0;
}

int UDTScp::set_server_flag(int svflag, char *nodemode)
{
   UDTScp::svflag   = svflag;
   UDTScp::nodemode = nodemode;
   return 0;
}

int UDTScp::set_config_path(char *path)
{
   UDTScp::cpath = path;
   return 0;
}

int UDTScp::set_showprogress(int mode)
{
   UDTScp::showprogress = mode;
   show_progress_mode = mode;
   return 0;
}

int UDTScp::get_showprogress()
{
   return (UDTScp::showprogress);
}

int UDTScp::set_show_unit(int mode)
{
   show_unit_mode = mode;
   return 0;
}

int UDTScp::set_hscp_protocol(int mode)
{
   hscp_protocol_mode = mode;
   return 0;
}

int UDTScp::set_ipv6()
{
   UDTScp::ipvx = 6;
   return 0;
}

void* monitor_s(void *s)
{
   UDTSOCKET u = *(UDTSOCKET*)s;

   UDT::TRACEINFO perf;

   int sec = 0;
   int min = 0;
   int hour = 0;
   int eta = 1;
   int lf = 0;
   int64_t rt;
   double rr;
   double sz;
   int     stalled_sec;
   int64_t stalled_size;

   if (show_progress_mode == 1) lf = 1; 
   sec = tsec;
   if (sec >= 60){
     min = (int)(sec / 60);
     sec = sec - min * 60;
   }
   if (min >= 60){
     hour = (int)(min / 60);
     min = min - hour * 60;
   }

   stalled_sec  = stalled_continuously;
   stalled_size = 0;
   for (int first = monitorfg;; first = 0)
   {
      if (monitorfg == 1) sleep(1);
      if (UDT::ERROR == UDT::perfmon(u, &perf))
      {
//         cerr << "perfmon: " << UDT::getlasterror().getErrorMessage() << endl;
         break;
      }
      if (show_progress_mode == 0){
        tsec++;
        sz = (double)perf.byteSendSize;
        if (sz < 1.0) sz = 1.0;
        rt = (int64_t)((double)perf.byteFileSize / sz * tsec - tsec + 0.9);
        if (rt == 0){
          eta = 0;
          rt = tsec;
        }
        hour = (int)(rt / 3600);
        rt = rt - hour * 3600;
        min = (int)(rt / 60);
        sec = rt - min * 60;
        if (hour > 99) hour = 99;
      }
      if (show_progress_mode == 1 || show_progress_mode == 2){
        tsec++;
        sec++;
        if (sec >= 60){ sec = 0; min++; }
        if (min >= 60){ min = 0; hour++; }
      }
      if (show_progress_mode < 3){
      if (first && show_progress_mode > 0) displayProgressHeader(0);

//      if (UDT::ERROR == UDT::perfmon(u, &perf))
//      {
//         cerr << "perfmon: " << UDT::getlasterror().getErrorMessage() << endl;
//         break;
//      }

      if (!first && !lf){ fprintf(stderr, "\r"); }
      if (!first &&  lf){ fprintf(stderr, "\n"); }
      if (show_progress_mode == 0) displayProgressSCP(
         perf.byteSendSize, 
         perf.rateSent, 
         hour, min, sec, eta, file_name);
      else displayProgressFooter(
         perf.mbpsSendRate, 
         perf.msRTT, 
         perf.pktCongestionWindow, 
         perf.usPktSndPeriod, 
         perf.pktRecvACK, 
         perf.pktRecvNAK, 
         perf.byteAvailSndBuf, 
         perf.rateSent, 
         hour, min, sec);
      }

      if (monitorfg == 0) break;
      if (perf.byteSendSize == perf.byteFileSize){
         tsec = -1;
         break;
      }

      if (show_progress_mode != 999 && stalled_continuously > 0){
        if (stalled_size + stalled_boundary >= perf.byteSendSize){
          stalled_sec--;
          if (show_progress_mode != 0 && show_progress_mode != 3) fprintf(stderr, "*");
          fflush(stderr);
          if (stalled_sec == 0){
            if (show_progress_mode != 3) fprintf(stderr, "\n");
            fprintf(stderr, "- stalled -\n");
            fflush(stderr);
            raise(SIGTERM);
            break;
          }
        }
        else {
          stalled_sec  = stalled_continuously;
        }
        stalled_size = perf.byteSendSize;
      }
   }
   return NULL;
}

void* monitor_r(void *s)
{
   UDTSOCKET u = *(UDTSOCKET*)s;

   UDT::TRACEINFO perf;

   int sec = 0;
   int min = 0;
   int hour = 0;
   int eta = 1;
   int lf = 0;
   int64_t rt;
   double rr;
   double sz;
   int     stalled_sec;
   int64_t stalled_size;

   if (show_progress_mode == 1) lf = 1; 
   sec = tsec;
   if (sec >= 60){
     min = (int)(sec / 60);
     sec = sec - min * 60;
   }
   if (min >= 60){
     hour = (int)(min / 60);
     min = min - hour * 60;
   }

   stalled_sec  = stalled_continuously;
   stalled_size = 0;
   for (int first = monitorfg;; first = 0)
   {
      if (monitorfg == 1) sleep(1);
      if (UDT::ERROR == UDT::perfmon(u, &perf))
      {
//         cerr << "perfmon: " << UDT::getlasterror().getErrorMessage() << endl;
         break;
      }
      if (show_progress_mode == 0){
        tsec++;
        sz = (double)perf.byteRecvSize;
        if (sz < 1.0) sz = 1.0;
        rt = (int64_t)((double)perf.byteFileSize / sz * tsec - tsec + 0.9);
        if (rt == 0){
          eta = 0;
          rt = tsec;
        }
        hour = (int)(rt / 3600);
        rt = rt - hour * 3600;
        min = (int)(rt / 60);
        sec = rt - min * 60;
        sz = (double)perf.byteRecvSize / tsec;
        if (hour > 99) hour = 99;
      }
      if (show_progress_mode == 1 || show_progress_mode == 2){
        tsec++;
        sec++;
        if (sec >= 60){ sec = 0; min++; }
        if (min >= 60){ min = 0; hour++; }
      }

      if (show_progress_mode < 3){
      if (first && show_progress_mode > 0) displayProgressHeader(1);

//      if (UDT::ERROR == UDT::perfmon(u, &perf))
//      {
//         cerr << "perfmon: " << UDT::getlasterror().getErrorMessage() << endl;
//         break;
//      }

      if (!first && !lf){ fprintf(stderr, "\r"); }
      if (!first &&  lf){ fprintf(stderr, "\n"); }
      if (show_progress_mode == 0) displayProgressSCP(
         perf.byteRecvSize, 
         perf.rateReceived, 
         hour, min, sec, eta, file_name);
      else displayProgressFooter(
         perf.mbpsRecvRate, 
         perf.msRTT, 
         perf.pktCongestionWindow, 
         perf.usPktSndPeriod, 
         perf.pktSentACK, 
         perf.pktSentNAK, 
         perf.byteAvailRcvBuf, 
         perf.rateReceived, 
         hour, min, sec);
      }

      if (monitorfg == 0) break;
      if (perf.byteRecvSize == perf.byteFileSize){
         tsec = -1;
         break;
      }

      if (show_progress_mode != 999 && stalled_continuously > 0){
        if (stalled_size + stalled_boundary >= perf.byteRecvSize){
          stalled_sec--;
          if (show_progress_mode != 0 && show_progress_mode != 3) fprintf(stderr, "*");
          fflush(stderr);
          if (stalled_sec == 0){
            if (show_progress_mode != 3) fprintf(stderr, "\n");
            fprintf(stderr, "- stalled -\n");
            fflush(stderr);
            raise(SIGTERM);
            break;
          }
        }
        else {
          stalled_sec  = stalled_continuously;
        }
        stalled_size = perf.byteRecvSize;
      }
   }
   return NULL;
}

