#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <string>
#include "win32_colorpicker.h"
#include "Toolbar.hpp"
#include "AnglePen.hpp"
#include "Bucket.hpp"
#include "LayoutConstants.hpp"
#include <Windows.h>
#include <Shlobj.h>

enum class Tool
{
    Stamp,
    AnglePen,
    PaintBucket
};

namespace hackforge {
    static SDL_Window* window = nullptr;
    static SDL_Renderer* renderer = nullptr;
    SDL_Texture* canvas = nullptr;

    static float currentPenX = 0;
    static float currentPenY = 0;
    static float previousPenX = 0;
    static float previousPenY = 0;
    static bool penDown = false;
    static bool shouldExit = false;
    static bool shouldClear = false;

    // --- State variables for the Custom Tooling colors ---
    static SDL_Color penColor = { 255, 255, 255, 255 };   // Default Pen: White
    static SDL_Color buttonColor = { 100, 100, 100, 255 }; // Default Button Background: Dark Gray

    Toolbar toolbar;
    Tool currentTool;
    AnglePen anglePen;


    class Marker
    {
        std::vector<int> m_indices;
        std::vector<SDL_Vertex> m_vertices;

        struct PressedState
        {
            float X1;
            float Y1;
            bool Valid;
        } m_pressedState;

        static SDL_FColor OpaqueUnormColorToOpaqueFloatColor(SDL_Color c)
        {
            SDL_FColor r;
            r.a = 1.0f;
            r.r = static_cast<float>(c.r) / 255.0f;
            r.g = static_cast<float>(c.g) / 255.0f;
            r.b = static_cast<float>(c.b) / 255.0f;
            return r;
        }

    public:

        void EnsureVerticesAndIndices()
        {
            if (m_indices.size() > 0)
                return;

            m_indices.push_back(0);
            m_indices.push_back(1);
            m_indices.push_back(2);
            m_indices.push_back(2);
            m_indices.push_back(1);
            m_indices.push_back(3);

            for (int i = 0; i < 4; ++i)
            {
                SDL_Vertex v{};
                v.color.a = 1.0f;
                v.color.r = 1.0f;
                m_vertices.push_back(v);
            }
        }

        void Render(SDL_Renderer* renderer, float x, float y, SDL_Color const& penColor, bool penDown)
        {
            EnsureVerticesAndIndices();

            if (penDown)
            {
                if (!m_pressedState.Valid)
                {
                    m_pressedState.X1 = x;
                    m_pressedState.Y1 = y;
                    m_vertices[2].position.x = x;
                    m_vertices[2].position.y = y;
                    m_vertices[3].position.x = x;
                    m_vertices[3].position.y = y;
                    m_pressedState.Valid = true;
                    return;
                }
            }
            else
            {
                m_pressedState.Valid = false;
                return;
            }

            float dx = x - m_pressedState.X1;
            float dy = y - m_pressedState.Y1;
            float length = sqrtf(dx * dx + dy * dy);

            m_pressedState.X1 = x;
            m_pressedState.Y1 = y;

            if (length == 0.0f) return;

            // Calculate perpendicular vector for thickness offset
            float thickness = 40.0f;

            float nx = -dy / length * (thickness / 2.0f);
            float ny = dx / length * (thickness / 2.0f);

            SDL_Vertex previousLeft = m_vertices[2];
            SDL_Vertex previousRight = m_vertices[3];

            m_vertices[0] = previousLeft;
            m_vertices[1] = previousRight;

            m_vertices[2].position.x = x + nx;
            m_vertices[2].position.y = y + ny;

            m_vertices[3].position.x = x - nx;
            m_vertices[3].position.y = y - ny;

            SDL_RenderGeometry(renderer, nullptr, m_vertices.data(), m_vertices.size(), m_indices.data(), m_indices.size());

        }
    };
    Marker marker;

} // namespace hackforge

