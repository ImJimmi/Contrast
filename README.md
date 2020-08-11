# CONTRAST

[![Build Status](https://dev.azure.com/ImJimmi/Contrast/_apis/build/status/ImJimmi.Contrast?branchName=azure_pipelines)](https://dev.azure.com/ImJimmi/Contrast/_build/latest?definitionId=3&branchName=azure_pipelines)

A bundle of free, open-source audio plugins built with [JUCE](https://juce.com/).

The plugins in this bundle are free for anyone to use and for anyone to edit. If you would like to contribute to this project you can make a [pull request](https://github.com/ImJimmi/Contrast/pulls), or raise an [issue](https://github.com/ImJimmi/Contrast/issues) to report a bug.

## PLUGINS
| PLUGIN | DESCRIPTION |
| ------ | ----------- |
| [GATE](Gate/) | Look-ahead noise gate with attack and release. |
| [PRESS](Press/) | Simple compressor. |
| [VERB](Verb/) | Simple mono or stereo reverb. |

## DOWNLOAD
![GitHub All Releases](https://img.shields.io/github/downloads/ImJimmi/Contrast/total?label=Total%20Downloads)

![GitHub tag (latest SemVer)](https://img.shields.io/github/v/tag/ImJimmi/Contrast?label=Latest%20Version)
![GitHub Releases](https://img.shields.io/github/downloads/ImJimmi/Contrast/latest/total?label=Latest%20Version%20Downloads)

[Download Latest Version](https://github.com/ImJimmi/Contrast/releases/latest)

## REQUIREMENTS
#### WINDOWS
- VST3 compatible host
- Windows 7 or later (recommended, earlier version may also work)
#### MAC
- VST3 compatible host
- AU compatible host
- OSX 10.7 or later

#### BUILDING FROM SOURCE
- Latest version of [JUCE](https://github.com/juce-framework/JUCE)
- A compatible C++ compiler

## HOW TO USE
### BUILDING
Each of the plugins in the bundle has its own subdirectory. Each of the subdirectories contains a `Source/` folder, a `.jucer` file, and any other resources for that particular plugin. You should use the [Projucer](https://juce.com/discover/projucer) to open the `.jucer` file to then create a project for your chosen IDE.

This repository also contains the [`contrast_shared_resources/`](https://github.com/ImJimmi/Contrast/tree/master/contrast_shared_resources) directory which is formatted as a JUCE module for easy integration. After opening the `.jucer` file in the Projucer you may need to set the proper search path to the directory.

### INSTALLING
To install any of the pre-built plugins, first download the latest release for your platform from the link above. The `.zip` file will contain each of the plugins in the bundle in each format that is available (x64 and Win32 on Windows, VST3 and AU on Mac). Extract the formats that are compatible with your system and place them in a directory where your DAW can find them. Below is a list of common plugin directories:

#### Windows
| Format | Directory |
| ------ | --------- |
| VST | C:\Program Files\VstPlugins |
| VST | C:\Program Files\Steinberg\VstPlugins |
| VST | C:\Program Files\Common Files\VST2 |
| VST | C:\Program Files\Common Files\Steinberg\VST2 |
| VST3 | C:\Program Files\Common Files\VST3 |

(For Win32 plugins, replace `Program Files\` above with `Program Files(x86)\`).

#### MacOS
| Format | Directory |
| ------ | --------- |
| VST | ~/Library/Audio/Plug-Ins/VST |
| VST | Macintosh HD/Library/Audio/Plug-Ins/VST |
| VST3 | ~/Library/Audio/Plug-Ins/VST3 |
| VST3 | Macintosh HD/Library/Audio/Plug-Ins/VST3 |
| AU | ~/Library/Audio/Plug-Ins/Components |
| AU | Macintosh HD/Library/Audio/Plug-Ins/Components |
