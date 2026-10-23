#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include "bucket.hpp"

#include <chrono>
#include <string>

#include "colorpicker.hpp"
#include "common.hpp"
#include "dialogbox.hpp"

void NewDocument()
{
    hackforge::dialogs.emplace_back(std::make_unique<SizeInputDialogBox>(
        DialogBoxCommon::DialogType::NewDocument,
        "New Document",
        hackforge::sc_default_canvas_width,
        hackforge::sc_default_canvas_height));
}

void AllocateScreenDoorTexture(SDL_Renderer* renderer) 
{
    // Create a tiny 2x2 texture with Alpha support
    hackforge::screenDoor = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 2, 2);

    // Enable alpha blending so the screen underneath shows through
    SDL_SetTextureBlendMode(hackforge::screenDoor, SDL_BLENDMODE_BLEND);

    const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32);
    Uint32 clear = SDL_MapRGBA(details, NULL, 0, 0, 0, 0x00);
    Uint32 translucent = SDL_MapRGBA(details, NULL, 0, 0, 0, 0xCC);

    Uint32 pixels[4] = { clear, translucent, translucent, translucent };
    SDL_UpdateTexture(hackforge::screenDoor, NULL, pixels, 2 * sizeof(Uint32));
}

void CreateWindowSizeDependentResources(SDL_Texture* pNewCanvas = nullptr) // new canvas may be externally created
{
    SDL_PixelFormat pixel_format = SDL_GetWindowPixelFormat(hackforge::window);
    if (pNewCanvas)
    {
        hackforge::canvas = pNewCanvas;
    }
    else
    {
        hackforge::canvas = SDL_CreateTexture(hackforge::renderer, pixel_format, SDL_TEXTUREACCESS_TARGET, hackforge::window_width, hackforge::window_height);
    }

    hackforge::undoBuffer = SDL_CreateTexture(hackforge::renderer, pixel_format, SDL_TEXTUREACCESS_TARGET, hackforge::window_width, hackforge::window_height);
    hackforge::scratchBuffer = SDL_CreateTexture(hackforge::renderer, pixel_format, SDL_TEXTUREACCESS_TARGET, hackforge::window_width, hackforge::window_height);
}

void DestroyWindowSizeDependentResources()
{
    SDL_DestroyTexture(hackforge::canvas);
    SDL_DestroyTexture(hackforge::undoBuffer);
    SDL_DestroyTexture(hackforge::scratchBuffer);
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
    hackforge::shouldClear = true;
    hackforge::shouldShowDemoMessageOnClear = true;

    CreateWindowSizeDependentResources();

    AllocateScreenDoorTexture(hackforge::renderer);

    hackforge::OnToolbarSetPencilTool();

    return SDL_APP_CONTINUE;
}

void OnMouseMove(SDL_Event* event)
{
    if (hackforge::dialogs.size() > 0)
    {
        hackforge::dialogs.back()->OnMouseMove(event->motion.x, event->motion.y, event->motion.state & SDL_BUTTON_LEFT);
        return;
    }

    bool shouldContinueProcessingMouse = true;
    hackforge::toolbar.OnMouseMove(event->motion.x, event->motion.y, &shouldContinueProcessingMouse);
    if (!shouldContinueProcessingMouse)
        return;

    hackforge::previousPenX = hackforge::currentPenX;
    hackforge::previousPenY = hackforge::currentPenY;

    hackforge::currentPenX = event->motion.x;
    hackforge::currentPenY = event->motion.y;
}

class SizeInputDialogValidation
{
    std::stringstream m_errorMessage;
    bool m_validWidth = true;
    bool m_validHeight = true;

public:
    SizeInputDialogValidation(int requestedWidth, int requestedHeight)
    {
        if (requestedWidth < hackforge::sc_minimum_canvas_width)
        {
            m_errorMessage << "The requested width is less than the minimum of " << hackforge::sc_minimum_canvas_width;
            m_validWidth = false;
        }
        else if (requestedWidth > hackforge::sc_maximum_canvas_width)
        {
            m_errorMessage << "The requested width exceeds the maximum of " << hackforge::sc_maximum_canvas_width;
            m_validWidth = false;
        }

        if (requestedHeight < hackforge::sc_minimum_canvas_height)
        {
            AddTextBeforeHeightErrorMessage();
            m_errorMessage << "he requested height is less than the minimum of " << hackforge::sc_minimum_canvas_height;
            m_validHeight = false;
        }
        else if (requestedHeight > hackforge::sc_maximum_canvas_height)
        {
            AddTextBeforeHeightErrorMessage();
            m_errorMessage << "he requested height exceeds the maximum of " << hackforge::sc_maximum_canvas_height;
            m_validHeight = false;
        }

        if (!m_validWidth || !m_validHeight)
        {
            m_errorMessage << ".";
        }
    }

