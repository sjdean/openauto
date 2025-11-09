#ifndef OPENAUTO_IANDROIDAUTOMONITOR_HPP
#define OPENAUTO_IANDROIDAUTOMONITOR_HPP

#pragma once
#include <string>

namespace f1x::openauto::autoapp::UI::Monitor {

  class IAndroidAutoMonitor {
  public:
    typedef std::shared_ptr<IAndroidAutoMonitor> Pointer;

    virtual ~IAndroidAutoMonitor() = default;

  };
}
#endif//OPENAUTO_IANDROIDAUTOMONITOR_HPP