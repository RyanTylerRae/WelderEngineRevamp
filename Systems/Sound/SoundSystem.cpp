///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{
namespace Tags
{
  DefineTag(Sound);
}

namespace Events
{
  DefineEvent(MIDINoteOn);
  DefineEvent(MIDINoteOff);
  DefineEvent(MIDIPitchWheel);
  DefineEvent(MIDIVolume);
  DefineEvent(MIDIModWheel);
  DefineEvent(MIDIOtherControl);
  DefineEvent(SoundInstancePlayed);
  DefineEvent(MicrophoneUncompressedFloatData);
  DefineEvent(MicrophoneCompressedByteData);
}

namespace Z
{
  SoundSystem* gSound;
}

System* CreateSoundSystem()
{
  return new SoundSystem();
}

//-------------------------------------------------------------------------------------- Sound Event

ZilchDefineType(SoundEvent, builder, type)
{
  ZeroBindDocumented();
}

//--------------------------------------------------------------------------------------- MIDI Event

ZilchDefineType(MidiEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindField(Channel);
  ZilchBindField(MIDINumber);
  ZilchBindField(Value);
}

//--------------------------------------------------------------------------- Audio Float Data Event

ZilchDefineType(AudioFloatDataEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindField(Channels);
  ZilchBindMember(AudioData);
}

//---------------------------------------------------------------------------- Audio Byte Data Event

ZilchDefineType(AudioByteDataEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindMember(AudioData);
}

//------------------------------------------------------------------------------------- Sound System

//**************************************************************************************************
ZilchDefineType(SoundSystem, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);

  ZilchBindGetterSetter(SystemVolume);
  ZilchBindGetter(PeakOutputLevel);
  ZilchBindGetter(RMSOutputLevel);
  ZilchBindGetter(PeakInputLevel);
  ZilchBindMethod(GetNodeGraphInfo);
  ZilchBindGetterSetter(LatencySetting);
  ZilchBindGetterSetter(DispatchMicrophoneUncompressedFloatData);
  ZilchBindGetterSetter(DispatchMicrophoneCompressedByteData);
  ZilchBindGetter(OutputChannels);
  ZilchBindGetterSetter(MuteAllAudio);

  ZilchBindMethod(VolumeNode);
  ZilchBindMethod(PanningNode);
  ZilchBindMethod(PitchNode);
  ZilchBindMethod(LowPassNode);
  ZilchBindMethod(HighPassNode);
  ZilchBindMethod(BandPassNode);
  ZilchBindMethod(EqualizerNode);
  ZilchBindMethod(ReverbNode);
  ZilchBindMethod(DelayNode);
  ZilchBindMethod(CustomAudioNode);
  ZilchBindMethod(SoundBuffer);
  ZilchBindMethod(FlangerNode);
  ZilchBindMethod(ChorusNode);
  ZilchBindMethod(CompressorNode);
  ZilchBindMethod(ExpanderNode);
  ZilchBindMethod(GeneratedWaveNode);
  ZilchBindMethod(RecordingNode);
  ZilchBindMethod(AddNoiseNode);
  ZilchBindMethod(AdditiveSynthNode);
  ZilchBindMethod(GranularSynthNode);
  ZilchBindMethod(ModulationNode);
  ZilchBindMethod(MicrophoneInputNode);
  ZilchBindMethod(SaveAudioNode);

  ZeroBindEvent(Events::MIDINoteOn, MidiEvent);
  ZeroBindEvent(Events::MIDINoteOff, MidiEvent);
  ZeroBindEvent(Events::MIDIPitchWheel, MidiEvent);
  ZeroBindEvent(Events::MIDIVolume, MidiEvent);
  ZeroBindEvent(Events::MIDIModWheel, MidiEvent);
  ZeroBindEvent(Events::MIDIOtherControl, MidiEvent);
  ZeroBindEvent(Events::SoundInstancePlayed, SoundInstanceEvent);
  ZeroBindEvent(Events::MicrophoneUncompressedFloatData, AudioFloatDataEvent);
  ZeroBindEvent(Events::MicrophoneCompressedByteData, AudioByteDataEvent);
}

//**************************************************************************************************
SoundSystem::SoundSystem() :
  mCounter(0),
  mPreviewInstance(0),
  mLatency(AudioLatency::Low),
  mSendMicEvents(false),
  mSendCompressedMicEvents(false),
  mSoundSpaceCounter(0)
{

}

