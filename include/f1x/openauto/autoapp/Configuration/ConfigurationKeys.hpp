#pragma once
#include <QString>

namespace f1x::openauto::autoapp::configuration {

    // -------------------------------------------------------------------------
    // Groups  (map to QSettings section names / INI groups)
    // -------------------------------------------------------------------------
    enum class ConfigGroup {
        Car,
        Screen,
        Video,
        Audio,
        Media,
        AndroidAuto,
        Bluetooth,
        System,
        Wireless,
    };

    // -------------------------------------------------------------------------
    // Keys  (map to QSettings key names within their group)
    // Names are prefixed with their group to avoid collisions between groups
    // that share a common key name (e.g. Enabled, Type).
    // -------------------------------------------------------------------------
    enum class ConfigKey {
        // Car ----------------------------------------------------------------
        CarMake,
        CarModel,
        CarFuelType,
        CarEvConnectorType,
        CarDriverPosition,
        CarId,

        // Screen -------------------------------------------------------------
        ScreenDayMin,
        ScreenDayMax,
        ScreenNightMin,
        ScreenNightMax,
        ScreenBrightness,
        ScreenDPI,
        ScreenUseClockDayNight,
        ScreenDarkMode,
        ScreenAccentPrimary,     // stored as empty string = use M3 default
        ScreenAccentBrand2,
        ScreenButtonOpacity,     // stored as double, default 0.33

        // Video --------------------------------------------------------------
        VideoHeight,
        VideoWidth,
        VideoRotate,

        // Audio --------------------------------------------------------------
        AudioPlaybackMin,
        AudioPlaybackMax,
        AudioCaptureMin,
        AudioCaptureMax,
        AudioPlaybackVolume,
        AudioCaptureVolume,
        AudioDebugRecord,
        AudioPlaybackDevice,
        AudioCaptureDevice,

        // Media --------------------------------------------------------------
        MediaAutoPlayback,
        MediaAutoStart,

        // AndroidAuto --------------------------------------------------------
        AndroidAutoMedia,
        AndroidAutoGuidance,
        AndroidAutoTelephony,
        AndroidAutoFrameRate,
        AndroidAutoResolution,
        AndroidAutoHomeButtonVisibility,

        // Bluetooth ----------------------------------------------------------
        BluetoothEnabled,
        BluetoothAdapterAddress,
        BluetoothPairedDeviceAddress,
        BluetoothIgnoredDevices,

        // System -------------------------------------------------------------
        SystemHeadUnitMode,
        SystemDesktopMode,

        // Wireless -----------------------------------------------------------
        WirelessEnabled,
        WirelessHotspotSSID,
        WirelessHotspotPassword,
        WirelessClientSSID,
        WirelessClientPassword,
        WirelessInterface,
        WirelessInterfaceMAC,
        WirelessType,
    };

    // -------------------------------------------------------------------------
    // String converters  (returns the exact string stored in the INI file)
    // -------------------------------------------------------------------------
    inline QString toQString(ConfigGroup group) {
        switch (group) {
            case ConfigGroup::Car:         return QStringLiteral("Car");
            case ConfigGroup::Screen:      return QStringLiteral("Screen");
            case ConfigGroup::Video:       return QStringLiteral("Video");
            case ConfigGroup::Audio:       return QStringLiteral("Audio");
            case ConfigGroup::Media:       return QStringLiteral("Media");
            case ConfigGroup::AndroidAuto: return QStringLiteral("AndroidAuto");
            case ConfigGroup::Bluetooth:   return QStringLiteral("Bluetooth");
            case ConfigGroup::System:      return QStringLiteral("System");
            case ConfigGroup::Wireless:    return QStringLiteral("Wireless");
        }
        return {};
    }

