#include "Toolbar.hpp"
#include "colorpicker.hpp"
#include "filepicker.hpp"
#include "infodialog.hpp"
#include "common.hpp"

namespace hackforge {

void OnToolbarNew() {
  NewDocument();
}

void OnToolbarExit() {
  hackforge::shouldExit = true;
}

void OnToolbarQuickSave() {
  hackforge::doQuickSave = true;
}

void OnToolbarOpen()
{
    hackforge::dialogs.emplace_back(std::make_unique<FilePickerDialogBox>(
        "Load File",
        DialogBoxCommon::DialogType::OpenFile,
        hackforge::window_width,
        hackforge::window_height));
}

void OnToolbarSaveAs() 
{
    hackforge::dialogs.emplace_back(std::make_unique<FilePickerDialogBox>(
        "Save File",
        DialogBoxCommon::DialogType::SaveFile,
        hackforge::window_width,
        hackforge::window_height));
}

void OnToolbarSetCanvasSize()
{
    hackforge::dialogs.emplace_back(std::make_unique<SizeInputDialogBox>(
        DialogBoxCommon::DialogType::Resize,
        "Set Canvas Size",
        hackforge::window_width,
        hackforge::window_height));
}

void OnToolbarUndo() {
    hackforge::doUndo = true;
}

void OnToolbarHorizontalFlip() {
    hackforge::doHorizontalFlip = true;
}

void OnToolbarVerticalFlip() {
    hackforge::doVerticalFlip = true;
}

void OnToolbarDesaturate() {
    hackforge::doDesaturate = true;
}

void OnToolbarSetPenColor() {
    hackforge::dialogs.emplace_back(std::make_unique<ColorPickerDialogBox>(
        DialogBoxCommon::DialogType::SetPenColor,
        "Set Colour",
        hackforge::window_width,
        hackforge::window_height,
        hackforge::penColor));
}

void OnToolbarSetPenRadius() {
    hackforge::dialogs.emplace_back(std::make_unique<SingleNumberInputDialogBox>(
        DialogBoxCommon::DialogType::SetPenRadius,
        "Set Radius",
        "Radius:",
        hackforge::window_width,
        hackforge::window_height,
        hackforge::penRadius));
}

void OnToolbarSetPenAngle() {
    hackforge::dialogs.emplace_back(std::make_unique<SingleNumberInputDialogBox>(
        DialogBoxCommon::DialogType::SetPenAngle,
        "Set Angle",
        "Angle:",
        hackforge::window_width,
        hackforge::window_height,
        hackforge::penAngle));
}

void OnToolbarSetUIColor() {
    hackforge::dialogs.emplace_back(std::make_unique<ColorPickerDialogBox>(
        DialogBoxCommon::DialogType::SetUIColor,
        "Set Colour",
        hackforge::window_width,
        hackforge::window_height,
        hackforge::buttonColor));
}

void OnToolbarSetPencilTool() {
    hackforge::currentTool = Tool::Pencil;
    hackforge::toolbar.CheckItemAndUncheckOthers(1, 0);
    hackforge::toolbar.EnableToolAttributes(ToolAttributes::Colour);
}

void OnToolbarSetStampTool() {
  hackforge::currentTool = Tool::Stamp;
  hackforge::toolbar.CheckItemAndUncheckOthers(1, 1);
  hackforge::toolbar.EnableToolAttributes(ToolAttributes::Colour | ToolAttributes::Radius);
}

void OnToolbarSetAnglePenTool() {
  hackforge::currentTool = Tool::AnglePen;
  hackforge::toolbar.CheckItemAndUncheckOthers(1, 2);
  hackforge::toolbar.EnableToolAttributes(ToolAttributes::Colour | ToolAttributes::Radius | ToolAttributes::Angle);
}

void OnToolbarSetPaintBucketTool() {
  hackforge::currentTool = hackforge::Tool::PaintBucket;
  hackforge::toolbar.CheckItemAndUncheckOthers(1, 3);
  hackforge::toolbar.EnableToolAttributes(ToolAttributes::Colour);
}

void OnToolbarSetEyedropperTool()
{
    hackforge::currentTool = hackforge::Tool::Eyedropper;
    hackforge::toolbar.CheckItemAndUncheckOthers(1, 4);
    hackforge::toolbar.EnableToolAttributes(ToolAttributes::None);
}

void OnToolbarSetSpraypaintTool() {
    hackforge::currentTool = Tool::Spraypaint;
    hackforge::toolbar.CheckItemAndUncheckOthers(1, 5);
    hackforge::toolbar.EnableToolAttributes(ToolAttributes::Colour | ToolAttributes::Radius);
}

void OnToolbarAbout()
{
    hackforge::dialogs.emplace_back(std::make_unique<InfoDialogBox>(
        DialogBoxCommon::DialogType::InfoOK,
        "Information",
        hackforge::window_width,
        hackforge::window_height,
        "WCCUG Paint version 0.1\nFeel free to get in touch with WCCUG if you'd like to make contributions.",
        28));
}

// Draw the toolbar
void hackforge::Toolbar::Render(SDL_Renderer* renderer, SDL_Color uiColor) {
    // Render a filled rectangle at the top
    {
        SDL_FRect rect{};
        rect.x = 0;
        rect.y = 0;
        rect.w = static_cast<float>(hackforge::window_width);
        rect.h = static_cast<float>(hackforge::toolbar_height);
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