    bool IsValid()
    {
        return m_validWidth && m_validHeight;
    }

    std::stringstream const& GetErrorText() const
    {
        return m_errorMessage;
    }

private:

    void AddTextBeforeHeightErrorMessage()
    {
        if (!m_validWidth)
        {
            m_errorMessage << ", and t";
        }
        else
        {
            m_errorMessage << "T";
        }
    }
};

void OnConfirmSetCanvasSizeDialog_NewDocument(int newWidth, int newHeight)
{
    // Validate the requested width and height are acceptable. Pop up an additional dialog box if not.
    SizeInputDialogValidation sidv(newWidth, newHeight);
    if (!sidv.IsValid())
    {
        std::string errorText = sidv.GetErrorText().str();

        hackforge::dialogs.emplace_back(std::make_unique<InfoDialogBox>(
            "Information",
            hackforge::window_width,
            hackforge::window_height,
            errorText.c_str(),
            28));
        return;
    }

    hackforge::dialogs.pop_back(); // Delete the size input dialog

    hackforge::window_width = newWidth;
    hackforge::window_height = newHeight;

    SDL_SetWindowSize(hackforge::window, hackforge::window_width, hackforge::window_height);
    SDL_PixelFormat pixel_format = SDL_GetWindowPixelFormat(hackforge::window);

    DestroyWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    hackforge::shouldClear = true;
    hackforge::shouldShowDemoMessageOnClear = false;
}

void OnConfirmSetCanvasSizeDialog_Resize(int newWidth, int newHeight)
{
    if (newWidth == hackforge::window_width && newHeight == hackforge::window_height)
    {
        // No change, nothing to do
        hackforge::dialogs.pop_back();
        return;
    }

    // Validate the requested width and height are acceptable. Pop up a  dialog box if not.
    SizeInputDialogValidation sidv(newWidth, newHeight);
    if (!sidv.IsValid())
    {
        std::string errorText = sidv.GetErrorText().str();
        hackforge::dialogs.emplace_back(std::make_unique<InfoDialogBox>(
            "Information",
            hackforge::window_width,
            hackforge::window_height,
            errorText.c_str(),
            28));
        return;
    }

    // Can go ahead, user's chosen size works
    hackforge::dialogs.pop_back();

    SDL_SetWindowSize(hackforge::window, newWidth, newHeight);

    SDL_PixelFormat pixel_format = SDL_GetWindowPixelFormat(hackforge::window);
    SDL_Texture* newCanvas = SDL_CreateTexture(hackforge::renderer, pixel_format, SDL_TEXTUREACCESS_TARGET, newWidth, newHeight);

    SDL_SetRenderTarget(hackforge::renderer, newCanvas);
    {
        SDL_FRect dstRect{};
        dstRect.w = static_cast<float>(hackforge::window_width);
        dstRect.h = static_cast<float>(hackforge::window_height);

        SDL_RenderTexture(hackforge::renderer, hackforge::canvas, NULL, &dstRect);
    }
    
    DestroyWindowSizeDependentResources();
    CreateWindowSizeDependentResources(newCanvas);

    if (newWidth < hackforge::window_width || newHeight < hackforge::window_height)
    {
        hackforge::dialogs.emplace_back(std::make_unique<InfoDialogBox>(
            "Information",
            newWidth,
            newHeight,
            "The requested size is smaller than the current size; some clipping has occurred.",
            28));
    }

    hackforge::window_width = newWidth;
    hackforge::window_height = newHeight;
}

void OnConfirmSetPenRadius(int requestedRadius)
{
    if (hackforge::penRadius == requestedRadius)
    {
        // Nothing to do
        hackforge::dialogs.pop_back();
        return;
    }

    if (requestedRadius < 1 || requestedRadius > 200)
    {
        std::string errorText = "Please choose a radius between 1 and 200, inclusive.";
        hackforge::dialogs.emplace_back(std::make_unique<InfoDialogBox>(
            "Information",
            hackforge::window_width,
            hackforge::window_height,
            errorText.c_str(),
            28));
        return;
    }

    // Can go ahead, user's chosen size works
    hackforge::dialogs.pop_back();
    hackforge::penRadius = requestedRadius;
}

