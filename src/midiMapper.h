//
//  midiMapper.h
//  bobbevy
//
//  Created by Brian Richardson on 5/1/13.
//
//

#ifndef __bobbevy__midiMapper__
#define __bobbevy__midiMapper__

#include <queue>
#include <unordered_map>

#include <boost/thread.hpp>

#include "LabMidi/LabMidiCommand.h"
#include "LabMidi/LabMidiIn.h"
#include "LabMidi/LabMidiOut.h"
#include "LabMidi/LabMidiPorts.h"
#include "LabMidi/LabMidiUtil.h"

#include "QTimeline.h"

typedef std::function<void (float)> ValueUpdateFn;

class MidiMapper
{
public:
  MidiMapper();
  ~MidiMapper();

  void init(QTimelineRef timeline);
  void update();
  
  // This will associate fn with the next midi event.  Returns a handle
  // that should be passed to midiforget
  int midiLearn(ValueUpdateFn fn);

  void midiForget(int handle);
public:
  static MidiMapper* instance() { return smInstance; }
private:
  // Midi
  Lab::MidiIn*         midiIn;
  Lab::MidiOut*        midiOut;

  // Command queue
  std::queue<Lab::MidiCommand> mCommandQueue;
  boost::mutex mCommandMutex;

  // Mapped midi events
  struct EventInfo
  {
    ValueUpdateFn mFn;
    int mHandle;
  };
  std::unordered_map<int, EventInfo> mEventMap;
  EventInfo mNextEventInfo;
  
  // Timeline (bit of a hack right now)
  QTimelineRef mTimeline;

  bool getNextCommand(Lab::MidiCommand* dest);
  
  int hashCommand(Lab::MidiCommand* c);
  void stop();
  void playCue(Lab::MidiCommand* c);
  void checkMapped(Lab::MidiCommand* c);

private:
  // Totally sweet hack d00d
  static MidiMapper* smInstance;
  static void midiCallback(void* userData, Lab::MidiCommand* m);
  
};

#endif /* defined(__bobbevy__midiMapper__) */
