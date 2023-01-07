#pragma once

#include "transport_guide.h"
#include "json.h"

#include <iostream>
#include <memory>
#include <variant>
#include <string>

namespace Requests {

    Json::Node ProcessStop(const TransportGuide &, const Json::Node &);

    Json::Node ProcessBus(const TransportGuide &, const Json::Node &);

    Json::Node ProcessAll(const TransportGuide &, const Json::Node &);

}