void OnConfirmSetPenAngle(int requestedAngle)
{
    if (hackforge::penAngle == requestedAngle)
    {
        // Nothing to do
        hackforge::dialogs.pop_back();
        return;
    }

    if (requestedAngle < 0 || requestedAngle > 180)
    {
        std::string errorText = "Please choose a radius between 0 and 180, inclusive.";
        hackforge::dialogs.emplace_back(std::make_unique<InfoDialogBox>(
            "Information",
            hackforge::window_width,
            hackforge::window_height,
            errorText.c_str(),
            28));
        return;
    }

    // Can go ahead, user's chosen size works
    hackforge::dialogs.pop_back();
    hackforge::penAngle = requestedAngle;
}

void OnTryToCloseFrontmostDialog()
{
    DialogBoxCommon::DialogType type = hackforge::dialogs.back()->GetType();
    if (type == DialogBoxCommon::DialogType::Info)
    {
        hackforge::dialogs.pop_back();
        return;
    }

    DialogResult result = hackforge::dialogs.back()->GetDialogResult();
    if (result == DialogResult::Cancel)
    {
        hackforge::dialogs.pop_back();
        return;
    }

    if (type == DialogBoxCommon::DialogType::SetPenRadius)
    {
        int requestedRadius = dynamic_cast<SingleNumberInputDialogBox*>(hackforge::dialogs.back().get())->GetRequestedValue();
        OnConfirmSetPenRadius(requestedRadius);
    }
    else if (type == DialogBoxCommon::DialogType::SetPenAngle)
    {
        int requestedAngle = dynamic_cast<SingleNumberInputDialogBox*>(hackforge::dialogs.back().get())->GetRequestedValue();
        OnConfirmSetPenAngle(requestedAngle);
    }
    else if (type == DialogBoxCommon::DialogType::SetPenColor)
    {
        hackforge::penColor = dynamic_cast<ColorPickerDialogBox*>(hackforge::dialogs.back().get())->GetRequestedColor();
        hackforge::dialogs.pop_back();

    }
    else if (type == DialogBoxCommon::DialogType::SetUIColor)
    {
        hackforge::buttonColor = dynamic_cast<ColorPickerDialogBox*>(hackforge::dialogs.back().get())->GetRequestedColor();
        hackforge::dialogs.pop_back();
    }
    else if (type == DialogBoxCommon::DialogType::NewDocument)
    {
        SizeInputDialogBox* pSizeInput = dynamic_cast<SizeInputDialogBox*>(hackforge::dialogs.back().get());
        OnConfirmSetCanvasSizeDialog_NewDocument(pSizeInput->GetRequestedCanvasWidth(), pSizeInput->GetRequestedCanvasHeight());
    }
    else if (type == DialogBoxCommon::DialogType::Resize)
    {
        SizeInputDialogBox* pSizeInput = dynamic_cast<SizeInputDialogBox*>(hackforge::dialogs.back().get());
        OnConfirmSetCanvasSizeDialog_Resize(pSizeInput->GetRequestedCanvasWidth(), pSizeInput->GetRequestedCanvasHeight());
    }

}

void OnKeyboardInput(SDL_Event* event)
{
    if (hackforge::dialogs.size() > 0)
    {
        bool tryToCloseDialog = false;
        hackforge::dialogs.back()->OnKeyboardInput(event->key.key, &tryToCloseDialog);

        if (tryToCloseDialog)
        {
            OnTryToCloseFrontmostDialog();
        }
        return;
    }

    // Look for Ctrl+Z to undo
    if (event->key.key == SDLK_Z && event->key.key & SDL_KMOD_CTRL)
    {
        hackforge::doUndo = true;
    }
}

void OnMouseLeftClick(SDL_Event* event)
{
    if (hackforge::dialogs.size() > 0)
    {
        bool tryToCloseDialog = false;
        hackforge::dialogs.back()->OnMouseClick(event->button.x, event->button.y, hackforge::renderer, &tryToCloseDialog);

        if (tryToCloseDialog)
        {
            OnTryToCloseFrontmostDialog();
        }
        return;
    }

    bool shouldContinueProcessingClicks = true;
    hackforge::toolbar.OnMouseClick(&shouldContinueProcessingClicks);
    if (!shouldContinueProcessingClicks)
        return;

    float mx = event->button.x;
    float my = event->button.y;

    if (!hackforge::penDown)
    {
        hackforge::onPenDownChange = true;
    }

    hackforge::penDown = true;
}

