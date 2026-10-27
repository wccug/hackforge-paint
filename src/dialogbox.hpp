#pragma once

#include <sstream>
#include <cassert>
#include <filesystem>
#include "uilayout.hpp"

enum class DialogResult
{
    None,
    OK,
    Cancel
};

class Button
{
    bool m_onMenubar{};
    bool m_highlight{};
    SDL_FRect m_buttonRect{};
    std::string m_buttonText{};
    float m_buttonTextX{};

public:
    void Layout(float* pLayoutX, float* pLayoutY, std::string const& text, float margin)
    {
        m_buttonText = text;
        m_buttonRect.x = *pLayoutX;
        m_buttonRect.y = *pLayoutY;

        float textWidth = hackforge::GetRenderedTextWidthInPixels(text.length());
        float requiredWidth = textWidth + margin + margin;

        m_buttonRect.w = requiredWidth;
        m_buttonRect.h = hackforge::toolbar_height;

        // Center the label onto the button
        m_buttonTextX = m_buttonRect.x + (m_buttonRect.w / 2) - (textWidth / 2);

        m_highlight = false;
    }

    void Draw(SDL_Renderer* renderer, SDL_Color uiColor)
    {
        {
            SDL_SetRenderScale(renderer, 1, 1);

            // Fill the background
            if (m_highlight || m_onMenubar)
            {
                SDL_SetRenderDrawColor(renderer, uiColor.r, uiColor.g, uiColor.b, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }
            SDL_RenderFillRect(renderer, &m_buttonRect);
        }
        {
            float textX = m_buttonTextX;
            float textY = m_buttonRect.y;
            const float textScale = hackforge::toolbar_text_scaling;
            SDL_SetRenderScale(renderer, textScale, textScale);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            if (m_highlight)
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
            SDL_RenderDebugText(
                renderer,
                textX / hackforge::toolbar_text_scaling,
                textY / hackforge::toolbar_text_scaling + (hackforge::toolbar_height / (hackforge::toolbar_text_scaling * 4)),
                m_buttonText.c_str());
        }
    }

    void OnMouseMove(float x, float y)
    {
        m_highlight = hackforge::IsInBounds(x, y, m_buttonRect);
    }

    float GetWidth() const
    {
        return m_buttonRect.w;
    }

    bool IsHighlighted() const { return m_highlight; }

    void SetOnMenubar(bool v) { m_onMenubar = v; }
};


class LabelledTextBox
{
    SDL_FRect m_textBoxRect;
    float m_labelX;
    float m_labelY;
    std::string m_labelText;
    std::string m_text;
    bool m_clearBufferNext;
    bool m_focused;

public:
    enum FormatMode
    {
        Decimal5Digits,
        Hex6Digits,
        AlphanumericChars
    };

    void Layout(float* pLayoutX, float* pLayoutY, float tabWidth, float textBoxWidth, std::string const& labelText, int numericalField, FormatMode formatMode)
    {
        m_labelText = labelText;
        m_formatMode = formatMode;

        SetNumericalValue(numericalField);

        m_labelX = *pLayoutX;
        m_labelY = *pLayoutY;

        m_textBoxRect.x = *pLayoutX + tabWidth;
        m_textBoxRect.y = *pLayoutY;
        m_textBoxRect.w = textBoxWidth;
        m_textBoxRect.h = hackforge::toolbar_height;

        *pLayoutY += hackforge::toolbar_height;

        m_focused = false;
        m_clearBufferNext = false;
    }

    void SetNumericalValueFromColor(SDL_Color col)
    {
        int rgbValue = (col.r << 16) | (col.g << 8) | col.b;
        SetNumericalValue(rgbValue);
    }

    void SetNumericalValue(int numericalField)
    {
        if (m_formatMode == FormatMode::Decimal5Digits)
        {
            m_text = std::to_string(numericalField).substr(0, 10);
        }
        else if (m_formatMode == FormatMode::Hex6Digits)
        {
            char buf[7];
            snprintf(buf, sizeof(buf), "%06X", numericalField);
            m_text = buf;
        }
    }

