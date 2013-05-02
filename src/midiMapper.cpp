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

  MidiMapper* bb = static_cast<MidiMapper*>(userData);
  boost::mutex::scoped_lock lock(bb->mCommandMutex);
  bb->mCommandQueue.push(*m);
}

void MidiMapper::update()
{
  Lab::MidiCommand cmd;
  while (getNextCommand(&cmd))
  {
    if (cmd.command == MIDI_NOTE_ON)
    {
      switch (cmd.byte1)
      {
        // Stop
        case 24 :
          {
            mTimeline->play(false);
            mTimeline->getTimelineRef()->stepTo(0.01f);
            mTimeline->update();
            mTimeline->getTimelineRef()->stepTo(0.0f);
            mTimeline->update();
            for (int i = 0; i < 32; i++)
              midiOut->sendNoteOn(0, i, 0);
          }
          break;
        // Play cue case
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
          {
            int offPos = cmd.byte1 - 36;
            // Play the cue
            if (mTimeline->isPlaying())
              mTimeline->play(false);
            mTimeline->playCue(offPos);
            
            // Turn on the lights.
            int pos = (offPos + 1) * 2;
            for (int i = pos; i < 32; i++)
            {
              midiOut->sendNoteOn(0, i, 0);
            }
            for (int i = 0; i < pos; i++)
            {
              midiOut->sendNoteOn(0, i, 127);
            }
          }
          break;
        default:
          break;
      }
    }
  }
}

bool MidiMapper::getNextCommand(Lab::MidiCommand* dest)
{
  boost::mutex::scoped_lock lock(mCommandMutex);
  if (!mCommandQueue.empty())
  {
    *dest = mCommandQueue.front();
    mCommandQueue.pop();
    return true;
  } else {
    return false;
  }
}