void ClearImpl()
{
    SDL_SetRenderTarget(hackforge::renderer, hackforge::canvas);
    SDL_SetRenderDrawColor(hackforge::renderer, 0, 0, 0, 255);
    SDL_RenderClear(hackforge::renderer);

    // Draw original project demo test message (Centered)
    if (hackforge::shouldShowDemoMessageOnClear)
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
    else if (event->type == SDL_EVENT_KEY_UP)
    {
        OnKeyboardInput(event);
    }
    return SDL_APP_CONTINUE;
}

void DrawCopyOfCanvasWithFlipMode(SDL_Renderer* renderer, SDL_Texture* canvas, SDL_FlipMode flipMode)
{
    SDL_PropertiesID props = SDL_GetTextureProperties(canvas);
    SDL_PixelFormat format = (SDL_PixelFormat)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_FORMAT_NUMBER, SDL_PIXELFORMAT_UNKNOWN);

    // Create a new target texture
    SDL_Texture* scratch = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_TARGET, hackforge::window_width, hackforge::window_height);

    // Set properties and copy contents
    SDL_SetRenderTarget(renderer, scratch);
    SDL_RenderTexture(renderer, canvas, NULL, NULL); // Render the original texture onto the new texture

    // Set target to canvas, and draw the copy flipped
    SDL_SetRenderTarget(hackforge::renderer, hackforge::canvas);
    SDL_RenderTextureRotated(
        hackforge::renderer,
        scratch,
        NULL,        // Source rectangle (NULL for entire texture)
        NULL,        // Destination rectangle on screen
        0.0,         // Rotation angle
        NULL,        // Center of rotation (NULL defaults to center)
        flipMode
    );

    SDL_DestroyTexture(scratch);
}

void BackupUndoBuffer()
{
    SDL_SetRenderTarget(hackforge::renderer, hackforge::undoBuffer);
    SDL_RenderClear(hackforge::renderer);
    SDL_RenderTexture(hackforge::renderer, hackforge::canvas, NULL, NULL);
    SDL_SetRenderTarget(hackforge::renderer, hackforge::canvas);
}

