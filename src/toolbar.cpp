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

void OnToolbarSave() {
  hackforge::doSave = true;
}
void OnToolbarResize()
{

}

void OnToolbarHorizontalFlip() {
    hackforge::doHorizontalFlip = true;
}

void OnToolbarVerticalFlip() {
    hackforge::doVerticalFlip = true;
}

void OnToolbarSetPenColor() {
  hackforge::penColor =
      hackforge::OpenNativeColorPicker(hackforge::window, hackforge::penColor);
}

void OnToolbarSetUIColor() {
  hackforge::buttonColor = hackforge::OpenNativeColorPicker(
      hackforge::window, hackforge::buttonColor);
}

void OnToolbarSetPencilTool() {
    hackforge::currentTool = Tool::Pencil;
    hackforge::toolbar.CheckItemAndUncheckOthers(1, 0);
}

void OnToolbarSetStampTool() {
  hackforge::currentTool = Tool::Stamp;
  hackforge::toolbar.CheckItemAndUncheckOthers(1, 1);
}

void OnToolbarSetAnglePenTool() {
  hackforge::currentTool = Tool::AnglePen;
  hackforge::toolbar.CheckItemAndUncheckOthers(1, 2);
}

void OnToolbarSetPaintBucketTool() {
  hackforge::currentTool = hackforge::Tool::PaintBucket;
  hackforge::toolbar.CheckItemAndUncheckOthers(1, 3);
}

} // namespace hackforge
