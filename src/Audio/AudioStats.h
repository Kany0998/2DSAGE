#ifndef AUDIOSTATS_H
#define AUDIOSTATS_H

struct AudioStats {
    int requested = 0;      //PlayAt/Play2D calls with a sound name
    int played = 0;         //actually got a channel
    int skippedGap = 0;     //too soon after the same sound
    int outOfRange = 0;     //too far from the listener (PlayAt only)
    int noChannel = 0;      //every channel busy
    int channelsBusy = 0;   //channels playing at the moment of the snapshot
    int loopsInRange = 0;   //Number of loops in range
    int loopsPlaying = 0;   //Number of playing loops in range for example max_loops = 5 but we have 6 loop emiters so we play only 5
};

#endif