#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <vector>

#include "LayoutConstants.hpp"

void OnToolbarNew();
void OnToolbarExit();
void OnToolbarSetPenColor();
void OnToolbarSetUIColor();
void OnToolbarSetStampTool();
void OnToolbarSetAnglePenTool();

class MenuItem; // Forward declare
class TopLevelMenuItem;

struct UILayoutState
{
    // For each of these, only one is permitted at a time.
    MenuItem* WhichItemMousedOver;
    TopLevelMenuItem* WhichItemExpanded;
};

// Common base class for top-level menu items and child menu items.
class MenuItem
{
public:

    MenuItem() : m_bounds{}, m_checkedState(false)
    {
    }

    virtual ~MenuItem() = default;

    void SetLabel(std::string const& label)
    {
        m_label = label;
    }

    void SetBounds(float x, float y, float w, float h)
    {
        m_bounds.x = x;
        m_bounds.y = y;
        m_bounds.w = w;
        m_bounds.h = h;
    }

    void RenderLabel(SDL_Renderer* renderer, UILayoutState const& uiLayoutState)
    {
        const float textScale = hackforge::toolbar_text_scaling;
        SDL_SetRenderScale(renderer, textScale, textScale);
        if (uiLayoutState.WhichItemMousedOver == this)
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        }
        else if (m_checkedState)
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }
        SDL_RenderDebugText(
            renderer, 
            m_bounds.x / hackforge::toolbar_text_scaling, 
            m_bounds.y / hackforge::toolbar_text_scaling + (hackforge::toolbar_height / (hackforge::toolbar_text_scaling * 4) ),
            m_label.c_str());        
    }

    void SetCheckedState(bool b)
    {
        m_checkedState = b;
    }

