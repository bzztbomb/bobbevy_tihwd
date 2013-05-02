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

typedef std::function<void (float, void*)> ValueUpdateFn;

class MidiMapper
{
public:
  MidiMapper();
  ~MidiMapper();

  void init(QTimelineRef timeline);
  void update();
  
  // This will associate fn with the next midi event.
  void midiLearn(ValueUpdateFn fn);
private:
  // Midi
  Lab::MidiIn*         midiIn;
  Lab::MidiOut*        midiOut;

  // Command queue
  std::queue<Lab::MidiCommand> mCommandQueue;
  boost::mutex mCommandMutex;

  // Mapped midi events
  std::unordered_map<int, ValueUpdateFn> mEventMap;
  
  // Timeline (bit of a hack right now)
  QTimelineRef mTimeline;

  bool getNextCommand(Lab::MidiCommand* dest);
  
  int hashE
private:
  static void midiCallback(void* userData, Lab::MidiCommand* m);
};

#endif /* defined(__bobbevy__midiMapper__) */
