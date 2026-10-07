#pragma once

#include <sstream>
#include <cassert>

enum class DialogResult
{
    None,
    OK,
    Cancel
};

class Button
{
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
            if (m_highlight)
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


    bool IsHighlighted() const { return m_highlight; }
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
    void Layout(float* pLayoutX, float* pLayoutY, float tabWidth, std::string const& labelText, int numericalField)
    {
        m_labelText = labelText;

        {
            char buf[5];
            _itoa_s(numericalField, buf, 10);
            m_text = buf;
        }

        m_labelX = *pLayoutX;
        m_labelY = *pLayoutY;

        m_textBoxRect.x = *pLayoutX + tabWidth;
        m_textBoxRect.y = *pLayoutY;
        m_textBoxRect.w = 100;
        m_textBoxRect.h = hackforge::toolbar_height;

        *pLayoutY += hackforge::toolbar_height;

        m_focused = false;
        m_clearBufferNext = false;
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
        if (key >= '0' && key <= '9')
        {
            if (m_clearBufferNext)
            {
                m_text.clear();
                m_clearBufferNext = false;
            }
            if (m_text.length() < 4)
            {
                char keyCh = key;
                m_text.push_back(keyCh);
            }
        }
        else if (key == 8) // backspace
        {
            if (m_text.length() > 0)
            {
                m_text.pop_back();
            }
        }

    }

    int GetTextFieldNumericValue() const
    {
        int value = atoi(m_text.c_str());
        return value;
    }

    bool IsFocused() const { return m_focused; }

    void SetFocus(bool value) { m_focused = value; }
};

class DialogBoxCommon
{
public:
    std::string m_dialogTitle;
    float m_menubarTextX;
    SDL_FRect m_dialogRect;

    DialogBoxCommon(const char* label)
    {
        m_dialogTitle = label;
    }

protected:

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

// A simple dialog box with an 'OK' button
class InfoDialogBox : public DialogBoxCommon
{
    std::vector<std::string> m_text;
    float m_textX;
    float m_textY;

    Button m_okButton;
public:

    InfoDialogBox(const char* dialogTitle, int parentWindowWidth, int parentWindowHeight, const char* infoText, int lineLength)
        : DialogBoxCommon(dialogTitle)
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

    void OnMouseMove(float x, float y)
    {
        m_okButton.OnMouseMove(x, y);
    }


    void OnMouseClick(float x, float y, bool* pCloseDialog)
    {
        if (m_okButton.IsHighlighted())
        {
            *pCloseDialog = true;
        }
    }

    void OnKeyboardInput(SDL_Keycode key, bool* pCloseDialog)
    {
        if (key == 13 || key == 27) // Enter or escape
        {
            *pCloseDialog = true;
            return;
        }
    }

