#define _USE_MATH_DEFINES
#include "geo.h"
 
#include <cmath>
 
namespace geo {
    
double ComputeDistance(Coordinates A, Coordinates B) {
    using namespace std;
    
    if (!(A == B)) {
        const double dr = PI / 180.;
        return acos(sin(A.latitude * dr) * sin(B.latitude * dr)
                    + cos(A.latitude * dr) * cos(B.latitude * dr) 
                    * cos(abs(A.longitude - B.longitude) * dr)) * EARTH_RADIUS;
    } else {
        return 0.0;
    }
}
    
}//end namespace geo
