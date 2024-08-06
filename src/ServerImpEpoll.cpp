#include "ServerImpEpoll.h"
#include "IServer.h"
#include <ctime>
#include <string>

ServerEpoll::ServerEpoll(const std::string &localIp, const int localPort,
                         const std::string &remoteIp, const int remotePort,
                         const ClientLogger::pointer &logger)
    : IServer(localIp, localPort, remoteIp, remotePort, logger), _servAddr{} {
  std::cout << "Epoll server !" << std::endl;
  _last_id = 0;
  _looping = true;
  _logger = logger;
};

ServerEpoll::~ServerEpoll() {
  if (_epfd != -1)
    close(_epfd);
}

void ServerEpoll::init() {

  // listening socket
  _servAddr.sin_port = htons(_localPort);
  _servAddr.sin_family = AF_INET;
  if (!inet_aton(_localIP.c_str(), &_servAddr.sin_addr))
    throw InitException((char *)"Invalid localIP address !\n");
  if ((_servSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    throw InitException(strerror(errno));
  int flags = fcntl(_servSock, F_GETFD);
  flags |= O_NONBLOCK;
  fcntl(_servSock, F_SETFD, flags);
  if (bind(_servSock, (const struct sockaddr *)&_servAddr, sizeof(_servAddr)) <
      0)
    throw InitException(strerror(errno));
  if (listen(_servSock, 0) < 0)
    throw InitException(strerror(errno));

  // epoll
  if ((_epfd = epoll_create1(0)) < 0)
    throw InitException(strerror(errno));

  epoll_event ev; // epoll events
  ev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
  ev.data.fd = _servSock;
  if (epoll_ctl(_epfd, EPOLL_CTL_ADD, _servSock, &ev) < 0)
    throw InitException(strerror(errno));

  // allocate memory for epoll events
  _ep_events.resize(MAX_EVENTS);
}

void ServerEpoll::stop() { _looping = false; }

void ServerEpoll::loop() {
  int nfds = 0;

  while (_looping) {

    // poll the sockets
    if ((nfds = epoll_wait(_epfd, _ep_events.data(), MAX_EVENTS, -1)) < 0)
      throw ProcessingException(strerror(errno));

    for (int i = 0; i < nfds; ++i) {
      // if there is an event from the proxy server socket
      if (_ep_events[i].data.fd == _servSock) {
        if ((_ep_events[i].events & EPOLLIN) == EPOLLIN)
          acceptNewClient();
        else if ((_ep_events[i].events & (EPOLLRDHUP | EPOLLERR | EPOLLHUP)) ==
                 (EPOLLRDHUP | EPOLLERR | EPOLLHUP))
          throw ProcessingException("Server socket error !");

      } else {
        // the event is either from a client or the remote server
        if ((_ep_events[i].events & EPOLLIN) == EPOLLIN) {
          // if the socket is ready to read
          std::unordered_map<int, Client::pointer>::iterator it;
          int fd = _ep_events[i].data.fd;

          if ((it = _fdClientMap.find(fd)) != _fdClientMap.end() &&
              it->second->readyForRead()) {
            // if the event came from a client socket
            it->second->readRequest();
            _logger->log(it->second);

          } else if ((it = _connClientMap.find(fd)) != _connClientMap.end() &&
                     it->second->readyToReadServerResp()) {
            // else if event came from a remote server's socket
            it->second->receiveResponse();
          }

        } else if ((_ep_events[i].events & EPOLLOUT) == EPOLLOUT) {
          // if the socket is ready to write
          std::unordered_map<int, Client::pointer>::iterator it;
          int fd = _ep_events[i].data.fd;

          if ((it = _fdClientMap.find(fd)) != _fdClientMap.end() &&
              it->second->readyForWrite()) {
            // if event from client socket
            it->second->sendResponse();

          } else if ((it = _connClientMap.find(fd)) != _connClientMap.end() &&
                     it->second->readyToQueryServer()) {
            // else if event from the remove server socker
            it->second->sendRequest();
          }
        }
      }
    }

    // clear diconnected clients
    clearDisconnected();
  }
}

void ServerEpoll::acceptNewClient() {
  sockaddr_in clt;
  unsigned int len = sizeof(clt);
  char c_ip[255] = {0};

  // create a client
  int fd = accept(_servSock, (sockaddr *)&clt, &len);
  if (fd < 0)
    throw InitException(strerror(errno));
  inet_ntop(AF_INET, &(clt.sin_addr), c_ip, 255);
  try {
    std::string ip(c_ip);
    auto c = std::make_shared<Client>(fd, ip, _remoteIP, _remotePort);
    _fdClientMap[c->getClientSocket()] = c;
    _connClientMap[c->getRemoteSocket()] = c;

    c->setID(++_last_id);
    std::cout << "client from address " << ip << " with id = " << c->getID()
              << " : is added" << std::endl;

    // add fds to epoll set
    epoll_event ev; // epoll events
    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.fd = c->getClientSocket();
    if (epoll_ctl(_epfd, EPOLL_CTL_ADD, c->getClientSocket(), &ev) < 0)
      throw ProcessingException(
          (char *)"Could not add the new client socket to the epoll set !");

    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.fd = c->getRemoteSocket();
    if (epoll_ctl(_epfd, EPOLL_CTL_ADD, c->getRemoteSocket(), &ev) < 0)
      throw ProcessingException(
          (char *)"Could not add the new connection socket to the epoll set !");

  } catch (const Connection::ConnectionException &e) {
    std::cerr << "Could not opent a connection with the remote server!"
              << std::endl;
    throw ServerEpoll::ProcessingException(e.what());
  } catch (const std::exception &) {
    throw ProcessingException((char *)"Could not add a new Client !");
  }
}

void ServerEpoll::clearDisconnected() {

  for (auto it = _fdClientMap.begin(); it != _fdClientMap.end();) {
    auto c = it->second;
    if (!c->isConnected()) {
      std::cout << "client from address " << c->getIP()
                << " with id = " << c->getID() << " : is disconnected !"
                << std::endl;

      if (epoll_ctl(_epfd, EPOLL_CTL_DEL, c->getClientSocket(), NULL) < 0)
        throw ProcessingException(
            (char *)"Could not delete the client socket from the epoll set !");

      if (epoll_ctl(_epfd, EPOLL_CTL_DEL, c->getRemoteSocket(), NULL) < 0)
        throw ProcessingException((char *)"Could not delete the connection "
                                          "socket from the epoll set !");

      auto p = _connClientMap.find(c->getRemoteSocket());
      _connClientMap.erase(p);
      it = _fdClientMap.erase(it);
    } else
      ++it;
  }
}

ServerEpoll::InitException::InitException() {
  e = std::string("An Error occurred while initializing the server!");
}
ServerEpoll::InitException::InitException(const char *e) : e(e) {}

ServerEpoll::InitException::~InitException() throw(){};

const char *ServerEpoll::InitException::what() const throw() {
  return e.c_str();
}

ServerEpoll::ProcessingException::ProcessingException() {
  e = std::string("An Error occurred while running the server!");
}
ServerEpoll::ProcessingException::ProcessingException(const char *e) : e(e) {}

ServerEpoll::ProcessingException::~ProcessingException() throw(){};

const char *ServerEpoll::ProcessingException::what() const throw() {
  return e.c_str();
}
