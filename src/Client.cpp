#include "Client.h"
#include <algorithm>
#include <cstddef>

Client::Client(const int clientSock, const std::string &localIP,
               const std::string &remoteIP, const int remotePort)
    : _clientSock(clientSock), _localIP(localIP), _remoteIP(remoteIP),
      _remotePort(remotePort) {

  _connection = std::make_unique<Connection>(_remoteIP, _remotePort);
  _mode = Client::Mode::CLIENT_READ;
  _ID = -1;
  _tmpBuff.resize(BUFF_SIZE);
}

Client::~Client() {
  if (_clientSock != -1)
    close(_clientSock);
}

bool Client::isConnected() const { return _mode != Mode::OFF; }

bool Client::readyForRead() const {
  return _mode == Mode::CLIENT_READ || _mode == Mode::REMOTE_READ;
}

bool Client::readyForWrite() const { return _mode == Mode::CLIENT_WRITE; }

bool Client::readyToQueryServer() const { return _mode == Mode::REMOTE_WRITE; }

bool Client::readyToReadServerResp() const {
  return _mode == Mode::REMOTE_READ || _mode == Mode::CLIENT_READ;
}

int Client::getClientSocket() const { return _clientSock; }

int Client::getRemoteSocket() const {
  return _connection->getConnectionSocket();
}

const std::vector<char> &Client::getBuffer() const { return _buffer; };

void Client::readRequest() {
  long len = 0;

  while ((len = recv(_clientSock, _tmpBuff.data(), BUFF_SIZE, 0)) > 0) {
    _buffer.insert(_buffer.end(), _tmpBuff.begin(), _tmpBuff.begin() + len);
    if (len < BUFF_SIZE)
      break;
  }

  if (len < 0) {
    _mode = Mode::OFF;
    throw ClientReadWriteException(strerror(errno));
  } else if (len == 0) {
    _mode = Mode::OFF;
  } else
    _mode = Mode::REMOTE_WRITE;
}

void Client::sendRequest() {
  long len = 0;

  if (!_buffer.empty())
    len = _connection->send(_buffer);

  if (len < 0) {
    _mode = Mode::OFF;
    throw Connection::ConnectionException(strerror(errno));
  }

  if (static_cast<std::size_t>(len) < _buffer.size()) {
    // in case not all the content of the buffer was sent due to some reason!
    std::rotate(_buffer.begin(), _buffer.begin() + len, _buffer.end());
    _buffer.resize(_buffer.size() - len);

  } else {
    // all the content of the buffer was sent to the remote server
    _buffer.clear();
    _mode = Mode::REMOTE_READ;
  }
}

void Client::receiveResponse() {
  long len;

  while ((len = _connection->receive(_tmpBuff, BUFF_SIZE)) > 0) {
    _buffer.insert(_buffer.end(), _tmpBuff.begin(), _tmpBuff.begin() + len);
    if (len < BUFF_SIZE)
      break;
  }

  if (len < 0) {
    _mode = Mode::OFF;
    throw Connection::ConnectionException(strerror(errno));
  } else if (len == 0) {
    _mode = Mode::OFF;
  } else
    _mode = Mode::CLIENT_WRITE;
}

void Client::sendResponse() {
  long len = 0;
  if (!_buffer.empty())
    len = send(_clientSock, _buffer.data(), _buffer.size(), 0);

  if (len < 0) {
    _mode = Mode::OFF;
    throw ClientReadWriteException(strerror(errno));
  }

  if (static_cast<std::size_t>(len) < _buffer.size()) {
    // in case not all the content of the buffer was sent due to some reason!
    std::rotate(_buffer.begin(), _buffer.begin() + len, _buffer.end());
    _buffer.resize(_buffer.size() - len);

  } else {
    // all the content of the buffer was sent to the client
    _buffer.clear();
    _mode = Mode::CLIENT_READ;
  }
}

void Client::relay() {
  long lenr, lenw;

  // read from the remote server
  while ((lenr = _connection->receive(_tmpBuff, BUFF_SIZE)) > 0) {

    // write directly to the client
    if ((lenw = send(_clientSock, _tmpBuff.data(), lenr, 0)) < 0)
      throw ClientReadWriteException(strerror(errno));

    // if the data written is less that was read
    if (lenw < lenr) {
      // buffering instead
      _buffer.insert(_buffer.end(), _tmpBuff.begin() + lenw,
                     _tmpBuff.begin() + lenr);

      // if all the data was read from the server
      if (lenr < BUFF_SIZE)
        _mode = Mode::CLIENT_WRITE;
      else // there is still data to be read from the server
        receiveResponse();
      return;
    }

    if (lenr < BUFF_SIZE)
      break;
  }

  if (lenr < 0) {
    _mode = Mode::OFF;
    throw Connection::ConnectionException(strerror(errno));
  } else if (lenr == 0) {
    _mode = Mode::OFF;
  } else // all the data was read from the server and written to the client
    _mode = Mode::CLIENT_READ;
}

std::string Client::getIP() const { return _localIP; }

int Client::getID() const { return _ID; }

void Client::setID(int id) { _ID = id; }

Client::ClientReadWriteException::ClientReadWriteException()
    : e("Error while reading/writing to client !") {}

Client::ClientReadWriteException::ClientReadWriteException(const char *e)
    : e(e){};

Client::ClientReadWriteException::~ClientReadWriteException() throw() {}

const char *Client::ClientReadWriteException::what() const throw() {
  return e.c_str();
}
