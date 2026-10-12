/*
  Copyright 2026 Peter Duanmu

  You should have received a copy of the GNU General Public License along
  with discord_ipc_cpp. If not, see <https://www.gnu.org/licenses/>.
*/

#include "discord_ipc_cpp/socket_client.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <cstring>
#include <string>
#include <optional>
#include <vector>

namespace discord_ipc_cpp::websockets {
SocketClient::SocketClient(
  const std::string& socket_file) :
_socket_file(socket_file) {
#ifdef _WIN32
  _client_pipe = INVALID_HANDLE_VALUE;
#else
  _client_socket = -1;
#endif
}

SocketClient::~SocketClient() {
  close();
}

#ifdef _WIN32
bool SocketClient::connect() {
  if (_client_pipe != INVALID_HANDLE_VALUE) {
    close();
  }

  _client_pipe = ::CreateFileA(
    _socket_file.c_str(),
    GENERIC_READ | GENERIC_WRITE,
    0,
    nullptr,
    OPEN_EXISTING,
    0,
    nullptr);

  return _client_pipe != INVALID_HANDLE_VALUE;
}

bool SocketClient::close() {
  if (_client_pipe != INVALID_HANDLE_VALUE) {
    ::CloseHandle(_client_pipe);
    _client_pipe = INVALID_HANDLE_VALUE;
    return true;
  } else {
    return false;
  }
}

bool SocketClient::send_data(const std::vector<char>& data) {
  if (_client_pipe == INVALID_HANDLE_VALUE) {
    return false;
  }

  DWORD written = 0;
  BOOL ret = ::WriteFile(_client_pipe,
                         data.data(),
                         static_cast<DWORD>(data.size()),
                         &written,
                         nullptr);

  return ret && written == data.size();
}

std::optional<std::vector<char>> SocketClient::recv_data(int buffer_size) {
  if (_client_pipe == INVALID_HANDLE_VALUE) {
    return std::nullopt;
  }

  std::vector<char> buffer(buffer_size);
  DWORD total = 0;
  while (total < static_cast<DWORD>(buffer_size)) {
    DWORD read = 0;
    BOOL ret = ::ReadFile(_client_pipe,
                          buffer.data() + total,
                          static_cast<DWORD>(buffer_size) - total,
                          &read,
                          nullptr);

    if (!ret || read == 0) {
      return std::nullopt;
    }

    total += read;
  }

  return buffer;
}

std::optional<std::vector<char>> SocketClient::recv_data(
  int buffer_size, int timeout
) {
  DWORD start = ::GetTickCount();
  while (true) {
    DWORD available = 0;
    if (!::PeekNamedPipe(_client_pipe,
                         nullptr,
                         0,
                         nullptr,
                         &available,
                         nullptr)) {
      return std::nullopt;
    }

    if (available > 0) {
      return recv_data(buffer_size);
    }

    if (timeout >= 0 &&
        ::GetTickCount() - start >= static_cast<DWORD>(timeout)) {
      return std::nullopt;
    }

    ::Sleep(10);
  }
}
#else
bool SocketClient::connect() {
  if (_client_socket >= 0) {
    close();
  }

  int opt = 1;

  _client_socket = ::socket(AF_UNIX, SOCK_STREAM, 0);
  setsockopt(_client_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  std::memset(&_server_addr, 0, sizeof(_server_addr));

  _server_addr.sun_family = AF_UNIX;
  std::strncpy(_server_addr.sun_path,
               _socket_file.c_str(),
               sizeof(_server_addr.sun_path) - 1);

  _fds[0].fd = _client_socket;
  _fds[0].events = POLLIN;

  if (_client_socket < 0) {
    return false;
  }

  int ret = ::connect(_client_socket,
                      reinterpret_cast<const sockaddr*>(&_server_addr),
                      sizeof(sockaddr_un));

  return ret != -1;
}

bool SocketClient::close() {
  if (_client_socket >= 0) {
    ::close(_client_socket);

    _client_socket = -1;

    return true;
  } else {
    return false;
  }
}

bool SocketClient::send_data(const std::vector<char>& data) {
  if (_client_socket < 0) {
    return false;
  }

  int ret = ::send(_client_socket, data.data(), data.size(), 0);

  return ret != -1;
}

std::optional<std::vector<char>> SocketClient::recv_data(int buffer_size) {
  std::vector<char> buffer(buffer_size);

  int ret = ::recv(_client_socket, buffer.data(), buffer.size(), 0);

  if (ret == -1) {
    return std::nullopt;
  }

  return buffer;
}

std::optional<std::vector<char>> SocketClient::recv_data(
  int buffer_size, int timeout
) {
  int ret = ::poll(_fds, 1, timeout);

  if (ret > 0 && _fds[0].revents & POLLIN) {
    return recv_data(buffer_size);
  } else {
    return std::nullopt;
  }
}
#endif
}  // namespace discord_ipc_cpp::websockets
