//
//  midiMapper.cpp
//  bobbevy
//
//  Created by Brian Richardson on 5/1/13.
//
//

#include "midiMapper.h"
#include <iostream>

using namespace std;

MidiMapper::MidiMapper()
: midiIn(NULL)
, midiOut(NULL)
{
  
}

MidiMapper::~MidiMapper()
{
  delete midiIn;
  delete midiOut;
}

void MidiMapper::init(QTimelineRef timeline)
{
  mTimeline = timeline;
  
  // MIDI init
  int inPort = 0;
  int outPort = 0;
  Lab::MidiPorts ports;
  std::string searchFor("QUNEO");
  for (int i = 0; i < ports.inPorts(); i++)
  {
    if (ports.inPort(i) == searchFor)
    {
      inPort = i;
      cout << "QuNeo input found." << endl;
    }
  }
  for (int i = 0; i < ports.outPorts(); i++)
  {
    if (ports.outPort(i) == searchFor)
    {
      outPort = i;
      cout << "QuNeo output found." << endl;
    }
  }
  
  midiIn = new Lab::MidiIn();
  midiOut = new Lab::MidiOut();
  midiIn->addCallback(MidiMapper::midiCallback, this);
  midiIn->openPort(inPort);
  midiOut->openPort(outPort);
}

void MidiMapper::midiCallback(void* userData, Lab::MidiCommand* m)
{
  if (!m)
    return;
  if (m->command == MIDI_NOTE_ON)
  {
    MidiMapper* bb = static_cast<MidiMapper*>(userData);
    int offPos = m->byte1 - 36;
    
    switch (offPos)
    {
      case -12:
        offPos = -1;
        break;
      default:
        if ((offPos < 0) || (offPos > 15))
          return;
    }
    
    // Push it to the command queue.
    {
      boost::mutex::scoped_lock lock(bb->mCommandMutex);
      bb->mCommandQueue.push(offPos);
    }
    
    std::cout << "NOTE: " << (int) m->byte1 << " " << Lab::noteName(m->byte1) << " POS: " << offPos << std::endl;
    int pos = (offPos + 1) * 2;
    for (int i = pos; i < 32; i++)
    {
      bb->midiOut->sendNoteOn(0, i, 0);
    }
    for (int i = 0; i < pos; i++)
    {
      bb->midiOut->sendNoteOn(0, i, 127);
    }
  }
}

void MidiMapper::update()
{
  boost::mutex::scoped_lock lock(mCommandMutex);
  int mCue = -2;
  while (!mCommandQueue.empty())
  {
    mCue = mCommandQueue.front();
    mCommandQueue.pop();
  }
  if (mCue != -2)
  {
    if (mCue != -1)
    {
      if (mTimeline->isPlaying())
        mTimeline->play(false);
      mTimeline->playCue(mCue);
    } else {
      mTimeline->play(false);
      mTimeline->getTimelineRef()->stepTo(0.01f);
      mTimeline->update();
      mTimeline->getTimelineRef()->stepTo(0.0f);
      mTimeline->update();
    }
  }
}

