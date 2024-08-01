#pragma once
#include <deque>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
 
#include "domain.h"
 
using namespace domain;
 
namespace transport_catalogue {   
 
    struct DistanceHasher {
        std::hash<const void*> hasher;
        std::size_t operator()(const std::pair<const Stop*, const Stop*>pair_stops) const noexcept {
            auto hash_first = std::hash<const void*>{}(pair_stops.first);
            auto hash_second = std::hash<const void*>{}(pair_stops.second);
            return hash_first + hash_second * 37;
        }
    };
    
    using StopMap = std::unordered_map<std::string_view, Stop*>;
    using BusMap = std::unordered_map<std::string_view, Bus*>;
    using DistanceMap = std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceHasher>;
 
class TransportCatalogue {
public:      
    void AddBus(const Bus& bus);
    void AddStop(const Stop& stop);
    void AddDistance(const std::vector<Distance>& distances);
    
    Bus* GetBus(std::string_view bus_name);
    Stop* GetStop(std::string_view stop_name);
    BusMap GetBusnameToBus() const;
    StopMap GetStopnameToStop() const;
    
    std::unordered_set<const Bus*> StopGetUniqBuses(const Stop* stop);    
    std::unordered_set<const Stop*> GetUniqStops(const Bus* bus);
    double GetGeoLength(Bus* bus);
    
    size_t GetDistanceStop(const Stop* from, const Stop* to);
    size_t GetDistanceToBus(const Bus* bus);
    
private:    
    std::deque<Stop> stops_;
    StopMap stopname_to_stop_;
    
    std::deque<Bus> buses_;
    BusMap busname_to_bus_;
    
    DistanceMap distance_to_stop_;
    
};
    
}//end namespace transport_catalogue
