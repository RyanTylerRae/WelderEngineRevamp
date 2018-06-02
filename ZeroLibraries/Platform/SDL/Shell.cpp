///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
// Notes:
//  - We do not handle mOnFrozenUpdate (Zero may freeze on window drags)

//----------------------------------------------------------------------Shell
struct ShellPrivateData
{
};

static const char* cShellWindow = "ShellWindow";

extern SDL_Cursor* gSDLCursors[];

// In SDL 'global' is synonymous with 'monitor' space and 'relative' means 'client' space.

Keys::Enum SDLScancodeToKey(SDL_Scancode code)
{
  switch (code)
  {
#define ProcessInput(Scancode, Keycode, ZeroValue) case Scancode: return ZeroValue;
#include "Keys.inl"
#undef ProcessInput
  }
  return Keys::Unknown;
}

SDL_Scancode KeyToSDLScancode(Keys::Enum key)
{
  switch (key)
  {
#define ProcessInput(Scancode, Keycode, ZeroValue) case ZeroValue: return Scancode;
#include "Keys.inl"
#undef ProcessInput
  }
  return SDL_SCANCODE_UNKNOWN;
}

Keys::Enum SDLKeycodeToKey(SDL_Keycode code)
{
  switch (code)
  {
#define ProcessInput(Scancode, Keycode, ZeroValue) case Keycode: return ZeroValue;
#include "Keys.inl"
#undef ProcessInput
  }
  return Keys::Unknown;
}

SDL_Keycode KeyToSDLKeycode(Keys::Enum key)
{
  switch (key)
  {
#define ProcessInput(Scancode, Keycode, ZeroValue) case ZeroValue: return Keycode;
#include "Keys.inl"
#undef ProcessInput
  }
  return SDLK_UNKNOWN;
}

MouseButtons::Enum SDLToMouseButton(int button)
{
  switch (button)
  {
#define ProcessInput(VKValue, ZeroValue) case VKValue: return ZeroValue;
#include "MouseButtons.inl"
#undef ProcessInput
  }
  return MouseButtons::None;
}

int MouseButtonToSDL(MouseButtons::Enum button)
{
  switch (button)
  {
#define ProcessInput(VKValue, ZeroValue) case ZeroValue: return VKValue;
#include "MouseButtons.inl"
#undef ProcessInput
  }
  return 0;
}

Shell::Shell() :
  mCursor(Cursor::Arrow),
  mMainWindow(nullptr),
  mUserData(nullptr)
{
  ZeroConstructPrivateData(ShellPrivateData);
}

Shell::~Shell()
{
  while (!mWindows.Empty())
    delete mWindows.Front();

  ZeroDestructPrivateData(ShellPrivateData);
}

String Shell::GetOsName()
{
  static const String name(SDL_GetPlatform());
  return name;
}

uint Shell::GetScrollLineCount()
{
  // Pick a good default since SDL has no query for this.
  return 3;
}

IntRect Shell::GetPrimaryMonitorRectangle()
{
  SDL_DisplayMode mode;
  if (SDL_GetCurrentDisplayMode(0, &mode) == 0)
  {
    // The display mode doesn't return the monitor coordinates, however we're going to
    // assume that these coordinates are 0, 0 since this is the primary monitor.
    return IntRect(0, 0, mode.w, mode.h);
  }

  // Return a default monitor size since we failed.
  return IntRect(0, 0, 1024, 768);
}

IntVec2 Shell::GetPrimaryMonitorSize()
{
  IntRect rect = GetPrimaryMonitorRectangle();
  return IntVec2(rect.X, rect.Y);
}

ByteColor Shell::GetColorAtMouse()
{
  // We can either attempt to use SDL_RenderReadPixels or the Renderer API to
  // grab this color this won't work for the rest of the desktop, however.
  return 0;
}

void Shell::SetMonitorCursorClip(const IntRect& monitorRectangle)
{
}

void Shell::ClearMonitorCursorClip()
{
}

Cursor::Enum Shell::GetMouseCursor()
{
  return mCursor;
}

void Shell::SetMonitorCursorPosition(Math::IntVec2Param monitorPosition)
{
  SDL_WarpMouseGlobal(monitorPosition.x, monitorPosition.y);
}

