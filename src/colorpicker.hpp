#ifndef COLORPICKER_H
#define COLORPICKER_H

#include "dialogbox.hpp"

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

    enum class Mode
    {
        SetPenColor,
        SetUIColor
    };

    ColorPickerDialogBox(const char* dialogTitle, int parentWindowWidth, int parentWindowHeight, Mode mode, SDL_Color selectedColor)
        : DialogBoxCommon(dialogTitle), m_mode(mode), m_selectedColor(selectedColor)
    {
        Layout(parentWindowWidth, parentWindowHeight);
        InitializeRainbowGradient();
        InitializeMainPanelGradient();

        m_currentColorRect.x = m_dialogRect.x + hackforge::sc_dialogbox_margin;
        m_currentColorRect.y = m_mainPanelRect.y + m_mainPanelRect.h + hackforge::sc_dialogbox_margin;
        m_currentColorRect.w = 50;
        m_currentColorRect.h = hackforge::sc_dialogbox_margin;

        m_dialogResult = DialogResult::None;

        m_cancelButton.SetOnMenubar(true);
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

        if (hackforge::IsInBounds(x, y, m_mainPanelRect))
        {
            mainPanelSelectionX = x - m_mainPanelRect.x;
            mainPanelSelectionY = y - m_mainPanelRect.y;

            selectedColorNeedsUpdateFromPicking = true;
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

        // For monochrome, arbitrarily choose red hue
        if (scaledR == scaledG && scaledR == scaledB)
        {
            scaledG = 0;
            scaledB = 0;
        }
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
        float okButtonX = m_dialogRect.x + (m_dialogRect.w / 2) - (okButtonWidth / 2);
        float okButtonY = m_dialogRect.y + m_dialogRect.h - hackforge::toolbar_height - hackforge::sc_dialogbox_margin;
        m_okButton.Layout(&okButtonX, &okButtonY, "OK", hackforge::sc_dialogbox_margin);

        float cancelButtonX = m_dialogRect.x + m_dialogRect.w - 56;
        float cancelButtonY = m_dialogRect.y;
        m_cancelButton.Layout(&cancelButtonX, &cancelButtonY, "X", hackforge::sc_dialogbox_margin);

        LayoutCommon(m_dialogRect.w);
    }

    Mode m_mode;
};

#endif
