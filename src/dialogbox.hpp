#pragma once

class ResizeDialogBox
{
    class Button
    {
    public:
        SDL_FRect m_buttonRect;
        std::string m_buttonText;
        float m_buttonTextX;

        void Layout(float* pLayoutX, float* pLayoutY, std::string const& text, float margin)
        {
            m_buttonText = text;
            m_buttonRect.x = *pLayoutX;
            m_buttonRect.y = *pLayoutY;

            float textWidth = text.length() * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * hackforge::toolbar_text_scaling;
            float requiredWidth = textWidth + margin + margin;

            m_buttonRect.w = requiredWidth;
            m_buttonRect.h = hackforge::toolbar_height;

            // Center the label onto the button
            m_buttonTextX = m_buttonRect.x + (m_buttonRect.w / 2) - (textWidth / 2);
        }

        void Draw(SDL_Renderer* renderer, SDL_Color uiColor)
        {
            {
                SDL_SetRenderScale(renderer, 1, 1);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &m_buttonRect);
            }
            {
                float textX = m_buttonTextX;
                float textY = m_buttonRect.y;
                const float textScale = hackforge::toolbar_text_scaling;
                SDL_SetRenderScale(renderer, textScale, textScale);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDebugText(
                    renderer,
                    textX / hackforge::toolbar_text_scaling,
                    textY / hackforge::toolbar_text_scaling + (hackforge::toolbar_height / (hackforge::toolbar_text_scaling * 4)),
                    m_buttonText.c_str());
            }
        }
    };

    class LabelledTextBox
    {
    public:
        SDL_FRect m_textBoxRect;
        float m_labelX;
        float m_labelY;
        std::string m_labelText;

        std::string m_text;

        void Layout(float* pLayoutX, float* pLayoutY, float tabWidth, std::string const& labelText, std::string const& text)
        {
            m_labelText = labelText;
            m_text = text;

            m_labelX = *pLayoutX;
            m_labelY = *pLayoutY;

            m_textBoxRect.x = *pLayoutX + tabWidth;
            m_textBoxRect.y = *pLayoutY;
            m_textBoxRect.w = 100;
            m_textBoxRect.h = hackforge::toolbar_height;

            *pLayoutY += hackforge::toolbar_height;
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

            {
                SDL_SetRenderScale(renderer, 1, 1);
                SDL_SetRenderDrawColor(renderer, uiColor.r, uiColor.g, uiColor.b, 255);
                SDL_RenderRect(renderer, &m_textBoxRect);
            }

            // Draw the text that goes in the box
            {
                float textX = m_textBoxRect.x + 40;
                float textY = m_textBoxRect.y;
                const float textScale = hackforge::toolbar_text_scaling;
                SDL_SetRenderScale(renderer, textScale, textScale);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDebugText(
                    renderer,
                    textX / hackforge::toolbar_text_scaling,
                    textY / hackforge::toolbar_text_scaling + (hackforge::toolbar_height / (hackforge::toolbar_text_scaling * 4)),
                    m_text.c_str());
            }
        }

    };

    std::string m_dialogTitle;
    float m_menubarTextX;
    LabelledTextBox m_widthTextBox;
    LabelledTextBox m_heightTextBox;
    Button m_okButton;
    Button m_cancelButton;

    SDL_FRect m_dialogRect;

    const float sc_margin = 20;
    const float sc_tabStop = 130;

public:

    ResizeDialogBox(int w, int h, const char* label) : m_dialogTitle(label){}

    void Layout()
    {
        // Lay out the overall size of the dialog box.
        m_dialogRect.x = 240;
        m_dialogRect.y = 100;
        m_dialogRect.w = 300;
        m_dialogRect.h = 280;

        // Center the menubar text
        float menubarTextWidth = m_dialogTitle.length() * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * hackforge::toolbar_text_scaling;
        m_menubarTextX = m_dialogRect.x + (m_dialogRect.w / 2) - (menubarTextWidth / 2);

        float layoutX = m_dialogRect.x + sc_margin;
        float layoutY = m_dialogRect.y + hackforge::toolbar_height + sc_margin;

        m_widthTextBox.Layout(&layoutX, &layoutY, sc_tabStop, "Width:", "800");
        layoutY += sc_margin;

        m_heightTextBox.Layout(&layoutX, &layoutY, sc_tabStop, "Height:", "600");
        layoutY += sc_margin;
        layoutY += sc_margin;

        m_okButton.Layout(&layoutX, &layoutY, "OK", sc_margin);
        layoutX += sc_tabStop;
        m_cancelButton.Layout(&layoutX, &layoutY, "Cancel", sc_margin);
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

    void ShowDialog(SDL_Renderer* renderer, SDL_Color uiColor)
    {
        Layout();

        DrawBlankWindow(renderer, uiColor);
        m_widthTextBox.Draw(renderer, uiColor);
        m_heightTextBox.Draw(renderer, uiColor);
        m_okButton.Draw(renderer, uiColor);
        m_cancelButton.Draw(renderer, uiColor);
    }

    void OnMouseMove(float x, float y)
    {

    }

    void OnMouseClick(float x, float y)
    {

    }

    int GetWidth() { return 0; }

    int GetHeight() { return 0; }
};