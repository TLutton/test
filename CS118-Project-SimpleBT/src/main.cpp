/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014,  Regents of the University of California
 *
 * This file is part of Simple BT.
 * See AUTHORS.md for complete list of Simple BT authors and contributors.
 *
 * NSL is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NSL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NSL, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author Yingdi Yu <yingdi@cs.ucla.edu>
 */

#include "client.hpp"
#include "meta-info.hpp"
#include "http/http-request.hpp"
#include "http/url-encoding.hpp"
#include "util/bencoding.hpp"
#include "http/http-response.hpp"
#include <fstream>
#include "common.hpp"
#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <netinet/in.h>
#include "tracker-response.hpp"
#include <climits>
using namespace sbt;

#define PEERLEN 20

int
main(int argc, char** argv)
{
  try
  {
    // Check command line arguments.
    if (argc != 3)
    {
      std::cerr << "Usage: simple-bt <port> <torrent_file>\n";
      return 1;
    }
    
        std::ifstream ifs (argv[2], std::ifstream::in);

        MetaInfo mi;
        mi.wireDecode(ifs);
        ConstBufferPtr cbp = mi.getHash();
        std::vector<uint8_t> v = *cbp;

        std::vector<uint8_t> vp;
        uint8_t it = 0;
        while (it < PEERLEN) {
                vp.push_back(it++);
        }

        std::string encodedHash = url::encode(cbp->get(), v.size());
        std::string encodedPeer = url::encode(&vp.front(), vp.size());
        //std:: << "encodedHash: " + encodedHash << std::endl;
        //std::cout << "encodedPeer: " + encodedPeer << std::endl;
        
        //std::cout << mi.getAnnounce() << " announcement\n";
        std::size_t pos = mi.getAnnounce().find("//");
        std::string host = mi.getAnnounce().substr(pos+2);
        std::string port = host.substr(host.find(":")+1);
        std::string location = port;
        location = location.substr(port.find("/"));
        port = port.substr(0, port.find("/"));
        pos = host.find(":");
        host = host.substr(0, pos);
        //std::cout << host << " HOST " << port << "PORT \n";

        // becker (client port stuff)
        /*int sfd = socket(AF_INET, SOCK_STREAM, 0);
        
        int yes = 1;
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
                perror("setsockopt");
                return 1;
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(std::atoi(argv[1]));
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

        if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        {
                perror("bind");
                return 2;
        }       

        if (listen(sfd, 1) == -1)
        {
                perror("listen");
                return 3;
        }
        
        //struct sockaddr_in clientA;
        //socklen_t clientASize;
        //int clientSfd = accepted(sfd, (struct sockaddr*)&clientA, &clientASize);
        */
        ///////

        HttpRequest req;
        req.setHost(host);
        req.setPort(std::stoi(port));
        req.setMethod(HttpRequest::GET);
        req.setPath(location+"?info_hash=" + encodedHash+"&peer_id="+encodedPeer+"&port="+argv[1]+"&uploaded=111&downloaded=1112&left=300&event=started"); //this needs to be the query
        req.setVersion("1.0");
        req.addHeader("Accept-Language", "en-US");
        std::size_t reqLen = req.getTotalLength();
        char* buf = new char[reqLen];
        req.formatRequest(buf);

        
        //resolve host name to IP
         struct addrinfo hints;
          struct addrinfo* res;


          // prepare hints
          memset(&hints, 0, sizeof(hints));
          hints.ai_family = AF_INET; // IPv4
          hints.ai_socktype = SOCK_STREAM;

          // get address
          int status = 0;
          if ((status = getaddrinfo(host.c_str(), port.c_str(), &hints, &res)) != 0) {
            std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
            return 2;
          }

          //std::cout << "IP addresses for " << host << std::endl;

         struct addrinfo* p = res;
         if(p==0)
                return 2;
            // convert address to IPv4 address
          struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;

           // convert the IP to a string and print it:
          char ipstr[INET_ADDRSTRLEN] = {'\0'};
          inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
          // std::cout << "  " << ipstr << std::endl;
          //std::cout << "  " << ipstr << " is the resolv ed ip"<< std::endl;
          

          freeaddrinfo(res); // free the linked list
         int sockfd = socket(AF_INET, SOCK_STREAM, 0);          
        struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(std::stoi(port));     // short, network byte order
  serverAddr.sin_addr.s_addr = inet_addr(ipstr);
  // connect to the server
  if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
    perror("connect");
    return 2;
  }

  struct sockaddr_in clientAddr;

  socklen_t clientAddrLen = sizeof(clientAddr);
  if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
    perror("getsockname");
    return 3;
  }

  char ipstr1[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr1, sizeof(ipstr1));
  //std::cout << "Set up a connection from: " << ipstr1 << ":" <<
    //ntohs(clientAddr.sin_port) << std::endl;


  // send/receive data to/from connection
 // bool isEnd = false;
  std::string input;
  char buf1[20000] = {0};
  std::stringstream ss;


    memset(buf1, '\0', sizeof(buf1));

    
    if (send(sockfd, buf, req.getTotalLength(), 0) == -1) {
      perror("send");
      return 4;
    }


    if (recv(sockfd, buf1, 20000, 0) == -1) {
      perror("recv");
      return 5;
    }

  close(sockfd);
    HttpResponse hres;
    hres.parseResponse(buf1, 20000);
    //std::cout << hres.getStatusMsg() << std::endl;
    char* body = strstr(buf1, "\r\n\r\n")+4;
    std::string bodys = body;
    std::istringstream iss(bodys);
    sbt::bencoding::Dictionary theD;
    theD.wireDecode(iss);
    TrackerResponse tr;
        tr.decode(theD);
    std::vector<PeerInfo> pi = tr.getPeers();
        for (PeerInfo i : pi)
        {
                std::cout << i.ip + ":" << i.port << std::endl;
        } 
    //std::cout << "test: " << bodys << std::endl;
    // Initialise the client.
    sbt::Client client(argv[1], argv[2]);
  }
  catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}
