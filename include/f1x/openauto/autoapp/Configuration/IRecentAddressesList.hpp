#pragma once

#include <deque>
#include <string>

namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace configuration {

        class IRecentAddressesList {
        public:
          typedef std::deque<std::string> RecentAddresses;

          virtual void read() = 0;

          virtual void insertAddress(const std::string &address) = 0;

          virtual RecentAddresses getList() const = 0;
        };

      }
    }
  }
}
