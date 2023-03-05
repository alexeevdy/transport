#pragma once

#include "descriptions.h"
#include "svg.h"

#include <memory>

namespace Data {

    struct Database {
        Descriptions::DictBus bus_descriptions;
        Descriptions::DictStop stop_descriptions;
        Descriptions::AdjacentStops adjacent_stops;
        std::unordered_map<std::string, Svg::Color> bus_colors;
    };

    using DataPtr = std::shared_ptr<Database>;

};