    void ShowDialog(SDL_Renderer* renderer, SDL_Color uiColor)
    {
        DrawBlankWindow(renderer, uiColor);

        SDL_SetRenderScale(renderer, hackforge::toolbar_text_scaling, hackforge::toolbar_text_scaling);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        float layoutY = m_textY;
        for (size_t i = 0; i < m_text.size(); ++i)
        {
            SDL_RenderDebugText(
                renderer,
                m_textX / hackforge::toolbar_text_scaling,
                layoutY / hackforge::toolbar_text_scaling + (hackforge::toolbar_height / (hackforge::toolbar_text_scaling * 4)),
                m_text[i].c_str());

            layoutY += hackforge::toolbar_height;
        }

        m_okButton.Draw(renderer, uiColor);
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

        // Position the OK button
        float okButtonWidth = hackforge::GetRenderedTextWidthInPixels(2) + hackforge::sc_dialogbox_margin + hackforge::sc_dialogbox_margin;
        float okButtonX = m_dialogRect.x + (m_dialogRect.w / 2) - (okButtonWidth / 2);
        float okButtonY = m_dialogRect.y + m_dialogRect.h - hackforge::toolbar_height - hackforge::toolbar_height;
        m_okButton.Layout(&okButtonX, &okButtonY, "OK", hackforge::sc_dialogbox_margin);

        LayoutCommon(m_dialogRect.w);
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

    enum class Mode
    {
        NewDocument,
        Resize
    };

    SizeInputDialogBox(const char* label, int parentWindowWidth, int parentWindowHeight, Mode mode) : DialogBoxCommon(label)
    {
        m_dialogResult = DialogResult::None;
        m_mode = mode;
        Layout(parentWindowWidth, parentWindowHeight);
    }

    Mode GetMode() const { return m_mode; }

    void OnMouseMove(float x, float y)
    {
        m_okButton.OnMouseMove(x, y);
        m_cancelButton.OnMouseMove(x, y);
    }

    void OnMouseClick(float x, float y, bool* pCloseDialog)
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

    void OnKeyboardInput(SDL_Keycode key, bool* pCloseDialog)
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

    void ShowDialog(SDL_Renderer* renderer, SDL_Color uiColor)
    {
        DrawBlankWindow(renderer, uiColor);
        m_widthTextBox.Draw(renderer, uiColor);
        m_heightTextBox.Draw(renderer, uiColor);
        m_okButton.Draw(renderer, uiColor);
        m_cancelButton.Draw(renderer, uiColor);
    }

    DialogResult GetDialogResult() const
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

    Mode m_mode;

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

        m_widthTextBox.Layout(&layoutX, &layoutY, hackforge::sc_dialogbox_tabStop, "Width:", parentWindowWidth);
        layoutY += hackforge::sc_dialogbox_margin;

        m_heightTextBox.Layout(&layoutX, &layoutY, hackforge::sc_dialogbox_tabStop, "Height:", parentWindowHeight);
        layoutY += hackforge::sc_dialogbox_margin;
        layoutY += hackforge::sc_dialogbox_margin;

        m_okButton.Layout(&layoutX, &layoutY, "OK", hackforge::sc_dialogbox_margin);
        layoutX += hackforge::sc_dialogbox_tabStop;
        m_cancelButton.Layout(&layoutX, &layoutY, "Cancel", hackforge::sc_dialogbox_margin);
    }
};


// A simple dialog box with an 'OK' button
class ColorPickerDialogBox : public DialogBoxCommon
{
    Button m_okButton;
    Button m_cancelButton;
    DialogResult m_dialogResult;

    SDL_FRect m_rainbowGradientRect;
    SDL_Vertex m_rainbowGradientVertices[14];
    int m_rainbowGradientIndices[36];

    SDL_FRect m_mainPanelRect;
    SDL_Vertex m_mainPanelGradientVertices[4];
    int m_mainPanelGradientIndices[6];

    SDL_FRect m_currentColorRect;

    SDL_Color m_selectedColor;
    bool selectedColorNeedsUpdateFromPicking = false;

    float mainPanelSelectionX = 0;
    float mainPanelSelectionY = 0;

    float rainbowGradientSelectionY = 0;

public:

    ColorPickerDialogBox(const char* dialogTitle, int parentWindowWidth, int parentWindowHeight, SDL_Color selectedColor)
        : DialogBoxCommon(dialogTitle), m_selectedColor(selectedColor)
    {
        Layout(parentWindowWidth, parentWindowHeight);
        InitializeRainbowGradient();
        InitializeMainPanelGradient();

        m_currentColorRect.x = m_dialogRect.x + hackforge::sc_dialogbox_margin;
        m_currentColorRect.y = m_mainPanelRect.y + m_mainPanelRect.h + hackforge::sc_dialogbox_margin;
        m_currentColorRect.w = 50;
        m_currentColorRect.h = hackforge::sc_dialogbox_margin;

        m_dialogResult = DialogResult::None;
    }

    SDL_Color GetRequestedColor() const { return m_selectedColor; }

