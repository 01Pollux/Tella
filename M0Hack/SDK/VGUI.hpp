#pragma once

#include "Interfaces.hpp"
#include "AppSystem.hpp"
#include "UtlVector.hpp"
#include "MathLib/Vector2D.hpp"
#include "Helper/Color.hpp"
#include "Helper/Offsets.hpp"
#include "Helper/GameProp.hpp"
#include <cstdlib>

class KeyValues;

namespace VGUI
{
	class IPanel;
	class ISurface;
}

namespace Interfaces
{
	extern VGUI::IPanel* VGUIPanel;
	constexpr const char* PanelName = "VGUI_Panel009";

	extern VGUI::ISurface* VGUISurface;
	constexpr const char* SurfaceName = "VGUI_Surface030";
}

namespace VGUI
{
	using VPANEL = uint32_t;
	using HScheme = uint32_t;
	using HFont = uint32_t;
	using HCursor = uint32_t;
	using HTexture = uint32_t;
	using ImageFormat = int32_t;

	class Panel;
	class SurfacePlat;
	class IHTMLEvents;
	class IHTML;
	class IVguiMatInfo;
	class IImage;

	namespace M0SurfaceTools
	{
		constexpr size_t StringOffset = 21;

		namespace Fonts
		{
			extern HFont Arial;
			extern HFont ArialSmall;
			extern HFont ArialLarge;
		}

		namespace Textures
		{
			extern int White;
			extern int Cyan;
			extern int Fuschia;
		}

		void Init();
	}

	struct IntRect
	{
		int x0;
		int y0;
		int x1;
		int y1;
	};

	struct Vertex
	{
		Vertex() = default;
		Vertex(const Vector2D& pos, const Vector2D& coord = Vector2D(0, 0)) : Position(pos), TextCord(coord) { }

		Vector2D Position;
		Vector2D TextCord;
	};

	enum class FontDrawType
	{
		// Use the "additive" value from the scheme file
		Default = 0,

		// Overrides
		NonAdditive,
		Additive,

		Count = 2,
	};

	struct CharRenderInfo
	{
		// Text pos
		int				X, Y;
		// Top left and bottom right
		// This is now a pointer to an array maintained by the surface, to avoid copying the data on the 360
		Vertex*			Verts;
		int				TextureId;
		int				abcA;
		int				abcB;
		int				abcC;
		int				FontTall;
		HFont			CurrentFont;
		// In:
		FontDrawType	DrawType;
		wchar_t			Ch;

		// Out
		bool			Valid;
		// In/Out (true by default)
		bool			ShouldClip;
	};


	class IPanel : public IBaseInterface
	{
	public:
		virtual void Init(VPANEL vguiPanel, Panel* panel) abstract;

		// methods
		virtual void SetPos(VPANEL vguiPanel, int x, int y) abstract;
		virtual void GetPos(VPANEL vguiPanel, int& x, int& y) abstract;
		virtual void SetSize(VPANEL vguiPanel, int wide, int tall) abstract;
		virtual void GetSize(VPANEL vguiPanel, int& wide, int& tall) abstract;
		virtual void SetMinimumSize(VPANEL vguiPanel, int wide, int tall) abstract;
		virtual void GetMinimumSize(VPANEL vguiPanel, int& wide, int& tall) abstract;
		virtual void SetZPos(VPANEL vguiPanel, int z) abstract;
		virtual int  GetZPos(VPANEL vguiPanel) abstract;

		virtual void GetAbsPos(VPANEL vguiPanel, int& x, int& y) abstract;
		virtual void GetClipRect(VPANEL vguiPanel, int& x0, int& y0, int& x1, int& y1) abstract;
		virtual void SetInset(VPANEL vguiPanel, int left, int top, int right, int bottom) abstract;
		virtual void GetInset(VPANEL vguiPanel, int& left, int& top, int& right, int& bottom) abstract;

