#ifndef FILEPICKER_H
#define FILEPICKER_H

#include "dialogbox.hpp"

class FilePickerDialogBox : public DialogBoxCommon
{
    struct MenuItem
    {
        enum Type { GoUpOne, DirectoryEntry } WhichType;
        std::filesystem::directory_entry Dir;
    };

    struct DisplayedItem
    {
        int Index;
        SDL_FRect Rect;
    };

    std::vector<MenuItem> m_fileList;
    std::vector<DisplayedItem> m_displayed;

    int m_numLinesDisplayed = 10;
    int m_displayStartIndex = 0;
    int m_displayEndIndex = 0;
    int m_selectionIndex = 0;
    std::filesystem::path m_targetDirectory;

    Button m_okButton;
    Button m_cancelButton;
    LabelledTextBox m_newFilenameTextBox;

    std::filesystem::directory_entry m_requestedFile;
    DialogResult m_dialogResult = DialogResult::None;

public:

    FilePickerDialogBox(const char* dialogTitle, DialogBoxCommon::DialogType type, int parentWindowWidth, int parentWindowHeight);
    void OnMouseMove(float x, float y, bool mouseButtonDown) override;
    DialogCloseResult ConfirmSelection();
    DialogCloseResult OnMouseClick(float x, float y, SDL_Renderer* renderer) override;
    void TryMoveSelectionDown();
    void TryMoveSelectionUp();
    void OnMouseWheel(SDL_MouseWheelDirection dir, float x, float y) override;
    DialogCloseResult OnKeyboardInput(SDL_Keycode key) override;
    void ShowDialog(SDL_Renderer* renderer, SDL_Color uiColor) override;
    DialogResult GetDialogResult() const override;
    std::filesystem::directory_entry GetRequestedFile() const;

private:
    void PopulateDisplayedList();
    void Layout(int parentWindowWidth, int parentWindowHeight);
};

#endif