#include "Toolbar.hpp"
#include "colorpicker.hpp"
#include "common.hpp"

namespace hackforge {

void OnToolbarNew() {
  NewDocument();
}

void OnToolbarExit() {
  hackforge::shouldExit = true;
}

void OnToolbarSetPenColor() {
  hackforge::penColor =
      hackforge::OpenNativeColorPicker(hackforge::window, hackforge::penColor);
}

void OnToolbarSetUIColor() {
  hackforge::buttonColor = hackforge::OpenNativeColorPicker(
      hackforge::window, hackforge::buttonColor);
}

void OnToolbarSetStampTool() {
  hackforge::currentTool = Tool::Stamp;
  hackforge::toolbar.SetChildMenuItemCheckedState(1, 0, true);
  hackforge::toolbar.SetChildMenuItemCheckedState(1, 1, false);
}

void OnToolbarSetAnglePenTool() {
  hackforge::currentTool = Tool::AnglePen;
  hackforge::toolbar.SetChildMenuItemCheckedState(1, 0, false);
  hackforge::toolbar.SetChildMenuItemCheckedState(1, 1, true);
}

} // namespace hackforge
