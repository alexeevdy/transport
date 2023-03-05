#pragma once

#include <cmath>

namespace Coordinates {

    const double EARTH_DIAMETER = 6'371'000;

    struct Point {
        double latitude;
        double longitude;
    };

    inline double ToRadiance(double degrees) {
        return degrees * M_PI / 180.0;
    }

    inline double DistanceBetween(Point lhs, Point rhs) {
        double lat1 = ToRadiance(lhs.latitude);
        double lat2 = ToRadiance(rhs.latitude);
        double lon1 = ToRadiance(lhs.longitude);
        double lon2 = ToRadiance(rhs.longitude);

        return EARTH_DIAMETER * acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon2 - lon1));
    }

}
