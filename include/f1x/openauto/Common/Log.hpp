#pragma once
#include <boost/log/trivial.hpp>

// TODO: Think about getting rid of OPENAUTO_LOG so we can switch to Qt logging.

#define OPENAUTO_LOG_CONTEXT "" //"(" << typeid(*this).name() << "::" << __func__ << ")"
#define OPENAUTO_LOG(severity) BOOST_LOG_TRIVIAL(severity) << "[OpenAuto] " << OPENAUTO_LOG_CONTEXT
