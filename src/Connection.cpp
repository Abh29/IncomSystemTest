#include "Connection.h"
#include <arpa/inet.h>
#include <string>
#include <sys/socket.h>

Connection::Connection(const std::string &connIP, const int connPort)
    : _connIP(connIP), _connPort(connPort) {

  _connAddr.sin_port = htons(_connPort);
  _connAddr.sin_family = AF_INET;
  if (!inet_aton(_connIP.c_str(), &_connAddr.sin_addr))
    throw ConnectionException(strerror(errno));
  if ((_connSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    throw ConnectionException(strerror(errno));
  int flags = fcntl(_connSock, F_GETFD);
  flags |= O_NONBLOCK;
  fcntl(_connSock, F_SETFD, flags);
  if (connect(_connSock, (const sockaddr *)&_connAddr, sizeof(_connAddr)) < 0)
    throw ConnectionException(strerror(errno));
}

Connection::Connection(Connection &&other)
    : _connIP(std::move(other._connIP)), _connPort(other._connPort),
      _connSock(other._connSock), _connAddr(std::move(other._connAddr)) {
  other._connSock = -1;
}

Connection::~Connection() {
  if (_connSock != -1)
    close(_connSock);
}

long Connection::send(const std::vector<char> &buff) const {
  long out = ::send(_connSock, buff.data(), buff.size(), 0);
  return out;
};

long Connection::receive(std::vector<char> &buff, size_t maxLen) const {
  buff.resize(maxLen);
  long out = recv(_connSock, buff.data(), maxLen, 0);
  return out;
}

int Connection::getConnectionSocket() const { return _connSock; }

Connection::ConnectionException::ConnectionException() {
  e = "Error while initializing a Connection !";
}
Connection::ConnectionException::ConnectionException(const char *e) : e(e) {}
Connection::ConnectionException::~ConnectionException() throw() {}

const char *Connection::ConnectionException::what() const throw() {
  return e.c_str();
}
