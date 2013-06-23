This is how we disappear
bobbevy

Realtime video program for a dance [performance](http://vimeo.com/65824287), seek to 20 minutes in.  Hard to see the video, the camerapersons are focusing on the dancers (as they should be!)  

Not super reusable, but I'm sharing this so others can peek at bits of code.

Useful bits of code for others:

* bbDepthProcessor.h/.cpp -> This does the background subtraction and blob processing
* BonjourPublisher.h/.mm -> This publishes an OSC port for use by [Control](http://charlie-roberts.com/Control/)  

Useful concepts?

I've got a "performance mode" which will display a fullscreen window with the content on any secondary displays as soon as they are connected.  This is nice because you can be connected/disconnected from a projector a few times before performing and you don't have to worry about it.

I'm using a [QuNeo](http://www.keithmcmillen.com/QuNeo/) to trigger cues using Andrea's really nice QTimeline block.

I've got a very hacked up "midi learn" feature for parameters as well.  Hoping to expand on this a bit. 


Building:

Probably not easy for someone else to build at this point.  But here are some notes:

* You need the dev branch of Cinder, specifically, this [one](https://github.com/bzztbomb/Cinder/tree/display_change_notifications)
* You need the CinderKinect, CinderOpenCV, and OSC blocks
* You need the QTimeline block, specifically, this [one](https://github.com/bzztbomb/QTimeline/tree/hacked_up_branch)
* You need [LabMidi](https://github.com/meshula/LabMidi)

I think that's it.  Point CINDER_PATH variable at the right place and update the LabMidi project link and you should be able to build and run.