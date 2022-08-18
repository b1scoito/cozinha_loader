## Info
Automatically inject a [_DLL_](https://en.wikipedia.org/wiki/Dynamic-link_library) into the target process with the [_VAC3 bypass_](https://github.com/zyhp/vac3_inhibitor).

This will only, most likely, work only with [_source engine_](https://en.wikipedia.org/wiki/Source_(game_engine)) games in [_Steam_](https://en.wikipedia.org/wiki/Steam_(service)) that use [_VAC3_](https://en.wikipedia.org/wiki/Valve_Anti-Cheat) as a protection measure and load `serverbrowser.dll` as a _process module_.

## Screenshot
![Screenshot 2021-12-29 155620](https://user-images.githubusercontent.com/17802984/147694699-226bb43d-7928-407a-91fd-af0930818a78.png)

## Compiling
### Prerequisites
- Microsoft Visual Studio (Preferably the latest version) with C++ installed.
- The [BlackBone Static Library](https://github.com/DarthTon/Blackbone).


### How to compile BlackBone and include it
Watch the video [here](https://youtu.be/1SYER_5QYHk).

### Compiling from the source
Open the solution file `cozinha_loader.sln`, add the BlackBone library path to the VC++ include directory, then select `Release | x86` on the Build configuration and press Build solution.

### Does not open
If your error is `msvcp140.dll`, Please install the following libraries, `x86` is required. `x64` is optional but recommended.
- https://aka.ms/vs/16/release/vc_redist.x86.exe
- https://aka.ms/vs/16/release/vc_redist.x64.exe

Any other issue related to the loader, please create an issue in the [issues section](https://github.com/b1scoito/cozinha_loader/issues).

### Backstory
I made this project for some friends to use, now open-source for you guys.
