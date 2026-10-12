/*
  Copyright 2026 Peter Duanmu

  You should have received a copy of the GNU General Public License along
  with discord_ipc_cpp. If not, see <https://www.gnu.org/licenses/>.
*/

#include "discord_ipc_cpp/discord_ipc_client.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <cstring>
#include <map>
#include <thread>
#include <string>
#include <optional>
#include <vector>
#include <utility>
#include <iostream>
#include <functional>

#include "discord_ipc_cpp/socket_client.hpp"
#include "discord_ipc_cpp/json.hpp"
#include "discord_ipc_cpp/parser.hpp"

#include "include/internal_ipc_types.hpp"
#include "include/utils.hpp"

namespace discord_ipc_cpp {
using discord_ipc_cpp::json::JSON;
using discord_ipc_cpp::json::JSONObject;
using discord_ipc_cpp::json::JSONArray;
using discord_ipc_cpp::json::Parser;

using discord_ipc_cpp::ipc_types::Opcode;
using discord_ipc_cpp::ipc_types::Payload;
using discord_ipc_cpp::ipc_types::RichPresence;

using discord_ipc_cpp::internal_ipc_types::AuthorizationRequest;
using discord_ipc_cpp::internal_ipc_types::CommandRequest;

using discord_ipc_cpp::utils::find_discord_ipc_file;

namespace {
int current_process_id() {
#ifdef _WIN32
  return static_cast<int>(::GetCurrentProcessId());
#else
  return static_cast<int>(::getpid());
#endif
}
}

std::vector<char> DiscordIPCClient::encode_packet(
  const Payload& payload
) {
  std::string data = payload.payload.to_string();
  int data_len = data.size();

  std::vector<char> packet(8 + data_len);

  std::memcpy(&packet[0], &payload.opcode, 4);
  std::memcpy(&packet[4], &data_len, 4);
  std::memcpy(&packet[8], data.data(), data_len);

  return packet;
}

void DiscordIPCClient::recv_thread(std::stop_token stop_token) {
  while (!stop_token.stop_requested()) {
    auto optional_payload = recv_packet();

    if (!optional_payload.has_value()) {
      continue;
    }

    Payload recv_payload = std::move(*optional_payload);

    switch (recv_payload.opcode) {
      case Opcode::op_ping:
        send_packet({ Opcode::op_pong, recv_payload.payload });

        break;
      case Opcode::op_frame: {
          CommandRequest response = CommandRequest::from_json(
            recv_payload.payload);

          if (response.cmd == CommandRequest::ct_dispatch) {
            _successful_auth = true;
          }
        }

        break;
      case Opcode::op_close:
        close(true);

        return;
      case Opcode::op_quit:
        close(false);

        return;
      default:
        break;
    }
  }
}

DiscordIPCClient::DiscordIPCClient(const std::string& client_id)
: _pid(current_process_id()),
_client_id(client_id),
_socket(utils::find_discord_ipc_file()),
_successful_auth(false) {}

DiscordIPCClient::~DiscordIPCClient() {
  close(true);
}

bool DiscordIPCClient::send_packet(const Payload& payload) {
  if (payload.opcode != Opcode::op_handshake &&
      payload.opcode != Opcode::op_close &&
      !_successful_auth) {
    return false;
  }

  std::vector<char> packet = encode_packet(payload);

  return _socket.send_data(packet);
}

std::optional<Payload> DiscordIPCClient::recv_packet() {
  int opcode, data_len;
  std::string data;
  std::vector<char> opcode_buffer, data_len_buffer, buffer;

  auto poll_buffer = _socket.recv_data(4, 1000);

  if (!poll_buffer.has_value()) {
    return std::nullopt;
  }

  opcode_buffer = std::move(*poll_buffer);

  data_len_buffer = std::move(*_socket.recv_data(4));

  std::memcpy(&opcode, opcode_buffer.data(), sizeof(opcode));
  std::memcpy(&data_len, data_len_buffer.data(), sizeof(data_len));

  buffer = std::move(*_socket.recv_data(data_len));

  data = std::string(buffer.begin(), buffer.end());

  bool has_data = data.length() > 0;

  return Payload {
    has_data ? static_cast<Opcode>(opcode) : Opcode::op_quit,
    has_data ? Parser::parse(data) : JSON()
  };
}

Payload DiscordIPCClient::construct_presence_payload(
  const std::optional<RichPresence>& presence) {
    std::map<std::string, CommandRequest::RequestArgs> args = {
      { "pid", _pid }
    };

    if (presence.has_value()) {
      args.insert({ "activity", presence.value() });
    }

    Payload payload = {
      .opcode = Opcode::op_frame,
      .payload = CommandRequest {
        .cmd = CommandRequest::ct_set_activity,
        .nonce = utils::generate_uuid(),
        .args = args
      }.to_json()
    };

  return payload;
}

bool DiscordIPCClient::attempt_send_payload(
  const ipc_types::Payload& payload,
  int max_retry_count
) {
  bool success;
  int retry_count = 0;

  do {
    success = send_packet(payload);

    if (!success) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));

      ++retry_count;
    }
  } while (!success && retry_count <= max_retry_count);

  return success;
}

bool DiscordIPCClient::connect() {
  bool ret = _socket.connect();

  if (!ret) {
    return false;
  }

  ret = send_packet({
    .opcode = Opcode::op_handshake,
    .payload = AuthorizationRequest {
      .version = "1",
      .client_id = _client_id
    }.to_json()
  });

  if (!ret) {
    return false;
  }

  _socket_recv_thread = std::jthread(
    std::bind_front(&DiscordIPCClient::recv_thread, this));

  return true;
}

bool DiscordIPCClient::has_successful_auth() {
  return _successful_auth;
}

bool DiscordIPCClient::close(bool write_close) {
  if (write_close) {
    set_empty_presence();

    send_packet({
      .opcode = Opcode::op_close,
      .payload = {}
    });
  }

  _socket_recv_thread.request_stop();

  int res = _socket.close();

  _successful_auth = false;

  std::this_thread::sleep_for(std::chrono::milliseconds(25));

  if (
    _socket_recv_thread.joinable() &&
    std::this_thread::get_id() != _socket_recv_thread.get_id()
  ) {
    _socket_recv_thread.join();
  }

  return res;
}

bool DiscordIPCClient::set_presence(const ipc_types::RichPresence& presence) {
  Payload payload = construct_presence_payload(presence);

  return attempt_send_payload(payload, 3);
}

bool DiscordIPCClient::set_empty_presence() {
  Payload payload = construct_presence_payload({});

  return attempt_send_payload(payload, 3);
}
}  // namespace discord_ipc_cpp