		virtual void SetVisible(VPANEL vguiPanel, bool state) abstract;
		virtual bool IsVisible(VPANEL vguiPanel) abstract;
		virtual void SetParent(VPANEL vguiPanel, VPANEL newParent) abstract;
		virtual int GetChildCount(VPANEL vguiPanel) abstract;
		virtual VPANEL GetChild(VPANEL vguiPanel, int index) abstract;
		virtual ValveUtlVector<VPANEL>& GetChildren(VPANEL vguiPanel) abstract;
		virtual VPANEL GetParent(VPANEL vguiPanel) abstract;
		virtual void MoveToFront(VPANEL vguiPanel) abstract;
		virtual void MoveToBack(VPANEL vguiPanel) abstract;
		virtual bool HasParent(VPANEL vguiPanel, VPANEL potentialParent) abstract;
		virtual bool IsPopup(VPANEL vguiPanel) abstract;
		virtual void SetPopup(VPANEL vguiPanel, bool state) abstract;
		virtual bool IsFullyVisible(VPANEL vguiPanel) abstract;

		// gets the scheme this panel uses
		virtual HScheme GetScheme(VPANEL vguiPanel) abstract;
		// gets whether or not this panel should scale with screen resolution
		virtual bool IsProportional(VPANEL vguiPanel) abstract;
		// returns true if auto-deletion flag is set
		virtual bool IsAutoDeleteSet(VPANEL vguiPanel) abstract;
		// deletes the Panel * associated with the vpanel
		virtual void DeletePanel(VPANEL vguiPanel) abstract;

		// input interest
		virtual void SetKeyBoardInputEnabled(VPANEL vguiPanel, bool state) abstract;
		virtual void SetMouseInputEnabled(VPANEL vguiPanel, bool state) abstract;
		virtual bool IsKeyBoardInputEnabled(VPANEL vguiPanel) abstract;
		virtual bool IsMouseInputEnabled(VPANEL vguiPanel) abstract;

		// calculates the panels current position within the hierarchy
		virtual void Solve(VPANEL vguiPanel) abstract;

		// gets names of the object (for debugging purposes)
		virtual const char* GetName(VPANEL vguiPanel) abstract;
		virtual const char* GetPaintClassName(VPANEL vguiPanel) abstract;

		// delivers a message to the panel
		virtual void SendPaintMessage(VPANEL vguiPanel, KeyValues* params, VPANEL ifromPanel) abstract;

		// these pass through to the IClientPanel
		virtual void Think(VPANEL vguiPanel) abstract;
		virtual void PerformApplySchemeSettings(VPANEL vguiPanel) abstract;
		virtual void PaintTraverse(VPANEL vguiPanel, bool forceRepaint, bool allowForce = true) abstract;
		virtual void Repaint(VPANEL vguiPanel) abstract;
		virtual VPANEL IsWithinTraverse(VPANEL vguiPanel, int x, int y, bool traversePopups) abstract;
		virtual void OnChildAdded(VPANEL vguiPanel, VPANEL child) abstract;
		virtual void OnSizeChanged(VPANEL vguiPanel, int newWide, int newTall) abstract;

		virtual void InternalFocusChanged(VPANEL vguiPanel, bool lost) abstract;
		virtual bool RequestInfo(VPANEL vguiPanel, KeyValues* outputData) abstract;
		virtual void RequestFocus(VPANEL vguiPanel, int direction = 0) abstract;
		virtual bool RequestFocusPrev(VPANEL vguiPanel, VPANEL existingPanel) abstract;
		virtual bool RequestFocusNext(VPANEL vguiPanel, VPANEL existingPanel) abstract;
		virtual VPANEL GetCurrentKeyFocus(VPANEL vguiPanel) abstract;
		virtual int GetTabPosition(VPANEL vguiPanel) abstract;

		// used by ISurface to store platform-specific data
		virtual SurfacePlat* Plat(VPANEL vguiPanel) abstract;
		virtual void SetPlat(VPANEL vguiPanel, SurfacePlat* Plat) abstract;

		// returns a pointer to the vgui controls baseclass Panel *
		// destinationModule needs to be passed in to verify that the returned Panel * is from the same module
		// it must be from the same module since Panel * vtbl may be different in each module
		virtual Panel* GetPanel(VPANEL vguiPanel, const char* destinationModule) abstract;

		virtual bool IsEnabled(VPANEL vguiPanel) abstract;
		virtual void SetEnabled(VPANEL vguiPanel, bool state) abstract;

