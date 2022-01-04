# sendosc
Lightweight Algorithm Sonification with Open Sound Control (OSC)

Sendosc is a lighwight implementation of the OSC protocol for data streams that shall be sonified.

Sonification denotes the modelling of acoustic representations of data.
Acoustic data modelling is particularly useful for the portrayal of time series.
Time series of data map naturally to acoustic time series and human auditory perception has a high temporal resolution, i.e., distinguishable events per second.

Runs of algorithms can be understood as a time series of internal states and events.
Algorithm sonification denotes the acoustic modelling of that data.
That involves two steps:

  * Data selection: Select data points for use in acoustic modelling
  * Acoustic modelling: Decide how data points shape the sound scape

Open Sound Control (OSC) is a UDP-based network protocol.
Most digital sound synthesizers implement interfaces to receive and process OSC signals.

Sendosc can be used by algorithm designers to analyse runs of their algorithms by means of their auditory perception organs.
Sendosc can _not_ be used for acoustic modelling, as this is done in digital sound synthesis tools which map the incoming OSC messages to actual sound.
Sendosc can be easily integrated in algorithms for sending data points in a format that is implemented by most sound synthesis tools.
