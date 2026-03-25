# SendOSC

Lightweight Algorithm Sonification with Open Sound Control (OSC):

SendOSC is a lightweight implementation of the OSC protocol for sonification data streams. Sonification is the process of creating acoustic representations of data. Acoustic data modeling is particularly useful for portraying time series, as they naturally correspond to acoustic time series. Furthermore, the human auditory system has high temporal resolution, meaning it can distinguish many events per second.

Runs of algorithms can be understood as time series of internal states and events. Algorithm sonification involves the acoustic modeling of that data. This process involves two steps:

1. Data Selection: Select data points to use in acoustic modeling.
2. Acoustic Modeling: Determine how the data points shape the soundscape.

Open Sound Control (OSC) is a User Datagram Protocol (UDP)-based network protocol. Most digital sound synthesizers have interfaces that can receive and process OSC signals.

SendOSC helps algorithm designers analyze their algorithms' performance using auditory perception. It can be integrated into algorithms to send data points in a format implemented by most sound synthesis tools. However, SendOSC cannot be used for acoustic modeling, which is a task performed by digital sound synthesis tools that map incoming OSC messages to sound.