Math::IntVec2 Shell::GetMonitorCursorPosition()
{
  IntVec2 result;
  SDL_GetGlobalMouseState(&result.x, &result.y);
  return result;
}

bool Shell::IsKeyDown(Keys::Enum key)
{
  int numKeys = 0;
  const Uint8* keys = SDL_GetKeyboardState(&numKeys);
  SDL_Scancode code = KeyToSDLScancode(key);
  return keys[code] != 0;
}

bool Shell::IsMouseDown(MouseButtons::Enum button)
{
  Error("Not implemented");
  return false;
}

void Shell::SetMouseCursor(Cursor::Enum cursor)
{
  ZeroGetPrivateData(ShellPrivateData);
  mCursor = cursor;

  SDL_SystemCursor sdlSystemCursor = SDL_SYSTEM_CURSOR_ARROW;

  switch (cursor)
  {
  case Cursor::Arrow:     sdlSystemCursor = SDL_SYSTEM_CURSOR_ARROW; break;
  case Cursor::Wait:      sdlSystemCursor = SDL_SYSTEM_CURSOR_WAIT; break;
  case Cursor::Cross:     sdlSystemCursor = SDL_SYSTEM_CURSOR_CROSSHAIR; break;
  case Cursor::SizeNWSE:  sdlSystemCursor = SDL_SYSTEM_CURSOR_SIZENWSE; break;
  case Cursor::SizeNESW:  sdlSystemCursor = SDL_SYSTEM_CURSOR_SIZENESW; break;
  case Cursor::SizeWE:    sdlSystemCursor = SDL_SYSTEM_CURSOR_SIZEWE; break;
  case Cursor::SizeNS:    sdlSystemCursor = SDL_SYSTEM_CURSOR_SIZENS; break;
  case Cursor::SizeAll:   sdlSystemCursor = SDL_SYSTEM_CURSOR_SIZEALL; break;
  case Cursor::TextBeam:  sdlSystemCursor = SDL_SYSTEM_CURSOR_IBEAM; break;
  case Cursor::Hand:      sdlSystemCursor = SDL_SYSTEM_CURSOR_HAND; break;
  case Cursor::Invisible: sdlSystemCursor = SDL_SYSTEM_CURSOR_ARROW; Error("Not implemented"); break;
  }

  SDL_Cursor*& sdlCursor = gSDLCursors[sdlSystemCursor];
  SDL_SetCursor(sdlCursor);
}

ShellWindow* Shell::FindWindowAt(Math::IntVec2Param monitorPosition)
{
  // Loop through all the windows and do rectangle checks?
  Error("Not implemented");
  return nullptr;
}

bool Shell::IsClipboardText()
{
  return SDL_HasClipboardText() == SDL_TRUE;
}

String Shell::GetClipboardText()
{
  return SDL_GetClipboardText();
}

void Shell::SetClipboardText(StringParam text)
{
  SDL_SetClipboardText(text.c_str());
}

bool Shell::IsClipboardImage()
{
  // SDL has no way of grabbing images from the clipboard.
  return false;
}

bool Shell::GetClipboardImage(Image* image)
{
  // SDL has no way of grabbing images from the clipboard.
  return false;
}

bool Shell::GetPrimaryMonitorImage(Image* image)
{
  // SDL cannot take a screen-shot of the entire monitor.
  // We could attempt to grab the renderer and just take a screenshot of the engine, but not the monitor...
  return false;
}

bool Shell::SupportsFileDialogs()
{
  return false;
}

bool Shell::OpenFile(FileDialogInfo& config)
{
  // SDL has no open file dialog. We should revert to using our own custom dialog that uses the file system API.
  return false;
}

bool Shell::SaveFile(FileDialogInfo& config)
{
  // SDL has no open file dialog. We should revert to using our own custom dialog that uses the file system API.
  return false;
}

void Shell::ShowMessageBox(StringParam title, StringParam message)
{
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title.c_str(), message.c_str(), nullptr);
}

