#ifndef LIGHTTEXTURECHOOSER_H_
#define LIGHTTEXTURECHOOSER_H_

#include "ui/common/ShaderSelector.h"
#include "gtkutil/window/BlockingTransientWindow.h"

#include <string>

// Forward decls
class Material;

namespace ui {

/**
 * A GTK dialog containing a ShaderSelector widget combo and OK/Cancel
 * buttons.
 */
class LightTextureChooser :
	public gtkutil::BlockingTransientWindow,
	public ShaderSelector::Client
{
private:
	// The ShaderSelector widget group, that contains the actual selection
	// tools (treeview etc.)
	ShaderSelector* _selector;

	// The user's texture selection
	std::string _selectedTexture;

private:
	// Widget construction helpers
	Gtk::Widget& createButtons();

	// gtkmm callbacks
	void callbackCancel();
	void callbackOK();

public:
	/**
	 * Construct the dialog window and its contents.
	 */
	LightTextureChooser();

	/**
	 * Display the dialog and block for a selection from the user.
	 *
	 * @returns
	 * The selected shader name, or "" if there was no selection or the dialog
	 * was cancelled.
	 */
	std::string chooseTexture();

	/** greebo: Gets called upon selection change, updates the infostore
	 * 			of the contained ShaderSelector helper class accordingly.
	 */
	void shaderSelectionChanged(const std::string& shaderName,
								const Glib::RefPtr<Gtk::ListStore>& listStore);

};

} // namespace ui

#endif /*LIGHTTEXTURECHOOSER_H_*/
