//
// Created by Simon Dean on 10/11/2025.
//
#pragma once

#ifndef OPENAUTO_IAUDIOHANDLER_H
#define OPENAUTO_IAUDIOHANDLER_H

#include <QString>
#include <QVariant>
#include <vector>
#include <string>
#include <memory>

// Define common types outside the interface
namespace f1x::openauto::autoapp::UI::Monitor {

    // EngineDevice and EngineDeviceList are part of the interface
    // so they must be defined here in this common header.
    class EngineDevice {
    public:
        explicit EngineDevice() : card(0), device(0) {}
        QString GuessIconName() const {
            // A generic stub implementation
            return QString();
        }
        QString description;
        QString value;
        QString iconname;
        int card;
        int device;
    };
    using EngineDeviceList = QList<EngineDevice>;

    // The abstract audio interface
    class IAudioHandler {
    public:
        virtual ~IAudioHandler() = default;

        virtual QString getDefaultSink() = 0;
        virtual QString getDefaultSource() = 0;

        virtual void setSinkMute(const QString& deviceName, bool mute) = 0;
        virtual void setSinkVolume(const QString& deviceName, int volume) = 0;
        virtual void setSourceMute(const QString& deviceName, bool mute) = 0;
        virtual void setSourceVolume(const QString& deviceName, int volume) = 0;

        virtual EngineDeviceList getSinks() = 0;
        virtual EngineDeviceList getSources() = 0;
        virtual std::vector<std::pair<std::string, std::string>> getDeviceList() = 0;
    };

    // A "do-nothing" stub implementation for non-Linux platforms
    class NullAudioHandler : public IAudioHandler {
    public:
        explicit NullAudioHandler() {}
        ~NullAudioHandler() override = default;

        QString getDefaultSink() override { return QString(); }
        QString getDefaultSource() override { return QString(); }
        void setSinkMute(const QString&, bool) override {}
        void setSinkVolume(const QString&, int) override {}
        void setSourceMute(const QString&, bool) override {}
        void setSourceVolume(const QString&, int) override {}
        EngineDeviceList getSinks() override { return EngineDeviceList(); }
        EngineDeviceList getSources() override { return EngineDeviceList(); }
        std::vector<std::pair<std::string, std::string>> getDeviceList() override { return {}; }
    };

} // namespace f1x::openauto::autoapp::UI::Monitor

#endif //OPENAUTO_IAUDIOHANDLER_H