    inline QString toQString(ConfigKey key) {
        switch (key) {
            case ConfigKey::CarMake:                         return QStringLiteral("Make");
            case ConfigKey::CarModel:                        return QStringLiteral("Model");
            case ConfigKey::CarFuelType:                     return QStringLiteral("FuelType");
            case ConfigKey::CarEvConnectorType:              return QStringLiteral("EvConnectorType");
            case ConfigKey::CarDriverPosition:               return QStringLiteral("DriverPosition");
            case ConfigKey::CarId:                           return QStringLiteral("Id");
            case ConfigKey::ScreenDayMin:                    return QStringLiteral("DayMin");
            case ConfigKey::ScreenDayMax:                    return QStringLiteral("DayMax");
            case ConfigKey::ScreenNightMin:                  return QStringLiteral("NightMin");
            case ConfigKey::ScreenNightMax:                  return QStringLiteral("NightMax");
            case ConfigKey::ScreenBrightness:                return QStringLiteral("Brightness");
            case ConfigKey::ScreenDPI:                       return QStringLiteral("DPI");
            case ConfigKey::ScreenUseClockDayNight:          return QStringLiteral("UseClockDayNight");
            case ConfigKey::ScreenDarkMode:                  return QStringLiteral("DarkMode");
            case ConfigKey::ScreenAccentPrimary:             return QStringLiteral("AccentPrimary");
            case ConfigKey::ScreenAccentBrand2:              return QStringLiteral("AccentBrand2");
            case ConfigKey::ScreenButtonOpacity:             return QStringLiteral("ButtonOpacity");
            case ConfigKey::VideoHeight:                     return QStringLiteral("Height");
            case ConfigKey::VideoWidth:                      return QStringLiteral("Width");
            case ConfigKey::VideoRotate:                     return QStringLiteral("Rotate");
            case ConfigKey::AudioPlaybackMin:                return QStringLiteral("PlaybackMin");
            case ConfigKey::AudioPlaybackMax:                return QStringLiteral("PlaybackMax");
            case ConfigKey::AudioCaptureMin:                 return QStringLiteral("CaptureMin");
            case ConfigKey::AudioCaptureMax:                 return QStringLiteral("CaptureMax");
            case ConfigKey::AudioPlaybackVolume:             return QStringLiteral("PlaybackVolume");
            case ConfigKey::AudioCaptureVolume:              return QStringLiteral("CaptureVolume");
            case ConfigKey::AudioDebugRecord:                return QStringLiteral("DebugRecord");
            case ConfigKey::AudioPlaybackDevice:             return QStringLiteral("PlaybackDevice");
            case ConfigKey::AudioCaptureDevice:              return QStringLiteral("CaptureDevice");
            case ConfigKey::MediaAutoPlayback:               return QStringLiteral("AutoPlayback");
            case ConfigKey::MediaAutoStart:                  return QStringLiteral("AutoStart");
            case ConfigKey::AndroidAutoMedia:                return QStringLiteral("Media");
            case ConfigKey::AndroidAutoGuidance:             return QStringLiteral("Guidance");
            case ConfigKey::AndroidAutoTelephony:            return QStringLiteral("Telephony");
            case ConfigKey::AndroidAutoFrameRate:            return QStringLiteral("FrameRate");
            case ConfigKey::AndroidAutoResolution:           return QStringLiteral("Resolution");
            case ConfigKey::AndroidAutoHomeButtonVisibility: return QStringLiteral("HomeButtonVisibility");
            case ConfigKey::BluetoothEnabled:                return QStringLiteral("Enabled");
            case ConfigKey::BluetoothAdapterAddress:         return QStringLiteral("AdapterAddress");
            case ConfigKey::BluetoothPairedDeviceAddress:    return QStringLiteral("PairedDeviceAddress");
            case ConfigKey::BluetoothIgnoredDevices:         return QStringLiteral("IgnoredDevices");
            case ConfigKey::SystemHeadUnitMode:              return QStringLiteral("HeadUnitMode");
            case ConfigKey::SystemDesktopMode:               return QStringLiteral("DesktopMode");
            case ConfigKey::WirelessEnabled:                 return QStringLiteral("Enabled");
            case ConfigKey::WirelessHotspotSSID:             return QStringLiteral("HotspotSSID");
            case ConfigKey::WirelessHotspotPassword:         return QStringLiteral("HotspotPassword");
            case ConfigKey::WirelessClientSSID:              return QStringLiteral("ClientSSID");
            case ConfigKey::WirelessClientPassword:          return QStringLiteral("ClientPassword");
            case ConfigKey::WirelessInterface:               return QStringLiteral("Interface");
            case ConfigKey::WirelessInterfaceMAC:            return QStringLiteral("InterfaceMAC");
            case ConfigKey::WirelessType:                    return QStringLiteral("Type");
        }
        return {};
    }

} // namespace f1x::openauto::autoapp::configuration