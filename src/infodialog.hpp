#ifndef INFODIALOG_H
#define INFODIALOG_H

#include "dialogbox.hpp"

// A simple dialog box with an 'OK' button
class InfoDialogBox : public DialogBoxCommon
{
    std::vector<std::string> m_text;
    float m_textX;
    float m_textY;

    Button m_okButton;
    Button m_cancelButton;
    DialogResult m_dialogResult = DialogResult::OK;
public:

    InfoDialogBox(DialogBoxCommon::DialogType type, const char* dialogTitle, int parentWindowWidth, int parentWindowHeight, const char* infoText, int lineLength)
        : DialogBoxCommon(type, dialogTitle)
    {
        m_text = SetStringWithLineBreaks(infoText, lineLength);
        Layout(parentWindowWidth, parentWindowHeight);
    }

    static std::vector<std::string> SetStringWithLineBreaks(const char* infoText, int lineLength)
    {
        std::vector<std::string> result;

        std::string input = infoText;
        int lastSpaceIndex = -1;
        size_t startOfCurrentLine = 0;
        for (size_t i = 0; i < input.length(); ++i)
        {
            if (input[i] == '\n')
            {
                size_t lineLength = i - startOfCurrentLine;
                std::string line = input.substr(startOfCurrentLine, lineLength);
                result.push_back(line);
                lastSpaceIndex = -1;
                startOfCurrentLine = i + 1;
                continue;
            }

            if (input[i] == ' ')
            {
                lastSpaceIndex = static_cast<int>(i);
            }
            int currentLineLength = static_cast<int>(i - startOfCurrentLine);
            if (currentLineLength >= lineLength)
            {
                if (lastSpaceIndex == -1)
                {
                    // Emergency break
                    size_t lineLength = i - startOfCurrentLine;
                    std::string line = input.substr(startOfCurrentLine, lineLength);
                    result.push_back(line);
                    startOfCurrentLine = i;
                }
                else
                {
                    // Typical line break
                    size_t lineLength = lastSpaceIndex - startOfCurrentLine;
                    std::string line = input.substr(startOfCurrentLine, lineLength);
                    result.push_back(line);

                    startOfCurrentLine = lastSpaceIndex + 1;
                    lastSpaceIndex = -1;
                }
            }
        }
        // Flush the remaining input, if any
        if (startOfCurrentLine < input.length())
        {
            size_t lineLength = input.length() - startOfCurrentLine;
            std::string line = input.substr(startOfCurrentLine, lineLength);
            result.push_back(line);
        }

        // More than 14 lines is unlikely to fit, given the font sizes that have been chosen. arbitrarily chop it.
        if (result.size() > 14)
        {
            assert(false);
            result.resize(14);
        }

        return result;
    }

    void OnMouseMove(float x, float y, bool) override
    {
        m_okButton.OnMouseMove(x, y);

        if (m_type == DialogType::SaveFile_ConfirmOverwrite_OKCancel)
        {
            m_cancelButton.OnMouseMove(x, y);
        }
    }


    DialogCloseResult OnMouseClick(float x, float y, SDL_Renderer* renderer) override
    {
        if (m_okButton.IsHighlighted())
        {
            m_dialogResult = DialogResult::OK;
            return DialogCloseResult::ShouldCloseDialog;
        }

        if (m_type == DialogType::SaveFile_ConfirmOverwrite_OKCancel)
        {
            if (m_cancelButton.IsHighlighted())
            {
                m_dialogResult = DialogResult::Cancel;
                return DialogCloseResult::ShouldCloseDialog;
            }
        }

        return DialogCloseResult::KeepOpen;
    }

