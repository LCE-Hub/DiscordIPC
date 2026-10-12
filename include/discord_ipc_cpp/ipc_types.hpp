/*
  Copyright 2026 Peter Duanmu

  You should have received a copy of the GNU General Public License along
  with discord_ipc_cpp. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DISCORD_IPC_CPP_INCLUDE_DISCORD_IPC_CPP_IPC_TYPES_HPP_
#define DISCORD_IPC_CPP_INCLUDE_DISCORD_IPC_CPP_IPC_TYPES_HPP_

#include <optional>
#include <string>
#include <vector>
#include <cstdint>

#include "discord_ipc_cpp/json.hpp"

/**
 * \namespace discord_ipc_cpp::ipc_types
 *
 * \brief Useful IPC data types and enumerators.
 *
 * Contains all the useful IPC data structure types necessary for constructing
 * payloads and rich presences, along with the necessary Op codes.
 */
namespace discord_ipc_cpp::ipc_types {
/**
 * \brief Request and response Op codes.
 */
enum Opcode : int {
  op_quit = -1,      ///< Discord app quit
  op_handshake = 0,  ///< Initiate connection
  op_frame = 1,      ///< General socket response
  op_close = 2,      ///< Close connection
  op_ping = 3,       ///< Heartbeat ping to socket
  op_pong = 4        ///< Heartbeat response from ping
};

/**
 * \brief Represents a data payload.
 *
 * A simple structure containing the necessary information for a request and
 * response payload.
 */
struct Payload {
  /**
   * \brief Op code of the payload.
   */
  const Opcode opcode;
  /**
   * \brief Content of the payload.
   */
  const json::JSON payload;
};

/**
 * \brief A Discord rich presence.
 *
 * Represents a Discord rich presence as a C++ structure to be easily
 * instantiated and converted into its JSON representation.
 */
struct RichPresence {
 public:
  /**
   * \brief Timestamp of the presence.
   *
   * If only \ref start is set, then the presence displays as an on-going
   * activity, similar to a game presence. If both \ref start and \ref end
   * are set, then the presence displays similarly to a song playing.
   */
  struct Timestamps {
    /**
     * \brief Optional start time of the presence in seconds.
     */
    std::optional<int64_t> start { std::nullopt };
    /**
     * \brief Optional end time of the presence in seconds.
     */
    std::optional<int64_t> end { std::nullopt };

   private:
    /**
     * \brief Converts the struct into a JSON.
     *
     * \return JSON representation of the struct.
     *
     * \see discord_ipc_cpp::json::JSON
     */
    json::JSON to_json() const;

    friend struct RichPresence;
  };

  /**
   * \brief Emoji for the activity.
   */
  struct ActivityEmoji {
    /**
     * \brief Name of the emoji.
     */
    std::string name;
    /**
     * \brief ID of the emoji.
     */
    std::optional<std::string> snowflake { std::nullopt };
    /**
     * \brief If emoji is animated.
     */
    std::optional<bool> animated { std::nullopt };

   private:
    /**
     * \brief Converts the struct into a JSON.
     *
     * \return JSON representation of the struct.
     *
     * \see discord_ipc_cpp::json::JSON
     */
    json::JSON to_json() const;

    friend struct RichPresence;
  };

  /**
   * \brief Current party for the activity.
   */
  struct Party {
    /**
     * \brief ID of the party.
     */
    std::optional<std::string> id { std::nullopt };
    /**
     * \brief Current and maximum size.
     *
     * The zeroth index represents the party's current size, while the first
     * index represents the party's maximum size.
     */
    std::optional<std::vector<int>> size { std::nullopt };

   private:
    /**
     * \brief Converts the struct into a JSON.
     *
     * \return JSON representation of the struct.
     *
     * \see discord_ipc_cpp::json::JSON
     */
    json::JSON to_json() const;

    friend struct RichPresence;
  };

  /**
   * \brief Images and hover texts of presence.
   */
  struct Assets {
    /**
     * \brief Large image of the presence.
     */
    std::optional<std::string> large_image { std::nullopt };
    /**
     * \brief Text on hover of large image of the presence.
     */
    std::optional<std::string> large_text { std::nullopt };
    /**
     * \brief URL for large image of the presence.
     */
    std::optional<std::string> large_url { std::nullopt };
    /**
     * \brief Small image of the presence.
     */
    std::optional<std::string> small_image { std::nullopt };
    /**
     * \brief Text on hover of small image of the presence.
     */
    std::optional<std::string> small_text { std::nullopt };
    /**
     * \brief URL for small image of the presence.
     */
    std::optional<std::string> small_url { std::nullopt };

   private:
    /**
     * \brief Converts the struct into a JSON.
     *
     * \return JSON representation of the struct.
     *
     * \see discord_ipc_cpp::json::JSON
     */
    json::JSON to_json() const;