ShellWindow* GetShellWindowFromSDLId(Uint32 id)
{
  SDL_Window* sdlWindow = SDL_GetWindowFromID(id);
  if (!sdlWindow)
    return nullptr;

  ShellWindow* window = (ShellWindow*)SDL_GetWindowData(sdlWindow, cShellWindow);
  return window;
}

PlatformInputDevice* PlatformInputDeviceFromSDL(SDL_JoystickID id)
{
  Error("Not implemented");
  return nullptr;
}

void Shell::Update()
{
  ShellWindow* main = mMainWindow;

  SDL_Event e;
  while (SDL_PollEvent(&e))
  {
    switch (e.type)
    {
      case SDL_QUIT:
      {
        if (main && main->mOnClose)
          main->mOnClose(main);
        break;
      }

      case SDL_WINDOWEVENT:
      {
        ShellWindow* window = GetShellWindowFromSDLId(e.window.windowID);
        if (!window)
          break;

        switch (e.window.event)
        {
          case SDL_WINDOWEVENT_FOCUS_GAINED:
          case SDL_WINDOWEVENT_FOCUS_LOST:

            if (window->mOnFocusChanged)
              window->mOnFocusChanged(e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED, window);
            break;

          case SDL_WINDOWEVENT_SIZE_CHANGED:
            if (window->mOnClientSizeChanged)
              window->mOnClientSizeChanged(IntVec2(e.window.data1, e.window.data2), window);
            break;

          case SDL_WINDOWEVENT_MINIMIZED:
            if (window->mOnMinimized)
              window->mOnMinimized(window);
            break;

          case SDL_WINDOWEVENT_RESTORED:
            if (window->mOnRestored)
              window->mOnRestored(window);
            break;
        }
        break;
      }

      case SDL_DROPFILE:
      {
        ShellWindow* window = GetShellWindowFromSDLId(e.drop.windowID);

        if (window && window->mOnMouseDropFiles)
        {
          IntVec2 clientPosition = IntVec2::cZero;
          SDL_GetMouseState(&clientPosition.x, &clientPosition.y);

          Array<String> files(ZeroInit, e.drop.file);
          window->mOnMouseDropFiles(clientPosition, files, window);
        }
        break;
      }

      case SDL_TEXTINPUT:
      {
        ShellWindow* window = GetShellWindowFromSDLId(e.text.windowID);
        if (window && window->mOnTextTyped)
        {
          String text = e.text.text;
          forRange(Rune rune, text)
            window->mOnTextTyped(rune, window);
        }
        break;
      }

      case SDL_KEYDOWN:
      {
        ShellWindow* window = GetShellWindowFromSDLId(e.key.windowID);
        if (window && window->mOnKeyDown)
          window->mOnKeyDown(SDLKeycodeToKey(e.key.keysym.sym), e.key.keysym.scancode, e.key.repeat != 0, window);
        break;
      }
      case SDL_KEYUP:
      {
        ShellWindow* window = GetShellWindowFromSDLId(e.key.windowID);
        if (window && window->mOnKeyUp)
          window->mOnKeyUp(SDLKeycodeToKey(e.key.keysym.sym), e.key.keysym.scancode, window);
        break;
      }
      case SDL_MOUSEBUTTONDOWN:
      {
        ShellWindow* window = GetShellWindowFromSDLId(e.button.windowID);
        if (window && window->mOnMouseDown)
          window->mOnMouseDown(IntVec2(e.button.x, e.button.y), SDLToMouseButton(e.button.button), window);
        break;
      }
      case SDL_MOUSEBUTTONUP:
      {
        ShellWindow* window = GetShellWindowFromSDLId(e.button.windowID);
        if (window && window->mOnMouseUp)
          window->mOnMouseUp(IntVec2(e.button.x, e.button.y), SDLToMouseButton(e.button.button), window);
        break;
      }
      case SDL_MOUSEMOTION:
      {
        ShellWindow* window = GetShellWindowFromSDLId(e.motion.windowID);
        if (window)
        {
          if (window->mOnMouseMove)
            window->mOnMouseMove(IntVec2(e.motion.x, e.motion.y), window);
          if (window->mOnRawMouseChanged)
            window->mOnRawMouseChanged(IntVec2(e.motion.xrel, e.motion.yrel), window);
        }
        break;
      }

      case SDL_MOUSEWHEEL:
      {
        IntVec2 clientPosition = IntVec2::cZero;
        SDL_GetMouseState(&clientPosition.x, &clientPosition.y);

        // May need to handle SDL_MOUSEWHEEL_FLIPPED here
        ShellWindow* window = GetShellWindowFromSDLId(e.wheel.windowID);
        if (window)
        {
          if (e.wheel.x && window->mOnMouseScrollX)
            window->mOnMouseScrollX(clientPosition, (float)e.wheel.x, window);
          if (e.wheel.y && window->mOnMouseScrollY)
            window->mOnMouseScrollY(clientPosition, (float)e.wheel.y, window);
        }
        break;
      }

      case SDL_JOYDEVICEADDED:
      case SDL_JOYDEVICEREMOVED:
      case SDL_AUDIODEVICEADDED:
      case SDL_AUDIODEVICEREMOVED:
      {
        if (main && main->mOnDevicesChanged)
          main->mOnDevicesChanged(main);
        break;
      }

      case SDL_JOYAXISMOTION:
      {
        Error("Not implemented");
        PlatformInputDevice* device = PlatformInputDeviceFromSDL(e.jaxis.which);
        if (device && main && main->mOnInputDeviceChanged)
          main->mOnInputDeviceChanged(*device, 0, Array<uint>(), DataBlock(), main);
        break;
      }
    }
  }
}

