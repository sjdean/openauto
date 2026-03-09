pragma Singleton
import QtQuick
import QtQuick.Studio.Application

// ─────────────────────────────────────────────────────────────────────────────
// JourneyOS Design Tokens
// ─────────────────────────────────────────────────────────────────────────────
// All colours, font sizes, and border radii for the UI live here.
//
// Future theming road-map
//   Phase 1 (now)  — single dark palette, centralised tokens
//   Phase 2        — Light / Dark mode: swap palette root values below
//   Phase 3        — User colour palettes: expose palette roots as settings
//
// Consumers should reference semantic aliases (e.g. Constants.textPrimary)
// rather than palette roots (e.g. Constants.paletteDark) so that switching
// palette roots automatically re-themes the whole UI.
// ─────────────────────────────────────────────────────────────────────────────

QtObject {

    // ── APPLICATION DIMENSIONS ───────────────────────────────────────────────
    readonly property int width:  800
    readonly property int height: 480

    // ── BORDER RADII ─────────────────────────────────────────────────────────
    // Reference these everywhere rather than hardcoding numbers.
    readonly property int radiusPopup:   10   // Full-screen popup windows & overlay cards
    readonly property int radiusCard:     8   // Inset cards / panels within popups
    readonly property int radiusButton:   5   // Standard JourneyButton background
    readonly property int radiusInput:    4   // Text fields, status strips, list items
    readonly property int radiusCircle:  14   // Circular elements (28 × 28 close button)
    readonly property int radiusSlider:  10   // Slider track and groove

    // ── TYPOGRAPHY ───────────────────────────────────────────────────────────
    // Standardised pixel sizes — reference these rather than hardcoding numbers.
    readonly property int fontTitle:    20   // Popup / page title  e.g. "Bluetooth"
    readonly property int fontHeading:  18   // Sub-page / dialog heading
    readonly property int fontSubtitle: 16   // Section headings within a page
    readonly property int fontBody:     14   // Primary body text, device names
    readonly property int fontLabel:    13   // Field labels, secondary text
    readonly property int fontCaption:  12   // Small info text, status strips
    readonly property int fontSmall:    11   // Metadata / smallest text

    property string relativeFontDirectory: "fonts"

    readonly property font font: Qt.font({
        family: Qt.application.font.family,
        pixelSize: Qt.application.font.pixelSize
    })
    readonly property font largeFont: Qt.font({
        family: Qt.application.font.family,
        pixelSize: Qt.application.font.pixelSize * 1.6
    })

    // ── PALETTE ROOTS ────────────────────────────────────────────────────────
    // These are the levers for future Light / Dark mode and user palettes.
    // Swap these values to re-theme the entire application.
    readonly property color paletteBackground:  "#000000"   // App window / main bg (dark)
    readonly property color palettePrimary:     "#7900B5"   // Brand accent — purple
    readonly property color paletteSecondary:   "#13A8FD"   // Brand accent 2 — blue
    readonly property color paletteSettingsBg:  "#A3CAED"   // Settings page tint (light)
    readonly property color paletteSettingsFg:  "#3A4856"   // Text on settings (light) bg

    // ── TEXT COLOURS ─────────────────────────────────────────────────────────
    readonly property color textPrimary:        "#FFFFFF"   // Primary text on dark bg
    readonly property color textSecondary:      "#BBBBBB"   // Dim / secondary on dark bg
    readonly property color textDisabled:       "#888888"   // Disabled / placeholder
    readonly property color textOnSettings:     "#3A4856"   // Text on light settings bg
    readonly property color textDimOnSettings:  "#9DADBC"   // Dim text on light settings bg
    readonly property color textNoteOnSettings: "#536878"   // Readable info/note text on light settings bg

    // ── STATUS / SEMANTIC COLOURS ─────────────────────────────────────────────
    readonly property color statusOk:           "#009900"   // Connected / good
    readonly property color statusWait:         "#999900"   // Waiting / indeterminate
    readonly property color statusBad:          "#990000"   // Disconnected / error
    readonly property color statusAction:       "#000099"   // Connecting / in-progress
    readonly property color statusNeutral:      "#999999"   // Neutral / base

    // Translucent status backgrounds (behind status indicator rows)
    readonly property color statusBgOk:         "#22009900"
    readonly property color statusBgBad:        "#22990000"

    // ── POPUP COLOURS ─────────────────────────────────────────────────────────
    // All overlaid popups (Bluetooth, Wi-Fi, AA info, Power) use the dark style.
    readonly property color popupBackground:            "#000000"   // Fully opaque fallback
    readonly property color popupBackgroundTranslucent: "#E6000000" // 90 % opaque dark
    readonly property color popupBorder:                "#555555"   // Subtle border ring
    readonly property color popupBorderStrong:          "#888888"   // More visible border
    readonly property color popupOverlayDim:            "#CC000000" // 80 % black modal veil

    // Utility overlays on dark backgrounds (scan list bg, AP press highlight)
    readonly property color overlaySubtle:              "#22FFFFFF"
    readonly property color overlayPress:               "#44FFFFFF"

    // ── SETTINGS PAGE COLOURS (light-tinted panel) ───────────────────────────
    // The settings view uses a distinct light/glass aesthetic.
    readonly property color settingsPageBackground:     "#A3CAED"
    readonly property color settingsSurface:            "#EBFBFF"
    readonly property color settingsBorder:             "#3A4856"
    readonly property color settingsFooter:             "#D9FFFFFF"
    readonly property color settingsFooterBorder:       "#E0E0E0"
    readonly property color settingsTabBar:             "#80FFFFFF"

    // ── BUTTON COLOURS ────────────────────────────────────────────────────────
    // Default / neutral button
    readonly property color btnNormalBg:            "#55999999"  // 33 % grey
    readonly property color btnNormalBgPressed:     "#BB999999"  // 73 % grey on press
    readonly property color btnNormalFg:            "#FFFFFF"
    readonly property color btnNormalFgPressed:     "#80FFFFFF"  // dimmed on press

    // Close / Cancel / Danger  — dark red background, vivid red label / icon
    readonly property color btnDangerBg:            "#CC660000"  // dark red, 80 % opaque
    readonly property color btnDangerBgPressed:     "#CC990000"  // brighter red on press
    readonly property color btnDangerFg:            "#FF6666"    // vivid red  ✕
    readonly property color btnDangerFgPressed:     "#FFAAAA"    // lighter on press

    // Confirm / Accept / Save — dark green background, white label / tick
    // Used for: Save Changes, OK, Accept
    readonly property color btnConfirmBg:           "#CC003300"  // dark green, 80 % opaque
    readonly property color btnConfirmBgPressed:    "#CC006600"  // brighter on press
    readonly property color btnConfirmFg:           "#FFFFFF"    // white  ✓
    readonly property color btnConfirmFgPressed:    "#CCFFCC"    // tinted on press

    // Action / Primary — deep blue background, white label
    // Used for: Reboot, Submit, primary non-destructive actions
    readonly property color btnActionBg:            "#CC1A0080"  // deep blue, 80 % opaque
    readonly property color btnActionBgPressed:     "#CC2A00C0"  // brighter on press
    readonly property color btnActionFg:            "#FFFFFF"    // white text

    // Cancel — transparent outline, red border + text
    // Used for: Cancel, Back (as dismiss), outline-only destructive hint
    readonly property color btnCancelBorder:        "#CC3333"    // vivid red outline
    readonly property color btnCancelFg:            "#CC3333"    // vivid red text

    // ── SLIDER COLOURS ────────────────────────────────────────────────────────
    // Volume slider: interpolates purple (low) ↔ blue (high)
    readonly property color sliderVolumeMin:        "#CC2D004D"  // dark purple — low vol
    readonly property color sliderVolumeMax:        "#D113A8FD"  // vivid blue  — high vol

    // Brightness slider: single-hue intensity variation (dark → lighter purple)
    readonly property color sliderBrightnessLow:    "#CC2D004D"  // darker  — dim screen
    readonly property color sliderBrightnessHigh:   "#AB381353"  // softer  — bright screen

    // Shared slider track / handle
    readonly property color sliderTrackBg:          "#77BDBEBF"  // translucent grey track
    readonly property color sliderHandle:           "#F6F6F6"    // handle fill (idle)
    readonly property color sliderHandleBorder:     "#BDBEBF"    // handle border (idle)
    readonly property color sliderHandlePressed:    "#2D004D"    // handle fill (pressed)

    // Slider popup side-panel background (volume / brightness panels)
    readonly property color sliderPopupBg:          "#AA000000"  // 67 % opaque dark

    // ── ANDROID AUTO OVERLAY ──────────────────────────────────────────────────
    readonly property color aaHomeBtnBg:            "#80000000"  // 50 % black
    readonly property color aaHomeBtnBorder:        "#40FFFFFF"  // subtle white ring

    // ── BACKWARD-COMPATIBILITY ALIASES ───────────────────────────────────────
    // Keep old property names working during migration to semantic aliases.
    // Do not add new code using these — prefer the semantic names above.

    readonly property color backgroundColor:                paletteBackground
    readonly property color primaryBackgroundPalette:       paletteBackground
    readonly property color primaryBackgroundColor:         paletteBackground
    readonly property color secondaryBackgroundPalette:     palettePrimary
    readonly property color settingsBackgroundPalette:      paletteSettingsBg

    readonly property color settingsPopupBackgroundColor:   popupBackgroundTranslucent
    readonly property color primaryTextColor:               textPrimary

    readonly property color okColor:                        statusOk
    readonly property color waitColor:                      statusWait
    readonly property color badColor:                       statusBad
    readonly property color actionColor:                    statusAction
    readonly property color baseColor:                      statusNeutral

    readonly property color sliderPrimaryPalette:           "#2D004D"
    readonly property color sliderPrimaryColor:             sliderVolumeMin
    readonly property color sliderSecondaryPalette:         "#13A8FD"
    readonly property color sliderAlternateColor:           sliderVolumeMax
    readonly property color sliderBackgroundColor:          sliderTrackBg
    readonly property color sliderPopupBackgroundColor:     sliderPopupBg

    readonly property color buttonBackgroundPalette:        "#999999"
    readonly property color buttonBackgroundColor:          btnNormalBg
    readonly property color buttonPressedBackgroundColor:   btnNormalBgPressed
    readonly property color buttonColor:                    btnNormalFg
    readonly property color buttonPressedColor:             btnNormalFgPressed

    readonly property color settingTabBarColor:             settingsTabBar
    readonly property color settingsTabBarPalette:          "#00CCCC"
    readonly property color settingFooterPalette:           "#7A1010"
    readonly property color settingFooterColor:             "#557A1010"

    // ── STUDIO APPLICATION ────────────────────────────────────────────────────
    property StudioApplication application: StudioApplication {
        fontPath: Qt.resolvedUrl("../JourneyOSContent/" + relativeFontDirectory)
    }
}