void NewDocument()
{
    hackforge::shouldClear = true;
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    /* Create the window and renderer */
    if (!SDL_CreateWindowAndRenderer("Paint", hackforge::window_width, hackforge::window_height,
        0, &hackforge::window, &hackforge::renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Initialize the canvas target background color to Black once at startup
    NewDocument();

    SDL_PixelFormat pixel_format = SDL_GetWindowPixelFormat(hackforge::window);
    hackforge::canvas = SDL_CreateTexture(hackforge::renderer, pixel_format, SDL_TEXTUREACCESS_TARGET, 800, 600);

    hackforge::currentTool = Tool::Stamp;

    return SDL_APP_CONTINUE;
}

void OnMouseMove(SDL_Event* event)
{
    bool shouldContinueProcessingMouse = true;
    hackforge::toolbar.OnMouseMove(event->motion.x, event->motion.y, &shouldContinueProcessingMouse);
    if (!shouldContinueProcessingMouse)
        return;

    hackforge::previousPenX = hackforge::currentPenX;
    hackforge::previousPenY = hackforge::currentPenY;

    hackforge::currentPenX = event->motion.x;
    hackforge::currentPenY = event->motion.y;
}

void OnMouseLeftClick(SDL_Event* event)
{
    bool shouldContinueProcessingClicks = true;
    hackforge::toolbar.OnMouseClick(&shouldContinueProcessingClicks);
    if (!shouldContinueProcessingClicks)
        return;

    float mx = event->button.x;
    float my = event->button.y;

    hackforge::penDown = true;
}

void ClearImpl()
{
    SDL_SetRenderTarget(hackforge::renderer, hackforge::canvas);
    SDL_SetRenderDrawColor(hackforge::renderer, 0, 0, 0, 255);
    SDL_RenderClear(hackforge::renderer);

    // Draw original project demo test message (Centered)
    {
        const std::string message = "Hello, Hackforge Paint!";
        int w = 0, h = 0;
        float x, y;
        const float scale = 4.0f;

        SDL_GetRenderOutputSize(hackforge::renderer, &w, &h);
        SDL_SetRenderScale(hackforge::renderer, scale, scale);
        x = ((w / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * message.size()) / 2;
        y = ((h / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2;

        SDL_SetRenderDrawColor(hackforge::renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(hackforge::renderer, x, y, message.c_str());
    }
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    else if (event->type == SDL_EVENT_MOUSE_MOTION)
    {
        OnMouseMove(event);
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        if (event->button.button == SDL_BUTTON_LEFT) {

            OnMouseLeftClick(event);
        }
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        if (event->button.button == SDL_BUTTON_LEFT) {
            hackforge::penDown = false;
        }
    }
    return SDL_APP_CONTINUE;
}


/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    if (hackforge::shouldExit)
        return SDL_APP_SUCCESS;

    if (hackforge::shouldClear)
    {
        ClearImpl();
        hackforge::shouldClear = false;
    }

    // --- 1. Canvas Drawing Pass ---
    SDL_SetRenderTarget(hackforge::renderer, hackforge::canvas);

    if (hackforge::currentTool == Tool::Stamp)
    {
        // Use dynamic active pen color chosen from picker
        SDL_SetRenderScale(hackforge::renderer, 1.0f, 1.0f);
        SDL_SetRenderDrawColor(hackforge::renderer, hackforge::penColor.r, hackforge::penColor.g, hackforge::penColor.b, hackforge::penColor.a);
        hackforge::marker.Render(hackforge::renderer, hackforge::currentPenX, hackforge::currentPenY, hackforge::penColor, hackforge::penDown);

    }
    else if (hackforge::currentTool == Tool::AnglePen)
    {
        if (hackforge::penDown)
        {
            SDL_SetRenderScale(hackforge::renderer, 1.0f, 1.0f);
            hackforge::anglePen.Render(hackforge::renderer, hackforge::previousPenX, hackforge::previousPenY, hackforge::currentPenX, hackforge::currentPenY, hackforge::penColor);
        }
    }
    else if (hackforge::currentTool == Tool::PaintBucket)
    {
        if (hackforge::penDown)
        {
            SDL_SetRenderScale(hackforge::renderer, 1.0f, 1.0f);
            RenderBucket(hackforge::renderer, hackforge::window_width, hackforge::window_height, hackforge::canvas, hackforge::penColor, hackforge::currentPenX, hackforge::currentPenY);
            hackforge::penDown = false;
        }
    }

    // --- 2. UI Layout Render Pass (Drawn over the canvas) ---
    SDL_SetRenderTarget(hackforge::renderer, nullptr); // Sets the window as the target
    SDL_SetRenderScale(hackforge::renderer, 1.0f, 1.0f);
    SDL_RenderTexture(hackforge::renderer, hackforge::canvas, NULL, NULL);
    hackforge::toolbar.Render(hackforge::renderer, hackforge::buttonColor);

    SDL_RenderPresent(hackforge::renderer);
    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    // SDL3 automates window and renderer cleanup inside standard callback shutdown hooks
}

void OnToolbarNew()
{
    NewDocument();
}

void OnToolbarSaveAs()
{
    SDL_PropertiesID props = SDL_GetWindowProperties(hackforge::window);
    void* hWndData = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    HWND hWnd = reinterpret_cast<HWND>(hWndData);

    char documentsPath[MAX_PATH];

    if (FAILED((SHGetFolderPathA(NULL,
        CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
        NULL,
        0,
        documentsPath))))
    {
        return;
    }

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    char szFile[MAX_PATH];
    strcpy_s(szFile, "image.png");

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "PNG File\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = documentsPath;
    ofn.lpstrDefExt = "png";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&ofn) == 0)
        return;

    std::string destFilename = ofn.lpstrFile;

    // In SDL, textures are GPU-side, surfaces are CPU-side.
    // We have to copy the GPU-side texture to CPU to save to disk.
    SDL_SetRenderTarget(hackforge::renderer, hackforge::canvas);
    SDL_Rect rect{};
    rect.w = hackforge::window_width;
    rect.h = hackforge::window_height;
    SDL_Surface* surface = SDL_RenderReadPixels(hackforge::renderer, &rect);
    SDL_CreateSurfaceFrom(hackforge::window_width, hackforge::window_height, hackforge::canvas->format, nullptr, 0);
    SDL_SavePNG(surface, destFilename.c_str());
    SDL_DestroySurface(surface);
}

void OnToolbarExit()
{
    hackforge::shouldExit = true;
}

void OnToolbarSetPenColor()
{
    hackforge::penColor = OpenNativeColorPicker(hackforge::window, hackforge::penColor);
}

void OnToolbarSetUIColor()
{
    hackforge::buttonColor = OpenNativeColorPicker(hackforge::window, hackforge::buttonColor);
}

void OnToolbarSetStampTool()
{
    hackforge::currentTool = Tool::Stamp;
    hackforge::toolbar.SetChildMenuItemCheckedState(1, 0, true);
    hackforge::toolbar.SetChildMenuItemCheckedState(1, 1, false);
    hackforge::toolbar.SetChildMenuItemCheckedState(1, 2, false);
}

void OnToolbarSetAnglePenTool()
{
    hackforge::currentTool = Tool::AnglePen;
    hackforge::toolbar.SetChildMenuItemCheckedState(1, 0, false);
    hackforge::toolbar.SetChildMenuItemCheckedState(1, 1, true);
    hackforge::toolbar.SetChildMenuItemCheckedState(1, 2, false);
}

void OnToolbarSetPaintBucketTool()
{
    hackforge::currentTool = Tool::PaintBucket;
    hackforge::toolbar.SetChildMenuItemCheckedState(1, 0, false);
    hackforge::toolbar.SetChildMenuItemCheckedState(1, 1, false);
    hackforge::toolbar.SetChildMenuItemCheckedState(1, 2, true);

}