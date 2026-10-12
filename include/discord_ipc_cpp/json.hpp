/*
  Copyright 2026 Peter Duanmu

  You should have received a copy of the GNU General Public License along
  with discord_ipc_cpp. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DISCORD_IPC_CPP_INCLUDE_DISCORD_IPC_CPP_JSON_HPP_
#define DISCORD_IPC_CPP_INCLUDE_DISCORD_IPC_CPP_JSON_HPP_

#include <map>
#include <string>
#include <sstream>
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

/**
 * \namespace discord_ipc_cpp::json
 *
 * \brief Custom JSON implementation.
 *
 * A useful and necessary implementation of JSON for the library and the user to
 * use. All data within payloads sent and received from the socket utilize the
 * JSON standard.
 */
namespace discord_ipc_cpp::json {
class JSON;

/**
 * \brief Representation of \c null.
 */
using JSONNull = std::nullptr_t;
/**
 * \brief Representation of \c string.
 */
using JSONString = std::string;
/**
 * \brief Representation of \c number.
 */
using JSONInt = int;
/**
 * \brief Representation of \c number.
 */
using JSONLong = int64_t;
/**
 * \brief Representation of \c number.
 */
using JSONDouble = double;
/**
 * \brief Representation of \c bool.
 */
using JSONBool = bool;
/**
 * \brief Representation of a key-value pair.
 */
using JSONObject = std::map<std::string, JSON>;
/**
 * \brief Representation of \c array.
 */
using JSONArray = std::vector<JSON>;

/**
 * \brief Basic abstract JSON item.
 *
 * Allows the user to construct JSON objects and array easily and in a C++,
 * object-oriented method.
 */
class JSON {
 private:
  /**
   * \brief Representation of the possible values within a JSON.
   */
  using JSONValue = std::variant<
    JSONNull,
    JSONString,
    JSONInt,
    JSONLong,
    JSONDouble,
    JSONBool,
    JSONArray,
    JSONObject
  >;

  /**
   * \brief Value of the JSON item.
   *
   * The value can be any of the variants specified in \ref JSONValue.
   */
  JSONValue _value;

 private:
  /**
   *\brief Converts the JSON and its nested items into a string.
   *
   * \param os Stream builder containing the stringified JSON.
   */
  void stringify(std::ostream& os) const;

 public:
  /**
   * \brief Creates an empty JSON object.
   *
   * \see discord_ipc_cpp::json::JSONObject
   */
  JSON();
  /**
   * \brief Creates a \c null JSON item.
   *
   * \param value The \c null value.
   *
   * \see discord_ipc_cpp::json::JSONNull
   */
  explicit JSON(JSONNull value);
  /**
   * \brief Creates a \c string JSON item.
   *
   * \param value The value to set the JSON item.
   *
   * \see discord_ipc_cpp::json::JSONString
   */
  explicit JSON(const JSONString& value);
  /**
   * \brief Creates a \c string JSON item.
   *
   * \param value The value to set the JSON item.
   *
   * \see discord_ipc_cpp::json::JSONString
   */
  explicit JSON(const char* value);
  /**
   * \brief Creates a \c number JSON item.
   *
   * \param value The value to set the JSON item.
   *
   * \see discord_ipc_cpp::json::JSONString
   */
  explicit JSON(JSONInt value);
  /**
   * \brief Creates a \c number JSON item.
   *
   * \param value The value to set the JSON item.
   *
   * \see discord_ipc_cpp::json::JSONInt
   */
  explicit JSON(JSONLong value);
  /**
   * \brief Creates a \c number JSON item.
   *
   * \param value The value to set the JSON item.
   *
   * \see discord_ipc_cpp::json::JSONLong
   */
  explicit JSON(JSONDouble value);
  /**
   * \brief Creates a \c bool JSON item.
   *
   * \param value The value to set the JSON item.
   *
   * \see discord_ipc_cpp::json::JSONDouble
   */
  explicit JSON(JSONBool value);
  /**
   * \brief Creates an \c array JSON item.
   *
   * \param value The value to set the JSON item.
   *
   * \see discord_ipc_cpp::json::JSONArray
   */
  explicit JSON(const JSONArray& value);
  /**
   * \brief Creates another nested JSON item.
   *
   * \param value The value to set the JSON item.
   *
   * \see discord_ipc_cpp::json::JSONObject
   */
  explicit JSON(const JSONObject& value);