		// Used by the drag/drop manager to always draw on top
		virtual bool IsTopmostPopup(VPANEL vguiPanel) abstract;
		virtual void SetTopmostPopup(VPANEL vguiPanel, bool state) abstract;

		// sibling pins
		virtual void SetSiblingPin(VPANEL vguiPanel, VPANEL newSibling, std::byte iMyCornerToPin = std::byte(0), std::byte iSiblingCornerToPinTo = std::byte(0)) abstract;
	};

	class ISurface : public IAppSystem
	{
	public:
		// call to Shutdown surface; surface can no longer be used after this is called
		virtual void Shutdown() abstract;

		// frame
		virtual void RunFrame() abstract;

		// hierarchy root
		virtual VPANEL GetEmbeddedPanel() abstract;
		virtual void SetEmbeddedPanel(VPANEL pPanel) abstract;

		// drawing context
		virtual void PushMakeCurrent(VPANEL panel, bool useInsets) abstract;
		virtual void PopMakeCurrent(VPANEL panel) abstract;

		// rendering functions
		virtual void DrawSetColor(int r, int g, int b, int a) abstract;
		virtual void DrawSetColor(color::u8rgba col) abstract;

		virtual void DrawFilledRect(int x0, int y0, int x1, int y1) abstract;
		virtual void DrawFilledRectArray(IntRect* pRects, int numRects) abstract;
		virtual void DrawOutlinedRect(int x0, int y0, int x1, int y1) abstract;

		virtual void DrawLine(int x0, int y0, int x1, int y1) abstract;
		virtual void DrawPolyLine(int* px, int* py, int numPoints) abstract;

		virtual void DrawSetTextFont(HFont font) abstract;
		virtual void DrawSetTextColor(int r, int g, int b, int a) abstract;
		virtual void DrawSetTextColor(color::u8rgba col) abstract;
		virtual void DrawSetTextPos(int x, int y) abstract;
		virtual void DrawGetTextPos(int& x, int& y) abstract;
		virtual void DrawPrintText(const wchar_t* text, int textLen, FontDrawType drawType = FontDrawType::Default) abstract;
		virtual void DrawUnicodeChar(wchar_t wch, FontDrawType drawType = FontDrawType::Default) abstract;

		virtual void DrawFlushText() abstract;		// flushes any buffered text (for rendering optimizations)
		virtual IHTML* CreateHTMLWindow(IHTMLEvents* events, VPANEL context) abstract;
		virtual void PaintHTMLWindow(IHTML* htmlwin) abstract;
		virtual void DeleteHTMLWindow(IHTML* htmlwin) abstract;

		enum ETextureFormat
		{
			eTextureFormat_RGBA,
			eTextureFormat_BGRA,
			eTextureFormat_BGRA_Opaque, // bgra format but alpha is always 255, CEF does this, we can use this fact for better perf on win32 gdi
		};

		virtual int	 DrawGetTextureId(char const* filename) abstract;
		virtual bool DrawGetTextureFile(int id, char* filename, int maxlen) abstract;
		virtual void DrawSetTextureFile(int id, const char* filename, int hardwareFilter, bool forceReload) abstract;
		virtual void DrawSetTextureRGBA(int id, const unsigned char* rgba, int wide, int tall, int hardwareFilter, bool forceReload) abstract;
		virtual void DrawSetTexture(int id) abstract;
		virtual void DrawGetTextureSize(int id, int& wide, int& tall) abstract;
		virtual void DrawTexturedRect(int x0, int y0, int x1, int y1) abstract;
		virtual bool IsTextureIDValid(int id) abstract;
		virtual bool DeleteTextureByID(int id) abstract;

		virtual int CreateNewTextureID(bool procedural = false) abstract;

		virtual void GetScreenSize(int& wide, int& tall) abstract;
		virtual void SetAsTopMost(VPANEL panel, bool state) abstract;
		virtual void BringToFront(VPANEL panel) abstract;
		virtual void SetForegroundWindow(VPANEL panel) abstract;
		virtual void SetPanelVisible(VPANEL panel, bool state) abstract;
		virtual void SetMinimized(VPANEL panel, bool state) abstract;
		virtual bool IsMinimized(VPANEL panel) abstract;
		virtual void FlashWindow(VPANEL panel, bool state) abstract;
		virtual void SetTitle(VPANEL panel, const wchar_t* title) abstract;
		virtual void SetAsToolBar(VPANEL panel, bool state) abstract;		// removes the window's task bar entry (for context menu's, etc.)

