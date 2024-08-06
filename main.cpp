#include "src/IServer.h"
#include "src/Logger.h"
#include "src/ServerImpEpoll.h"
#include <csignal>
#include <iostream>
#include <memory>

using ServerImp = ServerEpoll;

std::shared_ptr<IServer> g_server;

void sigHandler(int sig) {
  std::cout << "\nSignal " << sig << " received, stopping the server ..."
            << std::endl;
  g_server->stop();
}

int main(int argc, char **argv) {

  if (argc != 6) {
    std::cout << "./ProxyServer localIP localPort remoteIP remotePort logPath\n"
              << "localIP: is the ip (IPv4) of this server.\n"
              << "localPort: is the port for this server.\n"
              << "remoteIP: is the ip (IPv4) for the postgresql server.\n"
              << "remotePort: is the port for the postgresql server.\n"
              << "logPath: is the path for the log file." << std::endl;
    return 1;
  }

  signal(SIGPIPE, SIG_IGN);
  signal(SIGTERM, sigHandler);
  signal(SIGINT, sigHandler);
  signal(SIGQUIT, sigHandler);

  std::string localIP(argv[1]);
  std::string remoteIP(argv[3]);
  std::string logPath(argv[5]);
  int localPort = atoi(argv[2]);
  int remotePort = atoi(argv[4]);

  try {

    ClientLogger::pointer logger = std::make_shared<FileQueryLogger>(logPath);

    g_server = std::make_shared<ServerImp>(localIP, localPort, remoteIP,
                                           remotePort, logger);

    std::cout << "init ..." << std::endl;
    g_server->init();
    std::cout << "loop ..." << std::endl;
    g_server->loop();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  std::cout << "bye!" << std::endl;
  return 0;
}
