/*
  Copyright 2026 Peter Duanmu

  You should have received a copy of the GNU General Public License along
  with discord_ipc_cpp. If not, see <https://www.gnu.org/licenses/>.
*/

#include "include/utils.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <cstdio>
#include <map>
#include <string>
#include <optional>
#include <vector>
#include <random>

#include "include/internal_ipc_types.hpp"

namespace discord_ipc_cpp::utils {
using CommandType = internal_ipc_types::CommandRequest::CommandType;
using EventType = internal_ipc_types::CommandRequest::EventType;

std::string find_discord_ipc_file() {
#ifdef _WIN32
  std::string base_ipc_name = "\\\\.\\pipe\\discord-ipc-";
  for (int i = 0; i < 10; ++i) {
    std::string test_path = base_ipc_name + std::to_string(i);
    HANDLE pipe = ::CreateFileA(test_path.c_str(),
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                nullptr,
                                OPEN_EXISTING,
                                0,
                                nullptr);

    if (pipe != INVALID_HANDLE_VALUE) {
      ::CloseHandle(pipe);
      return test_path;
    }
  }

  return "";
#else
  const char* tmp_dir = std::getenv("TMPDIR");
  const char* env_dir = tmp_dir != nullptr ? tmp_dir : "/tmp";
  std::string user_tmp_dir = env_dir;
  std::string base_ipc_name = "discord-ipc-";

  for (int i = 0; i < 10; ++i) {
    std::string test_path = user_tmp_dir + "/" + base_ipc_name
      + std::to_string(i);

    if (access(test_path.c_str(), R_OK) >= 0) {
      return test_path;
    }
  }

  return "";
#endif
}

std::string escape_string(const std::string& input) {
  std::string out;
  out.reserve(input.size() + 16);

  for (unsigned char c : input) {
    switch (c) {
      case '"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\b':
        out += "\\b";
        break;
      case '\f':
        out += "\\f";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        if (c < 0x20) {
          char buf[7];
          std::snprintf(buf, sizeof(buf), "\\u%04x", c);
          out += buf;
        } else {
          out += static_cast<char>(c);
        }
    }
  }

  return out;
}

std::string unescape_string(const std::string& input) {
  std::string out;
  out.reserve(input.size());

  for (size_t i = 0; i < input.size(); ++i) {
    char c = input[i];

    if (c != '\\') {
      out += c;
      continue;
    }

    if (++i >= input.size()) {
      out += '\\';
      break;
    }  // trailing backslash

    switch (input[i]) {
      case '"':
        out += '"';
        break;
      case '\\':
        out += '\\';
        break;
      case '/':
        out += '/';
        break;
      case 'b':
        out += '\b';
        break;
      case 'f':
        out += '\f';
        break;
      case 'n':
        out += '\n';
        break;
      case 'r':
        out += '\r';
        break;
      case 't':
        out += '\t';
        break;
      case 'u': {
        // \uXXXX — 4 hex digits follow
        if (i + 4 < input.size()) {
          int cp = std::stoi(input.substr(i + 1, 4), nullptr, 16);

          i += 4;
          // NOTE: this only handles the basic case; see caveat below
          if (cp < 0x80) {
            out += static_cast<char>(cp);
          } else if (cp < 0x800) {
            out += static_cast<char>(0xC0 | (cp >> 6));
            out += static_cast<char>(0x80 | (cp & 0x3F));
          } else {
            out += static_cast<char>(0xE0 | (cp >> 12));
            out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (cp & 0x3F));
          }
        }

        break;
      }
      default:
        out += input[i];
        break;  // unknown escape, pass through
    }
  }

  return out;
}

template<typename T>
T generate_random_num(T min, T max) {
  static std::random_device _rdev;
  static std::mt19937 _rng(_rdev());
  static std::uniform_real_distribution<> _rdist(min, max);

  return _rdist(_rng);
}

std::string generate_uuid() {
  static const std::string _valid_chars = "0123456789abcdef";
  static const std::vector<int> _dashes = {
    0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0
  };

  std::string uuid = "";

  for (int i = 0; i < 16; ++i) {
    if (_dashes[i]) {
      uuid += '-';
    }

    uuid += _valid_chars[generate_random_num(0, 16)];
    uuid += _valid_chars[generate_random_num(0, 16)];
  }

  return uuid;
}

template<typename K, typename V>
std::optional<K> reverse_map_search(const std::map<K, V>& map, const V& item) {
  for (const auto& [key, value] : map) {
    if (value == item) {
      return key;
    }
  }

  return std::nullopt;
}

template double generate_random_num(double, double);
template int generate_random_num(int, int);

template std::optional<CommandType> reverse_map_search(
  const std::map<CommandType, std::string>&, const std::string& item);
template std::optional<EventType> reverse_map_search(
  const std::map<EventType, std::string>&, const std::string& item);
}  // namespace discord_ipc_cpp::utils
