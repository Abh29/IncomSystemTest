#ifndef __SERVER_EPOLL_HPP_
#define __SERVER_EPOLL_HPP_

/*
 * using epoll for Linux as it tends to be faster than select.
 */

#include <arpa/inet.h>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <memory>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unordered_map>
#include <vector>

#include "Client.h"
#include "IServer.h"
#include "Logger.h"

#define MAX_EVENTS 128

class Client;

class ServerEpoll : public IServer {

public:
  /*
   * @brief server constructor.
   *
   * @param localIP : the ip (ipv4) address of the client.
   * @param localPort : the port of the local (proxy) server.
   * @param remoteIP : the ip (ipv4) address of the remote server.
   * @param remotePort : the port of the remote server.
   * @param logger : the object responsible for logging the client state.
   *
   * @note initializes looping to true and logFile to null.
   */
  ServerEpoll(const std::string &localIp, const int localPort,
              const std::string &remoteIp, const int remotePort,
              const ClientLogger::pointer &logger);

  /*
   * @brief close the server socket and disconnects.
   */
  ~ServerEpoll();

  /*
   * @brief opens a listening socket (socket/bind/listen) and sets it to
   * NONBLOCK. then it creates an epoll instance with epoll_create1 and adds the
   * server socket to the epoll set of fds to be monitored.
   *
   * @throws InitException on error.
   */
  void init() override;

  /*
   * @brief works while _looping == true, uses epoll_wait with infinite time to
   * wait for io event to occur on one of the fds monitored by epoll if servSock
   * is ready for reading it accepts a new client then loops for each file
   * descriptor in the ep_events and checks if it is a client or connection
   * socket and accordingly to the client mode it performs a read/write to the
   * client/remoteServer finally it deletes the disconnected clients.
   *
   * throws ProcessingException on error.
   */
  void loop() override;

  /*
   * @brief sets looping parameter to false.
   */
  void stop() override;

  class InitException : public std::exception {
  private:
    std::string e;

  public:
    InitException();
    InitException(const char *e);
    virtual ~InitException() throw();
    virtual const char *what() const throw();
  };

  class ProcessingException : public std::exception {
  private:
    std::string e;

  public:
    ProcessingException();
    ProcessingException(const char *e);
    ProcessingException(const std::exception &err);
    virtual ~ProcessingException() throw();
    virtual const char *what() const throw();
  };

private:
  /*
   * @brief accepts a new connection to the server socket then creates a new
   *client object and adds it to the fdClientMap and connClientMap, then it adds
   *the client socket and the connection socket to the epoll set to be monitored
   *by epoll using epoll_ctl function, it monitors them for read and write by
   *setting the events mask to EPOLLIN | EPOLLOUT.
   *
   * @throws ProcessingException or InitException on error.
   */
  void acceptNewClient();

  /*
   * @brief loops through the clients list and deletes the disconnected client
   * and removes them from the epoll set.
   */
  void clearDisconnected();

  int _servSock;
  int _last_id;
  std::unordered_map<int, Client::pointer> _fdClientMap;
  std::unordered_map<int, Client::pointer> _connClientMap;
  sockaddr_in _servAddr;
  volatile bool _looping;
  ClientLogger::pointer _logger;
  int _epfd = -1; // epoll instance fd
  std::vector<epoll_event> _ep_events;
};

#endif
