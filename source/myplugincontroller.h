//------------------------------------------------------------------------
// Copyright(c) 2023 Baron von Wenzelheimer.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "uimessagecontroller.h"

namespace BaronVonWentz {

template <typename T>
class MorsalyzerMessageController;

//------------------------------------------------------------------------
//  MorsalyzerController
//------------------------------------------------------------------------
class MorsalyzerController : public Steinberg::Vst::EditControllerEx1, public VSTGUI::VST3EditorDelegate
{
public:
//------------------------------------------------------------------------
	using UIMessageController = MorsalyzerMessageController<MorsalyzerController>;
	using UTF8StringPtr = VSTGUI::UTF8StringPtr;
	using IUIDescription = VSTGUI::IUIDescription;
	using IController = VSTGUI::IController;
	using VST3Editor = VSTGUI::VST3Editor;

	MorsalyzerController () = default;
	~MorsalyzerController () SMTG_OVERRIDE = default;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/)
	{
		return (Steinberg::Vst::IEditController*)new MorsalyzerController;
	}

	// IPluginBase
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;

	// EditController
	Steinberg::tresult PLUGIN_API setComponentState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setParamNormalized (Steinberg::Vst::ParamID tag,
                                                      Steinberg::Vst::ParamValue value) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getParamStringByValue (Steinberg::Vst::ParamID tag,
                                                         Steinberg::Vst::ParamValue valueNormalized,
                                                         Steinberg::Vst::String128 string) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getParamValueByString (Steinberg::Vst::ParamID tag,
                                                         Steinberg::Vst::TChar* string,
                                                         Steinberg::Vst::ParamValue& valueNormalized) SMTG_OVERRIDE;

	//from VST3EditorDelegate
	IController* createSubController(UTF8StringPtr name, const IUIDescription* description,
		VST3Editor* editor) SMTG_OVERRIDE;

	//internal functions
	void addUIMessageController(UIMessageController* controller);
	void removeUIMessageController(UIMessageController* controller);

	void setDefaultMessageText(Steinberg::Vst::String128 text);
	Steinberg::Vst::TChar* getDefaultMessageText();

 	//---Interface---------
	DEFINE_INTERFACES
		// Here you can add more supported VST3 interfaces
		// DEF_INTERFACE (Vst::IXXX)
	END_DEFINE_INTERFACES (EditController)
    DELEGATE_REFCOUNT (EditController)

//------------------------------------------------------------------------
protected:
	using UIMessageControllerList = std::vector<UIMessageController*>;
	UIMessageControllerList uiMessageControllers;

	Steinberg::Vst::String128 defaultMessageText;
};

//------------------------------------------------------------------------
} // namespace BaronVonWentz
