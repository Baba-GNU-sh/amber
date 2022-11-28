# User Stories

## Initialisation
1. I open up amber and connect to a device.
2. I can see a list of signals produced from the device.
3. I select the signal(s) I want to monitor, and a trace immediately appears in the graph view.
4. I select another handful of signals which are all displayed on the same y-axis.

## Zooming
1. By default, the x axis spans about 5 seconds, the y axis for each graph view spans something sensible... TODO what is sensible?
2. Most of the zooming happens in the time axis, so when I zoom with the scroll wheel, the time axis zooms only.
3. I can zoom in on the y axis by scrolling in the left hand gutter.
4. Min/max zoom on the y axis is adjustable.. how?? Can set this to limit the mix max of all time using, or minmax in the current window with a button press.

## Analysis
1. The graph view pans to keep up with the latest samples, the graph scrolls past at a comfortable rate so I can see the signals waggling around.
2. The sample rate of the signal is higher than the resolution of my screen, so each column of pixels contains 100s of samples. I can see the min/max and average for each column of pixels using three individual lines.
3. I spot an interesting event (large min/max) and I stop the view from scrolling using a keypress.
4. Using the mouse I drag the view back in time to find the interesting event.
5. The event happened within a few milliseconds, so I zoom in on the time axis only using the scroll wheel to reveal more detail.
6. I place a marker on either side of the interesting event to measure the time interval which is shown in a readout.
7. The signal in the event looks oscilliatory, so I line the markers up the zero crossings and read out frequency out of a readout.

## Multiple Traces
1. I realise I need to see another signal with a different y offset. 
2. I can zoom in and out in the y axis of this graph window independenty, but the time (x) axes are synchronized.
3. I can select up to two signals on the same graph (y axes can be controlled indivudually), but I can create a few (how many?) separate graphs stacked vertically.
5. I select another handful of signals with very different y offsets, and pull them onto a second graph view.
6. This second graph view has an independent y axis, but the time (x) axes are synchronized.


