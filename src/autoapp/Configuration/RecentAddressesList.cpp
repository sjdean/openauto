#include <boost/property_tree/ini_parser.hpp>
#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Configuration/RecentAddressesList.hpp>

namespace f1x::openauto::autoapp::configuration
{

const std::string RecentAddressesList::cConfigFileName = "openauto_wifi_recent.ini";
const std::string RecentAddressesList::cRecentEntiresCount = "Recent.EntiresCount";
const std::string RecentAddressesList::cRecentEntryPrefix = "Recent.Entry_";

// TODO: Do we still need this? Or will Wi-Fi information be gathered from NetworkManager???
RecentAddressesList::RecentAddressesList(size_t maxListSize)
    : maxListSize_(maxListSize)
{

}

void RecentAddressesList::read()
{
    this->load();
}

void RecentAddressesList::insertAddress(const std::string& address)
{
    if(std::find(list_.begin(), list_.end(), address) != list_.end())
    {
        return;
    }

    if(list_.size() >= maxListSize_)
    {
        list_.pop_back();
    }

    list_.push_front(address);
    this->save();
}

RecentAddressesList::RecentAddresses RecentAddressesList::getList() const
{
    return list_;
}

void RecentAddressesList::load()
{
    boost::property_tree::ptree iniConfig;

    try
    {
        boost::property_tree::ini_parser::read_ini(cConfigFileName, iniConfig);

        const auto listSize = std::min(maxListSize_, iniConfig.get<size_t>(cRecentEntiresCount, 0));

        for(size_t i = 0; i < listSize; ++i)
        {
            const auto key = cRecentEntryPrefix + std::to_string(i);
            const auto address = iniConfig.get<RecentAddresses::value_type>(key, RecentAddresses::value_type());

            if(!address.empty())
            {
                list_.push_back(address);
            }
        }
    }
    catch(const boost::property_tree::ini_parser_error& e)
    {
        OPENAUTO_LOG(warning) << "[RecentAddressesList] failed to read configuration file: " << cConfigFileName
                            << ", error: " << e.what()
                            << ". Empty list will be used.";
    }
}

void RecentAddressesList::save()
{
    boost::property_tree::ptree iniConfig;

    const auto entiresCount = std::min(maxListSize_, list_.size());
    iniConfig.put<size_t>(cRecentEntiresCount, entiresCount);

    for(size_t i = 0; i < entiresCount; ++i)
    {
        const auto key = cRecentEntryPrefix + std::to_string(i);
        iniConfig.put<RecentAddresses::value_type>(key, list_.at(i));
    }

    boost::property_tree::ini_parser::write_ini(cConfigFileName, iniConfig);
}

}