//**************************************************************************************************
SoundSystem::~SoundSystem()
{
  // If currently previewing a sound, stop
  SoundInstance* previewInstance = mPreviewInstance;
  if (previewInstance)
  {
    previewInstance->Stop();
    previewInstance->mSoundNode->mNode = nullptr;
    mPreviewInstance = nullptr;
  }

  mOutputNode->mNode = nullptr;

  Zero::Status status;
  mAudioSystem->StopSystem(status);
  SafeDelete(mAudioSystem);
}

//**************************************************************************************************
void SoundSystem::Initialize(SystemInitializer& initializer)
{
  Z::gSound = this;

  //Create a System object and initialize.
  Zero::Status status;
  mAudioSystem = new Audio::AudioSystemInterface(this);
  mAudioSystem->StartSystem(status);
  if (status.Failed())
    DoNotifyWarning("Audio Initialization Unsuccessful", status.Message);

  SoundNode* node = new SoundNode();
  node->mNode = new Audio::CombineNode("AudioOutput", mCounter++, nullptr);
  mAudioSystem->AddNodeToOutput(node->mNode);
  mOutputNode = node;
  
  InitializeResourceManager(SoundManager);
  SoundManager::GetInstance()->SetSystem(mAudioSystem);
  InitializeResourceManager(SoundCueManager);
  InitializeResourceManager(SoundTagManager);
  InitializeResourceManager(SoundAttenuatorManager);
}

//**************************************************************************************************
NodeInfoListType::range SoundSystem::GetNodeGraphInfo()
{
  return NodeGraph.GetNodeInfoList();
}

//**************************************************************************************************
float SoundSystem::GetSystemVolume()
{
  return mAudioSystem->GetVolume();
}

//**************************************************************************************************
void SoundSystem::SetSystemVolume(float volume)
{
  mAudioSystem->SetVolume(Math::Max(volume, 0.0f));
}

//**************************************************************************************************
bool SoundSystem::GetMuteAllAudio()
{
  return mAudioSystem->GetMuteAllAudio();
}

//**************************************************************************************************
void SoundSystem::SetMuteAllAudio(bool muteAudio)
{
  mAudioSystem->SetMuteAllAudio(muteAudio);
}

//**************************************************************************************************
float SoundSystem::GetPeakOutputLevel()
{
  return mAudioSystem->GetPeakOutputVolume();
}

//**************************************************************************************************
float SoundSystem::GetRMSOutputLevel()
{
  return mAudioSystem->GetRMSOutputVolume();
}

//**************************************************************************************************
float SoundSystem::GetPeakInputLevel()
{
  Status status;
  float volume = mAudioSystem->GetPeakInputVolume(status);
  
  if (status.Failed())
    DoNotifyException("Audio Error", status.Message);

  return volume;
}

//**************************************************************************************************
Zero::AudioLatency::Enum SoundSystem::GetLatencySetting()
{
  return mLatency;
}

//**************************************************************************************************
void SoundSystem::SetLatencySetting(AudioLatency::Enum latency)
{
  mLatency = latency;

  if (latency == AudioLatency::High)
    mAudioSystem->UseHighLatency(true);
  else
    mAudioSystem->UseHighLatency(false);
}

//**************************************************************************************************
bool SoundSystem::GetDispatchMicrophoneUncompressedFloatData()
{
  return mSendMicEvents;
}

//**************************************************************************************************
void SoundSystem::SetDispatchMicrophoneUncompressedFloatData(bool dispatchData)
{
  mSendMicEvents = dispatchData;
  mAudioSystem->SetSendUncompressedMicInput(dispatchData);
}

//**************************************************************************************************
bool SoundSystem::GetDispatchMicrophoneCompressedByteData()
{
  return mSendCompressedMicEvents;
}

//**************************************************************************************************
void SoundSystem::SetDispatchMicrophoneCompressedByteData(bool dispatchData)
{
  mSendCompressedMicEvents = dispatchData;
  mAudioSystem->SetSendCompressedMicInput(dispatchData);
}

//**************************************************************************************************
int SoundSystem::GetOutputChannels()
{
  return mAudioSystem->GetOutputChannels();
}

//**************************************************************************************************
void SoundSystem::Update(bool debugger)
{
  if (debugger)
    return;
  
  // Update audio system 
  mAudioSystem->Update();

  // Update spaces (also updates emitters)
  forRange(SoundSpace& space, mSpaces.All())
    space.Update();
}

//**************************************************************************************************
void SoundSystem::StopPreview()
{
  SoundInstance *sound = mPreviewInstance;
  if (sound)
  {
    sound->Stop();
    mPreviewInstance = nullptr;
  }
}