		// windows stuff
		virtual void CreatePopup(VPANEL panel, bool minimised, bool showTaskbarIcon = true, bool disabled = false, bool mouseInput = true, bool kbInput = true) abstract;
		virtual void SwapBuffers(VPANEL panel) abstract;
		virtual void Invalidate(VPANEL panel) abstract;
		virtual void SetCursor(HCursor cursor) abstract;
		virtual void SetCursorAlwaysVisible(bool visible) abstract;
		virtual bool IsCursorVisible() abstract;
		virtual void ApplyChanges() abstract;
		virtual bool IsWithin(int x, int y) abstract;
		virtual bool HasFocus() abstract;

		// returns true if the surface supports minimize & maximize capabilities
		enum SurfaceFeature_e
		{
			ANTIALIASED_FONTS = 1,
			DROPSHADOW_FONTS = 2,
			ESCAPE_KEY = 3,
			OPENING_NEW_HTML_WINDOWS = 4,
			FRAME_MINIMIZE_MAXIMIZE = 5,
			OUTLINE_FONTS = 6,
			DIRECT_HWND_RENDER = 7,
		};
		virtual bool SupportsFeature(SurfaceFeature_e feature) abstract;

		// restricts what gets drawn to one panel and it's children
		// currently only works in the game
		virtual void RestrictPaintToSinglePanel(VPANEL panel) abstract;

		// these two functions obselete, use IInput::SetAppModalSurface() instead
		virtual void SetModalPanel(VPANEL) abstract;
		virtual VPANEL GetModalPanel() abstract;

		virtual void UnlockCursor() abstract;
		virtual void LockCursor() abstract;
		virtual void SetTranslateExtendedKeys(bool state) abstract;
		virtual VPANEL GetTopmostPopup() abstract;

		// engine-only focus handling (replacing WM_FOCUS windows handling)
		virtual void SetTopLevelFocus(VPANEL panel) abstract;

		// fonts
		// creates an empty handle to a vgui font.  windows fonts can be add to this via SetFontGlyphSet().
		virtual HFont CreateFont() abstract;

		// adds to the font
		enum EFontFlags
		{
			FONTFLAG_NONE,
			FONTFLAG_ITALIC = 0x001,
			FONTFLAG_UNDERLINE = 0x002,
			FONTFLAG_STRIKEOUT = 0x004,
			FONTFLAG_SYMBOL = 0x008,
			FONTFLAG_ANTIALIAS = 0x010,
			FONTFLAG_GAUSSIANBLUR = 0x020,
			FONTFLAG_ROTARY = 0x040,
			FONTFLAG_DROPSHADOW = 0x080,
			FONTFLAG_ADDITIVE = 0x100,
			FONTFLAG_OUTLINE = 0x200,
			FONTFLAG_CUSTOM = 0x400,		// custom generated font - never fall back to asian compatibility mode
			FONTFLAG_BITMAP = 0x800,		// compiled bitmap font - no fallbacks
		};

		virtual bool SetFontGlyphSet(HFont font, const char* windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin = 0, int nRangeMax = 0) abstract;

		// adds a custom font file (only supports true type font files (.ttf) for now)
		virtual bool AddCustomFontFile(const char* fontName, const char* fontFileName) abstract;

		// returns the details about the font
		virtual int GetFontTall(HFont font) abstract;
		virtual int GetFontTallRequested(HFont font) abstract;
		virtual int GetFontAscent(HFont font, wchar_t wch) abstract;
		virtual bool IsFontAdditive(HFont font) abstract;
		virtual void GetCharABCwide(HFont font, int ch, int& a, int& b, int& c) abstract;
		virtual int GetCharacterWidth(HFont font, int ch) abstract;
		virtual void GetTextSize(HFont font, const wchar_t* text, int& wide, int& tall) abstract;

