#ifndef AI_VOCAL_SET_PREVIEW_H_
#define AI_VOCAL_SET_PREVIEW_H_

#include "ieclass.h"
#include "gtkutil/ifc/Widget.h"

namespace ui {

/** 
 * greebo: This class provides the UI elements to listen
 * to a a given AI vocal set. On clicking the playback button
 * a random sound is chosen from the vocal set.
 * 
 * Use the getWidget() method to pack this into a
 * parent container. 
 */
class AIVocalSetPreview :
	public gtkutil::Widget
{
private:
	// The main container widget of this preview
	GtkWidget* _widget;
	
	GtkWidget* _playButton;
	GtkWidget* _stopButton;
	GtkWidget* _statusLabel;
	
	// The currently "previewed" vocal set
	IEntityClassPtr _vocalSetDef;

	typedef std::vector<std::string> SoundShaderList;
	SoundShaderList _setShaders;

protected:
   
	GtkWidget* _getWidget() const
	{
		return _widget;
	}

public:
	AIVocalSetPreview();

	/** 
	 * greebo: Sets the vocal set to preview. Set NULL to disable this panel.
	 */
	void setVocalSetEclass(const IEntityClassPtr& vocalSetDef);

private:
	/** 
	 * greebo: Returns a random file from the selected vocal set.
	 * @returns: the filename as defined in one of the shaders or "".
	 */
	std::string getRandomSoundFile();

	/** greebo: Creates the control widgets (play button) and such.
	 */
	GtkWidget* createControlPanel();

	/** greebo: Updates the list according to the active soundshader
	 */
	void update();
	
	// GTK Callbacks
	static void onPlay(GtkWidget* button, AIVocalSetPreview* self);
	static void onStop(GtkWidget* button, AIVocalSetPreview* self);
};
typedef boost::shared_ptr<AIVocalSetPreview> AIVocalSetPreviewPtr;

} // namespace ui

#endif /* AI_VOCAL_SET_PREVIEW_H_ */