    void Draw(SDL_Renderer* renderer, SDL_Color uiColor)
    {
        // Draw the label e.g., "Width:"
        {
            float textX = m_labelX;
            float textY = m_labelY;
            const float textScale = hackforge::toolbar_text_scaling;
            SDL_SetRenderScale(renderer, textScale, textScale);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDebugText(
                renderer,
                textX / hackforge::toolbar_text_scaling,
                textY / hackforge::toolbar_text_scaling + (hackforge::toolbar_height / (hackforge::toolbar_text_scaling * 4)),
                m_labelText.c_str());
        }

        // Draw the box
        {
            SDL_SetRenderScale(renderer, 1, 1);
            if (m_focused)
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &m_textBoxRect);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, uiColor.r, uiColor.g, uiColor.b, 255);
                SDL_RenderRect(renderer, &m_textBoxRect);
            }
        }

        // Draw the text that goes in the box.
        // Position it to be right-justified
        {
            float textWidth = hackforge::GetRenderedTextWidthInPixels(m_text.length());
            float textX = m_textBoxRect.x + m_textBoxRect.w - textWidth;
            float textY = m_textBoxRect.y;
            const float textScale = hackforge::toolbar_text_scaling;
            SDL_SetRenderScale(renderer, textScale, textScale);

            if (m_focused)
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }
            SDL_RenderDebugText(
                renderer,
                textX / hackforge::toolbar_text_scaling,
                textY / hackforge::toolbar_text_scaling + (hackforge::toolbar_height / (hackforge::toolbar_text_scaling * 4)),
                m_text.c_str());
        }
    }

    void OnMouseClick(float x, float y)
    {
        if (hackforge::IsInBounds(x, y, m_textBoxRect))
        {
            if (!m_focused)
            {
                m_clearBufferNext = true;
            }
            m_focused = true;
        }
        else
        {
            m_focused = false;
        }
    }

    void OnKeyboardInput(SDL_Keycode key)
    {
        if (m_clearBufferNext)
        {
            m_text.clear();
            m_clearBufferNext = false;
        }

        if (key == 8) // backspace
        {
            if (m_text.length() > 0)
            {
                m_text.pop_back();
            }
            return;
        }

        if (m_formatMode == FormatMode::AlphanumericChars)
        {
            static const int textLimit = 28;

            if (m_text.length() < textLimit)
            {
                bool isVisibleAscii = key >= 32 && key <= 126;
                if (isVisibleAscii)
                {
                    char keyCh = key;
                    m_text.push_back(keyCh);
                }
            }
            return;
        }
        else if (m_formatMode == FormatMode::Decimal5Digits)
        {
            static const int textLimit = 4;

            if (key >= '0' && key <= '9')
            {
                if (m_text.length() < textLimit)
                {
                    char keyCh = key;
                    m_text.push_back(keyCh);
                }
                return;
            }
        }
        else if (m_formatMode == FormatMode::Hex6Digits)
        {
            static const int textLimit = 6;

            if (key >= 'A' && key <= 'F')
            {
                if (m_text.length() < textLimit)
                {
                    char keyCh = key;
                    m_text.push_back(keyCh);
                }
                return;
            }

            if (key >= 'a' && key <= 'f')
            {
                if (m_text.length() < textLimit)
                {
                    char keyCh = key;
                    keyCh = toupper(keyCh);
                    m_text.push_back(keyCh);
                }
                return;
            }

            if (key >= '0' && key <= '9')
            {
                if (m_text.length() < textLimit)
                {
                    char keyCh = key;
                    m_text.push_back(keyCh);
                }
                return;
            }
        }
    }

    int GetTextFieldNumericValue() const
    {
        if (m_formatMode == FormatMode::Decimal5Digits)
        {
            int value = atoi(m_text.c_str());
            return value;
        }
        else if (m_formatMode == FormatMode::Hex6Digits)
        {
            char* endptr;
            int value = strtol(m_text.c_str(), &endptr, 16);
            return value;
        }

        return 0;
    }

    std::string GetTextFieldValue() const
    {
        return m_text;
    }

    bool IsFocused() const { return m_focused; }

    void SetFocus(bool value) { m_focused = value; }

    private:
        FormatMode m_formatMode;
};

