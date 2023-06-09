//------------------------------------------------------------------------
// Copyright(c) 2023 Baron von Wenzelheimer.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include <string>

namespace BaronVonWentz {

//------------------------------------------------------------------------
//  MorsalyzerProcessor
//------------------------------------------------------------------------
class MorsalyzerProcessor : public Steinberg::Vst::AudioEffect
{
public:
	MorsalyzerProcessor ();
	~MorsalyzerProcessor () SMTG_OVERRIDE;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/) 
	{ 
		return (Steinberg::Vst::IAudioProcessor*)new MorsalyzerProcessor; 
	}

	//--- ---------------------------------------------------------------------
	// AudioEffect overrides:
	//--- ---------------------------------------------------------------------
	/** Called at first after constructor */
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	
	/** Called at the end before destructor */
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;
	
	/** Switch the Plug-in on/off */
	Steinberg::tresult PLUGIN_API setActive (Steinberg::TBool state) SMTG_OVERRIDE;

	/** Will be called before any process call */
	Steinberg::tresult PLUGIN_API setupProcessing (Steinberg::Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;
	
	/** Asks if a given sample size is supported see SymbolicSampleSizes. */
	Steinberg::tresult PLUGIN_API canProcessSampleSize (Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

	/** Here we go...the process call */
	Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
		
	/** For persistence */
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

	Steinberg::tresult receiveText(const char* text) SMTG_OVERRIDE;

//------------------------------------------------------------------------
protected:
	void convertToMorse(std::string inText, std::string& morse);

	Steinberg::Vst::ParamValue mGain = 0.; 
	Steinberg::Vst::ParamValue mRate = .1f;
	Steinberg::Vst::ParamValue mRUnit = 0.f;
	Steinberg::Vst::ParamValue mEnvelope = .1f;
	Steinberg::Vst::ParamValue mInvert = 0.f;

	std::string newMorse, morse;
	int pos = 0, charPos = 0, length = 1;
	float envGain = 0.f;
	bool on = false, finished = false;
	//time value, read time in ms or timesig?, messagetosend
};

//------------------------------------------------------------------------
} // namespace BaronVonWentz
