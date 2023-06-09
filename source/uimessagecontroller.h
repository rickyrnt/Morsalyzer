#pragma once

#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"

using namespace Steinberg;

namespace BaronVonWentz {

template <typename ControllerType> 
class MorsalyzerMessageController : public VSTGUI::IController, public VSTGUI::ViewListenerAdapter {
public:
	MorsalyzerMessageController(ControllerType* UIcontroller) : UIcontroller(UIcontroller), textEdit(nullptr) {

	} ~MorsalyzerMessageController() override {
		if (textEdit)
			viewWillDelete(textEdit);
		UIcontroller->removeUIMessageController(this);
	}

	void setMessageText(Vst::String128 msgText) {
		if (!textEdit)
			return;
		String str (msgText);
		str.toMultiByte(kCP_Utf8);
		textEdit->setText(str.text8());
	}

private:
	using CControl = VSTGUI::CControl;
	using CView = VSTGUI::CView;
	using CTextEdit = VSTGUI::CTextEdit;
	using UTF8String = VSTGUI::UTF8String;
	using UIAttributes = VSTGUI::UIAttributes;
	using IUIDescription = VSTGUI::IUIDescription;

	// from IControlListener
	void valueChanged(CControl* /*pControl*/) override {
		FDebugPrint("Sending message!\n");
		UIcontroller->sendTextMessage(textEdit->getText().data());
	}
	void controlBeginEdit(CControl* /*pControl*/) override {}
	void controlEndEdit(CControl* pControl) override {}

	//called when a view is created
	CView* verifyView(CView* view, const UIAttributes& /*attributes*/,
		const IUIDescription* /*description*/) override {
		if (CTextEdit* te = dynamic_cast<CTextEdit*>(view)) {
			FDebugPrint("Initializing textedit!\n");

			//store pointer to textedit
			textEdit = te;

			// add as listener to get viewWillDelete and viewLostFocus
			textEdit->registerViewListener(this);
			textEdit->registerControlListener(this);

			//initialize it
			/*Steinberg::String str(UIcontroller->getDefaultMessageText());
			str.toMultiByte(kCP_Utf8);
			textEdit->setText(str.text8());*/
		}
		return view;
	}

	//from IViewListenerAdapter
	//called when view will be deleted
	void viewWillDelete(CView* view) override {
		if (dynamic_cast<CTextEdit*>(view) == textEdit) {
			textEdit->unregisterViewListener(this);
			textEdit->unregisterControlListener(this);
			textEdit = nullptr;
		}
	}
	// called when lost focus
	void viewLostFocus(CView* view) override {
		if (dynamic_cast<CTextEdit*>(view) == textEdit) {
			// save the last content of the textedit
			const UTF8String& text = textEdit->getText();
			Vst::String128 messageText;
			Steinberg::String str;
			str.fromUTF8(text.data());
			str.copyTo(messageText, 0, 128);
			UIcontroller->setDefaultMessageText(messageText);
		}
	}

	ControllerType* UIcontroller;
	CTextEdit* textEdit;
};
} // BaronVonWentz