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

void midiPrintCallback(void* userData, Lab::MidiCommand* c)
{
  if (c) {
    std::cout << Lab::commandName(c->command) << " "
    << Lab::noteName(c->byte1) << " ";
    
    if (c->command <= 0x9f)
      std::cout << "vel: ";
    
    uint8_t ct = c->command >> 4;
    if ((ct != 0xc) && (ct != 0xd))
      std::cout << int(c->byte2) << std::endl;
  }
}

// Totally sweet hack d00d
MidiMapper* MidiMapper::smInstance = NULL;

MidiMapper::MidiMapper()
: midiIn(NULL)
, midiOut(NULL)
{
  smInstance = this;
  mNextEventInfo.mHandle = 0;
}

MidiMapper::~MidiMapper()
{
  delete midiIn;
  delete midiOut;
}

void MidiMapper::init(QTimelineRef timeline, DepthProcessor* kinect)
{
  mTimeline = timeline;
  mKinect = kinect;
  
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
//  midiIn->addCallback(midiPrintCallback, NULL);
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
    // Convert note on to note off
    if ((cmd.command & 0xF0) == MIDI_NOTE_OFF)
    {
      cmd.command = MIDI_NOTE_ON | (cmd.command & 0x0F);
    }
    // Filter out pressure for now.
    if ((cmd.command & 0xF0) == MIDI_CONTROL_CHANGE)
    {
      if (cmd.byte1 >= 12)
        continue;
    }
    // Check for midi learn
    if (mNextEventInfo.mFn)
    {
      int key = hashCommand(&cmd);
      mEventMap[key] = mNextEventInfo;
      mNextEventInfo.mFn = NULL;
      mNextEventInfo.mHandle++;
      continue;
    }
    // Hardcoded note on stuff
    if (cmd.command == MIDI_NOTE_ON)
    {
      switch (cmd.byte1)
      {
        // Stop
        case 24 :
          stop();
          break;
        case 25 :
          mKinect->resetBackground();
          break;
        case 26 :
//          mKinect->setEnabled(!mKinect->getEnabled());
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
          playCue(&cmd);
          break;
        default:
          checkMapped(&cmd);
          break;
      }
    } else {
      checkMapped(&cmd);
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

int MidiMapper::hashCommand(Lab::MidiCommand* c)
{
  return (c->command << 8) + c->byte1;
}

void MidiMapper::stop()
{
  mTimeline->play(false);
  mTimeline->getTimelineRef()->stepTo(0.01f);
  mTimeline->update();
  mTimeline->getTimelineRef()->stepTo(0.0f);
  mTimeline->update();
  for (int i = 0; i < 32; i++)
    midiOut->sendNoteOn(0, i, 0);
}

void MidiMapper::playCue(Lab::MidiCommand* c)
{
  if (c->byte2 == 0)
    return;
  int offPos = c->byte1 - 36;
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

void MidiMapper::checkMapped(Lab::MidiCommand* c)
{
  int key = hashCommand(c);
  auto i = mEventMap.find(key);
  if (i != mEventMap.end())
  {
    i->second.mFn((float)c->byte2 / 127.0f);
  }
}

int MidiMapper::midiLearn(ValueUpdateFn fn)
{
  mNextEventInfo.mFn = fn;
  return mNextEventInfo.mHandle;
}

void MidiMapper::midiForget(int handle)
{
  for (auto i = mEventMap.begin(); i != mEventMap.end(); i++)
  {
    if (i->second.mHandle == handle)
    {
      mEventMap.erase(i);
      return;
    }
  }
}