//**************************************************************************************************
float SoundSystem::PitchToSemitones(float pitch)
{
  if (pitch == 0)
    return 0;
  else
    return 3986.0f * Math::Log10(Math::Exp2(pitch)) / 100.0f;
}

//**************************************************************************************************
float SoundSystem::SemitonesToPitch(float semitone)
{
  return Math::Log2(Math::Exp2(semitone / 12.0f));
}

//**************************************************************************************************
float SoundSystem::VolumeToDecibels(float volume)
{
  if (volume == 0.0f)
    return -100.0f;
  else
  {
    float decibels = 20.0f * Math::Log10(volume);
    if (decibels < -100.0f)
      decibels = -100.0f;
    return decibels;
  }
}

//**************************************************************************************************
float SoundSystem::DecibelsToVolume(float decibels)
{
  return Math::Pow(10.0f, decibels / 20.0f);
}

//**************************************************************************************************
void SoundSystem::SendAudioEvent(const Audio::AudioEventTypes::Enum eventType, void * data)
{
  if (eventType == Audio::AudioEventTypes::AudioClipping)
    DoNotifyWarning("Audio Error", "Audio is too loud and is being clipped. Reduce volume or number of sounds to avoid audio problems.");
  else if (eventType == Audio::AudioEventTypes::MidiNoteOn)
  {
    Audio::MidiData* midiData = (Audio::MidiData*)data;
    MidiEvent event((float)midiData->Channel, midiData->Value1, midiData->Value2);
    DispatchEvent(Events::MIDINoteOn, &event);
    delete (Audio::MidiData*)data;
  }
  else if (eventType == Audio::AudioEventTypes::MidiNoteOff)
  {
    Audio::MidiData* midiData = (Audio::MidiData*)data;
    MidiEvent event((float)midiData->Channel, midiData->Value1, 0);
    DispatchEvent(Events::MIDINoteOff, &event);
    delete (Audio::MidiData*)data;
  }
  else if (eventType == Audio::AudioEventTypes::MidiPitchWheel)
  {
    Audio::MidiData* midiData = (Audio::MidiData*)data;
    MidiEvent event((float)midiData->Channel, 0, midiData->Value1);
    DispatchEvent(Events::MIDIPitchWheel, &event);
    delete (Audio::MidiData*)data;
  }
  else if (eventType == Audio::AudioEventTypes::MidiVolume)
  {
    Audio::MidiData* midiData = (Audio::MidiData*)data;
    MidiEvent event((float)midiData->Channel, 0, midiData->Value1);
    DispatchEvent(Events::MIDIVolume, &event);
    delete (Audio::MidiData*)data;
  }
  else if (eventType == Audio::AudioEventTypes::MidiModWheel)
  {
    Audio::MidiData* midiData = (Audio::MidiData*)data;
    MidiEvent event((float)midiData->Channel, 0, midiData->Value1);
    DispatchEvent(Events::MIDIModWheel, &event);
    delete (Audio::MidiData*)data;
  }
  else if (eventType == Audio::AudioEventTypes::MidiControl)
  {
    Audio::MidiData* midiData = (Audio::MidiData*)data;
    MidiEvent event((float)midiData->Channel, midiData->Value1, midiData->Value2);
    DispatchEvent(Events::MIDIOtherControl, &event);
    delete (Audio::MidiData*)data;
  }
  else if (eventType == Audio::AudioEventTypes::MicInputData)
  {
    Array<float>* buffer = (Array<float>*)data;
    AudioFloatDataEvent event;
    event.Channels = 2;
    event.AudioData = ZilchAllocate(ArrayClass<float>);
    event.AudioData->NativeArray = *buffer;
    DispatchEvent(Events::MicrophoneUncompressedFloatData, &event);
  }
  else if (eventType == Audio::AudioEventTypes::CompressedMicInputData)
  {
    Array<byte>* buffer = (Array<byte>*)data;
    AudioByteDataEvent event;
    event.AudioData = ZilchAllocate(ArrayClass<byte>);
    event.AudioData->NativeArray = *buffer;
    DispatchEvent(Events::MicrophoneCompressedByteData, &event);
  }
}

//**************************************************************************************************
void SoundSystem::SendAudioError(const Zero::String message)
{
  DoNotifyWarning("Audio Error", message.c_str());
}

//**************************************************************************************************
void SoundSystem::AddSoundSpace(SoundSpace* space, bool isEditor)
{
  mSpaces.PushBack(space);

  // If not an editor space, increase the counter and notify tags if necessary
  if (!isEditor)
  {
    ++mSoundSpaceCounter;

    if (mSoundSpaceCounter == 1)
    {
      forRange(SoundTag& tag, mSoundTags.All())
        tag.CreateTag();
    }
  }
}

