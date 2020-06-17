# Contrast
A bundle of free, open-source audio plugins built with [JUCE](https://juce.com/).

The plugins in this bundle are free for anyone to use and for anyone to edit. If you would like to contribute to this project you can make a [pull request](https://github.com/ImJimmi/Contrast/pulls), or raise an [issue](https://github.com/ImJimmi/Contrast/issues) to report a bug.

## Plugins
- [Gate](Gate/) - Look-ahead noise gate with attack and release.
- [Press](Press/) - Simple compressor.
- [Verb](Verb/) - Simple mono or stereo reverb.

## Requirements
- [JUCE](https://github.com/juce-framework/JUCE)

## How to use
### Building
Each of the plugins in this bundle has its own subdirectory in this repository. Each of the subdirectories contains a `Source/` folder, a `.jucer`, and any other resources for that particular plugin. You should use the [Projucer](https://juce.com/discover/projucer) to open the `.jucer` file to then create a project for your chosen IDE.

This repository also contains the [`contrast_shared_resources`](https://github.com/ImJimmi/Contrast/tree/master/contrast_shared_resources) directory which is formatted as a JUCE module for easy integration. After opening the `.jucer` file in the Projucer you may need to set the proper search path to the directory.

### Installing
To install any of the pre-built plugins, go to that plugin's subdirectory using the links above and fnd the relavent download link for your system. There's no installers so you'll have to manually place the plugin files in the correct place and ensure whatever host you're using is set to search the same locatons.
