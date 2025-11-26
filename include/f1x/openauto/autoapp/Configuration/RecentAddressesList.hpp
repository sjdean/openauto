#pragma once

#include <deque>
#include <f1x/openauto/autoapp/Configuration/IRecentAddressesList.hpp>

// TODO: Check if this and its interface are still required
namespace f1x::openauto::autoapp::configuration {
    class RecentAddressesList : public IRecentAddressesList {
    public:
        RecentAddressesList(size_t maxListSize);

        void read() override;

        void insertAddress(const std::string &address) override;

        RecentAddresses getList() const override;

    private:
        void load();

        void save();

        size_t maxListSize_;
        RecentAddresses list_;

        static const std::string cConfigFileName;
        static const std::string cRecentEntiresCount;
        static const std::string cRecentEntryPrefix;
    };
}
