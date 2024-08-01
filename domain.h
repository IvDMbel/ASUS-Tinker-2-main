#pragma once
 
#include <algorithm>
#include <vector>
#include <string>
 
#include "geo.h"
 
namespace domain {
 
struct StatRequest { 
    int id;
    std::string type;
    std::string name;    
};
    
struct Bus;
    
struct Stop {    
    std::string name;
    geo::Coordinates coordinates;
    std::vector<Bus*> buses;
};
 
struct Bus { 
    std::string name;
    std::vector<Stop*> stops;
    bool is_roundtrip;
};
 
struct Distance {    
    const Stop* A;
    const Stop* B;
    int distance = 0;
};  
 
struct BusInfoRoute {
    std::string_view name;
    bool not_found = false;
    int stops_on_route = 0;
    int unique_stops = 0;
    int route_length = 0;
    double curvature = 0.0;
};    
   
struct StopInfo {
    std::string_view name;
    bool not_found;
    std::vector <std::string> buses_name;
};
 
}//end namespace domain
