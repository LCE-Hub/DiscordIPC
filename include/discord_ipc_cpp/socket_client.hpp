/*
  Copyright 2026 Peter Duanmu

  You should have received a copy of the GNU General Public License along
  with discord_ipc_cpp. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DISCORD_IPC_CPP_INCLUDE_DISCORD_IPC_CPP_SOCKET_CLIENT_HPP_
#define DISCORD_IPC_CPP_INCLUDE_DISCORD_IPC_CPP_SOCKET_CLIENT_HPP_

#ifdef _WIN32
#include <windows.h>
#else
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#endif

#include <string>
#include <optional>
#include <vector>

/**
 * \namespace discord_ipc_cpp::websockets
 *
 * \brief Underlying websocket communications.
 *
 * Contains the lowest level class for interacting with any IPC socket.
 */
namespace discord_ipc_cpp::websockets {
/**
 * \brief Interacts with sockets.
 *
 * Lowest level and simplest class for interacting with any IPC socket. Does not
 * work with websockets surprisingly.
 *
 * \see discord_ipc_cpp::DiscordIPCClient
 */
class SocketClient {
 private:
  /**
   * \brief Path to the IPC file.
   */
  std::string _socket_file;

#ifdef _WIN32
  HANDLE _client_pipe;
#else
  /**
   * \brief Success of binding to IPC file.
   */
  int _client_socket;
  /**
   * \brief Address information of the IPC file.
   */
  struct sockaddr_un _server_addr;

  /**
   * \brief Poll events of the socket.
   */
  struct pollfd _fds[1];
#endif

 public:
  /**
   * \brief Creates socket connection precursor information.
   */
  explicit SocketClient(const std::string& socket_file);
  /**
   * \brief Cleans up socket the socket connection.
   *
   * \see close
   */
  ~SocketClient();

  /**
   * \brief Attempts to connect to the socket file.
   *
   * \return Success of opening connection to socket file.
   */
  bool connect();
  /**
   * \brief Attempts to close connection to socket file.
   *
   * \return Success of closing connection to socket file.
   */
  bool close();

  /**
   * \brief Sends data to socket.
   *
   * This will attempt to send a byte buffer of data to the socket file.
   *
   * \param data Data to send.
   *
   * \return Success of sending data.
   */
  bool send_data(const std::vector<char>& data);
  /**
   * \brief Receive data from socket.
   *
   * Attempts to retrieve data from the socket in a blocking manner. An empty
   * optional will be returned if an error occurred while retrieving data from
   * the socket.
   *
   * \param buffer_size Size of data to retrieve.
   *
   * \return Data from socket if successful.
   *
   * \see recv_data(int,int)
   */
  std::optional<std::vector<char>> recv_data(int buffer_size);
  /**
   * \brief Receive data from socket on timeout.
   *
   * Polls the socket for data in a blocking manner. If the socket contains data
   * that can be retrieved, \ref recv_data(int) will be called to retrieve the
   * data. If the poll times out, then an empty optional will be returned.
   *
   * \param buffer_size Size of data to retrieve.
   * \param timeout Time to wait.
   *
   * \return Data from socket if existant.
   *
   * \see recv_data(int)
   */
  std::optional<std::vector<char>> recv_data(int buffer_size, int timeout);
};
}  // namespace discord_ipc_cpp::websockets

#endif  // DISCORD_IPC_CPP_INCLUDE_DISCORD_IPC_CPP_SOCKET_CLIENT_HPP_
