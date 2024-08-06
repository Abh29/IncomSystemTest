#ifndef __CLIENT_HPP_
#define __CLIENT_HPP_

#include <deque>
#include <memory>
#include <vector>

#include "Connection.h"
class Connection;

#define BUFF_SIZE 8192

/*
 * @brief this represents a new connection from a client to the remote server.
 * this class holds amongst other things two sockets, one for the client for
 * proxy-client communication, and another one for proxy-remote server
 * communication.
 */
class Client {

public:
  using pointer = std::shared_ptr<Client>;
  using uniq_ptr = std::unique_ptr<Client>;
  using raw_ptr = Client *;

  enum class Mode {
    CLIENT_READ,  // read request from the client
    CLIENT_WRITE, // send response to client
    REMOTE_READ,  // receive query response from the remote server
    REMOTE_WRITE, // send query to the remote server
    OFF           // the client is offline
  };

  /*
   * @brief the client constructor.
   *
   * create connection object and initializes private values,
   * sets the mode to Client_read and the ID to -1.
   *
   * @param clientSock : the socket fd of the client.
   * @param localIP : the ip (ipv4) address of the client.
   * @param remoteIP : the ip (ipv4) address of the remote server.
   * @param remotePort : the port of the remote server.
   *
   * @throws connection error on failure.
   */

  Client(const int clientSock, const std::string &localIP,
         const std::string &remoteIP, const int remotePort);

  /*
   * closes the client socket and delete the connection object
   * also delete the buffer if there is still some content
   */
  ~Client();

  /*
   * @brief reads the request from the client through the client socket
   * the request is then saved in the buffer and the mode is changed to
   * Remote_write on success or off on failure.
   *
   * @throws ClientReadWriteException on error.
   */
  void readRequest();

  /*
   * @brief sends the content of the buffer back to the remote server through
   * the connection->send() function, the buffer is cleared and the mode is
   * changed to Remote_read on success or off on failure.
   *
   * @throws ClientReadWriteException on error.
   */
  void sendRequest();

  /*
   * @brief reads the response from the remote server through the
   * connection->receive() function the response is then saved in the buffer the
   * mode is changed to Client_write on success or off or fail.
   *
   * @throws Connection::ConnectionException on error.
   */
  void receiveResponse();

  /*
   * @brief sends the content of the buffer back to the client through the
   * client socket and saved to the buffer and the mode is changed to
   * Client_read on success or off .
   *
   * @throws ClientReadWriteException on error.
   */
  void sendResponse();

  /*
   * @brief reads the incoming response from the connection socket through
   * connection->receive() then send it directly to the client through
   * the client socket without saving to buffer.
   * used when the client socket is ready for writing and the connection socket
   * is ready for reading simultaneously
   * in case of received data bigger then the sent data the rest is saved in
   * buffer then receiveResponse() is called.
   *
   * the mode is changed to Client_read if all the data is sent, or Client_write
   * in case of buffering in the case of error off is set.
   *
   * @throws ClientReadWriteException or Connection::ConnectionException on
   * error.
   */
  void relay();

  /*
   * @brief the server is ready to read the request from the client.
   * @return true if mode == Client_read .
   */
  bool readyForRead() const;

  /*
   * @brief the server is ready to write the response to the client.
   * @return true if mode == Client_write
   */
  bool readyForWrite() const;

  /*
   * @brief  the server is ready to  write the query to the remote server
   * through the connection socket.
   * @return true if mode == Remote_write.
   */
  bool readyToQueryServer() const;

  /*
   * @brief the server is ready for reading the query response from the
   * connection socket.
   * @return true if mode == Remote_read | Client_read.
   */
  bool readyToReadServerResp() const;

  /*
   * @brief checks if the client is still connected.
   * @return true if the mode is different than off.
   */
  bool isConnected() const;

  /*
   * @return _clientSock.
   */
  int getClientSocket() const;

  /*
   * @return connection->getConnectionSocket().
   */
  int getRemoteSocket() const;

  /*
   * @brief buffer is a vector of chars that holds the incoming data related to
   * this client.
   *
   * @return _buffer.
   */
  const std::vector<char> &getBuffer() const;

  /*
   * @return localIp  (the ip (ipv4) address of the client).
   */
  std::string getIP() const;

  /*
   * @brief sets the id of a client.
   * @param id: the id of the client.
   */
  void setID(int id);

  /*
   * @returns _ID (the id of the client).
   */
  int getID() const;

  class ClientReadWriteException : public std::exception {
  private:
    std::string e;

  public:
    ClientReadWriteException();
    ClientReadWriteException(const char *e);
    virtual ~ClientReadWriteException() throw();
    virtual const char *what() const throw();
  };

private:
  int _clientSock = -1;
  std::string _localIP;
  std::string _remoteIP;
  int _remotePort;
  Mode _mode = Mode::OFF;
  Connection::uniq_ptr _connection;
  std::vector<char> _buffer, _tmpBuff;
  int _ID;
};

#endif //__CLIENT_HPP_
