#ifndef __CONNECTION_HPP_
#define __CONNECTION_HPP_

#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

class Connection {

public:
  using raw_ptr = Connection *;
  using uniq_ptr = std::unique_ptr<Connection>;
  /*
   * @brief opens a socket to connect to the remote server.
   *
   * @param connIP: is the ip (ipv4) of the remote server.
   * @param connPort: is the remote server port.
   *
   * throws ConnectionException on error
   */
  Connection(const std::string &connIP, const int connPort);

  /*
   * @brief The object from this class is not copyable.
   */
  Connection(const Connection &other) = delete;

  /*
   * @brief The object from this class is however movable.
   */
  Connection(Connection &&other);

  /*
   * @brief close the socket.
   */
  ~Connection();

  /*
   * @brief send a buffer of chars through the socket.
   *
   * tries to write buff.size() bytes from buff to the socket using send
   * function from <sys/socket.h>
   *
   * @param buff vector or chars is the buffer which content is to be written
   *
   * returns the number of bytes sent, or -1 on error
   */
  long send(const std::vector<char> &buff) const;

  /*
   * @brief receives data from the server socket and writes it to buff.
   *
   * tries to read at most maxlen bytes of data using recv and writes it to the
   * buff, note that this functions does not change the size of the vector nor
   * it does allocate memory for the content, this should be the responsibility
   * of the caller.
   *
   * @param buff: is the buffer to which content will be written
   * @param maxLen: is the maximum number to read from the socket
   *
   * returns the number of bytes received, or -1 on error
   */
  long receive(std::vector<char> &buff, size_t maxLen) const;

  /*
   * returns _connSock
   */
  int getConnectionSocket() const;

  /*
   * @brief in case of error while reading/writing to the socket
   * this exception is thrown.
   */

  class ConnectionException : public std::exception {
  private:
    std::string e;

  public:
    ConnectionException();
    ConnectionException(const char *e);
    virtual ~ConnectionException() throw();
    virtual const char *what() const throw();
  };

private:
  std::string _connIP;
  int _connPort;
  int _connSock = -1;
  sockaddr_in _connAddr;
};

#endif //__CONNECTION_HPP_
