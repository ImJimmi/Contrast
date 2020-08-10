# Contrast
[![Build Status](https://dev.azure.com/ImJimmi/Contrast/_apis/build/status/ImJimmi.Contrast?branchName=azure_pipelines)](https://dev.azure.com/ImJimmi/Contrast/_build/latest?definitionId=3&branchName=azure_pipelines)

A bundle of free, open-source audio plugins built with [JUCE](https://juce.com/).

The plugins in this bundle are free for anyone to use and for anyone to edit. If you would like to contribute to this project you can make a [pull request](https://github.com/ImJimmi/Contrast/pulls), or raise an [issue](https://github.com/ImJimmi/Contrast/issues) to report a bug.

## Plugins
- [Gate](Gate/) - Look-ahead noise gate with attack and release.
- [Press](Press/) - Simple compressor.
- [Verb](Verb/) - Simple mono or stereo reverb.

## Download
![GitHub All Releases](https://img.shields.io/github/downloads/ImJimmi/Contrast/total?label=Total%20Downloads)
![GitHub Releases](https://img.shields.io/github/downloads/ImJimmi/Contrast/latest/total?label=Latest%20Version%20Downloads)

[Download Latest Version](https://github.com/ImJimmi/Contrast/releases/latest)

## Requirements
For running the pre-built plug-ins there are no other requirements besides a compatible host:
#### Windows
- VST3 compatible host
- Windows 7 or later (recommended, earlier version may also work)
#### MacOS
- VST3 compatible host
- AU compatible host
- OSX 10.7 or later

#### Requirements for development
- Latest version of [JUCE](https://github.com/juce-framework/JUCE)

## How to use
### Building
Each of the plugins in this bundle has its own subdirectory in this repository. Each of the subdirectories contains a `Source/` folder, a `.jucer`, and any other resources for that particular plugin. You should use the [Projucer](https://juce.com/discover/projucer) to open the `.jucer` file to then create a project for your chosen IDE.

This repository also contains the [`contrast_shared_resources`](https://github.com/ImJimmi/Contrast/tree/master/contrast_shared_resources) directory which is formatted as a JUCE module for easy integration. After opening the `.jucer` file in the Projucer you may need to set the proper search path to the directory.

### Installing
To install any of the pre-built plugins, go to that plugin's subdirectory using the links above and fnd the relavent download link for your system. There's no installers so you'll have to manually place the plugin files in the correct place and ensure whatever host you're using is set to search the same locatons.
