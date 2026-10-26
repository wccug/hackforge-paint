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

    FilePickerDialogBox(const char* dialogTitle, DialogBoxCommon::DialogType type, int parentWindowWidth, int parentWindowHeight)
        : DialogBoxCommon(type, dialogTitle)
    {
        if (type == DialogType::OpenFile) m_numLinesDisplayed = 10;
        else if (type == DialogType::SaveFile) m_numLinesDisplayed = 8;

        Layout(parentWindowWidth, parentWindowHeight);
        m_cancelButton.SetOnMenubar(true);
    }

    void OnMouseMove(float x, float y, bool) override
    {
        m_okButton.OnMouseMove(x, y);
        m_cancelButton.OnMouseMove(x, y);
    }

    void ConfirmSelection(bool* pCloseDialog)
    {
        if (m_type == DialogType::SaveFile && m_selectionIndex == -1)
        {
            std::filesystem::path baseDir = m_targetDirectory;
            std::filesystem::directory_entry selectedFile{ baseDir / m_newFilenameTextBox.GetTextFieldValue() };

            m_requestedFile = selectedFile;
            m_dialogResult = DialogResult::OK;
            *pCloseDialog = true;
            return;
        }

        MenuItem* pMenuItem = &m_fileList[m_selectionIndex];

        if (pMenuItem->WhichType == MenuItem::DirectoryEntry)
        {
            if (pMenuItem->Dir.is_directory())
            {
                // Go into folder
                m_targetDirectory = pMenuItem->Dir;
                PopulateDisplayedList();
            }
            else if (pMenuItem->Dir.is_regular_file())
            {
                // Choose this file
                m_requestedFile = pMenuItem->Dir;
                m_dialogResult = DialogResult::OK;
                *pCloseDialog = true;
            }

        }
        else if (pMenuItem->WhichType == MenuItem::GoUpOne)
        {
            m_targetDirectory = m_targetDirectory.parent_path();
            PopulateDisplayedList();
        }
    }

    void OnMouseClick(float x, float y, SDL_Renderer* renderer, bool* pCloseDialog) override
    {
        if (m_okButton.IsHighlighted())
        {
            ConfirmSelection(pCloseDialog);
            return;
        }

        if (m_cancelButton.IsHighlighted())
        {
            *pCloseDialog = true;
            m_dialogResult = DialogResult::Cancel;
            return;
        }

        // Check if we clicked on any of the menu items
        // This is coupled to the display logic
        for (size_t i = 0; i < m_displayed.size(); ++i)
        {
            if (!hackforge::IsInBounds(x, y, m_displayed[i].Rect))
                continue;

            size_t requestedSelectionIndex = m_displayed[i].Index;
            if (m_selectionIndex == requestedSelectionIndex)
            {
                ConfirmSelection(pCloseDialog);
            }
            else
            {
                m_selectionIndex = m_displayed[i].Index;
                m_newFilenameTextBox.SetFocus(false);
            }
            return;
        }

        if (m_type == DialogType::SaveFile)
        {
            m_newFilenameTextBox.OnMouseClick(x, y);
            if (m_newFilenameTextBox.IsFocused())
            {
                m_selectionIndex = -1;
            }
        }
    }

    void TryMoveSelectionDown()
    {
        if (m_selectionIndex < m_fileList.size() - 1)
        {
            m_selectionIndex++;
            if (m_selectionIndex > m_displayEndIndex)
            {
                m_displayStartIndex++;
                m_displayEndIndex++;
            }
        }
    }

    void TryMoveSelectionUp()
    {
        if (m_selectionIndex > 0)
        {
            m_selectionIndex--;
            if (m_selectionIndex < m_displayStartIndex)
            {
                m_displayStartIndex--;
                m_displayEndIndex--;
            }
        }
    }

    void OnMouseWheel(SDL_MouseWheelDirection dir, float x, float y) override
    {
        if (y < 0)
        {
            TryMoveSelectionDown();
        }
        else if (y > 0)
        {
            TryMoveSelectionUp();
        }
    }

    void OnKeyboardInput(SDL_Keycode key, bool* pCloseDialog) override
    {
        if (key == 27) // Escape
        {
            m_dialogResult = DialogResult::Cancel;
            *pCloseDialog = true;
            return;
        }

        if (key == 13) // Enter
        {
            ConfirmSelection(pCloseDialog);
            return;
        }

        if (key == SDLK_DOWN)
        {
            TryMoveSelectionDown();
            return;
        }

        if (key == SDLK_UP)
        {
            TryMoveSelectionUp();
            return;
        }

        if (m_type == DialogType::SaveFile)
        {
            m_selectionIndex = -1;
            m_newFilenameTextBox.SetFocus(true);
            m_newFilenameTextBox.OnKeyboardInput(key);
            return;
        }
    }

    void ShowDialog(SDL_Renderer* renderer, SDL_Color uiColor) override
    {
        DrawBlankWindow(renderer, uiColor);

        float layoutY = m_dialogRect.y + hackforge::toolbar_height + hackforge::sc_dialogbox_margin;
        float textX = m_dialogRect.x + hackforge::sc_dialogbox_margin;

        // Display the current folder
        SDL_SetRenderScale(renderer, hackforge::toolbar_text_scaling, hackforge::toolbar_text_scaling);
        {
            SDL_SetRenderDrawColor(renderer, uiColor.r, uiColor.g, uiColor.b, uiColor.a);
            std::string displayedString = "Folder: ";
            displayedString += std::filesystem::absolute(m_targetDirectory).string();
            SDL_RenderDebugText(
                renderer,
                textX / hackforge::toolbar_text_scaling,
                layoutY / hackforge::toolbar_text_scaling,
                displayedString.c_str());
        }
        layoutY += hackforge::toolbar_height;

        m_displayed.clear();

        for (size_t i = m_displayStartIndex; i <= m_displayEndIndex; ++i)
        {
            if (i >= m_fileList.size()) break;

            std::string displayedString;
            if (m_fileList[i].WhichType == MenuItem::GoUpOne)
            {
                displayedString = "[..] Go Up One Directory";
            }
            else if (m_fileList[i].WhichType == MenuItem::DirectoryEntry)
            {
                displayedString = m_fileList[i].Dir.path().filename().string();
            }

            SDL_FRect rect{};
            rect.x = textX;
            rect.y = layoutY - (hackforge::toolbar_height / 4);
            rect.w = hackforge::GetRenderedTextWidthInPixels(displayedString.size());
            rect.h = hackforge::toolbar_height;

            {
                DisplayedItem di{};
                di.Rect = rect;
                di.Index = static_cast<int>(i);
                m_displayed.push_back(di);
            }

            if (m_selectionIndex == i)
            {
                SDL_SetRenderScale(renderer, 1, 1);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_FRect rect{};
                rect.x = textX;
                rect.y = layoutY - (hackforge::toolbar_height / 4);
                rect.w = hackforge::GetRenderedTextWidthInPixels(displayedString.size());
                rect.h = hackforge::toolbar_height;
                SDL_RenderFillRect(renderer, &rect);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }

            SDL_SetRenderScale(renderer, hackforge::toolbar_text_scaling, hackforge::toolbar_text_scaling);

            SDL_RenderDebugText(
                renderer,
                textX / hackforge::toolbar_text_scaling,
                layoutY / hackforge::toolbar_text_scaling,
                displayedString.c_str());

            layoutY += hackforge::toolbar_height;
        }

        // Show the text field for SaveAs
        if (m_type == DialogType::SaveFile)
        {
            m_newFilenameTextBox.Draw(renderer, uiColor);
        }

        m_okButton.Draw(renderer, uiColor);
        m_cancelButton.Draw(renderer, uiColor);
    }

    DialogResult GetDialogResult() const override
    {
        return m_dialogResult;
    }

    std::filesystem::directory_entry GetRequestedFile() const
    {
        return m_requestedFile;
    }

