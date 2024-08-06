#ifndef __I_SERVER_HPP_
#define __I_SERVER_HPP_

#include "Logger.h"
#include <string>

class IServer {

public:
  /*
   * @param localIP : the ip (ipv4) address of the client
   * @param localPort : the port of the local (proxy) server
   * @param remoteIP : the ip (ipv4) address of the remote server
   * @param remotePort : the port of the remote server
   * @param clientLogger : the object responsible of logging the client
   */

  IServer(const std::string &localIP, const int localPort,
          const std::string &remoteIP, const int remotePort,
          const ClientLogger::pointer &logger)
      : _localIP(localIP), _localPort(localPort), _remoteIP(remoteIP),
        _remotePort(remotePort), _logger(logger) {}

  /*
   * destructor for the server
   */
  virtual ~IServer() = default;

  /*
   * initializes the server
   */
  virtual void init() = 0;

  /*
   * the main loop of the server
   */
  virtual void loop() = 0;

  /*
   * stops the server's loop
   */
  virtual void stop() = 0;

protected:
  std::string _localIP;
  uint32_t _localPort;
  std::string _remoteIP;
  uint32_t _remotePort;
  ClientLogger::pointer _logger;
};

#endif // __I_SERVER_HPP_