    DialogResult GetDialogResult() const
    {
        return m_dialogResult;
    }

    void OnMouseMove(float x, float y, bool mouseButtonDown)
    {
        m_okButton.OnMouseMove(x, y);
        m_cancelButton.OnMouseMove(x, y);

        if (hackforge::IsInBounds(x, y, m_mainPanelRect) && mouseButtonDown)
        {
            mainPanelSelectionX = x - m_mainPanelRect.x;
            mainPanelSelectionY = y - m_mainPanelRect.y;

            selectedColorNeedsUpdateFromPicking = true;
            return;
        }
    }

    void OnMouseClick(float x, float y, SDL_Renderer* renderer, bool* pCloseDialog)
    {
        if (m_okButton.IsHighlighted())
        {
            *pCloseDialog = true;
            m_dialogResult = DialogResult::OK;
            return;
        }

        if (m_cancelButton.IsHighlighted())
        {
            *pCloseDialog = true;
            m_dialogResult = DialogResult::Cancel;
            return;
        }

        if (hackforge::IsInBounds(x, y, m_rainbowGradientRect))
        {
            rainbowGradientSelectionY = y - m_rainbowGradientRect.y;

            SDL_Color px = SinglePixelCpuReadback((int)x, (int)y, renderer);

            // Re-colorize the main panel
            m_mainPanelGradientVertices[1].color = hackforge::OpaqueUnormColorToOpaqueFloatColor(px);

            selectedColorNeedsUpdateFromPicking = true;
            return;
        }
    }

    SDL_Color SinglePixelCpuReadback(int x, int y, SDL_Renderer* renderer)
    {
        // Sigh
        SDL_Color c;
        SDL_Rect rect = { (int)x, (int)y, 1, 1 };
        SDL_Surface* temporaryCpuRead = SDL_RenderReadPixels(renderer, &rect);
        SDL_ReadSurfacePixel(temporaryCpuRead, 0, 0, &c.r, &c.g, &c.b, &c.a);
        SDL_DestroySurface(temporaryCpuRead);
        return c;
    }

    void OnKeyboardInput(SDL_Keycode key, bool* pCloseDialog)
    {
        if (key == 13 || key == 27) // Enter or escape
        {
            *pCloseDialog = true;
            return;
        }
    }

    void InitializeMainPanelGradient()
    {
        m_mainPanelRect.w = 200;
        m_mainPanelRect.h = 200;
        m_mainPanelRect.x = this->m_dialogRect.x + hackforge::sc_dialogbox_margin;
        m_mainPanelRect.y = this->m_dialogRect.y + hackforge::toolbar_line_height + hackforge::sc_dialogbox_margin;

        // Figure out what initial hue to use
        Uint8 m = m_selectedColor.r;
        m = std::max(m, m_selectedColor.g);
        m = std::max(m, m_selectedColor.b);
        float scale = static_cast<float>(m);
        float scaledR = float(m_selectedColor.r) / scale;
        float scaledG = float(m_selectedColor.g) / scale;
        float scaledB = float(m_selectedColor.b) / scale;
        SDL_FColor maxSat{};
        maxSat.a = 1.0f;
        maxSat.r = scaledR;
        maxSat.g = scaledG;
        maxSat.b = scaledB;

        const SDL_FColor panel[4] = {
            {1.0f, 1.0f, 1.0f, 1.0f}, // White
            maxSat, // Magenta
            {0.0f, 0.0f, 0.0f, 1.0f}, // Black
            {0.0f, 0.0f, 0.0f, 1.0f}, // Black
        };

        m_mainPanelGradientVertices[0].position.x = m_mainPanelRect.x;
        m_mainPanelGradientVertices[0].position.y = m_mainPanelRect.y;
        m_mainPanelGradientVertices[0].color = panel[0];

        m_mainPanelGradientVertices[1].position.x = m_mainPanelRect.x + m_mainPanelRect.w;
        m_mainPanelGradientVertices[1].position.y = m_mainPanelRect.y;
        m_mainPanelGradientVertices[1].color = panel[1];

        m_mainPanelGradientVertices[2].position.x = m_mainPanelRect.x;
        m_mainPanelGradientVertices[2].position.y = m_mainPanelRect.y + m_mainPanelRect.h;
        m_mainPanelGradientVertices[2].color = panel[2];

        m_mainPanelGradientVertices[3].position.x = m_mainPanelRect.x + m_mainPanelRect.w;
        m_mainPanelGradientVertices[3].position.y = m_mainPanelRect.y + m_mainPanelRect.h;
        m_mainPanelGradientVertices[3].color = panel[3];

        m_mainPanelGradientIndices[0] = 0; // Top left
        m_mainPanelGradientIndices[1] = 1; // Top right
        m_mainPanelGradientIndices[2] = 2; // Bottom left

        m_mainPanelGradientIndices[3] = 1; // Top right
        m_mainPanelGradientIndices[4] = 3; // Bottom right
        m_mainPanelGradientIndices[5] = 2; // Bottom left
    }