    friend struct RichPresence;
  };

  /**
   * \brief Secrets for joining and spectating presence.
   */
  struct Secrets {
    /**
     * \brief Secret for joining party.
     */
    std::optional<std::string> join { std::nullopt };
    /**
     * \brief Secret for specific instanced match.
     */
    std::optional<std::string> match { std::nullopt };
    /**
     * \brief Secret for spectating game.
     */
    std::optional<std::string> spectate { std::nullopt };

   private:
    /**
     * \brief Converts the struct into a JSON.
     *
     * \return JSON representation of the struct.
     *
     * \see discord_ipc_cpp::json::JSON
     */
    json::JSON to_json() const;

    friend struct RichPresence;
  };

  /**
   * \brief Custom buttons to show on presence.
   *
   * There is a maximum of two buttons that can be shown on a single presence.
   */
  struct Button {
    /**
     * \brief Label of the button.
     */
    std::string label;
    /**
     * \brief URL that is opened when the button is clicked.
     */
    std::string url;

   private:
    /**
     * \brief Converts the struct into a JSON.
     *
     * \return JSON representation of the struct.
     *
     * \see discord_ipc_cpp::json::JSON
     */
    json::JSON to_json() const;

    friend struct RichPresence;
  };

  /**
   * \brief Section of presence to display at a quick glance.
   */
  enum StatusDisplayType : int {
    sdt_name = 0,    ///< Displays the name section
    sdt_state = 1,   ///< Displays the state section
    sdt_details = 2  ///< Displays the details section
  };

  /**
   * \brief Type of activity the presence is.
   *
   * \note \ref at_streaming is only valid for YouTube and Twitch URLs.
   */
  enum ActivityType : int {
    at_game = 0,       ///< "Playing {name}"
    at_streaming = 1,  ///< "Streaming {details}"
    at_listening = 2,  ///< "Listening to {state}"
    at_watching = 3,   ///< "Watching {name}"
    at_custom = 4,     ///< "{emoji} {state}"
    at_competing = 5   ///< "Competing in {name}"
  };

 public:
  /**
    * \brief Name of the presence.
    */
  std::string name;
  /**
    * \brief Type of the activity.
    */
  ActivityType type;
  /**
    * \brief Stream URL.
    *
    * Only used when \ref type is \ref at_streaming.
    */
  std::optional<std::string> url { std::nullopt };
  /**
    * \brief Timestamp when activity was added to session.
    */
  std::optional<int64_t> created_at { std::nullopt };
  /**
    * \brief Timestamp of start and/or end of game.
    */
  std::optional<Timestamps> timestamps { std::nullopt };
  /**
    * \brief Application ID for the game.
    */
  std::optional<std::string> application_id { std::nullopt };
  /**
    * \brief Status display type of the presence.
    *
    * Controls which field is displayed in the user's status text in the member
    * list.
    */
  std::optional<StatusDisplayType> status_display_type { std::nullopt };
  /**
    * \brief What the user is currently doing.
    */
  std::optional<std::string> details { std::nullopt };
  /**
    * \brief URL that is linked to details text.
    */
  std::optional<std::string> details_url { std::nullopt };
  /**
    * \brief User's current party status or custom status.
    */
  std::optional<std::string> state { std::nullopt };
  /**
    * \brief URL that is linked to status text.
    */
  std::optional<std::string> state_url { std::nullopt };
  /**
    * \brief Emoji used for custom status.
    */
  std::optional<ActivityEmoji> emoji { std::nullopt };
  /**
    * \brief Information for current party of the player.
    */
  std::optional<Party> party { std::nullopt };
  /**
    * \brief Images for the presence and their hover text and URL.
    */
  std::optional<Assets> assets { std::nullopt };
  /**
    * \brief Secrets for joining and spectating presence.
    */
  std::optional<Secrets> secrets { std::nullopt };
  /**
    * \brief Whether activity is an instanced game session.
    */
  std::optional<bool> instance { std::nullopt };
  /**
    * \brief Activity flags describing the payload.
    *
    * These flags are \c OR d together.
    */
  std::optional<int64_t> flags { std::nullopt };
  /**
    * \brief Custom buttons to show in rich presence.
    *
    * There is a maximum of two buttons that can be displayed on a single
    * presence.
    */
  std::optional<std::vector<Button>> buttons { std::nullopt };

 public:
  /**
   * \brief Converts the struct into a JSON.
   *
   * \return JSON representation of the struct.
   *
   * \see discord_ipc_cpp::json::JSON
   */
  json::JSON to_json() const;
};
}  // namespace discord_ipc_cpp::ipc_types

#endif  // DISCORD_IPC_CPP_INCLUDE_DISCORD_IPC_CPP_IPC_TYPES_HPP_