void RestoreUndoBuffer()
{
    // Copy canvas to scratch
    SDL_SetRenderTarget(hackforge::renderer, hackforge::scratchBuffer);
    SDL_RenderClear(hackforge::renderer);
    SDL_RenderTexture(hackforge::renderer, hackforge::canvas, NULL, NULL);

    // Copy undo to canvas
    SDL_SetRenderTarget(hackforge::renderer, hackforge::canvas);
    SDL_RenderClear(hackforge::renderer);
    SDL_RenderTexture(hackforge::renderer, hackforge::undoBuffer, NULL, NULL);

    // Copy scratch to undo
    SDL_SetRenderTarget(hackforge::renderer, hackforge::undoBuffer);
    SDL_RenderClear(hackforge::renderer);
    SDL_RenderTexture(hackforge::renderer, hackforge::scratchBuffer, NULL, NULL);

    SDL_SetRenderTarget(hackforge::renderer, hackforge::canvas);
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

    if (hackforge::doQuickSave) {
        
        // make up a filename
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        const auto filename = "hackforge-paint-file-" + std::to_string(ms) + ".png";

        SDL_SetRenderTarget(hackforge::renderer, hackforge::canvas);
        SDL_Surface* surface = SDL_RenderReadPixels(hackforge::renderer, nullptr);
        IMG_SavePNG(surface,filename.c_str());
        SDL_DestroySurface(surface);
        hackforge::doQuickSave = false;

        std::stringstream messageBoxText;
        messageBoxText << "File " << filename << " saved.";

        hackforge::dialogs.emplace_back(std::make_unique<InfoDialogBox>(
            "Information",
            hackforge::window_width,
            hackforge::window_height,
            messageBoxText.str().c_str(),
            28));
    }

    if (hackforge::doUndo)
    {
        RestoreUndoBuffer();
        hackforge::doUndo = false;
    }

    if (hackforge::doHorizontalFlip)
    {
        BackupUndoBuffer();
        DrawCopyOfCanvasWithFlipMode(hackforge::renderer, hackforge::canvas, SDL_FLIP_HORIZONTAL);
        hackforge::doHorizontalFlip = false;
    }

    if (hackforge::doVerticalFlip)
    {
        BackupUndoBuffer();
        DrawCopyOfCanvasWithFlipMode(hackforge::renderer, hackforge::canvas, SDL_FLIP_VERTICAL);
        hackforge::doVerticalFlip = false;
    }

    // --- 1. Canvas Drawing Pass ---
    SDL_SetRenderTarget(hackforge::renderer, hackforge::canvas);

    SDL_SetRenderScale(hackforge::renderer, 1.0f, 1.0f);

    if (hackforge::penDown)
    {
        if (hackforge::currentTool == hackforge::Tool::Pencil)
        {
            if (hackforge::onPenDownChange)
            {
                BackupUndoBuffer();
            }

            SDL_SetRenderDrawColor(hackforge::renderer, hackforge::penColor.r, hackforge::penColor.g, hackforge::penColor.b, hackforge::penColor.a);
            SDL_RenderLine(hackforge::renderer, hackforge::previousPenX, hackforge::previousPenY, hackforge::currentPenX, hackforge::currentPenY);
        }
        else if (hackforge::currentTool == hackforge::Tool::Stamp)
        {
            if (hackforge::onPenDownChange)
            {
                BackupUndoBuffer();
            }

            float radiusF = static_cast<float>(hackforge::penRadius);
            float diameter = radiusF * 2;

            SDL_FRect rect{};
            rect.x = hackforge::currentPenX - radiusF;
            rect.y = hackforge::currentPenY - radiusF;
            rect.w = diameter;
            rect.h = diameter;

            // Use dynamic active pen color chosen from picker
            SDL_SetRenderDrawColor(hackforge::renderer, hackforge::penColor.r, hackforge::penColor.g, hackforge::penColor.b, hackforge::penColor.a);
            SDL_RenderFillRect(hackforge::renderer, &rect);
        }
        else if (hackforge::currentTool == hackforge::Tool::AnglePen)
        {
            if (hackforge::onPenDownChange)
            {
                BackupUndoBuffer();
            }

            hackforge::anglePen.Render(
                hackforge::renderer,
                hackforge::previousPenX,
                hackforge::previousPenY,
                hackforge::currentPenX,
                hackforge::currentPenY,
                hackforge::penColor,
                hackforge::penRadius,
                hackforge::penAngle);
        }
        else if (hackforge::currentTool == hackforge::Tool::PaintBucket)
        {
            BackupUndoBuffer();
            RenderBucket(hackforge::renderer, hackforge::window_width, hackforge::window_height, hackforge::canvas, hackforge::penColor, hackforge::currentPenX, hackforge::currentPenY);
            hackforge::penDown = false;
        }
        else if (hackforge::currentTool == hackforge::Tool::Eyedropper)
        {
            SDL_Rect rect = { (int)hackforge::currentPenX, (int)hackforge::currentPenY, 1, 1 };
            SDL_Surface* temporaryCpuRead = SDL_RenderReadPixels(hackforge::renderer, &rect);
            SDL_Color read{};
            SDL_ReadSurfacePixel(temporaryCpuRead, 0, 0, &read.r, &read.g, &read.b, &read.a);
            SDL_DestroySurface(temporaryCpuRead);
            hackforge::penColor = read;
        }
        else if (hackforge::currentTool == hackforge::Tool::Spraypaint)
        {
            if (hackforge::onPenDownChange)
            {
                BackupUndoBuffer();
            }

            hackforge::spraypaint.Render(
                hackforge::renderer,
                hackforge::currentPenX,
                hackforge::currentPenY,
                hackforge::penColor,
                hackforge::penRadius);
        }
    }

    hackforge::onPenDownChange = false;

    // --- 2. UI Layout Render Pass (Drawn over the canvas) ---
    SDL_SetRenderTarget(hackforge::renderer, nullptr); // Sets the window as the target
    SDL_SetRenderScale(hackforge::renderer, 1.0f, 1.0f);
    SDL_RenderTexture(hackforge::renderer, hackforge::canvas, NULL, NULL);
    hackforge::toolbar.Render(hackforge::renderer, hackforge::buttonColor);

    // --- 3. Draw modal dialogs ---
    // Draw a screen door effect if there's any kind of dialog visible.
    bool drawScreenDoor = hackforge::dialogs.size() > 0;
    if (drawScreenDoor)
    {
        // Show the screen door effect over the background
        SDL_RenderTextureTiled(hackforge::renderer, hackforge::screenDoor, NULL, 1.0f, NULL);
    }

    for (size_t i = 0; i < hackforge::dialogs.size(); ++i)
    {
        hackforge::dialogs[i]->ShowDialog(hackforge::renderer, hackforge::buttonColor);
    }

    SDL_RenderPresent(hackforge::renderer);
    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    // SDL3 automates window and renderer cleanup inside standard callback shutdown hooks
}