    void InitializeRainbowGradient()
    {
        m_rainbowGradientRect.w = 20;
        m_rainbowGradientRect.h = 200;
        m_rainbowGradientRect.x = this->m_dialogRect.x + this->m_dialogRect.w - m_rainbowGradientRect.w - hackforge::sc_dialogbox_margin;
        m_rainbowGradientRect.y = this->m_dialogRect.y + hackforge::toolbar_line_height + hackforge::sc_dialogbox_margin;

        const SDL_FColor rainbow[7] = {
            {1.0f, 0.0f, 0.0f, 1.0f}, // Red
            {1.0f, 1.0f, 0.0f, 1.0f}, // Yellow
            {0.0f, 1.0f, 0.0f, 1.0f}, // Green
            {0.0f, 1.0f, 1.0f, 1.0f}, // Cyan
            {0.0f, 0.0f, 1.0f, 1.0f}, // Blue
            {1.0f, 0.0f, 1.0f, 1.0f}, // Magenta
            {1.0f, 0.0f, 0.0f, 1.0f}  // Back to Red
        };

        // 7 stops in all
        float segment_size = m_rainbowGradientRect.h / 6.0f;
        for (int i = 0; i < 7; i++)
        {
            float vy = m_rainbowGradientRect.y + (i * segment_size);
            SDL_FColor color = rainbow[i];

            // Top vertex of the column
            m_rainbowGradientVertices[i * 2].position.x = m_rainbowGradientRect.x;
            m_rainbowGradientVertices[i * 2].position.y = vy;
            m_rainbowGradientVertices[i * 2].color = color;

            // Bottom vertex of the column
            m_rainbowGradientVertices[i * 2 + 1].position.x = m_rainbowGradientRect.x + m_rainbowGradientRect.w;
            m_rainbowGradientVertices[i * 2 + 1].position.y = vy;
            m_rainbowGradientVertices[i * 2 + 1].color = color;
        }

        // Six squares of two triangles each
        int index_count = 0;
        for (int i = 0; i < 6; i++) {
            int tl = i * 2;       // Top Left
            int bl = i * 2 + 1;   // Bottom Left
            int tr = (i + 1) * 2; // Top Right
            int br = (i + 1) * 2 + 1; // Bottom Right

            m_rainbowGradientIndices[index_count++] = tl; // top left
            m_rainbowGradientIndices[index_count++] = tr; // top right
            m_rainbowGradientIndices[index_count++] = bl; // bottom left

            m_rainbowGradientIndices[index_count++] = tr; // top right
            m_rainbowGradientIndices[index_count++] = br; // bottom right
            m_rainbowGradientIndices[index_count++] = bl; // bottom left
        }
    }

