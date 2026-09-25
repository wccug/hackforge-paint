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
    hackforge::resizeDialog.reset(new SetCanvasSizeDialogBox("Set Canvas Size", hackforge::window_width, hackforge::window_height));
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

void hackforge::Toolbar::Render(SDL_Renderer* renderer, SDL_Color uiColor) {
    // Render a filled rectangle at the top
    {
        SDL_FRect rect{};
        rect.x = 0;
        rect.y = 0;
        rect.w = hackforge::window_width;
        rect.h = hackforge::toolbar_height;
        SDL_SetRenderScale(renderer, 1, 1);
        SDL_SetRenderDrawColor(renderer, uiColor.r, uiColor.g, uiColor.b, 255);
        SDL_RenderFillRect(renderer, &rect);
    }

    // Draw the toolbar UI for child items
    for (size_t i = 0; i < m_toolbarItems.size(); ++i) {
        m_toolbarItems[i].Render(renderer, uiLayoutState, uiColor);
    }
}

} // namespace hackforge
