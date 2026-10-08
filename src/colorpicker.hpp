#ifndef COLORPICKER_H
#define COLORPICKER_H

#include "dialogbox.hpp"

class ColorPickerDialogBox : public DialogBoxCommon
{
    Button m_okButton;
    Button m_cancelButton;
    LabelledTextBox m_hexColorTextBox;

    DialogResult m_dialogResult;

    SDL_FRect m_hueRect;
    SDL_Vertex m_rainbowGradientVertices[14];
    int m_rainbowGradientIndices[36];

    SDL_FRect m_mainPanelRect;
    SDL_Vertex m_mainPanelGradientVertices[4];
    int m_mainPanelGradientIndices[6];

    SDL_FRect m_currentColorRect;

    SDL_Color m_selectedColor;
    bool m_rereadSelectedColor = false;
    bool m_recolorizeMainPanel = false;

    float m_mainPanelSelectionX = 0;
    float m_mainPanelSelectionY = 0;

    float m_hueSelectionY = 0;

public:

    enum class Mode
    {
        SetPenColor,
        SetUIColor
    };

    ColorPickerDialogBox(const char* dialogTitle, int parentWindowWidth, int parentWindowHeight, Mode mode, SDL_Color selectedColor)
        : DialogBoxCommon(dialogTitle), m_mode(mode), m_selectedColor(selectedColor)
    {
        Layout(parentWindowWidth, parentWindowHeight);

        InitializeSelectionUI();

        m_dialogResult = DialogResult::None;

        m_cancelButton.SetOnMenubar(true);

        m_recolorizeMainPanel = true;
    }

    SDL_Color GetRequestedColor() const { return m_selectedColor; }

    Mode GetMode() const { return m_mode; }

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
            m_mainPanelSelectionX = x - m_mainPanelRect.x;
            m_mainPanelSelectionY = y - m_mainPanelRect.y;

            m_rereadSelectedColor = true;
            return;
        }

        if (hackforge::IsInBounds(x, y, m_hueRect) && mouseButtonDown)
        {
            m_hueSelectionY = y - m_hueRect.y;
            m_recolorizeMainPanel = true;
            m_rereadSelectedColor = true;
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

        if (hackforge::IsInBounds(x, y, m_mainPanelRect))
        {
            m_mainPanelSelectionX = x - m_mainPanelRect.x;
            m_mainPanelSelectionY = y - m_mainPanelRect.y;

            m_rereadSelectedColor = true;
            return;
        }

        if (hackforge::IsInBounds(x, y, m_hueRect))
        {
            m_hueSelectionY = y - m_hueRect.y;
            m_recolorizeMainPanel = true;
            m_rereadSelectedColor = true;
            return;
        }

        m_hexColorTextBox.OnMouseClick(x, y);
    }

    void OnKeyboardInput(SDL_Keycode key, bool* pCloseDialog)
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

        if (m_hexColorTextBox.IsFocused())
        {
            int previousNumericValue = m_hexColorTextBox.GetTextFieldNumericValue();
            m_hexColorTextBox.OnKeyboardInput(key);
            int newNumericValue = m_hexColorTextBox.GetTextFieldNumericValue();

            if (previousNumericValue == newNumericValue) return;

            // Set the current color based on the typed selection
            m_selectedColor.b = newNumericValue & 0xFF;
            m_selectedColor.g = (newNumericValue >> 8) & 0xFF;
            m_selectedColor.r = (newNumericValue >> 16) & 0xFF;
            InitializeSelectionUI();
            m_recolorizeMainPanel = true;
            return;
        }
    }

    void ShowDialog(SDL_Renderer* renderer, SDL_Color uiColor)
    {
        DrawBlankWindow(renderer, uiColor);

        SDL_SetRenderScale(renderer, 1, 1);
        SDL_RenderGeometry(renderer, NULL, m_rainbowGradientVertices, 14, m_rainbowGradientIndices, 36);

        if (m_recolorizeMainPanel)
        {
            SDL_Color px = SinglePixelCpuReadback(static_cast<int>(m_hueRect.x + 2), static_cast<int>(m_hueSelectionY + m_hueRect.y), renderer);

            // Re-colorize the main panel
            m_mainPanelGradientVertices[1].color = hackforge::OpaqueUnormColorToOpaqueFloatColor(px);
            m_recolorizeMainPanel = false;
        }

        SDL_RenderGeometry(renderer, NULL, m_mainPanelGradientVertices, 4, m_mainPanelGradientIndices, 6);

        // Draw a rectangle around the currently-selected item on the main panel

        float selectionIndicatorSize = 9;
        float selectionIndicatorSizeDiv2 = selectionIndicatorSize / 2.0f;
        {
            SDL_FRect selectRect{};
            selectRect.x = m_mainPanelRect.x + m_mainPanelSelectionX - selectionIndicatorSizeDiv2 + 1;
            selectRect.y = m_mainPanelRect.y + m_mainPanelSelectionY - selectionIndicatorSizeDiv2 + 1;
            selectRect.w = selectionIndicatorSize;
            selectRect.h = selectionIndicatorSize;
            DrawSelectionRect(renderer, selectRect);
        }

        // Highlight the currently-selected item on the rainbow gradient
        {
            SDL_FRect selectRect{};
            selectRect.x = m_hueRect.x;
            selectRect.y = m_hueRect.y + m_hueSelectionY - selectionIndicatorSizeDiv2 + 1;
            selectRect.w = m_hueRect.w;
            selectRect.h = selectionIndicatorSize;
            DrawSelectionRect(renderer, selectRect);
        }

        if (m_rereadSelectedColor)
        {
            m_selectedColor = SinglePixelCpuReadback(
                (int)(m_mainPanelSelectionX + m_mainPanelRect.x),
                (int)(m_mainPanelSelectionY + m_mainPanelRect.y),
                renderer);
            m_hexColorTextBox.SetNumericalValueFromColor(m_selectedColor);
            m_rereadSelectedColor = false;
        }
        // Draw the currently selected color 
        SDL_SetRenderDrawColor(renderer, m_selectedColor.r, m_selectedColor.g, m_selectedColor.b, 255);
        SDL_RenderFillRect(renderer, &m_currentColorRect);

        // Draw outlines around the various UI elements
        StrokeOutline(renderer, m_mainPanelRect, uiColor);
        StrokeOutline(renderer, m_hueRect, uiColor);
        StrokeOutline(renderer, m_currentColorRect, uiColor);

        m_okButton.Draw(renderer, uiColor);
        m_cancelButton.Draw(renderer, uiColor);
        m_hexColorTextBox.Draw(renderer, uiColor);
    }