class DialogBoxCommon
{
public:
    enum class DialogType
    {
        InfoOK,
        SaveFile_ConfirmOverwrite_OKCancel,
        SetPenColor,
        SetUIColor,
        SetPenRadius,
        SetPenAngle,
        NewDocument,
        Resize,
        OpenFile,
        SaveFile
    };

    DialogBoxCommon(DialogType type, const char* label)
    {
        m_type = type;
        m_dialogTitle = label;
    }

    DialogType GetType() const { return m_type; }

    virtual void OnMouseMove(float x, float y, bool mouseButtonDown) = 0;
    virtual void OnMouseClick(float x, float y, SDL_Renderer* renderer, bool* pCloseDialog) = 0;
    virtual void OnMouseWheel(SDL_MouseWheelDirection dir, float x, float y) {};
    virtual void OnKeyboardInput(SDL_Keycode key, bool* pCloseDialog) = 0;
    virtual void ShowDialog(SDL_Renderer* renderer, SDL_Color uiColor) = 0;
    virtual DialogResult GetDialogResult() const = 0;

protected:
    DialogType m_type;
    std::string m_dialogTitle;
    float m_menubarTextX;
    SDL_FRect m_dialogRect;

    void LayoutCommon(float dialogRectWidth)
    {
        // Center the menubar text
        float menubarTextWidth = hackforge::GetRenderedTextWidthInPixels(m_dialogTitle.length());
        m_menubarTextX = m_dialogRect.x + (dialogRectWidth / 2) - (menubarTextWidth / 2);
    }

    void DrawBlankWindow(SDL_Renderer* renderer, SDL_Color uiColor)
    {
        // Draw a solid rect for the background
        {
            SDL_SetRenderScale(renderer, 1, 1);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &m_dialogRect);

            SDL_SetRenderDrawColor(renderer, uiColor.r, uiColor.g, uiColor.b, 255);
            SDL_RenderRect(renderer, &m_dialogRect);
        }

        // Draw a filled rect for the top "menubar" of the dialog
        {
            SDL_FRect rect{};
            rect.x = m_dialogRect.x;
            rect.y = m_dialogRect.y;
            rect.w = m_dialogRect.w;
            rect.h = hackforge::toolbar_height;
            SDL_SetRenderDrawColor(renderer, uiColor.r, uiColor.g, uiColor.b, 255);
            SDL_RenderFillRect(renderer, &rect);
        }

        // Draw the menubar label
        {
            float textX = m_menubarTextX;
            float textY = m_dialogRect.y;
            const float textScale = hackforge::toolbar_text_scaling;
            SDL_SetRenderScale(renderer, textScale, textScale);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDebugText(
                renderer,
                textX / hackforge::toolbar_text_scaling,
                textY / hackforge::toolbar_text_scaling + (hackforge::toolbar_height / (hackforge::toolbar_text_scaling * 4)),
                m_dialogTitle.c_str());
        }
    }
};

class SingleNumberInputDialogBox : public DialogBoxCommon
{
    LabelledTextBox m_textBox;
    Button m_okButton;
    Button m_cancelButton;
    DialogResult m_dialogResult;
    int m_minValue;
    int m_maxValue;

public:
    SingleNumberInputDialogBox(DialogType type, const char* label, const char* valueLabel, int parentWindowWidth, int parentWindowHeight, int numericValue) 
        : DialogBoxCommon(type, label)
    {
        m_dialogResult = DialogResult::None;
        Layout(parentWindowWidth, parentWindowHeight, numericValue, valueLabel);
    }

    void OnMouseMove(float x, float y, bool) override
    {
        m_okButton.OnMouseMove(x, y);
        m_cancelButton.OnMouseMove(x, y);
    }

