// @disable-check M208
pragma Singleton
import QtQuick

// ─────────────────────────────────────────────────────────────────────────────
// JourneyOS Design Tokens — Material 3 scheme
// Seed colour: #7900B5  •  Generated with Material Theme Builder
// ─────────────────────────────────────────────────────────────────────────────
// Toggle isDarkMode to switch the entire UI between dark and light palettes.
// All colour properties are computed bindings — changing isDarkMode at runtime
// re-themes every bound control instantly.
//
// Naming convention follows Material 3:
//   surface        — a background that things sit on
//   onSurface      — content (text / icons) rendered on top of `surface`
//   container      — a muted, tonal version of a role used as a background
//   onXxxContainer — content rendered on top of `xxxContainer`
// ─────────────────────────────────────────────────────────────────────────────

QtObject {
    id: root

    // ── THEME MODE ────────────────────────────────────────────────────────────
    property bool isDarkMode: true

    // ── ALPHA HELPER ──────────────────────────────────────────────────────────
    // Usage: Constants.withAlpha(Constants.onSurface, 0.38)
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
    // MATERIAL 3 COLOUR ROLES  (seed: #7900B5)
    // ════════════════════════════════════════════════════════════════════════

    // ── PRIMARY (purple) ──────────────────────────────────────────────────────
    readonly property color primary:             isDarkMode ? "#DEB3FF" : "#7E14BA"
    readonly property color onPrimary:           isDarkMode ? "#4D0183" : "#FFFFFF"
    readonly property color primaryContainer:    isDarkMode ? "#640FA1" : "#F2D7FF"
    readonly property color onPrimaryContainer:  isDarkMode ? "#F2D7FF" : "#36006A"

    // ── SECONDARY (muted purple) ──────────────────────────────────────────────
    readonly property color secondary:            isDarkMode ? "#CEB9E3" : "#6E5787"
    readonly property color onSecondary:          isDarkMode ? "#3C2756" : "#FFFFFF"
    readonly property color secondaryContainer:   isDarkMode ? "#543E6E" : "#ECDDF3"
    readonly property color onSecondaryContainer: isDarkMode ? "#ECDDF3" : "#271140"

    // ── TERTIARY (harmonised — rose / pink) ───────────────────────────────────
    readonly property color tertiary:            isDarkMode ? "#EFB4C3" : "#8E3B50"
    readonly property color onTertiary:          isDarkMode ? "#54243A" : "#FFFFFF"
    readonly property color tertiaryContainer:   isDarkMode ? "#72253B" : "#FFD9E3"
    readonly property color onTertiaryContainer: isDarkMode ? "#FFD9E3" : "#3C0719"

    // ── ERROR ─────────────────────────────────────────────────────────────────
    readonly property color error:            isDarkMode ? "#FFB4AB" : "#BA1A1A"
    readonly property color onError:          isDarkMode ? "#690005" : "#FFFFFF"
    readonly property color errorContainer:   isDarkMode ? "#93000A" : "#FFDAD6"
    readonly property color onErrorContainer: isDarkMode ? "#FFDAD6" : "#410002"

    // ── SURFACE ───────────────────────────────────────────────────────────────
    readonly property color surface:               isDarkMode ? "#141218" : "#FBF8FF"
    readonly property color onSurface:             isDarkMode ? "#E7E0EB" : "#1C1A22"
    readonly property color surfaceVariant:        isDarkMode ? "#4D4352" : "#EDE0F2"
    readonly property color onSurfaceVariant:      isDarkMode ? "#CCC3CF" : "#4B4356"
    readonly property color onSurfaceDisabled:     withAlpha(onSurface, 0.38)

    readonly property color surfaceDim:              isDarkMode ? "#141218" : "#DDD8E5"
    readonly property color surfaceBright:           isDarkMode ? "#3B3740" : "#FBF8FF"
    readonly property color surfaceContainerLowest:  isDarkMode ? "#0F0D13" : "#FFFFFF"
    readonly property color surfaceContainerLow:     isDarkMode ? "#1D1B23" : "#F7F2FF"
    readonly property color surfaceContainer:        isDarkMode ? "#221F28" : "#F1ECF9"
    readonly property color surfaceContainerHigh:    isDarkMode ? "#2C2A33" : "#EBE6F3"
    readonly property color surfaceContainerHighest: isDarkMode ? "#37343F" : "#E6E0EE"

    readonly property color inverseSurface:   isDarkMode ? "#E7E0EB" : "#321F39"
    readonly property color inverseOnSurface: isDarkMode ? "#321F39" : "#F5EDFC"
    readonly property color inversePrimary:   isDarkMode ? "#7E14BA" : "#DEB3FF"

    // ── OUTLINE ───────────────────────────────────────────────────────────────
    readonly property color outline:        isDarkMode ? "#998E9E" : "#7C7289"
    readonly property color outlineVariant: isDarkMode ? "#4D4352" : "#CCC3CF"

    // ── SCRIM ─────────────────────────────────────────────────────────────────
    readonly property color scrim: "#000000"

    // ════════════════════════════════════════════════════════════════════════
    // CUSTOM EXTENSIONS (not in standard M3 spec)
    // ════════════════════════════════════════════════════════════════════════

    // Success — green
    readonly property color success:            isDarkMode ? "#78DC77" : "#1A6E1A"
    readonly property color onSuccess:          isDarkMode ? "#00390A" : "#FFFFFF"
    readonly property color successContainer:   isDarkMode ? "#005315" : "#A3F79F"
    readonly property color onSuccessContainer: isDarkMode ? "#A3F79F" : "#002105"

    // Warning — amber
    readonly property color warning:            isDarkMode ? "#E4C600" : "#615B00"
    readonly property color onWarning:          isDarkMode ? "#322F00" : "#FFFFFF"
    readonly property color warningContainer:   isDarkMode ? "#494400" : "#FFED3C"
    readonly property color onWarningContainer: isDarkMode ? "#FFED3C" : "#1D1C00"

    // Brand2 / Active — blue (second brand accent; used for connecting / in-progress state)
    readonly property color brand2:            isDarkMode ? "#7ED6FF" : "#005F8A"
    readonly property color onBrand2:          isDarkMode ? "#003549" : "#FFFFFF"
    readonly property color brand2Container:   isDarkMode ? "#004D6A" : "#C3EAFF"
    readonly property color onBrand2Container: isDarkMode ? "#C3EAFF" : "#001E2D"

    // Semantic alias: "active" = in-progress / connecting state
    readonly property color active:   brand2
    readonly property color onActive: onBrand2

    // ════════════════════════════════════════════════════════════════════════
    // DERIVED SEMANTIC ROLES
    // ════════════════════════════════════════════════════════════════════════

    // Popup overlay background (semi-transparent dark surface)
    readonly property color surfaceOverlay: withAlpha(surfaceContainerLowest, isDarkMode ? 0.93 : 0.95)
    // Modal veil behind full-screen popups
    readonly property color scrimVeil: withAlpha(scrim, 0.80)

    // Status surface backgrounds (translucent tints behind status indicator rows)
    readonly property color successSurface: withAlpha(success, 0.13)
    readonly property color errorSurface:   withAlpha(error,   0.13)

    // List item backgrounds on dark overlays
    readonly property color listSurface:        withAlpha(onSurface, 0.13)
    readonly property color listSurfacePressed: withAlpha(onSurface, 0.27)

    // Android Auto home button overlay
    readonly property color aaHomeSurface: withAlpha(scrim, 0.50)
    readonly property color aaHomeOutline: withAlpha(onSurface, 0.25)

    // ── SETTINGS SURFACE (fixed light-glass — independent of isDarkMode) ──────
    // The settings view always uses a light glass aesthetic.
    readonly property color surfaceSettings:          "#A3CAED"
    readonly property color surfaceSettingsContainer: "#EBFBFF"
    readonly property color onSurfaceSettings:        "#3A4856"
    readonly property color onSurfaceSettingsVariant: "#9DADBC"
    readonly property color onSurfaceSettingsSubtle:  "#536878"
    readonly property color outlineSettings:          "#3A4856"
    readonly property color surfaceSettingsFooter:    "#D9FFFFFF"
    readonly property color outlineSettingsFooter:    "#E0E0E0"
    readonly property color surfaceSettingsTabBar:    "#80FFFFFF"

    // ── BUTTONS ───────────────────────────────────────────────────────────────
    // Default / neutral
    readonly property color btnSurface:        withAlpha(onSurface, 0.33)
    readonly property color btnSurfacePressed: withAlpha(onSurface, 0.73)
    readonly property color btnContent:        onSurface
    readonly property color btnContentPressed: withAlpha(onSurface, 0.50)

    // Danger — dark red bg, vivid red label
    readonly property color btnDangerSurface:        withAlpha(errorContainer, 0.80)
    readonly property color btnDangerSurfacePressed: withAlpha(error, 0.60)
    readonly property color btnDangerContent:        onErrorContainer

    // Confirm — dark green bg, light green label
    readonly property color btnConfirmSurface:        withAlpha(successContainer, 0.80)
    readonly property color btnConfirmSurfacePressed: withAlpha(success, 0.60)
    readonly property color btnConfirmContent:        onSuccessContainer

    // Action — dark blue bg, light blue label
    readonly property color btnActionSurface:        withAlpha(brand2Container, 0.80)
    readonly property color btnActionSurfacePressed: withAlpha(brand2, 0.60)
    readonly property color btnActionContent:        onBrand2Container

    // Cancel — transparent outline, red border + text
    readonly property color btnCancelOutline: error
    readonly property color btnCancelContent: error

    // ── SLIDERS ───────────────────────────────────────────────────────────────
    // Volume: purple (low) → blue (high)
    readonly property color sliderActiveTrackLow:  withAlpha(primaryContainer, 0.80)
    readonly property color sliderActiveTrackHigh: withAlpha(brand2, 0.82)

    // Brightness: dark purple → lighter purple
    readonly property color sliderBrightnessTrackLow:  withAlpha(primaryContainer, 0.80)
    readonly property color sliderBrightnessTrackHigh: withAlpha(primary, 0.67)

    // Shared slider track / handle
    readonly property color sliderInactiveTrack: withAlpha(onSurface, 0.47)
    readonly property color sliderHandle:        "#F6F6F6"
    readonly property color sliderHandleOutline: outlineVariant
    readonly property color sliderHandlePressed: primaryContainer

    // Slider popup side-panel background
    readonly property color sliderPanelSurface: withAlpha(scrim, 0.67)

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
    readonly property color textPrimary:        onSurface
    readonly property color textSecondary:      onSurfaceVariant
    readonly property color textDisabled:       onSurfaceDisabled
    readonly property color textOnSettings:     onSurfaceSettings
    readonly property color textDimOnSettings:  onSurfaceSettingsVariant
    readonly property color textNoteOnSettings: onSurfaceSettingsSubtle

    // Status / semantic colours
    readonly property color statusOk:       success
    readonly property color statusWait:     warning
    readonly property color statusBad:      error
    readonly property color statusAction:   active
    readonly property color statusNeutral:  onSurfaceDisabled
    readonly property color statusBgOk:     successSurface
    readonly property color statusBgBad:    errorSurface
    readonly property color okColor:        success
    readonly property color waitColor:      warning
    readonly property color badColor:       error
    readonly property color actionColor:    active
    readonly property color baseColor:      onSurfaceDisabled

    // Popup colours
    readonly property color popupBackground:            surface
    readonly property color popupBackgroundTranslucent: surfaceOverlay
    readonly property color popupBorder:                outlineVariant
    readonly property color popupBorderStrong:          outline
    readonly property color popupOverlayDim:            scrimVeil

    // Overlay
    readonly property color overlaySubtle: listSurface
    readonly property color overlayPress:  listSurfacePressed

    // Settings colours
    readonly property color settingsPageBackground:       surfaceSettings
    readonly property color settingsSurface:              surfaceSettingsContainer
    readonly property color settingsBorder:               outlineSettings
    readonly property color settingsFooter:               surfaceSettingsFooter
    readonly property color settingsFooterBorder:         outlineSettingsFooter
    readonly property color settingsTabBar:               surfaceSettingsTabBar
    readonly property color backgroundColor:              surface
    readonly property color primaryBackgroundPalette:     surface
    readonly property color primaryBackgroundColor:       surface
    readonly property color secondaryBackgroundPalette:   primary
    readonly property color settingsBackgroundPalette:    surfaceSettings
    readonly property color settingsPopupBackgroundColor: surfaceOverlay
    readonly property color primaryTextColor:             onSurface
    readonly property color settingTabBarColor:           surfaceSettingsTabBar
    readonly property color settingsTabBarPalette:        surfaceSettingsTabBar
    readonly property color settingFooterPalette:         surfaceSettingsFooter
    readonly property color settingFooterColor:           surfaceSettingsFooter
    readonly property color paletteBackground:            surface
    readonly property color palettePrimary:               primary
    readonly property color paletteSecondary:             brand2
    readonly property color paletteSettingsBg:            surfaceSettings
    readonly property color paletteSettingsFg:            onSurfaceSettings

    // Button colours
    readonly property color btnNormalBg:                  btnSurface
    readonly property color btnNormalBgPressed:           btnSurfacePressed
    readonly property color btnNormalFg:                  btnContent
    readonly property color btnNormalFgPressed:           btnContentPressed
    readonly property color btnDangerBg:                  btnDangerSurface
    readonly property color btnDangerBgPressed:           btnDangerSurfacePressed
    readonly property color btnDangerFg:                  btnDangerContent
    readonly property color btnDangerFgPressed:           withAlpha(error, 0.67)
    readonly property color btnConfirmBg:                 btnConfirmSurface
    readonly property color btnConfirmBgPressed:          btnConfirmSurfacePressed
    readonly property color btnConfirmFg:                 btnConfirmContent
    readonly property color btnConfirmFgPressed:          withAlpha(onSuccessContainer, 0.80)
    readonly property color btnActionBg:                  btnActionSurface
    readonly property color btnActionBgPressed:           btnActionSurfacePressed
    readonly property color btnActionFg:                  btnActionContent
    readonly property color btnCancelBorder:              btnCancelOutline
    readonly property color btnCancelFg:                  btnCancelContent
    readonly property color buttonBackgroundPalette:      onSurface
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
    readonly property color sliderPrimaryPalette:        primaryContainer
    readonly property color sliderPrimaryColor:          sliderActiveTrackLow
    readonly property color sliderSecondaryPalette:      brand2
    readonly property color sliderAlternateColor:        sliderActiveTrackHigh
    readonly property color sliderBackgroundColor:       sliderInactiveTrack
    readonly property color sliderPopupBackgroundColor:  sliderPanelSurface

    // Android Auto overlay
    readonly property color aaHomeBtnBg:     aaHomeSurface
    readonly property color aaHomeBtnBorder: aaHomeOutline
}
