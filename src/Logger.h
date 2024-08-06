#ifndef __LOGGER_HPP_
#define __LOGGER_HPP_

#include "Client.h"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

class ClientLogger {

public:
  using pointer = std::shared_ptr<ClientLogger>;
  using uniq_ptr = std::unique_ptr<ClientLogger>;

  ClientLogger() = default;
  virtual ~ClientLogger() = default;

  virtual void log(const Client::pointer &c) = 0;
};

/*
 * This is an implementation for the client logger interface
 * that reads the content of the buffer inside the client and
 * determines if it is a query to then be written to a file.
 *
 */

class FileQueryLogger : public ClientLogger {

public:
  /*
   * @brief construct a file logger.
   *
   * @param filePath a const string representing the path to the logging file.
   * @param append a const boolean determining the mode of opening the file.
   *
   * @thorws std::ios_base::failure if the file stream failed to open or write.
   *
   */
  FileQueryLogger(const std::string &filePath, const bool append = true);

  /*
   * default desctructor.
   */
  ~FileQueryLogger() = default;

  /*
   * @brief writes the query from the client buffer to the log file.
   *
   * this log function uses std::ofstream to write to the file, it works in
   * the same thread (does not handle the logging in a separate thread and does
   * not use async).
   *
   * @param c a pointer (shared pointer) to a client.
   *
   * @throws upon failure to write to the file this function throws a
   * std::ios_base::failure.
   */

  void log(const Client::pointer &c) override;

  /*
   * @brief _filepath getter.
   * @return the path of the logging file.
   */
  std::string getFilePath() const;

private:
  /*
   * messageTypes contains a char which is the first bite of a received request
   * from the client and maps it to a string used in logging this function fills
   * messageTypes map with predefined char values and their meaning
   * https://www.postgresql.org/docs/current/protocol-message-formats.html
   * we can add more entries to this function to log other messages (commands,
   * copy ...)
   */
  void fillMessageTypes();

  std::string _filePath;
  std::ofstream _outStream;
  std::map<char, std::string> _messageTypes;
};

#endif // !__LOGGER_HPP_