const Array<PlatformInputDevice>& Shell::ScanInputDevices()
{
  mInputDevices.Clear();

  return mInputDevices;
}

//----------------------------------------------------------------ShellWindow
ShellWindow::ShellWindow(
  Shell* shell,
  StringParam windowName,
  Math::IntVec2Param clientSize,
  Math::IntVec2Param monitorClientPos,
  ShellWindow* parentWindow,
  WindowStyleFlags::Enum flags) :
  mShell(shell),
  mMinClientSize(IntVec2(10, 10)),
  mParent(parentWindow),
  mHandle(nullptr),
  mStyle(flags),
  mProgress(0),
  mClientSize(clientSize),
  mClientMousePosition(IntVec2(-1, -1)),
  mCapture(false),
  mUserData(nullptr),
  mOnClose(nullptr),
  mOnFocusChanged(nullptr),
  mOnMouseDropFiles(nullptr),
  mOnFrozenUpdate(nullptr),
  mOnClientSizeChanged(nullptr),
  mOnMinimized(nullptr),
  mOnRestored(nullptr),
  mOnTextTyped(nullptr),
  mOnKeyDown(nullptr),
  mOnKeyUp(nullptr),
  mOnMouseDown(nullptr),
  mOnMouseUp(nullptr),
  mOnMouseMove(nullptr),
  mOnMouseScrollY(nullptr),
  mOnMouseScrollX(nullptr),
  mOnDevicesChanged(nullptr),
  mOnRawMouseChanged(nullptr),
  mOnInputDeviceChanged(nullptr)
{
  Uint32 sdlFlags = SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_OPENGL;
  if (flags & WindowStyleFlags::NotVisible)
    sdlFlags |= SDL_WINDOW_HIDDEN;
  if (flags & WindowStyleFlags::Resizable)
    sdlFlags |= SDL_WINDOW_RESIZABLE;
  if (flags & WindowStyleFlags::ClientOnly)
    sdlFlags |= SDL_WINDOW_BORDERLESS;

  // ??
  if (!(flags & WindowStyleFlags::TitleBar))
    sdlFlags |= SDL_WINDOW_UTILITY;
  if (!(flags & WindowStyleFlags::Close))
    sdlFlags |= SDL_WINDOW_TOOLTIP;

  if (!(flags & WindowStyleFlags::OnTaskBar))
    sdlFlags |= SDL_WINDOW_SKIP_TASKBAR;

  // Is the width and height in client space? SDL doesn't say...
  SDL_Window* sdlWindow = SDL_CreateWindow(
    windowName.c_str(),
    monitorClientPos.x,
    monitorClientPos.y,
    clientSize.x,
    clientSize.y,
    sdlFlags);
  mHandle = sdlWindow;

  SDL_SetWindowData(sdlWindow, cShellWindow, this);

  if (WindowStyleFlags::MainWindow & flags)
  {
    ErrorIf(shell->mMainWindow != nullptr, "Another main window already exists");
    shell->mMainWindow = this;
  }
  shell->mWindows.PushBack(this);
}

