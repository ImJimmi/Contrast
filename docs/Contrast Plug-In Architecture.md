# Contrast Plug-In Architecture

Although the plug-ins in Contrast are very straightforward, following a suitable, scalable, consistent architecture across the bundle ensures maintenance is kept to a minimum.

## Model-View-Presenter

Contrast follows the Model-View-Presenter architecture, as described by Martin Fowler in [GUI Architectures](https://www.martinfowler.com/eaaDev/uiArchs.html#Model-view-presentermvp). Since this pattern is most commonly used in standalone desktop applications, and sometimes web apps, there are some considerations to make when adapting to an audio plug-in.

### Ownership

In a typical desktop application, some sort of `App` class would be the entry point for the application. The `App` class would own the top-level node of the Model, and the top-level node of Presenters (usually a `WindowPresenter` of some kind).

In a JUCE audio plug-in, instead of an `App` class to represent an instance of an application, an `AudioProcessor` is used to represent an instance of a plug-in. For that reason, each project should start with a class derived from `juce::AudioProcessor` which should own the top-level node of the plug-in's Model, a Presenter for presenting the plug-in's audio, and a Presenter responsible for presenting the plug-in's editor (if it has one).

### Real-Time Thread-Safe Models

In MVP, and other similar architectures, the Model is responsible for storing, and managing access to, the system's state. In an audio plug-in, most of the state will be the values of the plug-in's audio parameters, and since these values are typically required to be accessed from both a real-time thread (the audio thread) and a non-real-time thread (the message thread), it is important to ensure the implementation of a plug-in's Model is real-time thread safe.

Most JUCE plug-ins nowadays make use of the `AudioProcessorValueTreeState`, or APVTS, to store and manage the values of the plug-in's parameters. Although a very useful class, the APVTS misses some niceties expected from a typical Model - such as type-safety, or a fully representative interface - so Contrast wraps the APVTS in `PlugInState` classes to introduce the nicer, project-specific interface on top.
