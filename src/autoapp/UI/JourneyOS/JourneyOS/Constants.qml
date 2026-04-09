// @disable-check M208
pragma Singleton
import QtQuick

// ─────────────────────────────────────────────────────────────────────────────
// JourneyOS Design Tokens — Material 3 scheme
// Seed colour: #7900B5  •  Generated with Material Theme Builder
// ─────────────────────────────────────────────────────────────────────────────
// NAMING CONVENTION
//   _xxx     Raw M3 palette / internal token — do NOT reference from outside
//             this file. QML reserves any property starting with "on" + uppercase
//             as a signal handler, so all M3 "on-role" tokens must carry the _
//             prefix to avoid parse errors.
//   xxx      Public semantic token — use these throughout the UI.
//             These are either derived semantic roles (surfaceOverlay, btnSurface…)
//             or backward-compatibility aliases (textPrimary, popupBackground…).
//
// Toggle isDarkMode to switch the entire UI between dark and light palettes.
// All colour properties are computed bindings — changing isDarkMode at runtime
// re-themes every bound control instantly.
// ─────────────────────────────────────────────────────────────────────────────

QtObject {
    id: root

    // ── THEME MODE ────────────────────────────────────────────────────────────
    property bool isDarkMode: true

    // ── USER-OVERRIDABLE ACCENT COLOURS ───────────────────────────────────────
    // Empty string = use M3 default for current mode; set via settingsViewHandler
    property string accentPrimary: ""
    property string accentBrand2:  ""
    // JourneyButton surface mist opacity (0.0 = invisible, 1.0 = opaque)
    property double buttonSurfaceOpacity: 0.33

    // ── ALPHA HELPER ──────────────────────────────────────────────────────────
    // Usage: Constants.withAlpha(Constants.textPrimary, 0.38)
    function withAlpha(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha)
    }

    // ── APPLICATION DIMENSIONS ────────────────────────────────────────────────
    readonly property int appWidth:  800
    readonly property int appHeight: 480

    // ── SHAPE (border radii) ──────────────────────────────────────────────────
    readonly property int shapeOverlay: 10   // Full-screen popups & overlay cards
    readonly property int shapeCard:     8   // Inset cards / panels within popups
    readonly property int shapeButton:   5   // Standard JourneyButton background
    readonly property int shapeInput:    4   // Text fields, status strips, list items
    readonly property int shapeFull:    14   // Circular elements (28 × 28 close button)
    readonly property int shapeSlider:  10   // Slider track and groove

    // ── LAYOUT METRICS ────────────────────────────────────────────────────────
    readonly property int settingsPaddingOuter:  30
    readonly property int settingsLabelWidth:   280
    readonly property int settingsControlHeight: 40
    readonly property int settingsHeaderHeight:  70
    readonly property int settingsFooterHeight:  70

    readonly property int sliderTrackWidth:  20
    readonly property int sliderTrackHeight: 200
    readonly property int sliderHandleSize:  26

    // ── TYPOGRAPHY ────────────────────────────────────────────────────────────
    readonly property int typeTitleLarge:  20   // Popup / page title — e.g. "Bluetooth"
    readonly property int typeTitleMedium: 18   // Sub-page / dialog heading
    readonly property int typeTitleSmall:  16   // Section headings within a page
    readonly property int typeBodyLarge:   14   // Primary body text, device names
    readonly property int typeLabelLarge:  13   // Field labels, secondary text
    readonly property int typeLabelMedium: 12   // Small info text, status strips
    readonly property int typeLabelSmall:  11   // Metadata / smallest text

    readonly property string relativeFontDirectory: "fonts"
    readonly property font font: Qt.font({
        family:    Qt.application.font.family,
        pixelSize: Qt.application.font.pixelSize
    })
    readonly property font largeFont: Qt.font({
        family:    Qt.application.font.family,
        pixelSize: Qt.application.font.pixelSize * 1.6
    })

    // ════════════════════════════════════════════════════════════════════════
    // RAW M3 PALETTE  (all prefixed with _  — internal use only)
    // ════════════════════════════════════════════════════════════════════════

    // ── PRIMARY (purple) ──────────────────────────────────────────────────────
    readonly property color _primary:            accentPrimary !== "" ? accentPrimary
                                                                       : (isDarkMode ? "#DEB3FF" : "#7E14BA")
    readonly property color _onPrimary:          isDarkMode ? "#4D0183" : "#FFFFFF"
    readonly property color _primaryContainer:   isDarkMode ? "#640FA1" : "#F2D7FF"
    readonly property color _onPrimaryContainer: isDarkMode ? "#F2D7FF" : "#36006A"

    // ── SECONDARY (muted purple) ──────────────────────────────────────────────
    readonly property color _secondary:            isDarkMode ? "#CEB9E3" : "#6E5787"
    readonly property color _onSecondary:          isDarkMode ? "#3C2756" : "#FFFFFF"
    readonly property color _secondaryContainer:   isDarkMode ? "#543E6E" : "#ECDDF3"
    readonly property color _onSecondaryContainer: isDarkMode ? "#ECDDF3" : "#271140"

    // ── TERTIARY (harmonised — rose / pink) ───────────────────────────────────
    readonly property color _tertiary:            isDarkMode ? "#EFB4C3" : "#8E3B50"
    readonly property color _onTertiary:          isDarkMode ? "#54243A" : "#FFFFFF"
    readonly property color _tertiaryContainer:   isDarkMode ? "#72253B" : "#FFD9E3"
    readonly property color _onTertiaryContainer: isDarkMode ? "#FFD9E3" : "#3C0719"

    // ── ERROR ─────────────────────────────────────────────────────────────────
    readonly property color _error:            isDarkMode ? "#FFB4AB" : "#BA1A1A"
    readonly property color _onError:          isDarkMode ? "#690005" : "#FFFFFF"
    readonly property color _errorContainer:   isDarkMode ? "#93000A" : "#FFDAD6"
    readonly property color _onErrorContainer: isDarkMode ? "#FFDAD6" : "#410002"

    // ── SURFACE ───────────────────────────────────────────────────────────────
    readonly property color _surface:               isDarkMode ? "#141218" : "#FBF8FF"
    readonly property color _onSurface:             isDarkMode ? "#E7E0EB" : "#1C1A22"
    readonly property color _surfaceVariant:        isDarkMode ? "#4D4352" : "#EDE0F2"
    readonly property color _onSurfaceVariant:      isDarkMode ? "#CCC3CF" : "#4B4356"
    readonly property color _onSurfaceDisabled:     withAlpha(_onSurface, 0.38)

    readonly property color _surfaceDim:              isDarkMode ? "#141218" : "#DDD8E5"
    readonly property color _surfaceBright:           isDarkMode ? "#3B3740" : "#FBF8FF"
    readonly property color _surfaceContainerLowest:  isDarkMode ? "#0F0D13" : "#FFFFFF"
    readonly property color _surfaceContainerLow:     isDarkMode ? "#1D1B23" : "#F7F2FF"
    readonly property color _surfaceContainer:        isDarkMode ? "#221F28" : "#F1ECF9"
    readonly property color _surfaceContainerHigh:    isDarkMode ? "#2C2A33" : "#EBE6F3"
    readonly property color _surfaceContainerHighest: isDarkMode ? "#37343F" : "#E6E0EE"

    readonly property color _inverseSurface:   isDarkMode ? "#E7E0EB" : "#321F39"
    readonly property color _inverseOnSurface: isDarkMode ? "#321F39" : "#F5EDFC"
    readonly property color _inversePrimary:   isDarkMode ? "#7E14BA" : "#DEB3FF"

    // ── OUTLINE ───────────────────────────────────────────────────────────────
    readonly property color _outline:        isDarkMode ? "#998E9E" : "#7C7289"
    readonly property color _outlineVariant: isDarkMode ? "#4D4352" : "#CCC3CF"

    // ── SCRIM ─────────────────────────────────────────────────────────────────
    readonly property color _scrim: "#000000"

    // ── CUSTOM EXTENSIONS (success / warning / brand2) ────────────────────────

    // Success — green
    readonly property color _success:            isDarkMode ? "#78DC77" : "#1A6E1A"
    readonly property color _onSuccess:          isDarkMode ? "#00390A" : "#FFFFFF"
    readonly property color _successContainer:   isDarkMode ? "#005315" : "#A3F79F"
    readonly property color _onSuccessContainer: isDarkMode ? "#A3F79F" : "#002105"

    // Warning — amber
    readonly property color _warning:            isDarkMode ? "#E4C600" : "#615B00"
    readonly property color _onWarning:          isDarkMode ? "#322F00" : "#FFFFFF"
    readonly property color _warningContainer:   isDarkMode ? "#494400" : "#FFED3C"
    readonly property color _onWarningContainer: isDarkMode ? "#FFED3C" : "#1D1C00"

    // Brand2 / Active — blue (second brand accent; used for connecting / in-progress state)
    readonly property color _brand2:            accentBrand2 !== "" ? accentBrand2
                                                                     : (isDarkMode ? "#7ED6FF" : "#005F8A")
    readonly property color _onBrand2:          isDarkMode ? "#003549" : "#FFFFFF"
    readonly property color _brand2Container:   isDarkMode ? "#004D6A" : "#C3EAFF"
    readonly property color _onBrand2Container: isDarkMode ? "#C3EAFF" : "#001E2D"

    // Semantic alias: "active" = in-progress / connecting state
    readonly property color _active:   _brand2
    readonly property color _onActive: _onBrand2

    // ── SETTINGS PALETTE (fixed light-glass — independent of isDarkMode) ──────
    // Plain string literals are used here.  The _ prefix is essential: QML would
    // treat any bare "on<Upper>" name as a signal handler and reject the literal.
    readonly property color _surfaceSettings:          "#A3CAED"
    readonly property color _surfaceSettingsContainer: "#EBFBFF"
    readonly property color _onSurfaceSettings:        "#3A4856"
    readonly property color _onSurfaceSettingsVariant: "#9DADBC"
    readonly property color _onSurfaceSettingsSubtle:  "#536878"
    readonly property color _outlineSettings:          "#3A4856"
    readonly property color _surfaceSettingsFooter:    "#D9FFFFFF"
    readonly property color _outlineSettingsFooter:    "#E0E0E0"
    readonly property color _surfaceSettingsTabBar:    "#80FFFFFF"

    // ════════════════════════════════════════════════════════════════════════
    // PUBLIC SEMANTIC TOKENS  (no _ prefix — use these in the UI)
    // ════════════════════════════════════════════════════════════════════════

    // ── DERIVED SURFACE / OVERLAY ROLES ──────────────────────────────────────
    // Popup overlay background (semi-transparent dark surface)
    readonly property color surfaceOverlay: withAlpha(_surfaceContainerLowest, isDarkMode ? 0.93 : 0.95)
    // Modal veil behind full-screen popups
    readonly property color scrimVeil: withAlpha(_scrim, 0.80)

    // Status surface backgrounds (translucent tints behind status indicator rows)
    readonly property color successSurface: withAlpha(_success, 0.13)
    readonly property color errorSurface:   withAlpha(_error,   0.13)

    // List item backgrounds on dark overlays
    readonly property color listSurface:        withAlpha(_onSurface, 0.13)
    readonly property color listSurfacePressed: withAlpha(_onSurface, 0.27)

    // Android Auto home button overlay
    readonly property color aaHomeSurface: withAlpha(_scrim, 0.50)
    readonly property color aaHomeOutline: withAlpha(_onSurface, 0.25)

    // ── BUTTONS ───────────────────────────────────────────────────────────────
    // Default / neutral
    // btnSurface is always a semi-transparent _onSurface tint over the page bg:
    //   dark mode: light tint on dark bg  →  medium-dark surface
    //   light mode: dark tint on light bg →  medium-dark surface
    // Both cases produce a darker surface, so content is always a fixed white.
    readonly property color btnSurface:        withAlpha(_onSurface, buttonSurfaceOpacity)
    readonly property color btnSurfacePressed: withAlpha(_onSurface, 0.73)
    readonly property color btnContent:        "#FFFFFF"
    readonly property color btnContentPressed: Qt.rgba(1, 1, 1, 0.65)

    // Danger — dark red bg, vivid red label
    readonly property color btnDangerSurface:        withAlpha(_errorContainer, 0.80)
    readonly property color btnDangerSurfacePressed: withAlpha(_error, 0.60)
    readonly property color btnDangerContent:        _onErrorContainer

    // Confirm — dark green bg, light green label
    readonly property color btnConfirmSurface:        withAlpha(_successContainer, 0.80)
    readonly property color btnConfirmSurfacePressed: withAlpha(_success, 0.60)
    readonly property color btnConfirmContent:        _onSuccessContainer

    // Action — dark blue bg, light blue label
    readonly property color btnActionSurface:        withAlpha(_brand2Container, 0.80)
    readonly property color btnActionSurfacePressed: withAlpha(_brand2, 0.60)
    readonly property color btnActionContent:        _onBrand2Container

    // Cancel — transparent outline, red border + text
    readonly property color btnCancelOutline: _error
    readonly property color btnCancelContent: _error

    // ── SLIDERS ───────────────────────────────────────────────────────────────
    // Volume: purple (low) → blue (high)
    readonly property color sliderActiveTrackLow:  withAlpha(_primaryContainer, 0.80)
    readonly property color sliderActiveTrackHigh: withAlpha(_brand2, 0.82)

    // Brightness: dark purple → lighter purple
    readonly property color sliderBrightnessTrackLow:  withAlpha(_primaryContainer, 0.80)
    readonly property color sliderBrightnessTrackHigh: withAlpha(_primary, 0.67)

    // Shared slider track / handle
    readonly property color sliderInactiveTrack: withAlpha(_onSurface, 0.47)
    readonly property color sliderHandle:        "#F6F6F6"
    readonly property color sliderHandleOutline: _outlineVariant
    readonly property color sliderHandlePressed: _primaryContainer

    // Slider popup side-panel background
    readonly property color sliderPanelSurface: withAlpha(_scrim, 0.67)

    // ════════════════════════════════════════════════════════════════════════
    // BACKWARD-COMPATIBILITY ALIASES
    // These keep all existing QML references working without changes.
    // Migrate usages to the semantic names above and remove these over time.
    // ════════════════════════════════════════════════════════════════════════

    // Dimensions
    readonly property int width:  appWidth
    readonly property int height: appHeight

    // Shape
    readonly property int radiusPopup:  shapeOverlay
    readonly property int radiusCard:   shapeCard
    readonly property int radiusButton: shapeButton
    readonly property int radiusInput:  shapeInput
    readonly property int radiusCircle: shapeFull
    readonly property int radiusSlider: shapeSlider

    // Typography
    readonly property int fontTitle:    typeTitleLarge
    readonly property int fontHeading:  typeTitleMedium
    readonly property int fontSubtitle: typeTitleSmall
    readonly property int fontBody:     typeBodyLarge
    readonly property int fontLabel:    typeLabelLarge
    readonly property int fontCaption:  typeLabelMedium
    readonly property int fontSmall:    typeLabelSmall

    // Text colours
    readonly property color textPrimary:        _onSurface
    readonly property color textSecondary:      _onSurfaceVariant
    readonly property color textDisabled:       _onSurfaceDisabled
    readonly property color textOnSettings:     _onSurfaceSettings
    readonly property color textDimOnSettings:  _onSurfaceSettingsVariant
    readonly property color textNoteOnSettings: _onSurfaceSettingsSubtle

    // Status / semantic colours
    readonly property color statusOk:       _success
    readonly property color statusWait:     _warning
    readonly property color statusBad:      _error
    readonly property color statusAction:   _active
    readonly property color statusNeutral:  _onSurfaceDisabled
    readonly property color statusBgOk:     successSurface
    readonly property color statusBgBad:    errorSurface
    readonly property color okColor:        _success
    readonly property color waitColor:      _warning
    readonly property color badColor:       _error
    readonly property color actionColor:    _active
    readonly property color baseColor:      _onSurfaceDisabled

    // Popup colours
    readonly property color popupBackground:            _surface
    readonly property color popupBackgroundTranslucent: surfaceOverlay
    readonly property color popupBorder:                _outlineVariant
    readonly property color popupBorderStrong:          _outline
    readonly property color popupOverlayDim:            scrimVeil

    // Overlay
    readonly property color overlaySubtle: listSurface
    readonly property color overlayPress:  listSurfacePressed

    // Settings colours
    readonly property color settingsPageBackground:       _surfaceSettings
    readonly property color settingsSurface:              _surfaceSettingsContainer
    readonly property color settingsBorder:               _outlineSettings
    readonly property color settingsFooter:               _surfaceSettingsFooter
    readonly property color settingsFooterBorder:         _outlineSettingsFooter
    readonly property color settingsTabBar:               _surfaceSettingsTabBar
    readonly property color backgroundColor:              _surface
    readonly property color primaryBackgroundPalette:     _surface
    readonly property color primaryBackgroundColor:       _surface
    readonly property color secondaryBackgroundPalette:   _primary
    readonly property color settingsBackgroundPalette:    _surfaceSettings
    readonly property color settingsPopupBackgroundColor: surfaceOverlay
    readonly property color primaryTextColor:             _onSurface
    readonly property color settingTabBarColor:           _surfaceSettingsTabBar
    readonly property color settingsTabBarPalette:        _surfaceSettingsTabBar
    readonly property color settingFooterPalette:         _surfaceSettingsFooter
    readonly property color settingFooterColor:           _surfaceSettingsFooter
    readonly property color paletteBackground:            _surface
    readonly property color palettePrimary:               _primary
    readonly property color paletteSecondary:             _brand2
    readonly property color paletteSettingsBg:            _surfaceSettings
    readonly property color paletteSettingsFg:            _onSurfaceSettings

    // Button colours
    readonly property color btnNormalBg:                  btnSurface
    readonly property color btnNormalBgPressed:           btnSurfacePressed
    readonly property color btnNormalFg:                  btnContent
    readonly property color btnNormalFgPressed:           btnContentPressed
    readonly property color btnDangerBg:                  btnDangerSurface
    readonly property color btnDangerBgPressed:           btnDangerSurfacePressed
    readonly property color btnDangerFg:                  btnDangerContent
    readonly property color btnDangerFgPressed:           withAlpha(_error, 0.67)
    readonly property color btnConfirmBg:                 btnConfirmSurface
    readonly property color btnConfirmBgPressed:          btnConfirmSurfacePressed
    readonly property color btnConfirmFg:                 btnConfirmContent
    readonly property color btnConfirmFgPressed:          withAlpha(_onSuccessContainer, 0.80)
    readonly property color btnActionBg:                  btnActionSurface
    readonly property color btnActionBgPressed:           btnActionSurfacePressed
    readonly property color btnActionFg:                  btnActionContent
    readonly property color btnCancelBorder:              btnCancelOutline
    readonly property color btnCancelFg:                  btnCancelContent
    readonly property color buttonBackgroundPalette:      _onSurface
    readonly property color buttonBackgroundColor:        btnSurface
    readonly property color buttonPressedBackgroundColor: btnSurfacePressed
    readonly property color buttonColor:                  btnContent
    readonly property color buttonPressedColor:           btnContentPressed

    // Slider colours
    readonly property color sliderVolumeMin:             sliderActiveTrackLow
    readonly property color sliderVolumeMax:             sliderActiveTrackHigh
    readonly property color sliderBrightnessLow:         sliderBrightnessTrackLow
    readonly property color sliderBrightnessHigh:        sliderBrightnessTrackHigh
    readonly property color sliderTrackBg:               sliderInactiveTrack
    readonly property color sliderHandleBorder:          sliderHandleOutline
    readonly property color sliderPopupBg:               sliderPanelSurface
    readonly property color sliderPrimaryPalette:        _primaryContainer
    readonly property color sliderPrimaryColor:          sliderActiveTrackLow
    readonly property color sliderSecondaryPalette:      _brand2
    readonly property color sliderAlternateColor:        sliderActiveTrackHigh
    readonly property color sliderBackgroundColor:       sliderInactiveTrack
    readonly property color sliderPopupBackgroundColor:  sliderPanelSurface

    // Android Auto overlay
    readonly property color aaHomeBtnBg:     aaHomeSurface
    readonly property color aaHomeBtnBorder: aaHomeOutline
}