    void DrawSelectionRect(SDL_Renderer* renderer, SDL_FRect rect)
    {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderRect(renderer, &rect);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        rect.x -= 1;
        rect.y -= 1;
        rect.w += 2;
        rect.h += 2;
        SDL_RenderRect(renderer, &rect);
    }

    void ShowDialog(SDL_Renderer* renderer, SDL_Color uiColor)
    {
        DrawBlankWindow(renderer, uiColor);

        SDL_SetRenderScale(renderer, hackforge::toolbar_text_scaling, hackforge::toolbar_text_scaling);

        SDL_SetRenderScale(renderer, 1, 1);
        SDL_RenderGeometry(renderer, NULL, m_rainbowGradientVertices, 14, m_rainbowGradientIndices, 36);
        SDL_RenderGeometry(renderer, NULL, m_mainPanelGradientVertices, 4, m_mainPanelGradientIndices, 6);

        // Draw a rectangle around the currently-selected item on the main panel

        float selectionIndicatorSize = 9;
        float selectionIndicatorSizeDiv2 = selectionIndicatorSize / 2.0f;
        {
            SDL_FRect selectRect{};
            selectRect.x = m_mainPanelRect.x + mainPanelSelectionX - selectionIndicatorSizeDiv2 + 1;
            selectRect.y = m_mainPanelRect.y + mainPanelSelectionY - selectionIndicatorSizeDiv2 + 1;
            selectRect.w = selectionIndicatorSize;
            selectRect.h = selectionIndicatorSize;
            DrawSelectionRect(renderer, selectRect);
        }

        // Highlight the currently-selected item on the rainbow gradient
        {
            SDL_FRect selectRect{};
            selectRect.x = m_rainbowGradientRect.x;
            selectRect.y = m_rainbowGradientRect.y + rainbowGradientSelectionY - selectionIndicatorSizeDiv2 + 1;
            selectRect.w = m_rainbowGradientRect.w;
            selectRect.h = selectionIndicatorSize;
            DrawSelectionRect(renderer, selectRect); 
        }

        if (selectedColorNeedsUpdateFromPicking)
        {
            m_selectedColor = SinglePixelCpuReadback(
                (int)(mainPanelSelectionX + m_mainPanelRect.x), 
                (int)(mainPanelSelectionY + m_mainPanelRect.y), 
                renderer);
            selectedColorNeedsUpdateFromPicking = false;
        }
        // Draw the currently selected color 
        SDL_SetRenderDrawColor(renderer, m_selectedColor.r, m_selectedColor.g, m_selectedColor.b, 255);
        SDL_RenderFillRect(renderer, &m_currentColorRect);

        m_okButton.Draw(renderer, uiColor);
        m_cancelButton.Draw(renderer, uiColor);
    }

private:
    void Layout(int parentWindowWidth, int parentWindowHeight)
    {
        // Size chosen based on the baked-in choice of elements on the dialog
        int dialogWidth = 280;
        int dialogHeight = 400;

        // Center the dialog
        m_dialogRect.x = static_cast<float>((parentWindowWidth / 2) - (dialogWidth / 2));
        m_dialogRect.y = static_cast<float>((parentWindowHeight / 2) - (dialogHeight / 2));

        m_dialogRect.w = static_cast<float>(dialogWidth);
        m_dialogRect.h = static_cast<float>(dialogHeight);

        // Position the OK button
        float okButtonWidth = hackforge::GetRenderedTextWidthInPixels(2) + hackforge::sc_dialogbox_margin + hackforge::sc_dialogbox_margin;
        float okButtonX = m_dialogRect.x + hackforge::sc_dialogbox_margin;
        float okButtonY = m_dialogRect.y + m_dialogRect.h - hackforge::toolbar_height - hackforge::sc_dialogbox_margin;
        m_okButton.Layout(&okButtonX, &okButtonY, "OK", hackforge::sc_dialogbox_margin);

        LayoutCommon(m_dialogRect.w);
    }
};