private:
    void PopulateDisplayedList()
    {
        m_fileList.clear();
        m_displayed.clear();

        {
            MenuItem mi{};
            mi.WhichType = MenuItem::GoUpOne;
            m_fileList.push_back(mi);
        }

        try
        {
            if (std::filesystem::exists(m_targetDirectory) && std::filesystem::is_directory(m_targetDirectory)) {
                // Loop through directory entries
                for (const auto& entry : std::filesystem::directory_iterator(m_targetDirectory))
                {
                    MenuItem mi{};
                    mi.WhichType = MenuItem::DirectoryEntry;
                    mi.Dir = entry;
                    m_fileList.push_back(mi);
                }
            }
        }
        catch (const std::filesystem::filesystem_error&)
        {
        }

        m_selectionIndex = 0;
        m_displayStartIndex = 0;
        m_displayEndIndex = std::min(m_displayStartIndex + m_numLinesDisplayed + 1, static_cast<int>(m_fileList.size() - 1));
    }

    void Layout(int parentWindowWidth, int parentWindowHeight)
    {
        // Size chosen based on the baked-in choice of elements on the dialog
        float dialogWidth = parentWindowWidth - hackforge::sc_dialogbox_margin;
        float dialogHeight = parentWindowHeight - hackforge::toolbar_height - hackforge::sc_dialogbox_margin;

        // Center the dialog
        m_dialogRect.x = static_cast<float>((parentWindowWidth / 2) - (dialogWidth / 2));
        m_dialogRect.y = static_cast<float>((parentWindowHeight / 2) - (dialogHeight / 2));

        m_dialogRect.w = static_cast<float>(dialogWidth);
        m_dialogRect.h = static_cast<float>(dialogHeight);

        m_targetDirectory = ".";
        m_targetDirectory = std::filesystem::absolute(m_targetDirectory);
        PopulateDisplayedList();

        if (m_type == DialogType::SaveFile)
        {
            float layoutX = m_dialogRect.x + hackforge::sc_dialogbox_margin;
            float layoutY = m_dialogRect.y + hackforge::sc_dialogbox_margin + ((m_numLinesDisplayed + 4) * hackforge::toolbar_height);
            m_newFilenameTextBox.Layout(&layoutX, &layoutY, hackforge::GetRenderedTextWidthInPixels(9), 500, "Filename:", 0, LabelledTextBox::FormatMode::AlphanumericChars);
        }

        // Position the OK button
        float okButtonWidth = hackforge::GetRenderedTextWidthInPixels(2) + hackforge::sc_dialogbox_margin + hackforge::sc_dialogbox_margin;
        float okButtonX = m_dialogRect.x + (m_dialogRect.w / 2) - (okButtonWidth / 2);
        float okButtonY = m_dialogRect.y + m_dialogRect.h - hackforge::toolbar_height - hackforge::toolbar_height;
        m_okButton.Layout(&okButtonX, &okButtonY, "OK", hackforge::sc_dialogbox_margin);

        float cancelButtonX = m_dialogRect.x + m_dialogRect.w - 56;
        float cancelButtonY = m_dialogRect.y;
        m_cancelButton.Layout(&cancelButtonX, &cancelButtonY, "X", hackforge::sc_dialogbox_margin);


        LayoutCommon(m_dialogRect.w);
    }
};

#endif