protected:
    SDL_FRect m_bounds;
    std::string m_label;
    bool m_checkedState;

    bool IsInBounds(float x, float y)
    {
        if (x > m_bounds.x &&
            x < m_bounds.x + m_bounds.w &&
            y > m_bounds.y &&
            y < m_bounds.y + m_bounds.h)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

class ChildMenuItem : public MenuItem
{

public:
    void (*m_fn)();

    ChildMenuItem(): m_fn(nullptr)
    {
    }

    void OnMouseMove(float x, float y, UILayoutState* uiLayoutState, bool* continueProcessing)
    {
        if (IsInBounds(x, y))
        {
            uiLayoutState->WhichItemMousedOver = this;
            *continueProcessing = false;
        }
        else
        {
            *continueProcessing = true;
        }
    }

    void SetWidth(float w)
    {
        m_bounds.w = w;
    }
};

class TopLevelMenuItem : public MenuItem
{
    size_t m_longestLabelLength;
    float m_expandedBoxWidth;

public:

    TopLevelMenuItem() : m_longestLabelLength(0)
    {
    }

    void AddChildMenuItem(std::string label, float x, void (*fn)())
    {
        float y = hackforge::toolbar_height + (static_cast<float>(m_childMenuItems.size()) * hackforge::toolbar_line_height);

        ChildMenuItem c;
        c.SetLabel(label);
        c.SetBounds(x, y, 0, hackforge::toolbar_line_height);
        c.m_fn = fn;
        m_childMenuItems.push_back(c);

        if (label.length() > m_longestLabelLength)
        {
            m_longestLabelLength = label.length();
        }
    }

    void OnMouseMove(float x, float y, UILayoutState* mouseOver, bool* continueProcessing)
    {
        if (IsInBounds(x, y))
        {
            // Mouse is hovering over this top-level heading
            mouseOver->WhichItemMousedOver = this;
            *continueProcessing = false;
            return;
        }

        if (mouseOver->WhichItemExpanded == this)
        {
            // Check if mouse is hovering over a child menu item
            for (size_t i = 0; i < m_childMenuItems.size(); ++i)
            {
                m_childMenuItems[i].OnMouseMove(x, y, mouseOver, continueProcessing);
                if (!(*continueProcessing))
                {
                    return;
                }
            }
        }
    }

    void Render(SDL_Renderer* renderer, UILayoutState const& mouseOver, SDL_Color uiColor)
    {
        RenderLabel(renderer, mouseOver);

        if (mouseOver.WhichItemExpanded != this)
            return;

        // Filled rect for the expanded menu box
        SDL_FRect rect{};
        rect.x = m_bounds.x;
        rect.w = m_expandedBoxWidth;
        rect.y = hackforge::toolbar_height;
        rect.h = hackforge::toolbar_line_height * static_cast<float>(m_childMenuItems.size());
        SDL_SetRenderScale(renderer, 1, 1);
        SDL_SetRenderDrawColor(renderer, uiColor.r, uiColor.g, uiColor.b, 255);
        SDL_RenderFillRect(renderer, &rect);

        // Draw all the child labels
        for (size_t i = 0; i < m_childMenuItems.size(); ++i)
        {
            m_childMenuItems[i].RenderLabel(renderer, mouseOver);
        }
    }

    void FinishLayout(float x)
    {
        m_bounds.x = x;
        m_bounds.y = 0;
        m_bounds.h = hackforge::toolbar_height;
        float labelWidth = m_label.length() * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * hackforge::toolbar_text_scaling;
        m_bounds.w = labelWidth;

        m_expandedBoxWidth = m_longestLabelLength * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * hackforge::toolbar_text_scaling;
        for (size_t i = 0; i < m_childMenuItems.size(); ++i)
        {
            m_childMenuItems[i].SetWidth(m_expandedBoxWidth);
        }
    }

    float GetWidth() const
    {
        return m_bounds.w;
    }

    size_t GetNumChildMenuItems() const { return m_childMenuItems.size(); }

    void SetChildMenuItemCheckedState(size_t index, bool checkedState) { m_childMenuItems[index].SetCheckedState(checkedState); }

private:
    std::vector<ChildMenuItem> m_childMenuItems;
};

class Toolbar
{
    UILayoutState uiLayoutState;
    std::vector<TopLevelMenuItem> m_toolbarItems;

public:
    Toolbar()
    {
        uiLayoutState.WhichItemMousedOver = nullptr;

        float x = 0;
        {
            TopLevelMenuItem file;
            file.SetLabel("File");
            file.AddChildMenuItem("New", x, OnToolbarNew);
            file.AddChildMenuItem("Exit", x, OnToolbarExit);
            file.FinishLayout(x);
            float toolbarWidth = file.GetWidth();
            m_toolbarItems.push_back(file);

            x += toolbarWidth;
            x += hackforge::toolbar_top_level_menu_horizontal_spacing;
        }
        {
            TopLevelMenuItem tool;
            tool.SetLabel("Tool");
            tool.AddChildMenuItem("Stamp", x, OnToolbarSetStampTool);
            tool.AddChildMenuItem("Angle Pen", x, OnToolbarSetAnglePenTool);
            tool.FinishLayout(x);
            float toolbarWidth = tool.GetWidth();
            m_toolbarItems.push_back(tool);
            m_toolbarItems[1].SetChildMenuItemCheckedState(0, true);

            x += toolbarWidth;
            x += hackforge::toolbar_top_level_menu_horizontal_spacing;
        }
        {
            TopLevelMenuItem color;
            color.SetLabel("Color");
            color.AddChildMenuItem("Pen Color", x, OnToolbarSetPenColor);
            color.FinishLayout(x);
            float toolbarWidth = color.GetWidth();
            m_toolbarItems.push_back(color);

            x += toolbarWidth;
            x += hackforge::toolbar_top_level_menu_horizontal_spacing;
        }
        {
            TopLevelMenuItem view;
            view.SetLabel("View");
            view.AddChildMenuItem("UI Color", x, OnToolbarSetUIColor);
            view.FinishLayout(x);
            float toolbarWidth = view.GetWidth();
            m_toolbarItems.push_back(view);

            x += toolbarWidth;
            x += hackforge::toolbar_top_level_menu_horizontal_spacing;
        }
    }

    void OnMouseMove(float x, float y, bool* continueProcessing)
    {
        uiLayoutState.WhichItemMousedOver = nullptr;

        for (size_t i = 0; i < m_toolbarItems.size(); ++i)
        {
            m_toolbarItems[i].OnMouseMove(x, y, &uiLayoutState, continueProcessing);
            if (!(*continueProcessing))
                return;
        }
    }

    void OnMouseClick(bool* continueProcessing)
    {
        TopLevelMenuItem* topLevelSelected = dynamic_cast<TopLevelMenuItem*>(uiLayoutState.WhichItemMousedOver);
        if (topLevelSelected)
        {
            // User clicked a menu heading. Toggle it
            if (topLevelSelected == uiLayoutState.WhichItemExpanded)
            {
                uiLayoutState.WhichItemExpanded = nullptr;
            }
            else
            {
                uiLayoutState.WhichItemExpanded = topLevelSelected;
            }
            *continueProcessing = false;
            return;
        }

        ChildMenuItem* childItemSelected = dynamic_cast<ChildMenuItem*>(uiLayoutState.WhichItemMousedOver);
        if (childItemSelected)
        {
            // User clicked a child item. Run the callback and close the menu
            childItemSelected->m_fn();
            uiLayoutState.WhichItemExpanded = nullptr;
            *continueProcessing = false;
            return;
        }

        if (uiLayoutState.WhichItemExpanded != nullptr)
        {
            // If the user clicked back onto the canvas, then automatically collapse the toolbar.
            uiLayoutState.WhichItemExpanded = nullptr;
            *continueProcessing = false;
        }
    }

    void Render(SDL_Renderer* renderer, SDL_Color uiColor)
    {
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
        for (size_t i = 0; i < m_toolbarItems.size(); ++i)
        {
            m_toolbarItems[i].Render(renderer, uiLayoutState, uiColor);
        }
    }

    void SetChildMenuItemCheckedState(size_t toolbarIndex, size_t childMenuItemIndex, bool checkedState) 
    { 
        m_toolbarItems[toolbarIndex].SetChildMenuItemCheckedState(childMenuItemIndex, checkedState);
    }
};