private:

    void StrokeOutline(SDL_Renderer* renderer, SDL_FRect rect, SDL_Color uiColor)
    {
        static float const border = 2.0f;

        rect.x -= border;
        rect.y -= border;
        rect.w += border * 2;
        rect.h += border * 2;

        SDL_SetRenderDrawColor(renderer, uiColor.r, uiColor.g, uiColor.b, 255);
        SDL_RenderRect(renderer, &rect);
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

    void RGBToHSV(SDL_Color color, float* pRainbowPosition, float* pSaturation, float* pBrightness) 
    {
        SDL_FColor fColor = hackforge::OpaqueUnormColorToOpaqueFloatColor(color);

        // Sanitize inputs to the valid [0.0, 1.0] range
        float r = std::max(0.0f, std::min(1.0f, fColor.r));
        float g = std::max(0.0f, std::min(1.0f, fColor.g));
        float b = std::max(0.0f, std::min(1.0f, fColor.b));

        // Find the maximum and minimum color channels
        float cMax = std::max({ r, g, b });
        float cMin = std::min({ r, g, b });
        float delta = cMax - cMin;

        *pBrightness = cMax;

        *pSaturation = 0.0f;
        if (cMax != 0.0f) 
        {
            *pSaturation = delta / cMax;
        }

        // Handle grayscale edge case (where hue is undefined)
        if (delta == 0.0f) 
        {
            *pRainbowPosition = -1.0f;
            return;
        }

        // Calculate hue based on whichever channel is dominant
        float hue = 0.0f;
        if (cMax == r) 
        {
            hue = 60.0f * std::fmod(((g - b) / delta), 6.0f);
        }
        else if (cMax == g) 
        {
            hue = 60.0f * (((b - r) / delta) + 2.0f);
        }
        else if (cMax == b) {
            hue = 60.0f * (((r - g) / delta) + 4.0f);
        }

        // Ensure Hue is positive (maps -60°...0° to 300°...360°)
        if (hue < 0.0f) 
        {
            hue += 360.0f;
        }

        // Map Hue back to the [0.0, 1.0] gradient position
        *pRainbowPosition = hue / 360.0f;
    }

    void InitializeSelectionUI()
    {
        // Figure out what initial hue to use
        float rainbowPosition, saturation, brightness{};
        RGBToHSV(m_selectedColor, &rainbowPosition, &saturation, &brightness);

        m_hueSelectionY = 0; // Note: monochrome will leave this at 0, defaulting to red 
        if (rainbowPosition != -1)  
        {
            m_hueSelectionY = m_hueRect.h * rainbowPosition;
        }

        m_hueSelectionY = std::max(0.0f, m_hueSelectionY);
        m_hueSelectionY = std::min(m_hueRect.h - 1.0f, m_hueSelectionY);

        m_mainPanelSelectionX = m_mainPanelRect.w * saturation;
        m_mainPanelSelectionX = std::max(0.0f, m_mainPanelSelectionX);
        m_mainPanelSelectionX = std::min(m_mainPanelRect.w - 1.0f, m_mainPanelSelectionX);

        m_mainPanelSelectionY = m_mainPanelRect.h * (1.0f - brightness);
        m_mainPanelSelectionY = std::max(0.0f, m_mainPanelSelectionY);
        m_mainPanelSelectionY = std::min(m_mainPanelRect.h - 1.0f, m_mainPanelSelectionY);
    }

    void InitializeMainPanelGradient()
    {
        m_mainPanelRect.w = 200;
        m_mainPanelRect.h = 200;
        m_mainPanelRect.x = this->m_dialogRect.x + hackforge::sc_dialogbox_margin;
        m_mainPanelRect.y = this->m_dialogRect.y + hackforge::toolbar_line_height + hackforge::sc_dialogbox_margin;

        const SDL_FColor panel[4] = {
            {1.0f, 1.0f, 1.0f, 1.0f}, // White
            {1.0f, 0.0f, 1.0f, 1.0f}, // Magenta
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
        m_hueRect.w = 20;
        m_hueRect.h = 200;
        m_hueRect.x = this->m_dialogRect.x + this->m_dialogRect.w - m_hueRect.w - hackforge::sc_dialogbox_margin;
        m_hueRect.y = this->m_dialogRect.y + hackforge::toolbar_line_height + hackforge::sc_dialogbox_margin;

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
        float segment_size = m_hueRect.h / 6.0f;
        for (int i = 0; i < 7; i++)
        {
            float vy = m_hueRect.y + (i * segment_size);
            SDL_FColor color = rainbow[i];

            // Top vertex of the column
            m_rainbowGradientVertices[i * 2].position.x = m_hueRect.x;
            m_rainbowGradientVertices[i * 2].position.y = vy;
            m_rainbowGradientVertices[i * 2].color = color;

            // Bottom vertex of the column
            m_rainbowGradientVertices[i * 2 + 1].position.x = m_hueRect.x + m_hueRect.w;
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
        float okButtonX = m_dialogRect.x + (m_dialogRect.w / 2) - (okButtonWidth / 2);
        float okButtonY = m_dialogRect.y + m_dialogRect.h - hackforge::toolbar_height - hackforge::sc_dialogbox_margin;
        m_okButton.Layout(&okButtonX, &okButtonY, "OK", hackforge::sc_dialogbox_margin);

        float cancelButtonX = m_dialogRect.x + m_dialogRect.w - 56;
        float cancelButtonY = m_dialogRect.y;
        m_cancelButton.Layout(&cancelButtonX, &cancelButtonY, "X", hackforge::sc_dialogbox_margin);

        LayoutCommon(m_dialogRect.w);

        InitializeRainbowGradient();
        InitializeMainPanelGradient();

        // Position the "current color" UI, which flows just below the main panel gradient
        m_currentColorRect.x = m_dialogRect.x + hackforge::sc_dialogbox_margin;
        m_currentColorRect.y = m_mainPanelRect.y + m_mainPanelRect.h + hackforge::sc_dialogbox_margin;
        m_currentColorRect.w = 50;
        m_currentColorRect.h = hackforge::toolbar_height;

        // Put the hex color just to the right
        float layoutX = m_currentColorRect.x + m_currentColorRect.w + hackforge::sc_dialogbox_margin;
        float layoutY = m_currentColorRect.y;

        int rgbValue = (m_selectedColor.r << 16) | (m_selectedColor.g << 8) | m_selectedColor.b;
        m_hexColorTextBox.Layout(&layoutX, &layoutY, 16, "#", rgbValue, LabelledTextBox::FormatMode::Hex6Digits);
    }

    Mode m_mode;
};

#endif
