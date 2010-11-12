#ifndef MAPINFODIALOG_H_
#define MAPINFODIALOG_H_

#include "icommandsystem.h"
#include "gtkutil/window/BlockingTransientWindow.h"
#include "EntityInfoTab.h"
#include "ShaderInfoTab.h"
#include "ModelInfoTab.h"

namespace Gtk
{
	class Notebook;
}

namespace ui {

class MapInfoDialog :
	public gtkutil::BlockingTransientWindow
{
	// The helper classes displaying the statistics
	EntityInfoTab _entityInfo;
	ShaderInfoTab _shaderInfo;
	ModelInfoTab _modelInfo;

	// The tabs of this dialog
	Gtk::Notebook* _notebook;

public:
	// Constructor
	MapInfoDialog();

	/**
	 * greebo: Shows the dialog (allocates on heap, dialog self-destructs)
	 */
	static void showDialog(const cmd::ArgumentList& args);

private:
	// This is called to create the widgets
	void populateWindow();

	// Disconnect this window from the eventmanagaer
	void shutdown();

	// Helper method to create the OK/Cancel button
	Gtk::Widget& createButtons();

	Gtk::Widget& createTabLabel(const std::string& label, const std::string& iconName);

	// The callback for the buttons
	void onClose();

}; // class MapInfoDialog

} // namespace ui

#endif /*MAPINFODIALOG_H_*/
