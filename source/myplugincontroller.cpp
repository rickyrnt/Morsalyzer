//------------------------------------------------------------------------
// Copyright(c) 2023 Baron von Wenzelheimer.
//------------------------------------------------------------------------

#include "myplugincontroller.h"
#include "myplugincids.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "base/source/fstreamer.h"

#include <cmath>

using namespace Steinberg;
using namespace VSTGUI;

namespace BaronVonWentz {

//------------------------------------------------------------------------
// MorsalyzerController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerController::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize (context);
	if (result != kResultOk)
	{
		return result;
	}

	// Here you could register some parameters
	parameters.addParameter(STR16("Gain"), STR16("dB"), 0, 0, Vst::ParameterInfo::kCanAutomate, GainParams::kParamGainId, 0);
	parameters.addParameter(STR16("Rate"), STR16("ms"), 0, .1, Vst::ParameterInfo::kCanAutomate, GainParams::kParamRateId, 0);
	parameters.addParameter(STR16("Rate Units"), STR16(""), 0, 0, Vst::ParameterInfo::kIsList, GainParams::kParamRUnitId, 0);
	parameters.addParameter(STR16("Envelope"), STR16("ms"), 0, .1, Vst::ParameterInfo::kCanAutomate, GainParams::kParamEnvelopeId, 0);
	parameters.addParameter(STR16("Invert"), STR16(""), 0, 0, Vst::ParameterInfo::kCanAutomate, GainParams::kParamInvertId, 0);

	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerController::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerController::setComponentState (IBStream* state)
{
	// Here you get the state of the component (Processor part)
	if (!state)
		return kResultFalse;

	IBStreamer streamer(state, kLittleEndian);
	float gainParam = 0.f;
	float rateParam = 0.f;
	if (streamer.readFloat(gainParam) == false)
		return kResultFalse;
	if (streamer.readFloat(rateParam) == false)
		return kResultFalse;

	if (auto param = parameters.getParameter(GainParams::kParamGainId))
		param->setNormalized(gainParam);
	if (auto param = parameters.getParameter(GainParams::kParamRateId))
		param->setNormalized(rateParam);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerController::setState (IBStream* state)
{
	// Here you get the state of the controller

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerController::getState (IBStream* state)
{
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor

	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API MorsalyzerController::createView (FIDString name)
{
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
		// create your editor here and return a IPlugView ptr of it
		auto* view = new VSTGUI::VST3Editor (this, "view", "myplugineditor.uidesc");

		setKnobMode(Steinberg::Vst::KnobModes::kLinearMode);
		sendTextMessage("Hello world!");
		return view;
	}
	return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerController::setParamNormalized (Vst::ParamID tag, Vst::ParamValue value)
{
	// called by host to update your parameters
	tresult result = EditControllerEx1::setParamNormalized (tag, value);
	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerController::getParamStringByValue (Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
{
	// called by host to get a string for given normalized value of a specific parameter
	// (without having to set the value!)
	return EditControllerEx1::getParamStringByValue (tag, valueNormalized, string);
}

//------------------------------------------------------------------------
tresult PLUGIN_API MorsalyzerController::getParamValueByString (Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
{
	// called by host to get a normalized value from a string representation of a specific parameter
	// (without having to set the value!)
	return EditControllerEx1::getParamValueByString (tag, string, valueNormalized);
}

IController* MorsalyzerController::createSubController(UTF8StringPtr name, const IUIDescription* description,
	VST3Editor* editor) {
	if (UTF8StringView(name) == "MessageController")
	{
		auto* controller = new UIMessageController(this);
		addUIMessageController(controller);
		return controller;
	}
	return nullptr;
}

void MorsalyzerController::addUIMessageController(UIMessageController* controller) {
	uiMessageControllers.push_back(controller);
}

void MorsalyzerController::removeUIMessageController(UIMessageController* controller) {
	UIMessageControllerList::const_iterator it =
		std::find(uiMessageControllers.begin(), uiMessageControllers.end(), controller);
		if (it != uiMessageControllers.end())
			uiMessageControllers.erase(it);
}

void MorsalyzerController::setDefaultMessageText(Steinberg::Vst::String128 text)
{
	Steinberg::String tmp(text);
	tmp.copyTo16(defaultMessageText, 0, 127);
}

//------------------------------------------------------------------------
Steinberg::Vst::TChar* MorsalyzerController::getDefaultMessageText()
{
	return defaultMessageText;
}

//------------------------------------------------------------------------
} // namespace BaronVonWentz