    void OnMouseClick(float x, float y, SDL_Renderer* renderer, bool* pCloseDialog) override
    {
        if (m_cancelButton.IsHighlighted())
        {
            *pCloseDialog = true;
            m_dialogResult = DialogResult::Cancel;
            return;
        }

        if (m_okButton.IsHighlighted())
        {
            *pCloseDialog = true;
            m_dialogResult = DialogResult::OK;
            return;
        }

        m_textBox.OnMouseClick(x, y);
    }

    void OnKeyboardInput(SDL_Keycode key, bool* pCloseDialog) override
    {
        if (key == 13) // Enter
        {
            *pCloseDialog = true;
            m_dialogResult = DialogResult::OK;
            return;
        }

        if (key == 27) // Escape
        {
            *pCloseDialog = true;
            m_dialogResult = DialogResult::Cancel;
            return;
        }

        if (m_textBox.IsFocused())
        {
            m_textBox.OnKeyboardInput(key);
            return;
        }
    }

    void ShowDialog(SDL_Renderer* renderer, SDL_Color uiColor) override
    {
        DrawBlankWindow(renderer, uiColor);
        m_textBox.Draw(renderer, uiColor);
        m_okButton.Draw(renderer, uiColor);
        m_cancelButton.Draw(renderer, uiColor);
    }

    DialogResult GetDialogResult() const override
    {
        return m_dialogResult;
    }

    int GetRequestedValue()
    {
        int value = m_textBox.GetTextFieldNumericValue();
        return value;
    }

private:

    void Layout(int parentWindowWidth, int parentWindowHeight, int numericValue, const char* valueLabel)
    {
        // Size chosen based on the baked-in choice of elements on the dialog
        int dialogWidth = 300;
        int dialogHeight = 260;

        // Center the dialog
        m_dialogRect.x = static_cast<float>((parentWindowWidth / 2) - (dialogWidth / 2));
        m_dialogRect.y = static_cast<float>((parentWindowHeight / 2) - (dialogHeight / 2));

        m_dialogRect.w = static_cast<float>(dialogWidth);
        m_dialogRect.h = static_cast<float>(dialogHeight);

        // Center the menubar text
        LayoutCommon(m_dialogRect.w);

        float layoutX = m_dialogRect.x + hackforge::sc_dialogbox_margin;
        float layoutY = m_dialogRect.y + hackforge::toolbar_height + hackforge::sc_dialogbox_margin;

        m_textBox.Layout(&layoutX, &layoutY, hackforge::sc_dialogbox_tabStop, 100, valueLabel, numericValue, LabelledTextBox::FormatMode::Decimal5Digits);
        layoutY += hackforge::sc_dialogbox_margin;
        layoutY += hackforge::sc_dialogbox_margin;

        m_okButton.Layout(&layoutX, &layoutY, "OK", hackforge::sc_dialogbox_margin);
        layoutX += hackforge::sc_dialogbox_tabStop;
        m_cancelButton.Layout(&layoutX, &layoutY, "Cancel", hackforge::sc_dialogbox_margin);
    }
};

class SizeInputDialogBox : public DialogBoxCommon
{
    LabelledTextBox m_widthTextBox;
    LabelledTextBox m_heightTextBox;
    Button m_okButton;
    Button m_cancelButton;
    DialogResult m_dialogResult;

public:

    SizeInputDialogBox(DialogType type, const char* label, int parentWindowWidth, int parentWindowHeight) : DialogBoxCommon(type, label)
    {
        m_dialogResult = DialogResult::None;
        Layout(parentWindowWidth, parentWindowHeight);
    }

    void OnMouseMove(float x, float y, bool) override
    {
        m_okButton.OnMouseMove(x, y);
        m_cancelButton.OnMouseMove(x, y);
    }

