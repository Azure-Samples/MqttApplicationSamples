# Using C Samples

## Unique to C Samples/mosquitto Client Library

- If you set KEEP_ALIVE_IN_SECONDS to `0`, no keepalive checks are made and the client will never be disconnected by the broker if no messages are received. The minimum value for mosquitto is `5`, and the max is `65535`. There is no default value for the mosquitto library, but if you haven't passed a value to the environment variable, we will set it to `30` to align with the other language samples.

## C Specific Prerequisites

> Note: Some of these may be installed automatically if you use VS Code Extensions
- [CMake](https://cmake.org/download/) Version 3.20 or higher to use CMake presets
- [Mosquitto](https://mosquitto.org/download/) Version 2.0.0 or higher
- [Ninja build system](https://github.com/ninja-build/ninja/releases) Version 1.10 or higher
- GNU C++ compiler
- SSL
- [JSON-C](https://github.com/json-c/json-c) if running a sample that uses JSON - currently these are the Telemetry Samples
- UUID Library (if running a sample that uses correlation IDs - currently these are the Command Samples)
- [protobuf-c](https://github.com/protobuf-c/protobuf-c) If running a sample that uses protobuf - currently these are the Command Samples. Note that you'll need protobuf-c-compiler and libprotobuf-dev as well if you're generating code for new proto files.

An example of installing these tools (other than CMake) is shown below:

``` bash
sudo apt-add-repository ppa:mosquitto-dev/mosquitto-ppa
sudo apt-get update && sudo apt-get install g++-multilib ninja-build libmosquitto-dev libssl-dev -y
# If running a sample that uses JSON
sudo apt-get install libjson-c-dev
# If running a sample that uses Correlation IDs
sudo apt-get install uuid-dev
# If running a sample that uses protobuf
sudo apt-get install libprotobuf-c-dev
```

## Using the Command Line

### Building the Samples

From the root of the repo, run the following commands for which sample you want to build (ex. `getting_started` or `telemetry`)

With CMake presets (requires version >= 3.20):

``` bash
cmake --preset=<sample name>
cmake --build --preset=<sample name>
```

Without CMake presets:

``` bash
cmake -G Ninja -Bscenarios/<sample name>/c/build -DPRESET_PATH=scenarios/<sample name>/c .
cmake --build scenarios/<sample name>/c/build
```

### Running the Samples

- Generate .env file(s) and key/pem files as directed in main readmes
- Navigate to scenario folder (ex. `cd scenarios/telemetry`)
- Execute the binary from this location. The binary for all scenarios will be located at `./c/build/<scenario name>`. As a command line argument, pass in the path to the .env file to use. If you do not specify a .env file, the program will search for a file named `.env` in the current directory. Example:
    ``` bash
    ./c/build/telemetry_producer vehicle01.env
    ```
- If for some reason, you need to run the sample from a different location, the argument for the .env file and the file paths within the .env file should be written as their absolute paths.

## Using VS Code

- Install the VS Code extension `ms-vscode.cpptools-extension-pack`
- sudo apt-get install build-essential gdb
- Generate .env file(s) and key/pem files as directed in main readmes
- Go to the `Run and Debug` tab in VS Code and select one of the C samples from the dropdown
- Click the Green Play button and you should be good to go!

## Using the CMake Extension in VS Code

> Note: This is not a default supported configuration, but can be useful for testing. Configuration changes needed for this to work are listed below

- Install the VS Code extension `ms-vscode.cpptools-extension-pack`
- Generate .env file(s) and key/pem files as directed in main readmes
- The CMake extension doesn't allow us to dictate where the sample runs from, so modify your .env files to use absolute paths for the CA_FILE, CERT_FILE, and KEY_FILE if needed for your sample
- The CMake extension has limited capabilities for passing in command line arguments, so rename your .env files to be the name of the sample executable so they can be properly passed in (ex. `telemetry_producer.env`), and add the following to your settings.json to have the .env file be passed in:
    ```json
    "cmake.debugConfig": {
        "args": ["${workspaceFolder}/scenarios/${command:cmake.activeBuildPresetName}/${command:cmake.buildTargetName}.env"],
    },
    ```
- Now you can select the Configure Preset, Build Preset, and Build Target on the bottom bar of VS Code and use the Build/Run buttons from there.

## Contributing

We use [clang-format](https://releases.llvm.org/download.html#9.0.0) to format the code properly. Note that you NEED clang-format from Clang version 9.0.0. Subsequent versions format code differently and we settled on this one for consistency. If you download the pre-built binaries version, it should be located at `<expanded clang dir>/bin/clang-format`. On ubuntu, you can install it with this command:

``` bash
sudo apt install -y clang-format-9
```

To fix any style errors, run this command from the root of the repo (if you used a different install method than apt install, use `clang-format` instead of `clang-format-9`):

``` bash
clang-format-9 -style=file -i $(find . -name "*.[ch]" -not -path "./*/build/*" -not -name "*.pb-c.*")
```

## Running Tests
The Unit Tests are using the [CMocka](https://cmocka.org/) framework.
On Ubuntu, this can be installed by running:

`sudo apt install libcmocka-dev libcmocka0`

To configure and build the unit tests:

```bash
# from folder mqttclients/c/tests
mkdir build
cd build
cmake ..
cmake --build .
ctest
```

## Additional Resources

- To print out all mosquitto logs, set cmake option `LOG_ALL_MOSQUITTO` to ON. When set to OFF (the default value), only ping requests/responses get printed.
- For a complete list of available functions from the mosquitto library, see their [api reference](https://mosquitto.org/api/files/mosquitto-h.html).
- To declutter the bottom bar in VS Code a bit, you can hide some CMake buttons that we aren't using in your settings.json

    ``` json
    "cmake.statusbar.advanced": {
        "ctest": {"visibility": "hidden"},
        "testPreset": {"visibility": "hidden"},
        "launchTarget": {"visibility": "hidden"},
    }
    ```