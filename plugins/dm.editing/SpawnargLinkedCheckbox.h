#pragma once

#include "iundo.h"
#include "ieclass.h"
#include "ientity.h"
#include <wx/checkbox.h>

namespace ui
{

/**
 * An enhanced checkbox that is updating the named
 * entity property (spawnarg) when toggled.
 *
 * The logic toggled = "1" can be optionally inversed such that 
 * an unchecked box reflects a property value of "1".
 */
class SpawnargLinkedCheckbox : 
	public wxCheckBox
{
private:
	bool _inverseLogic;

	std::string _propertyName;

	Entity* _entity;

	bool _updateLock;

public:
	SpawnargLinkedCheckbox(wxWindow* parent, const std::string& label, 
						   const std::string& propertyName, 
						   bool inverseLogic = false) :
		wxCheckBox(parent, wxID_ANY, label),
		_inverseLogic(inverseLogic),
		_propertyName(propertyName),
		_entity(NULL),
		_updateLock(false)
	{
		Connect(wxEVT_CHECKBOX, wxCommandEventHandler(SpawnargLinkedCheckbox::onToggle), NULL, this);
	}

	// Sets the edited Entity object
	void setEntity(Entity* entity)
	{
		_entity = entity;

		if (_entity == NULL) 
		{
			SetToolTip("");
			return;
		}

		SetToolTip(_propertyName + ": " + _entity->getEntityClass()->getAttribute(_propertyName).getDescription());

		bool value = _entity->getKeyValue(_propertyName) == "1";

		_updateLock = true;
		SetValue(_inverseLogic ? !value : value);
		_updateLock = false;
	}

protected:
	void onToggle(wxCommandEvent& ev)
	{
		ev.Skip();

		// Update the spawnarg if we have a valid entity
		if (!_updateLock && _entity != NULL)
		{
			UndoableCommand cmd("editAIProperties");

			std::string newValue = "";

			if (_inverseLogic)
			{
				newValue = GetValue() ? "0" : "1"; // Active => "0"
			}
			else
			{
				newValue = GetValue() ? "1" : "0";
			}

			// Check if the new value conincides with an inherited one
			if (_entity->getEntityClass()->getAttribute(_propertyName).getValue() == newValue)
			{
				// in which case the property just gets removed from the entity
				newValue = "";
			}

			_entity->setKeyValue(_propertyName, newValue);
		}
	}
};

} // namespace
