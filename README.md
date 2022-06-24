# CONTRAST

A bundle of free, open-source audio plug-ins built with [JUCE](https://juce.com/).

[![LICENSE: MIT](https://img.shields.io/github/license/ImJimmi/Contrast?style=for-the-badge)](LICENSE)

[![Azure DevOps builds (branch)](https://img.shields.io/azure-devops/build/ImJimmi/368c7d2c-96f4-40c9-8470-644e4b4bab68/3/refs/pull/1/merge?label=AZURE%20PIPELINES&logo=Azure%20Pipelines&style=for-the-badge)](https://dev.azure.com/ImJimmi/Contrast/_build) ![GitHub last commit](https://img.shields.io/github/last-commit/ImJimmi/Contrast?style=for-the-badge)

## PLUG-INS

![GitHub release (latest by date)](https://img.shields.io/github/v/release/ImJimmi/Contrast?label=LATEST%20VERSION&style=for-the-badge)
![GitHub Release Date](https://img.shields.io/github/release-date/ImJimmi/Contrast?style=for-the-badge)

[CHANGES](CHANGES.md)

[![Download](https://img.shields.io/github/v/release/ImJimmi/Contrast?label=DOWNLOAD&style=for-the-badge&logo=GitHub&color=success)](https://github.com/ImJimmi/Contrast/releases/latest)

| NAME | DESCRIPTION |
| ------ | ----------- |
| [GATE](Gate/) | Look-ahead noise gate with attack and release. |
| [PITCH](Pitch/) | Pitch shifter. |
| [PRESS](Press/) | Straightforward compressor. |
| [VERB](Verb/) | Straightforward mono or stereo reverb. |

## REQUIREMENTS

### ![Mac OS](https://img.shields.io/badge/mac%20os-000000?style=for-the-badge&logo=apple&logoColor=F0F0F0)

- VST3 or AU compatible host
- OSX 10.7 or later

### ![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)

- VST3 compatible host
- Windows 7 or later

## INSTALLING

1. [Download Latest Version](https://github.com/ImJimmi/Contrast/releases/latest)
2. Extract files to relevant directories:

### ![Mac OS](https://img.shields.io/badge/mac%20os-000000?style=for-the-badge&logo=apple&logoColor=F0F0F0)

| FORMAT | DIRECTORY |
| ------ | --------- |
| VST3 | `~/Library/Audio/Plug-Ins/VST3`<br>`Macintosh HD/Library/Audio/Plug-Ins/VST3` |
| AU | `~/Library/Audio/Plug-Ins/Components`<br>`Macintosh HD/Library/Audio/Plug-Ins/Components` |

### ![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)

| FORMAT | DIRECTORY |
| ------ | --------- |
| VST3<br>x64 | `C:\Program Files\Common Files\VST3` |
| VST3<br>Win32 (x86) | `C:\Program Files(x86)\Common Files\VST3` |

## BUILDING

### ![Visual Studio Code](https://img.shields.io/badge/Visual%20Studio%20Code-0078d7.svg?style=for-the-badge&logo=visual-studio-code&logoColor=white)

```bash
code .
```

- (Recommended) Install [C/C++ Extension Pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack)

### ![Xcode](https://img.shields.io/badge/Xcode-007ACC?style=for-the-badge&logo=Xcode&logoColor=white)

```bash
cmake -Bbuild -GXcode -DJUCE_PATH="<path_to_JUCE>"
```

- Open `build/contrast.xcodeproj`

### ![Visual Studio](https://img.shields.io/badge/Visual%20Studio-5C2D91.svg?style=for-the-badge&logo=visual-studio&logoColor=white)

```bash
cmake -Bbuild -DJUCE_PATH="<path_to_JUCE>"
```

- Open `build/contrast.sln`

### ![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)

```bash
cmake -Bbuild -GNinja -DJUCE_PATH="<path_to_JUCE>"
cmake --build build --config Debug
```
