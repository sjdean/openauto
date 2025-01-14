#pragma once
#include <boost/log/trivial.hpp>

#define OPENAUTO_LOG_CONTEXT "" //"(" << typeid(*this).name() << "::" << __func__ << ")"
#define OPENAUTO_LOG(severity) BOOST_LOG_TRIVIAL(severity) << "[OpenAuto] " << OPENAUTO_LOG_CONTEXT