ShellWindow::~ShellWindow()
{
  Destroy();
}

void ShellWindow::Destroy()
{
  if (!mHandle)
    return;

  if (mShell && mShell->mMainWindow == this)
    mShell->mMainWindow = nullptr;

  mShell->mWindows.EraseValue(this);

  SDL_DestroyWindow((SDL_Window*)mHandle);

  mHandle = nullptr;
}

IntRect ShellWindow::GetMonitorClientRectangle()
{
  IntRect monitorClientRectangle;
  SDL_GetWindowSize((SDL_Window*)mHandle, &monitorClientRectangle.SizeX, &monitorClientRectangle.SizeY);
  SDL_GetWindowPosition((SDL_Window*)mHandle, &monitorClientRectangle.X, &monitorClientRectangle.Y);
  return monitorClientRectangle;
}

void ShellWindow::SetMonitorClientRectangle(const IntRect& monitorRectangle)
{
  SDL_SetWindowPosition((SDL_Window*)mHandle, monitorRectangle.X, monitorRectangle.Y);
  SDL_SetWindowSize((SDL_Window*)mHandle, monitorRectangle.SizeX, monitorRectangle.SizeY);
  mClientSize = monitorRectangle.Size();
}

IntRect ShellWindow::GetMonitorBorderedRectangle()
{
  int top = 0;
  int left = 0;
  int bottom = 0;
  int right = 0;

  SDL_GetWindowBordersSize((SDL_Window*)mHandle, &top, &left, &bottom, &right);

  IntRect monitorBorderedRectangle = GetMonitorClientRectangle();

  monitorBorderedRectangle.X -= left;
  monitorBorderedRectangle.Y -= top;

  monitorBorderedRectangle.SizeX += right;
  monitorBorderedRectangle.SizeY += bottom;

  return monitorBorderedRectangle;
}

void ShellWindow::SetMonitorBorderedRectangle(const IntRect& monitorBorderedRectangle)
{
  int top = 0;
  int left = 0;
  int bottom = 0;
  int right = 0;

  SDL_GetWindowBordersSize((SDL_Window*)mHandle, &top, &left, &bottom, &right);

  IntRect monitorClientRectangle = monitorBorderedRectangle;

  monitorClientRectangle.X += left;
  monitorClientRectangle.Y += top;

  monitorClientRectangle.SizeX -= right;
  monitorClientRectangle.SizeY -= bottom;

  SetMonitorClientRectangle(monitorClientRectangle);

  mClientSize = monitorClientRectangle.Size();
}

IntVec2 ShellWindow::GetMinClientSize()
{
  return mMinClientSize;
}

void ShellWindow::SetMinClientSize(Math::IntVec2Param minSize)
{
  SDL_SetWindowMinimumSize((SDL_Window*)mHandle, minSize.x, minSize.y);
  mMinClientSize = minSize;
}

ShellWindow* ShellWindow::GetParent()
{
  return mParent;
}

IntVec2 ShellWindow::MonitorToClient(Math::IntVec2Param monitorPosition)
{
  return monitorPosition - GetMonitorClientPosition();
}

IntVec2 ShellWindow::MonitorToBordered(Math::IntVec2Param monitorPosition)
{
  return ClientToBordered(MonitorToClient(monitorPosition));
}

IntVec2 ShellWindow::ClientToMonitor(Math::IntVec2Param clientPosition)
{
  return clientPosition + GetMonitorClientPosition();
}

IntVec2 ShellWindow::ClientToBordered(Math::IntVec2Param clientPosition)
{
  int top = 0;
  int left = 0;

  SDL_GetWindowBordersSize((SDL_Window*)mHandle, &top, &left, nullptr, nullptr);

  return IntVec2(clientPosition.x + left, clientPosition.y + top);
}