		// notify icons?!?
		virtual VPANEL GetNotifyPanel() abstract;
		virtual void SetNotifyIcon(VPANEL context, HTexture icon, VPANEL panelToReceiveMessages, const char* text) abstract;

		// plays a sound
		virtual void PlaySound(const char* fileName) abstract;

		//!! these functions should not be accessed directly, but only through other vgui items
		//!! need to move these to seperate interface
		virtual int GetPopupCount() abstract;
		virtual VPANEL GetPopup(int index) abstract;
		virtual bool ShouldPaintChildPanel(VPANEL childPanel) abstract;
		virtual bool RecreateContext(VPANEL panel) abstract;
		virtual void AddPanel(VPANEL panel) abstract;
		virtual void ReleasePanel(VPANEL panel) abstract;
		virtual void MovePopupToFront(VPANEL panel) abstract;
		virtual void MovePopupToBack(VPANEL panel) abstract;

		virtual void SolveTraverse(VPANEL panel, bool forceApplySchemeSettings = false) abstract;
		virtual void PaintTraverse(VPANEL panel) abstract;

		virtual void EnableMouseCapture(VPANEL panel, bool state) abstract;

		// returns the size of the workspace
		virtual void GetWorkspaceBounds(int& x, int& y, int& wide, int& tall) abstract;

		// gets the absolute coordinates of the screen (in windows space)
		virtual void GetAbsoluteWindowBounds(int& x, int& y, int& wide, int& tall) abstract;

		// gets the base resolution used in proportional mode
		virtual void GetProportionalBase(int& width, int& height) abstract;

		virtual void CalculateMouseVisible() abstract;
		virtual bool NeedKBInput() abstract;

		virtual bool HasCursorPosFunctions() abstract;
		virtual void SurfaceGetCursorPos(int& x, int& y) abstract;
		virtual void SurfaceSetCursorPos(int x, int y) abstract;

		// SRC only functions!!!
		virtual void DrawTexturedLine(const Vertex& a, const Vertex& b) abstract;
		virtual void DrawOutlinedCircle(int x, int y, int radius, int segments) abstract;
		virtual void DrawTexturedPolyLine(const Vertex* p, int n) abstract; // (Note: this connects the first and last points).
		virtual void DrawTexturedSubRect(int x0, int y0, int x1, int y1, float texs0, float text0, float texs1, float text1) abstract;
		virtual void DrawTexturedPolygon(int n, Vertex* pVertice, bool bClipVertices = true) abstract;
		virtual const wchar_t* GetTitle(VPANEL panel) abstract;
		virtual bool IsCursorLocked(void) const abstract;
		virtual void SetWorkspaceInsets(int left, int top, int right, int bottom) abstract;

		// Lower level char drawing code, call DrawGet then pass in info to DrawRender
		virtual bool DrawGetUnicodeCharRenderInfo(wchar_t ch, CharRenderInfo& info) abstract;
		virtual void DrawRenderCharFromInfo(const CharRenderInfo& info) abstract;

		// global alpha setting functions
		// affect all subsequent draw calls - shouldn't normally be used directly, only in Panel::PaintTraverse()
		virtual void DrawSetAlphaMultiplier(float alpha /* [0..1] */) abstract;
		virtual float DrawGetAlphaMultiplier() abstract;

		// web browser
		virtual void SetAllowHTMLJavaScript(bool state) abstract;

		// video mode changing
		virtual void OnScreenSizeChanged(int nOldWidth, int nOldHeight) abstract;

		virtual HCursor CreateCursorFromFile(char const* curOrAniFile, char const* pPathID = 0) abstract;

		// create IVguiMatInfo object ( IMaterial wrapper in VguiMatSurface, NULL in CWin32Surface )
		virtual IVguiMatInfo* DrawGetTextureMatInfoFactory(int id) abstract;

		virtual void PaintTraverseEx(VPANEL panel, bool paintPopups = false) abstract;

		virtual float GetZPos() const abstract;