  /**
   * \brief Retrieve a nested value.
   *
   * This method retrieves the nested JSON value by reference, allowing the
   * value to be changed. This will attempt to get the current item of the JSON
   * as a JSON object before getting a value by key.
   *
   * \param key Key of the value.
   *
   * \return The value from the key-value pair.
   *
   * \throws std::bad_variant_access If the key does not exist within the JSON
   *         item or if the current JSON item is not a JSON object.
   */
  JSON& operator[](const JSONString& key);
  /**
   * \brief Retrieve a nested value.
   *
   * This method retrieves the nested JSON value by a constant reference.
   * This will attempt to get the current item of the JSON as a JSON object
   * before getting a value by key.
   *
   * \param key Key of the value.
   *
   * \return The value from the key-value pair.
   *
   * \throws std::bad_variant_access If the key does not exist within the JSON
   *         item or if the current JSON item is not a JSON object.
   */
  const JSON& operator[](const JSONString& key) const;

  /**
   * \brief Retrieves a nested value safely.
   *
   * This method will attempt to retrieve the nested value. This will return an
   * empty optional if the current JSON item is not a JSON object, or if the
   * key does not exist within the current JSON object.
   *
   * \param key Key of the value.
   *
   * \return An optional value from the key-value pair.
   */
  const std::optional<JSON> safe_at(const JSONString& key) const;

  /**
   * \brief Retrieve the current JSON item as a specific type.
   *
   * Attempts to unsafely retrieve the current JSON item as the specified type.
   *
   * \tparam T Type to retrieve current JSON item as.
   *
   * \return The current JSON item as type \c T.
   *
   * \throws std::bad_variant_access If the current JSON item is not the
   *         specified type.
   */
  template<typename T>
  T as() const;
  /**
   * \brief Retrieve the current JSON item as a specific type.
   *
   * Attempts to safely retrieve the current JSON item as the specified type.
   * The return value will default to an empty optional if the JSON item is not
   * the specified type.
   *
   * \tparam T Type to retrieve current JSON item as.
   *
   * \return The current JSON item as an optional type \c T.
   */
  template<typename T>
  std::optional<T> safe_as() const;

  /**
   * \brief Checks current JSON item type.
   *
   * This method checks to see if there is a corresponding type for the current
   * JSON item.
   *
   * \tparam T Type to check the current JSON item.
   *
   * \return If the current JSON item is of type \c T.
   */
  template<typename T>
  bool is() const;

  /**
   * \brief Checks current JSON for key.
   *
   * Will attempt to unsafely check if the current JSON item has the key.
   *
   * \param key The key in the key-value pair.
   *
   * \return If the key exists within the JSON item.
   *
   * \throws std::bad_variant_access If the current JSON item is not type JSON
   *         object.
   */
  bool has(const JSONString& key) const;

  /**
   * \brief Appends item to current JSON.
   *
   * Will attempt to unsafely append a value to the current JSON item as a JSON
   * array.
   *
   * \param item Item to append.
   *
   * \throws std::bad_variant_access If the current JSON item is not of type
   *         JSON array.
   */
  void push_back(const JSON& item);

  /**
   * \brief Stringifies JSON object.
   *
   * Converts the current JSON item and all of its nested objects and arrays
   * into the proper JSON format.
   */
  std::string to_string() const;
};
}  // namespace discord_ipc_cpp::json

#endif  // DISCORD_IPC_CPP_INCLUDE_DISCORD_IPC_CPP_JSON_HPP_
