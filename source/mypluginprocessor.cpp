//------------------------------------------------------------------------
// Copyright(c) 2023 Baron von Wenzelheimer.
//------------------------------------------------------------------------

#include "mypluginprocessor.h"
#include "myplugincids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/vstaudioprocessoralgo.h"

using namespace Steinberg;

namespace BaronVonWentz {
//------------------------------------------------------------------------
// MorsalyzerProcessor
//------------------------------------------------------------------------
MorsalyzerProcessor::MorsalyzerProcessor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (kMorsalyzerControllerUID);
}

//------------------------------------------------------------------------
MorsalyzerProcessor::~MorsalyzerProcessor ()
{}

//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerProcessor::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated
	
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk)
	{
		return result;
	}

	//--- create Audio IO ------
	addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

	pos = charPos = 0;
	on = false;

	/* If you don't need an event bus, you can remove the next line */
	addEventInput (STR16 ("Event In"), 1);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerProcessor::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!
	
	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerProcessor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----

	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerProcessor::process (Vst::ProcessData& data)
{
	//--- First : Read inputs parameter changes-----------

    if (data.inputParameterChanges)
    {
        int32 numParamsChanged = data.inputParameterChanges->getParameterCount ();
        for (int32 index = 0; index < numParamsChanged; index++)
        {
            if (auto* paramQueue = data.inputParameterChanges->getParameterData (index))
            {
                Vst::ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount ();
                switch (paramQueue->getParameterId ())
                {
				case GainParams::kParamGainId:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
						mGain = value;
					break;
				case GainParams::kParamRateId:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
						mRate = value;
					break;
				case GainParams::kParamRUnitId:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
						mRUnit = value;
					break;
				case GainParams::kParamEnvelopeId:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
						mEnvelope = value;
					break;
				case GainParams::kParamInvertId:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
						mInvert = value;
					break;
				}
			}
		}
	}

	//-- Flush case: no processing possible
	if (data.numInputs == 0 || data.numSamples == 0)
		return kResultOk;
	
	//--- Here you have to implement your processing

	int32 numChannels = data.inputs[0].numChannels;

	//get audio buffers
	uint32 sampleFramesSize = getSampleFramesSizeInBytes(processSetup, data.numSamples);
	void** in = getChannelBuffersPointer(processSetup, data.inputs[0]);
	void** out = getChannelBuffersPointer(processSetup, data.outputs[0]);

	// check silence flags
	if (data.inputs[0].silenceFlags != 0) {
		//then output is also silence + reset position
		data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;
		pos = charPos = 0;

		//make sure output buffer is clear
		for (int32 i = 0; i < numChannels; i++) {
			if (in[i] != out[i]) {
				memset(out[i], 0, sampleFramesSize);
			}
		}
		
		return kResultOk;
	}

	// produce outputs (not silent)
	data.outputs[0].silenceFlags = 0;

	float gain = mGain;
	int32 rate = std::max<int32>(1, (int32)(mRate * processSetup.sampleRate));
	float env = 1. / (std::min<float>(1., (float)(mEnvelope * processSetup.sampleRate * 10)));
	
	if (morse.compare(newMorse) != 0) {
		morse = newMorse;
		pos = charPos = 0;
	}

	// foreach channel
	for (int32 i = 0; i < numChannels; i++) {
		int32 samples = data.numSamples;
		Vst::Sample32* ptrIn = (Vst::Sample32*)in[i];
		Vst::Sample32* ptrOut = (Vst::Sample32*)out[i];
		Vst::Sample32 tmp;

		//check here to make sure morse is not empty and going to explode before going ahead
		if (morse.length() > 0) {
			//foreach sample in channel
			while (--samples >= 0) {
				//if out of character limit get new character
				if (charPos > length) {
					charPos = 0;

					if (finished) {
						pos++;
						if (pos >= morse.length()) pos = 0;

						switch (morse[pos]) {
						case '.':
							length = rate;
							on = true;
							break;
						case '-':
							length = rate * 3;
							on = true;
							break;
						case ' ':
							length = rate;
							break;
						default:
							break;
						}

						finished = false;
					} else {
						length = rate;
						on = false;
						finished = true;
					}
				}
				// otherwise gain = gain or 1 depending on where we're at in character
				//apply gain
				charPos++;
				if (on && envGain < 1) {
					envGain = std::min<float>(1, envGain + env);
				} else if (!on && envGain > gain){
					envGain = std::max<float>(gain, envGain - env);
				}
				tmp = (*ptrIn++) * envGain;
				(*ptrOut++) = tmp;
			}
		} else { //empty string: bypass
			tmp = (*ptrIn++);
			(*ptrOut++) = tmp;
		}
	}

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	/* if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue; */

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerProcessor::setState (IBStream* state)
{
	if (!state)
		return kResultFalse;

	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer (state, kLittleEndian);
	float gainParam = 0.f;
	if (streamer.readFloat(gainParam) == false)
		return kResultFalse;
	float rateParam = 0.f;
	if (streamer.readFloat(rateParam) == false)
		return kResultFalse;
	
	mGain = gainParam;
	mRate = rateParam;

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerProcessor::getState (IBStream* state)
{
	// here we need to save the model
	float gainSave = mGain;
	float rateSave = mRate;
	IBStreamer streamer (state, kLittleEndian);
	streamer.writeFloat(gainSave);
	streamer.writeFloat(rateSave);

	return kResultOk;
}

tresult MorsalyzerProcessor::receiveText(const char* text) {
	//FDebugPrint("I have recieved '");
	//FDebugPrint(text);
	//FDebugPrint("'!\n");
	
	std::string inText = text;
	//convert to lowercase for convenience
	std::transform(inText.begin(), inText.end(), inText.begin(),
		[](unsigned char c) { return std::tolower(c); });

	//convert to morse
	convertToMorse(inText, newMorse);
	//FDebugPrint(newMorse.c_str());
	//FDebugPrint("\n");
	
	return kResultOk;
}

void MorsalyzerProcessor::convertToMorse(std::string inText, std::string& morse) {
	morse.clear();
	for (char c : inText) {
		switch (c){
		case 'a':
			morse.append(".-");
			break;
		case 'b':
			morse.append("-...");
			break;
		case 'c':
			morse.append("-.-.");
			break;
		case 'd':
			morse.append("-..");
			break;
		case 'e':
			morse.append(".");
			break;
		case 'f':
			morse.append("..-.");
			break;
		case 'g':
			morse.append("--.");
			break;
		case 'h':
			morse.append("....");
			break;
		case 'i':
			morse.append("..");
			break;
		case 'j':
			morse.append(".---");
			break;
		case 'k':
			morse.append("-.-");
			break;
		case 'l':
			morse.append(".-..");
			break;
		case 'm':
			morse.append("--");
			break;
		case 'n':
			morse.append("-.");
			break;
		case 'o':
			morse.append("---");
			break;
		case 'p':
			morse.append(".--.");
			break;
		case 'q':
			morse.append("--.-");
			break;
		case 'r':
			morse.append(".-.");
			break;
		case 's':
			morse.append("...");
			break;
		case 't':
			morse.append("-");
			break;
		case 'u':
			morse.append("..-");
			break;
		case 'v':
			morse.append("...-");
			break;
		case 'w':
			morse.append(".--");
			break;
		case 'x':
			morse.append("-..-");
			break;
		case 'y':
			morse.append("-.--");
			break;
		case 'z':
			morse.append("--..");
			break;
		case '1':
			morse.append(".----");
			break;
		case '2':
			morse.append("..---");
			break;
		case '3':
			morse.append("...--");
			break;
		case '4':
			morse.append("....-");
			break;
		case '5':
			morse.append(".....");
			break;
		case '6':
			morse.append("-....");
			break;
		case '7':
			morse.append("--...");
			break;
		case '8':
			morse.append("---..");
			break;
		case '9':
			morse.append("----.");
			break;
		case '0':
			morse.append("-----");
			break;
		case ' ':
			morse.append(" ");
			break;
		default:
			break;
		}
		morse.append(" ");
	}
}

//------------------------------------------------------------------------
} // namespace BaronVonWentz