//**************************************************************************************************
void SoundSystem::RemoveSoundSpace(SoundSpace* space, bool isEditor)
{
  mSpaces.Erase(space);

  // If not an editor space, decrease the counter and notify tags if necessary
  if (!isEditor)
  {
    --mSoundSpaceCounter;

    ErrorIf(mSoundSpaceCounter < 0, "SoundSystem's space tracking has become negative");

    if (mSoundSpaceCounter == 0)
    {
      forRange(SoundTag& tag, mSoundTags.All())
        tag.ReleaseTag();
    }
  }
}

//----------------------------------------------------------------------------------- Audio Settings

//**************************************************************************************************
ZilchDefineType(AudioSettings, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindTag(Tags::Sound);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(SystemVolume)->Add(new EditorSlider(0.0f, 2.0f, 0.01f));
  ZilchBindGetterSetterProperty(MuteAllAudio);
  ZilchBindGetterSetterProperty(MixType); 
  ZilchBindGetterSetterProperty(MinVolumeThreshold)->Add(new EditorSlider(0.0f, 0.2f, 0.001f));
  ZilchBindGetterSetterProperty(LatencySetting);
}

//**************************************************************************************************
void AudioSettings::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSystemVolume, 1.0f);
  SerializeEnumNameDefault(AudioMixTypes, mMixType, AudioMixTypes::AutoDetect);
  SerializeNameDefault(mMinVolumeThreshold, 0.015f);
  SerializeEnumNameDefault(AudioLatency, mLatency, AudioLatency::Low);
}

//**************************************************************************************************
void AudioSettings::Initialize(CogInitializer& initializer)
{
  Z::gSound->mAudioSystem->SetVolume(mSystemVolume);
  SetMixType(mMixType);
  Z::gSound->mAudioSystem->SetMinimumVolumeThreshold(mMinVolumeThreshold);
  Z::gSound->SetLatencySetting(mLatency);
}

//**************************************************************************************************
float AudioSettings::GetSystemVolume()
{
  mSystemVolume = Z::gSound->mAudioSystem->GetVolume();
  return mSystemVolume;
}

//**************************************************************************************************
void AudioSettings::SetSystemVolume(float volume)
{
  mSystemVolume = Math::Max(volume, 0.0f);

  Z::gSound->mAudioSystem->SetVolume(mSystemVolume);
}

//**************************************************************************************************
bool AudioSettings::GetMuteAllAudio()
{
  return Z::gSound->GetMuteAllAudio();
}

//**************************************************************************************************
void AudioSettings::SetMuteAllAudio(bool muteAudio)
{
  Z::gSound->SetMuteAllAudio(muteAudio);
}

//**************************************************************************************************
AudioMixTypes::Enum AudioSettings::GetMixType()
{
  return mMixType;
}

//**************************************************************************************************
void AudioSettings::SetMixType(AudioMixTypes::Enum mixType)
{
  mMixType = mixType;

  switch (mixType)
  {
  case AudioMixTypes::AutoDetect:
    Z::gSound->mAudioSystem->SetOutputChannels(0);
    break;
  case AudioMixTypes::Mono:
    Z::gSound->mAudioSystem->SetOutputChannels(1);
    break;
  case AudioMixTypes::Stereo:
    Z::gSound->mAudioSystem->SetOutputChannels(2);
    break;
  case AudioMixTypes::Quad:
    Z::gSound->mAudioSystem->SetOutputChannels(4);
    break;
  case AudioMixTypes::FiveOne:
    Z::gSound->mAudioSystem->SetOutputChannels(6);
    break;
  case AudioMixTypes::SevenOne:
    Z::gSound->mAudioSystem->SetOutputChannels(8);
    break;
  default:
    Z::gSound->mAudioSystem->SetOutputChannels(2);
    break;
  }
}

//**************************************************************************************************
float AudioSettings::GetMinVolumeThreshold()
{
  return mMinVolumeThreshold;
}

//**************************************************************************************************
void AudioSettings::SetMinVolumeThreshold(float volume)
{
  mMinVolumeThreshold = Math::Max(volume, 0.0f);
  Z::gSound->mAudioSystem->SetMinimumVolumeThreshold(mMinVolumeThreshold);
}

//**************************************************************************************************
Zero::AudioLatency::Enum AudioSettings::GetLatencySetting()
{
  return mLatency;
}

//**************************************************************************************************
void AudioSettings::SetLatencySetting(AudioLatency::Enum latency)
{
  mLatency = latency;
  Z::gSound->SetLatencySetting(latency);
}

}//namespace Zero
