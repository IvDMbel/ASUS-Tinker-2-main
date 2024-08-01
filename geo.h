#pragma once
#include <cmath>
 
namespace geo {
    
const int EARTH_RADIUS = 6371000;
const double PI = 3.1415926535;
    
struct Coordinates {
    double latitude = 0.0;
    double longitude = 0.0;
 
    bool operator==(const Coordinates& other) const {
        return latitude == other.latitude && longitude == other.longitude;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};
    
double ComputeDistance(Coordinates A, Coordinates B);
    
}//end namespace geo
