/*
  Copyright 2026 Peter Duanmu

  You should have received a copy of the GNU General Public License along
  with discord_ipc_cpp. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DISCORD_IPC_CPP_INCLUDE_DISCORD_IPC_CPP_DISCORD_IPC_CLIENT_HPP_
#define DISCORD_IPC_CPP_INCLUDE_DISCORD_IPC_CPP_DISCORD_IPC_CLIENT_HPP_

#include <thread>
#include <string>
#include <optional>
#include <vector>

#include "discord_ipc_cpp/socket_client.hpp"
#include "discord_ipc_cpp/ipc_types.hpp"
#include "discord_ipc_cpp/json.hpp"

/**
 * \namespace discord_ipc_cpp
 *
 * \brief High level namespace for the \c discord_ipc_cpp package.
 *
 * Contains all the necessary classes to interact with Discord's IPC socket,
 * with other helpful classes neatly categorized.
 */
namespace discord_ipc_cpp {
/**
 * \brief Represents a connection to Discord's IPC socket.
 *
 * This class is the highest level and least-customizable class used to interact
 * with Discord's IPC socket. It is specially designed to handle connections
 * with the purpose of sending rich presence updates.
 *
 * \see discord_ipc_cpp::websockets::SocketClient
 */
class DiscordIPCClient {
 private:
  /**
   * \brief Stores the executable's process ID.
   *
   * Each rich presence is required to have an attached process ID to
   * distinguish between different applications.
   */
  const int _pid;
  /**
   * \brief ID of the application.
   *
   * This points to the Discord application that the rich presence is sent from.
   */
  std::string _client_id;

  /**
   * \brief Underlying socket connection.
   *
   * This is the actual class that handles connections to Discord's IPC socket.
   *
   * \see discord_ipc_cpp::websockets::SocketClient
   */
  websockets::SocketClient _socket;
  /**
   * \brief Handles incoming packets.
   *
   * This thread handles incoming packets sent by Discord through the IPC
   * socket.
   *
   * \see recv_thread
   */
  std::jthread _socket_recv_thread;

  /**
   * \brief Indicates successful authentication with Discord socket.
   *
   * This variable ensures that packets are only sent after successful
   * authentication with Discord's IPC socket. Otherwise, each \ref send_packet
   * request will automatically return \c false.
   */
  std::atomic_bool _successful_auth;

 private:
  /**
   * \brief Encodes payload into byte buffer.
   *
   * Takes an input \p payload and converts it into the formatted byte buffer
   * with the proper size that the Discord IPC socket expects.
   *
   * \param payload Data payload to encode.
   *
   * \return The \p payload converted into a byte buffer.
   */
  static std::vector<char> encode_packet(const ipc_types::Payload& payload);
  /**
   * \brief Receives and handles incoming packets.
   *
   * This function is wrapped by the thread \ref _socket_recv_thread to receive
   * incoming packets from the socket, ensuring the main thread is not blocked.
   * The longevity of the function is controlled by the variable
   * \ref _stop_recv_thread, which is the boolean checker within the \c while
   * statement.
   *
   * \param stop_token Token to stop the receiver thread.
   */
  void recv_thread(std::stop_token stop_token);

 protected:
 /**
  * \brief Sends packet to socket.
  *
  * Attempts to send a payload packet to the Discord IPC socket and indicates
  * the success. This function is a wrapper for
  * \ref discord_ipc_cpp::websockets::SocketClient::send_data, as it takes an
  * input \p payload and encodes it first with \ref encode_packet before calling
  * the underlying method.
  *
  * \param payload Payload to send.
  *
  * \return Success of sending the packet.
  *
  * \see discord_ipc_cpp::websockets::SocketClient::send_data
  */
  bool send_packet(const ipc_types::Payload& payload);

  /**
   * \brief Receive packet from socket.
   *
   * Attempts to receive a packet from the socket using
   * \ref discord_ipc_cpp::websockets::SocketClient::recv_data(int,int) before
   * retrieving the rest of packet with
   * \ref discord_ipc_cpp::websockets::SocketClient::recv_data(int). The initial
   * attempt to retrieve a packet contains a timeout, after which it returns an
   * empty value.
   *
   * \return An optional payload.
   *
   * \see discord_ipc_cpp::websockets::SocketClient::recv_data(int)
   * \see discord_ipc_cpp::websockets::SocketClient::recv_data(int,int)
   */
  std::optional<ipc_types::Payload> recv_packet();

  /**
   * \brief Constructs a presence payload.
   *
   * The converts an optional input \p presence into a payload that can be sent
   * to the socket.
   *
   * \param presence Presence to set into a payload.
   *
   * \return Payload with the corresponding \p presence.
   */
  ipc_types::Payload construct_presence_payload(
    const std::optional<ipc_types::RichPresence>& presence);

  /**
   * \brief Attempts to send a payload.
   *
   * Wrapping around the method \ref send_packet, this method attempts to send
   * the \p payload \p max_retry_count times with a delay of one second between
   * each attempt.
   *
   * \param payload Payload to attempt sending.
   * \param max_retry_count Maximum number of times to attempt sending payload.
   *
   * \return Success of the attempt to send \p payload.
   */
  bool attempt_send_payload(
    const ipc_types::Payload& payload, int max_retry_count);

 public:
  /**
   * \brief Constructs the IPC client.
   *
   * Creates the IPC client with the \p client_id to be used when sending
   * rich presences.
   */
  explicit DiscordIPCClient(const std::string& client_id);
  /**
   * \brief Deallocates IPC client.
   *
   * Cleans up the IPC client by releasing the underlying socket connection.
   *
   * \see close
   */
  ~DiscordIPCClient();

  /**
   * \brief Connect to IPC socket.
   *
   * Attempts to connect to IPC socket and authorize the application. After
   * proper authorization, \ref _socket_recv_thread will be set and the
   * application will start listening for incoming packets.
   *
   * \return Success of the attempt to connect.
   *
   * \see discord_ipc_cpp::websockets::SocketClient::connect
   */
  bool connect();
  /**
   * \brief Close connection to IPC socket.
   *
   * Attempts to close the connection with the IPC socket by first sending a
   * closure Op code and then signalling for \ref _socket_recv_thread to stop by
   * setting \ref _stop_recv_thread to \c false. Lastly, the underlying socket
   * is closed.
   *
   * \param write_close Should send disconnect packet before closing socket.
   *
   * \return Success of the attempt to close connection.
   *
   * \see discord_ipc_cpp::websockets::SocketClient::close
   */
  bool close(bool write_close);

  /**
   * \brief Checks for successful authentication with socket.
   *
   * Returns internal tracking variable that tracks to see if the socket
   * has successful returned a dispatch response.
   *
   * \return Authentication success.
   *
   * \see _successful_auth
   */
  bool has_successful_auth();

  /**
   * \brief Sets the presence in Discord.
   *
   * Sends a request to set the presence of the connected Discord user with
   * \p presence.
   *
   * \param presence Presence to set.
   *
   * \return Success of setting the presence.
   */
  bool set_presence(const ipc_types::RichPresence& presence);
  /**
   * \brief Sets an empty presence in Discord.
   *
   * Sends a request to set an empty presence.
   *
   * \return Success fo setting an empty presence.
   */
  bool set_empty_presence();
};
}  // namespace discord_ipc_cpp

#endif  // DISCORD_IPC_CPP_INCLUDE_DISCORD_IPC_CPP_DISCORD_IPC_CLIENT_HPP_
