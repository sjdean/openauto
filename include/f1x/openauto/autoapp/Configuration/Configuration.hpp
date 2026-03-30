#pragma once
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <fstream>

namespace f1x::openauto::autoapp::configuration {
    class Configuration : public IConfiguration {
    public:
        Configuration();

        bool hasTouchScreen() const override;

        void save() const override;

        void onSettingChanged(const QString& group, const QString& key, const QVariant& value) override;

    private:
        static QString generateRandomString(size_t length);

        mutable QSettings m_settings;

        mutable std::optional<bool> hasTouchScreenCache_;
    };
}
