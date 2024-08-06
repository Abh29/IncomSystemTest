#include "Logger.h"
#include "Client.h"
#include <cerrno>
#include <cstring>
#include <fstream>
#include <ios>
#include <string>
#include <vector>

FileQueryLogger::FileQueryLogger(const std::string &filePath, const bool append)
    : _filePath(filePath) {

  if (append)
    _outStream.open(_filePath, std::ios_base::app);
  else
    _outStream.open(_filePath);

  if (!_outStream.is_open() || _outStream.fail())
    throw std::ios_base::failure(strerror(errno));

  fillMessageTypes();
}

void FileQueryLogger::log(const Client::pointer &c) {

  if (c->getBuffer().empty())
    return;

  auto &tmp = c->getBuffer();
  auto qtype = _messageTypes.find(tmp[0]);

  if (qtype == _messageTypes.end())
    return;

  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);

  _outStream << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d\t%X")
             << "\t\t-\tIP: " << c->getIP() << "\t-\tclient " << c->getID()
             << ": (" << qtype->second << ")\t\t"
             << std::string(tmp.begin() + 5, tmp.end()) << "\n";

  if (_outStream.fail())
    throw std::ios_base::failure(strerror(errno));
}

std::string FileQueryLogger::getFilePath() const { return _filePath; }

void FileQueryLogger::fillMessageTypes() {
  _messageTypes.insert(std::make_pair<char, std::string>('Q', "simple query"));
  _messageTypes.insert(
      std::make_pair<char, std::string>('B', "extended query bind"));
  _messageTypes.insert(
      std::make_pair<char, std::string>('P', "extended query parse"));
  _messageTypes.insert(
      std::make_pair<char, std::string>('D', "extended query describe"));
  _messageTypes.insert(
      std::make_pair<char, std::string>('E', "extended query execute"));
  _messageTypes.insert(
      std::make_pair<char, std::string>('C', "extended query close"));
  _messageTypes.insert(
      std::make_pair<char, std::string>('F', "extended function call"));
}