IntVec2 ShellWindow::BorderedToMonitor(Math::IntVec2Param borderedPosition)
{
  return ClientToMonitor(BorderedToClient(borderedPosition));
}

IntVec2 ShellWindow::BorderedToClient(Math::IntVec2Param borderedPosition)
{
  int top = 0;
  int left = 0;

  SDL_GetWindowBordersSize((SDL_Window*)mHandle, &top, &left, nullptr, nullptr);

  return IntVec2(borderedPosition.x - left, borderedPosition.y - top);
}

WindowStyleFlags::Enum ShellWindow::GetStyle()
{
  return mStyle.Field;
}

void ShellWindow::SetStyle(WindowStyleFlags::Enum style)
{
  mStyle = style;
  Error("Not implemented");
}

bool ShellWindow::GetVisible()
{
  return (SDL_GetWindowFlags((SDL_Window*)mHandle) & SDL_WINDOW_SHOWN) != 0;
}

void ShellWindow::SetVisible(bool visible)
{
  if (visible)
    SDL_ShowWindow((SDL_Window*)mHandle);
  else
    SDL_HideWindow((SDL_Window*)mHandle);
}

String ShellWindow::GetTitle()
{
  return SDL_GetWindowTitle((SDL_Window*)mHandle);
}

void ShellWindow::SetTitle(StringParam title)
{
  SDL_SetWindowTitle((SDL_Window*)mHandle, title.c_str());
}

WindowState::Enum ShellWindow::GetState()
{
  Uint32 flags = SDL_GetWindowFlags((SDL_Window*)mHandle);

  // Restore is never returned.
  if (flags & SDL_WINDOW_MINIMIZED)
    return WindowState::Minimized;
  if (flags & SDL_WINDOW_MAXIMIZED)
    return WindowState::Maximized;
  if (flags & SDL_WINDOW_FULLSCREEN || flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
    return WindowState::Fullscreen;

  return WindowState::Windowed;
}

void ShellWindow::SetState(WindowState::Enum windowState)
{
  switch (windowState)
  {
    case WindowState::Minimized:
    {
      SDL_MinimizeWindow((SDL_Window*)mHandle);
      break;
    }

    case WindowState::Windowed:
    {
      // Not sure if this is correct...
      SDL_SetWindowFullscreen((SDL_Window*)mHandle, 0);
      break;
    }

    case WindowState::Maximized:
    {
      SDL_MaximizeWindow((SDL_Window*)mHandle);
      break;
    }

    case WindowState::Fullscreen:
    {
      SDL_SetWindowFullscreen((SDL_Window*)mHandle, SDL_WINDOW_FULLSCREEN_DESKTOP);
      break;
    }

    case WindowState::Restore:
    {
      SDL_RestoreWindow((SDL_Window*)mHandle);
      break;
    }
  }
}

void ShellWindow::SetMouseCapture(bool capture)
{
  SDL_CaptureMouse((SDL_bool)capture);
  mCapture = capture;
}

bool ShellWindow::GetMouseCapture()
{
  return mCapture;
}

void ShellWindow::TakeFocus()
{
  SDL_RaiseWindow((SDL_Window*)mHandle);
}

bool ShellWindow::HasFocus()
{
  return SDL_GetKeyboardFocus() == mHandle;
}

bool ShellWindow::GetImage(Image* image)
{
  Error("Not implemented");
  return false;
}

void ShellWindow::Close()
{
  SDL_Event e;
  memset(&e, 0, sizeof(e));
  e.type = SDL_WINDOWEVENT;
  e.window.timestamp = SDL_GetTicks();
  e.window.event = SDL_WINDOWEVENT_CLOSE;
  e.window.windowID = SDL_GetWindowID((SDL_Window*)mHandle);
  SDL_PushEvent(&e);
}

void ShellWindow::ManipulateWindow(WindowBorderArea::Enum area)
{
  Error("Not implemented");
}

float ShellWindow::GetProgress()
{
  return mProgress;
}

void ShellWindow::SetProgress(ProgressType::Enum progressType, float progress)
{
  mProgress = progress;
}

void ShellWindow::PlatformSpecificFixup()
{
  // SDL doesn't need anything special here.
}

}//namespace Zero