		// From the Xbox
		virtual void SetPanelForInput(VPANEL vpanel) abstract;
		virtual void DrawFilledRectFastFade(int x0, int y0, int x1, int y1, int fadeStartPt, int fadeEndPt, unsigned int alpha0, unsigned int alpha1, bool bHorizontal) abstract;
		virtual void DrawFilledRectFade(int x0, int y0, int x1, int y1, unsigned int alpha0, unsigned int alpha1, bool bHorizontal) abstract;
		virtual void DrawSetTextureRGBAEx(int id, const unsigned char* rgba, int wide, int tall, ImageFormat imageFormat) abstract;
		virtual void DrawSetTextScale(float sx, float sy) abstract;
		virtual bool SetBitmapFontGlyphSet(HFont font, const char* windowsFontName, float scalex, float scaley, int flags) abstract;
		// adds a bitmap font file
		virtual bool AddBitmapFontFile(const char* fontFileName) abstract;
		// sets a symbol for the bitmap font
		virtual void SetBitmapFontName(const char* pName, const char* pFontFilename) abstract;
		// gets the bitmap font filename
		virtual const char* GetBitmapFontName(const char* pName) abstract;
		virtual void ClearTemporaryFontCache(void) abstract;

		virtual IImage* GetIconImageForFullPath(char const* pFullPath) abstract;
		virtual void DrawUnicodeString(const wchar_t* pwString, FontDrawType drawType = FontDrawType::Default) abstract;
		virtual void PrecacheFontCharacters(HFont font, const wchar_t* pCharacters) abstract;
		// Console-only.  Get the string to use for the current video mode for layout files.
		virtual const char* GetResolutionKey(void) const abstract;

		virtual const char* GetFontName(HFont font) abstract;
		virtual const char* GetFontFamilyName(HFont font) abstract;
		virtual void GetKernedCharWidth(HFont font, wchar_t ch, wchar_t chBefore, wchar_t chAfter, float& wide, float& abcA) abstract;

		virtual bool ForceScreenSizeOverride(bool bState, int wide, int tall) abstract;
		// LocalToScreen, ParentLocalToScreen fixups for explicit PaintTraverse calls on Panels not at 0, 0 position
		virtual bool ForceScreenPosOffset(bool bState, int x, int y) abstract;
		virtual void OffsetAbsPos(int& x, int& y) abstract;


		// Causes fonts to get reloaded, etc.
		virtual void ResetFontCaches() abstract;

		virtual int GetTextureNumFrames(int id) abstract;
		virtual void DrawSetTextureFrame(int id, int nFrame, unsigned int* pFrameCache) abstract;
		virtual bool IsScreenSizeOverrideActive(void) abstract;
		virtual bool IsScreenPosOverrideActive(void) abstract;

		virtual void DestroyTextureID(int id) abstract;

		virtual void DrawUpdateRegionTextureRGBA(int nTextureID, int x, int y, const unsigned char* pchData, int wide, int tall, ImageFormat imageFormat) abstract;
		virtual bool BHTMLWindowNeedsPaint(IHTML* htmlwin) abstract;

		virtual const char* GetWebkitHTMLUserAgentString() abstract;

		virtual void* Deprecated_AccessChromeHTMLController() abstract;

		// the origin of the viewport on the framebuffer (Which might not be 0,0 for stereo)
		virtual void SetFullscreenViewport(int x, int y, int w, int h) abstract; // this uses NULL for the render target.
		virtual void GetFullscreenViewport(int& x, int& y, int& w, int& h) abstract;
		virtual void PushFullscreenViewport() abstract;
		virtual void PopFullscreenViewport() abstract;

		// handles support for software cursors
		virtual void SetSoftwareCursor(bool bUseSoftwareCursor) abstract;
		virtual void PaintSoftwareCursor() abstract;
	};

	class Panel
	{
	public:
		GAMEPROP_DECL_OFFSET(VPANEL, VPanel, Offsets::VGUI::Panel::VPanel);
		GAMEPROP_DECL_OFFSET(const char*, Name, Offsets::VGUI::Panel::Name);

		GAMEPROP_DECL_OFFSET(color::u8rgba, FGColor, Offsets::VGUI::Panel::FGColor);
		GAMEPROP_DECL_OFFSET(color::u8rgba, BGColor, Offsets::VGUI::Panel::BGColor);
	};

	extern VPANEL FocusOverlayPanel;
}
