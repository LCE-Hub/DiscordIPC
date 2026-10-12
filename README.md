# Discord IPC CPP

C++ library for interfacing with Discord IPC socket. This is a fork of [TheMoonThatRises/discord_ipc_cpp](https://github.com/TheMoonThatRises/discord_ipc_cpp) made for the Project Radium ([Emerald Launcher v2](https://github.com/LCE-Hub/LCE-Emerald-Launcher)). This fork adds Windows support on top of the original Unix implementation. Because of the criteria for the parent project, this library is entirely self-contained, and does not include any external libraries.

## Documentation (for the original project)

Read the full documentation here: [https://plduanm.com/projects/documentation/discord_ipc_cpp](https://plduanm.com/projects/documentation/discord_ipc_cpp)

## Installation

Requirements:

- A Unix system or Windows
- CMake >= 3.25
- C++ >= 20

Clone this repository as a submodule under `src/`:

```bash
git submodule add https://github.com/LCE-Hub/DiscordIPC src/discord_ipc_cpp
```

And add the following into your `CMakeLists.txt`:

```cmake
...

add_subdirectory(src/discord_ipc_cpp)

...

target_link_libraries(<EXECUTABLE> PRIVATE discord_ipc_cpp)
```

## Basic Usage

All of the code below will incorporate the following metacode:

```c++
#include <discord_ipc_cpp/discord_ipc_client.hpp>
#include <discord_ipc_cpp/ipc_types.hpp>
using discord_ipc_cpp::DiscordIPCClient;
using discord_ipc_cpp::ipc_types::RichPresence;
```

Create a Discord IPC client and connect to it:

```c++
DiscordIPCClient client("<APPLICATION ID>");
// indicates success of connecting to socket
bool ret = client.connect();
```

Construct a rich presence:

```c++
RichPresence presence = {
  .name = "Apple Music",
  .type = RichPresence::at_listening,
  .status_display_type = RichPresence::sdt_details,
  .assets = {
    .large_text = "Kawakiwoameku - EP"
  },
  .details = "Hollowness",
  .state = "Minami"
}
```

Set the user's rich presence:
```c++
client.set_presence(presence);
```

Clear the user's rich presence:

```c++
client.set_empty_presence();
```

Close the IPC connection:

```c++
client.close();
```
