#include "request_handler.h"
 
namespace request_handler {
    
Node RequestHandler::ExecuteMakeNodeStop(int id_request, const StopInfo& stop_info){
    Node result;
    Array buses;
    Builder builder;
 
    std::string str_not_found = "not found";
 
    if (stop_info.not_found) {
        builder.StartDict()
               .Key("request_id").Value(id_request)
               .Key("error_message").Value(str_not_found)
               .EndDict();
        
        result = builder.Build();
        
    } else {
        builder.StartDict()
               .Key("request_id").Value(id_request)
               .Key("buses").StartArray();
 
        for (std::string bus_name : stop_info.buses_name) {
            builder.Value(bus_name);
        }
 
        builder.EndArray().EndDict();
        
        result = builder.Build();
    }
 
    return result;
}
 
Node RequestHandler::ExecuteMakeNodeBus(int id_request, const BusInfoRoute& bus_info){
    Node result;
    std::string str_not_found = "not found";
 
    if (bus_info.not_found) {
        result = Builder{}.StartDict()
                          .Key("request_id").Value(id_request)
                          .Key("error_message").Value(str_not_found)
                          .EndDict()
                          .Build();
    } else {
        result = Builder{}.StartDict()
                          .Key("request_id").Value(id_request)
                          .Key("curvature").Value(bus_info.curvature)
                          .Key("route_length").Value(bus_info.route_length)
                          .Key("stop_count").Value(bus_info.stops_on_route)
                          .Key("unique_stop_count").Value(bus_info.unique_stops)
                          .EndDict()
                          .Build();
    }
 
    return result;
}    
      
Node RequestHandler::ExecuteMakeNodeMap(int id_request, TransportCatalogue& catalogue_, RenderSettings render_settings){
    Node result;
 
    std::ostringstream map_stream;
    std::string map_str;
 
    MapRenderer map_catalogue(render_settings);
    
    map_catalogue.InitSphereProjector(GetStopsCoordinates(catalogue_));
    
    ExecuteRenderMap(map_catalogue, catalogue_);
    map_catalogue.GetStreamMap(map_stream);
    map_str = map_stream.str();
 
    result = Builder{}.StartDict()
                      .Key("request_id").Value(id_request)
                      .Key("map").Value(map_str)
                      .EndDict()
                      .Build();
 
    return result;
}
    
void RequestHandler::ExecuteQueries(TransportCatalogue& catalogue, 
                                     std::vector<StatRequest>& stat_requests, 
                                     RenderSettings& render_settings) {
    std::vector<Node> result_request;
    for (StatRequest req : stat_requests) {
 
        if (req.type == "Stop") {
            result_request.push_back(ExecuteMakeNodeStop(req.id, StopQuery(catalogue, req.name)));
            
        } else if (req.type == "Bus") {
            result_request.push_back(ExecuteMakeNodeBus(req.id, BusQuery(catalogue, req.name)));
            
        } else if (req.type == "Map") {
            result_request.push_back(ExecuteMakeNodeMap(req.id, catalogue, render_settings));
        }
    }
 
    doc_out = Document{Node(result_request)};
}
 
void RequestHandler::ExecuteRenderMap(MapRenderer& map_catalogue, TransportCatalogue& catalogue) const {   
    std::vector<std::pair<Bus*, int>> buses_palette;  
    std::vector<Stop*> stops_sort;
    size_t palette_size = 0;
    size_t palette_index = 0;
    
    palette_size = map_catalogue.GetPaletteSize();
    if (palette_size == 0) {
        std::cout << "color palette is empty";
        return;
    }
 
    auto buses = catalogue.GetBusnameToBus();   
    if (buses.size() > 0) {
        
        for (std::string_view bus_name : GetSortBusesNames(catalogue)) {
            Bus* bus_info = catalogue.GetBus(bus_name);
 
            if (bus_info) {  
                if (bus_info->stops.size() > 0) {
                    buses_palette.push_back(std::make_pair(bus_info, palette_index));
                    palette_index++;
                    
                    if (palette_index == palette_size) {
                        palette_index = 0;
                    }
                }
            }
        }
 
        if (buses_palette.size() > 0) {
            map_catalogue.AddLine(buses_palette);
            map_catalogue.AddBusesName(buses_palette);            
        }          
    }
 
    auto stops = catalogue.GetStopnameToStop();   
    if (stops.size() > 0) {
        std::vector<std::string_view> stops_name;
 
        for (auto& [stop_name, stop] : stops) {
            
            if (stop->buses.size() > 0) {
                stops_name.push_back(stop_name);
            }
        }
        
        std::sort(stops_name.begin(), stops_name.end());
        
        for(std::string_view stop_name : stops_name){
            Stop* stop = catalogue.GetStop(stop_name);
            if(stop){
                stops_sort.push_back(stop);
            }
        }
        
        if (stops_sort.size() > 0) { 
            map_catalogue.AddStopsCircle(stops_sort);
            map_catalogue.AddStopsName(stops_sort);
        }
    }
}
   
std::vector<geo::Coordinates> RequestHandler::GetStopsCoordinates(TransportCatalogue& catalogue_) const {
    
    std::vector<geo::Coordinates> stops_coordinates; 
    auto buses = catalogue_.GetBusnameToBus();
    
    for (auto& [busname, bus] : buses) {
        for (auto& stop : bus->stops) {
            geo::Coordinates coordinates;
            coordinates.latitude = stop->coordinates.latitude;
            coordinates.longitude = stop->coordinates.longitude;
            
            stops_coordinates.push_back(coordinates);
        }
    }
    return stops_coordinates;
}
    
std::vector<std::string_view> RequestHandler::GetSortBusesNames(TransportCatalogue& catalogue_) const {
    std::vector<std::string_view> buses_names;
    
    auto buses = catalogue_.GetBusnameToBus();
    if (buses.size() > 0) {
        
        for (auto& [busname, bus] : buses) {
            buses_names.push_back(busname);
        }   
 
        std::sort(buses_names.begin(), buses_names.end());
        
        return buses_names;
        
    } else {
        return {};
    }
}
 
BusInfoRoute RequestHandler::BusQuery(TransportCatalogue& catalogue, std::string_view bus_name) {
    BusInfoRoute bus_info;
    Bus* bus = catalogue.GetBus(bus_name);
    
    if (bus != nullptr) {                
        bus_info.name = bus->name;
        bus_info.not_found = false;
        bus_info.stops_on_route = static_cast<int>(bus->stops.size());
        bus_info.unique_stops = static_cast<int>(catalogue.GetUniqStops(bus).size());
        bus_info.route_length = static_cast<int>(catalogue.GetDistanceToBus(bus));
        bus_info.curvature = double(catalogue.GetDistanceToBus(bus)
                                    /catalogue.GetGeoLength(bus));                    
    } else {  
        bus_info.name = bus_name;
        bus_info.not_found = true;
    }
    
    return bus_info;
}   
    
StopInfo RequestHandler::StopQuery(TransportCatalogue& catalogue, std::string_view stop_name) {
    std::unordered_set<const Bus*> unique_buses;
    StopInfo stop_info;
    Stop* stop = catalogue.GetStop(stop_name);
 
    if (stop != NULL) {
        
        stop_info.name = stop->name;
        stop_info.not_found = false;
        unique_buses = catalogue.StopGetUniqBuses(stop);
        
        if (unique_buses.size() > 0){
            for (const Bus* bus : unique_buses) {
                stop_info.buses_name.push_back(bus->name);
            }
            
            std::sort(stop_info.buses_name.begin(), stop_info.buses_name.end());
        }   
        
    } else {    
        stop_info.name = stop_name;
        stop_info.not_found = true;
    }
    
    return stop_info;
}
    
const Document& RequestHandler::GetDocument() {return doc_out;}
    
}//end namespace request_handler