    DialogCloseResult OnKeyboardInput(SDL_Keycode key) override
    {
        if (m_type == DialogType::SaveFile_ConfirmOverwrite_OKCancel)
        {
            if (key == SDLK_RETURN)
            {
                m_dialogResult = DialogResult::OK;
                return DialogCloseResult::ShouldCloseDialog;
            }
            else if (key == SDLK_ESCAPE)
            {
                m_dialogResult = DialogResult::Cancel;
                return DialogCloseResult::ShouldCloseDialog;
            }
            return DialogCloseResult::KeepOpen;
        }
        
        if (m_type == DialogType::InfoOK)
        {
            if (key == SDLK_RETURN || key == SDLK_ESCAPE)
            {
                m_dialogResult = DialogResult::OK;
                return DialogCloseResult::ShouldCloseDialog;
            }
            return DialogCloseResult::KeepOpen;
        }

        return DialogCloseResult::KeepOpen;
    }

    void ShowDialog(SDL_Renderer* renderer, SDL_Color uiColor) override
    {
        DrawBlankWindow(renderer, uiColor);

        SDL_SetRenderScale(renderer, hackforge::toolbar_text_scaling, hackforge::toolbar_text_scaling);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        float layoutY = m_textY;
        for (const std::string line : m_text)
        {
            SDL_RenderDebugText(
                renderer,
                m_textX / hackforge::toolbar_text_scaling,
                layoutY / hackforge::toolbar_text_scaling + (hackforge::toolbar_height / (hackforge::toolbar_text_scaling * 4)),
                line.c_str());

            layoutY += hackforge::toolbar_height;
        }

        m_okButton.Draw(renderer, uiColor);

        if (m_type == DialogType::SaveFile_ConfirmOverwrite_OKCancel)
        {
            m_cancelButton.Draw(renderer, uiColor);
        }
    }

    DialogResult GetDialogResult() const override
    {
        return m_dialogResult;
    }

private:
    void Layout(int parentWindowWidth, int parentWindowHeight)
    {
        // Size chosen based on the baked-in choice of elements on the dialog
        int dialogWidth = 500;
        int dialogHeight = 132 + static_cast<int>(m_text.size()) * hackforge::toolbar_height;

        // Center the dialog
        m_dialogRect.x = static_cast<float>((parentWindowWidth / 2) - (dialogWidth / 2));
        m_dialogRect.y = static_cast<float>((parentWindowHeight / 2) - (dialogHeight / 2));

        m_dialogRect.w = static_cast<float>(dialogWidth);
        m_dialogRect.h = static_cast<float>(dialogHeight);

        m_textX = m_dialogRect.x + hackforge::sc_dialogbox_margin;
        m_textY = m_dialogRect.y + hackforge::toolbar_height + hackforge::sc_dialogbox_margin;

        float buttonLayoutY = m_dialogRect.y + m_dialogRect.h - hackforge::toolbar_height - hackforge::toolbar_height;

        if (m_type == DialogType::SaveFile_ConfirmOverwrite_OKCancel)
        {
            float okButtonX = m_dialogRect.x + hackforge::sc_dialogbox_margin;
            float okButtonY = buttonLayoutY;
            m_okButton.Layout(&okButtonX, &okButtonY, "OK", hackforge::sc_dialogbox_margin);

            float cancelButtonWidth = hackforge::GetRenderedTextWidthInPixels(6) + hackforge::sc_dialogbox_margin + hackforge::sc_dialogbox_margin;
            float cancelButtonX = m_dialogRect.x + m_dialogRect.w - cancelButtonWidth - hackforge::sc_dialogbox_margin;
            float cancelButtonY = buttonLayoutY;
            m_cancelButton.Layout(&cancelButtonX, &cancelButtonY, "Cancel", hackforge::sc_dialogbox_margin);
        }
        else if (m_type == DialogType::InfoOK)
        {
            // Center the OK button
            float okButtonWidth = hackforge::GetRenderedTextWidthInPixels(2) + hackforge::sc_dialogbox_margin + hackforge::sc_dialogbox_margin;
            float okButtonX = m_dialogRect.x + (m_dialogRect.w / 2) - (okButtonWidth / 2);
            float okButtonY = buttonLayoutY;
            m_okButton.Layout(&okButtonX, &okButtonY, "OK", hackforge::sc_dialogbox_margin);
        }

        LayoutCommon(m_dialogRect.w);
    }
};

#endif