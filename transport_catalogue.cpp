#include "transport_catalogue.h"
#include <execution>
 
namespace transport_catalogue {  
    
void TransportCatalogue::AddStop(const Stop& stop_) {
    stops_.push_back(stop_);
    Stop* stop_buf = &stops_.back();
    stopname_to_stop_.insert(transport_catalogue::StopMap::value_type(stop_buf->name, stop_buf));
}
 
void TransportCatalogue::AddBus(const Bus& bus_) {
    Bus* bus_buf;

    buses_.push_back(bus_);
    bus_buf = &buses_.back();
    busname_to_bus_.insert(BusMap::value_type(bus_buf->name, bus_buf));

    for (Stop* stop : bus_buf->stops) {
        stop->buses.push_back(bus_buf);
    }
}
     
void TransportCatalogue::AddDistance(const std::vector<Distance>& distances) {
    for (auto &distance : distances){
        auto dist_pair = std::make_pair(distance.A, distance.B);
        distance_to_stop_.insert(DistanceMap::value_type(dist_pair, distance.distance));
    }
}
 
Bus* TransportCatalogue::GetBus(std::string_view bus_name) {   
    if (busname_to_bus_.count(bus_name)) {
        return busname_to_bus_.at(bus_name);
    }
    else
        return nullptr;
}
 
Stop* TransportCatalogue::GetStop(std::string_view stop_name) {
    if (stopname_to_stop_.count(stop_name)) {
        return stopname_to_stop_.at(stop_name);
    }
    else
        return nullptr;
}
    
BusMap TransportCatalogue::GetBusnameToBus() const {
    return busname_to_bus_;
}
    
StopMap TransportCatalogue::GetStopnameToStop() const {
    return stopname_to_stop_;
}
 
std::unordered_set<const Stop*> TransportCatalogue::GetUniqStops(const Bus* bus) {
    std::unordered_set<const Stop*> unique_stops;
    
    unique_stops.insert(bus->stops.begin(), bus->stops.end());
    
    return unique_stops;
}
 
double TransportCatalogue::GetGeoLength(Bus* bus) {
    double length = 0.0;
    for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
        auto from = bus->stops[i];
        auto to = bus->stops[i + 1];
        length += geo::ComputeDistance({ from->coordinates.latitude, from->coordinates.longitude }, { to->coordinates.latitude, to->coordinates.longitude });
    }

    return length;
}
 
std::unordered_set<const Bus*> TransportCatalogue::StopGetUniqBuses(const Stop* stop){    
    std::unordered_set<const Bus*> unique_stops;    
      
    unique_stops.insert(stop->buses.begin(), stop->buses.end());   
    
    return unique_stops;
}
 
size_t TransportCatalogue::GetDistanceStop(const Stop* from, const Stop* to) {
    if (distance_to_stop_.count({ from,to })) {
        return distance_to_stop_.at({ from,to });
    }
    else if (distance_to_stop_.count({ to,from })) {
        return distance_to_stop_.at({ to,from });
    }
    else return 0;
}
 
size_t TransportCatalogue::GetDistanceToBus(const Bus* bus) {
    size_t distance = 0;
    auto stops_size = bus->stops.size() - 1;
    for (int i = 0; i < static_cast<int>(stops_size); ++i) {
        distance += GetDistanceStop(bus->stops[i], bus->stops[i+1]);
    }
    return distance;
}
    
}//end namespace transport_catalogue
