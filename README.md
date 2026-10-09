# win-client-event-api

A small Windows console client for an HTTPS events API, written in C++17 on top of the native **WinHTTP** API (no libcurl).

It can:

- check the connection to the server (`GET` status endpoint);
- list events, and list events that have no image yet;
- create an event (`POST` JSON) with a client-generated `event_uid`;
- upload a `.jpg` / `.png` image (max 5 MiB) for an event (`POST` multipart/form-data);

## Project layout

| File | Purpose |
|---|---|
| `win-client-event-api.cpp` | `main`: loads URL and key, starts the UI |
| `UI.h/.cpp` | Console menu and input validation |
| `ApiClient.h/.cpp` | API-level calls: status, events, create event, upload image |
| `HttpClient.h/.cpp` | `WinHttpClient`: GET/POST, headers, response reading, retry |
| `CfgReader.h/.cpp` | JSON config loading, path resolution relative to the exe |
| `defines.h` | Relative paths to the three config files |

## Build and Configuration

### Requirements

* Windows
* Visual Studio with C++ development tools
* C++17 or later
* [vcpkg](https://github.com/microsoft/vcpkg) for dependency management

### Dependencies

The project uses the following dependencies:

* `WinHttp.lib` — Windows HTTP client API.
* `Rpcrt4.lib` — Windows RPC runtime library, used for UUID generation.
* `nlohmann/json` — JSON parsing and serialization, managed through `vcpkg.json`.

The Windows libraries are provided by the Windows SDK. The `nlohmann/json` dependency is declared in `vcpkg.json`.

### Build

1. Clone the repository.

2. Open a terminal in the project directory.

3. Run the following command to install the required dependencies:

   ```bash
   vcpkg install
   ```

4. Open the project or solution in Visual Studio.

5. Select the desired build configuration (Debug or Release) and platform (e.g., x64).

6. Build the solution.

### Configuration

Before running the application, configure `server_config.json` with your server URL and API token.

Example:

```json
{
    "url": "https://your-server.example.com",
    "api_key": "YOUR_API_TOKEN"
}
```

Replace the example values with your actual server URL and API token.

Make sure `server_config.json` is located at the path expected by the application and is accessible when the application starts.

**Security note:** Do not commit real API tokens to the repository or share them publicly. Keep your actual token in your local configuration file.

## Known Limitations and Future Improvements

Given more time, I would focus on the following improvements:

* **Multithreading and event processing:** Separate event generation from HTTP request processing using independent threads and a thread-safe queue. This would allow events to be generated while previous events are being sent, improving throughput and decoupling event production from network latency.
* **Error handling and retries:** Extend HTTP error handling and implement a configurable retry policy with exponential backoff and support for the `Retry-After` header.
* **Automated testing:** Add unit and integration tests covering HTTP responses, error handling, JSON parsing, multipart file uploads, and retry behavior.
* **Configuration management and security:** Improve configuration file discovery by using paths relative to the executable or a dedicated application configuration directory instead of relying on the current working directory. Improve configuration validation and provide a safer way to manage API tokens without storing sensitive credentials in version control.
* **Logging:** Introduce structured logging to simplify debugging and troubleshooting of HTTP requests.
* **Request validation:** Add more comprehensive validation for request parameters, JSON payloads, and uploaded files before sending requests.
* **Build portability:** Improve build configuration and dependency management to make the project easier to build in different development environments.
