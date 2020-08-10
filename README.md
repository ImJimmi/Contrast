# CONTRAST

[![Build Status](https://dev.azure.com/ImJimmi/Contrast/_apis/build/status/ImJimmi.Contrast?branchName=azure_pipelines)](https://dev.azure.com/ImJimmi/Contrast/_build/latest?definitionId=3&branchName=azure_pipelines)

A bundle of free, open-source audio plugins built with [JUCE](https://juce.com/).

The plugins in this bundle are free for anyone to use and for anyone to edit. If you would like to contribute to this project you can make a [pull request](https://github.com/ImJimmi/Contrast/pulls), or raise an [issue](https://github.com/ImJimmi/Contrast/issues) to report a bug.

## PLUGINS
- [Gate](Gate/) - Look-ahead noise gate with attack and release.
- [Press](Press/) - Simple compressor.
- [Verb](Verb/) - Simple mono or stereo reverb.

## DOWNLOAD
![GitHub All Releases](https://img.shields.io/github/downloads/ImJimmi/Contrast/total?label=Total%20Downloads)

![GitHub tag (latest SemVer)](https://img.shields.io/github/v/tag/ImJimmi/Contrast?label=Latest%20Version)
![GitHub Releases](https://img.shields.io/github/downloads/ImJimmi/Contrast/latest/total?label=Latest%20Version%20Downloads)

[Download Latest Version](https://github.com/ImJimmi/Contrast/releases/latest)

## REQUIREMENTS
For running the pre-built plug-ins there are no other requirements besides a compatible host:
#### WINDOWS
- VST3 compatible host
- Windows 7 or later (recommended, earlier version may also work)
#### MAC
- VST3 compatible host
- AU compatible host
- OSX 10.7 or later

#### DEVELOPMENT
- Latest version of [JUCE](https://github.com/juce-framework/JUCE)

## HOW TO USE
### BUILDING
Each of the plugins in this bundle has its own subdirectory in this repository. Each of the subdirectories contains a `Source/` folder, a `.jucer`, and any other resources for that particular plugin. You should use the [Projucer](https://juce.com/discover/projucer) to open the `.jucer` file to then create a project for your chosen IDE.

This repository also contains the [`contrast_shared_resources`](https://github.com/ImJimmi/Contrast/tree/master/contrast_shared_resources) directory which is formatted as a JUCE module for easy integration. After opening the `.jucer` file in the Projucer you may need to set the proper search path to the directory.

### INSTALLING
To install any of the pre-built plugins, go to that plugin's subdirectory using the links above and fnd the relavent download link for your system. There's no installers so you'll have to manually place the plugin files in the correct place and ensure whatever host you're using is set to search the same locatons.