    void OnMouseClick(float x, float y, SDL_Renderer* renderer, bool* pCloseDialog) override
    {
        if (m_cancelButton.IsHighlighted())
        {
            *pCloseDialog = true;
            m_dialogResult = DialogResult::Cancel;
            return;
        }

        if (m_okButton.IsHighlighted())
        {
            *pCloseDialog = true;
            m_dialogResult = DialogResult::OK;
            return;
        }

        m_widthTextBox.OnMouseClick(x, y);
        m_heightTextBox.OnMouseClick(x, y);
    }

    void OnKeyboardInput(SDL_Keycode key, bool* pCloseDialog) override
    {
        if (key == 9) // tab
        {
            if (m_widthTextBox.IsFocused())
            {
                m_widthTextBox.SetFocus(false);
                m_heightTextBox.SetFocus(true);
            }
            else if (m_heightTextBox.IsFocused())
            {
                m_widthTextBox.SetFocus(true);
                m_heightTextBox.SetFocus(false);
            }
            return;
        }

        if (key == 13) // Enter
        {
            *pCloseDialog = true;
            m_dialogResult = DialogResult::OK;
            return;
        }

        if (key == 27) // Escape
        {
            *pCloseDialog = true;
            m_dialogResult = DialogResult::Cancel;
            return;
        }

        if (m_widthTextBox.IsFocused())
        {
            m_widthTextBox.OnKeyboardInput(key);
            return;
        }

        if (m_heightTextBox.IsFocused())
        {
            m_heightTextBox.OnKeyboardInput(key);
            return;
        }
    }

    void ShowDialog(SDL_Renderer* renderer, SDL_Color uiColor) override
    {
        DrawBlankWindow(renderer, uiColor);
        m_widthTextBox.Draw(renderer, uiColor);
        m_heightTextBox.Draw(renderer, uiColor);
        m_okButton.Draw(renderer, uiColor);
        m_cancelButton.Draw(renderer, uiColor);
    }

    DialogResult GetDialogResult() const override
    {
        return m_dialogResult;
    }

    int GetRequestedCanvasWidth()
    {
        int value = m_widthTextBox.GetTextFieldNumericValue();
        return value;
    }

    int GetRequestedCanvasHeight()
    {
        int value = m_heightTextBox.GetTextFieldNumericValue();
        return value;
    }

private:

    void Layout(int parentWindowWidth, int parentWindowHeight)
    {
        // Size chosen based on the baked-in choice of elements on the dialog
        int dialogWidth = 300;
        int dialogHeight = 260;

        // Center the dialog
        m_dialogRect.x = static_cast<float>((parentWindowWidth / 2) - (dialogWidth / 2));
        m_dialogRect.y = static_cast<float>((parentWindowHeight / 2) - (dialogHeight / 2));

        m_dialogRect.w = static_cast<float>(dialogWidth);
        m_dialogRect.h = static_cast<float>(dialogHeight);

        // Center the menubar text
        LayoutCommon(m_dialogRect.w);

        float layoutX = m_dialogRect.x + hackforge::sc_dialogbox_margin;
        float layoutY = m_dialogRect.y + hackforge::toolbar_height + hackforge::sc_dialogbox_margin;

        m_widthTextBox.Layout(&layoutX, &layoutY, hackforge::sc_dialogbox_tabStop, 100, "Width:", parentWindowWidth, LabelledTextBox::FormatMode::Decimal5Digits);
        layoutY += hackforge::sc_dialogbox_margin;

        m_heightTextBox.Layout(&layoutX, &layoutY, hackforge::sc_dialogbox_tabStop, 100, "Height:", parentWindowHeight, LabelledTextBox::FormatMode::Decimal5Digits);
        layoutY += hackforge::sc_dialogbox_margin;
        layoutY += hackforge::sc_dialogbox_margin;

        m_okButton.Layout(&layoutX, &layoutY, "OK", hackforge::sc_dialogbox_margin);
        layoutX += hackforge::sc_dialogbox_tabStop;
        m_cancelButton.Layout(&layoutX, &layoutY, "Cancel", hackforge::sc_dialogbox_margin);
    }
};

