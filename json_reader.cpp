#include "json_reader.h"
 
namespace transport_catalogue {
namespace detail {
namespace json {
    
JSONReader::JSONReader(Document doc) : document_(std::move(doc)) {}
JSONReader::JSONReader(std::istream& input) : document_(json::Load(input)) {}    
    
Stop JSONReader::ParseNodeStop(Node& node) {
    Stop stop;
    Dict stop_node;
    
    if (node.IsDict()) {
        stop_node = node.AsDict();
        stop.name = stop_node.at("name").AsString();
        stop.coordinates.latitude = stop_node.at("latitude").AsDouble();
        stop.coordinates.longitude = stop_node.at("longitude").AsDouble();
    }
    
    return stop;
}
    
std::vector<Distance> JSONReader::ParseNodeDistances(Node& node, TransportCatalogue& catalogue) {
    std::vector<Distance> distances;
    Dict stop_node;
    Dict stop_road_map;
    std::string begin_name;
    std::string last_name;
    int distance;
    
    if (node.IsDict()) {
        stop_node = node.AsDict();
        begin_name = stop_node.at("name").AsString();
        
        try {
            stop_road_map = stop_node.at("road_distances").AsDict();
            
            for (auto &[key, value] : stop_road_map) {
                last_name = key;
                distance = value.AsInt();
                distances.push_back({catalogue.GetStop(begin_name), 
                                     catalogue.GetStop(last_name), distance});
            } 
            
        } catch(...) {
            std::cout << "invalide road" << std::endl;
        }        
    }
    
    return distances;
}
 
Bus JSONReader::ParseNodeBus(Node& node, TransportCatalogue& catalogue) {
    Bus bus;
    Dict bus_node;
    Array bus_stops;
    
    if (node.IsDict()) {
        bus_node = node.AsDict();
        bus.name = bus_node.at("name").AsString();
        bus.is_roundtrip = bus_node.at("is_roundtrip").AsBool();
 
        try {
            bus_stops = bus_node.at("stops").AsArray();
            
            for (Node stop : bus_stops) {
                bus.stops.push_back(catalogue.GetStop(stop.AsString()));
            }
 
            if (!bus.is_roundtrip) {
                size_t size = bus.stops.size() - 1;
                
                for (size_t i = size; i > 0; i--) {
                    bus.stops.push_back(bus.stops[i-1]);
                }
                
            } 
            
        } catch(...) {
            std::cout << "base_requests: bus: stops is empty" << std::endl;
        }        
    }
    
    return bus;
}
    
void JSONReader::ParseNodeBase(const Node& root, TransportCatalogue& catalogue){
    Array base_requests;
    Dict req_map;
    Node req_node;
    
    std::vector<Node> buses;
    std::vector<Node> stops;
    
    if (root.IsArray()) {
        base_requests = root.AsArray();
        
        for (Node& node : base_requests) {            
            if (node.IsDict()) {
                req_map = node.AsDict();
                
                try {
                    req_node = req_map.at("type");
                    
                    if (req_node.IsString()) {
                        
                        if (req_node.AsString() == "Bus") {
                            buses.push_back(req_map);
                        } else if (req_node.AsString() == "Stop") {
                            stops.push_back(req_map);
                        } else {
                            std::cout << "base_requests are invalid";
                        }
                    }
                    
                } catch(...) {
                    std::cout << "base_requests does not have type value";
                }                
            }
        }   
        
        for (auto &stop : stops) {
            catalogue.AddStop(ParseNodeStop(stop));
        }
        
        for (auto &stop : stops) {
            catalogue.AddDistance(ParseNodeDistances(stop, catalogue));
        }
        
        for (auto &bus : buses) {
            catalogue.AddBus(ParseNodeBus(bus, catalogue));
        }
        
    } else {
        std::cout << "base_requests is not an array";
    }  
}
 
void JSONReader::ParseNodeStat(const Node& node, std::vector<StatRequest>& stat_request){
    Array stat_requests;
    Dict req_map;
    StatRequest req;
    
    if (node.IsArray()) {
        stat_requests = node.AsArray();
        
        for (Node req_node : stat_requests) {
            
            if (req_node.IsDict()) {
                req_map = req_node.AsDict();
                req.id = req_map.at("id").AsInt();
                req.type = req_map.at("type").AsString();
 
                if (req.type != "Map") {
                    req.name = req_map.at("name").AsString();
                } else {
                    req.name = "";
                }
                
                stat_request.push_back(req);
                
            }   
        } 
        
    } else {
        std::cout << "base_requests is not array";
    }
}
  
void JSONReader::ParseNodeRender(const Node& node, map_renderer::RenderSettings& rend_set){
    Dict rend_map;
    Array bus_lab_offset;
    Array stop_lab_offset;
    Array arr_color;
    Array arr_palette;
    int red_;
    int green_;
    int blue_;
    double opacity_;
 
    if (node.IsDict()) {
        rend_map = node.AsDict();
        
        try {
            rend_set.width_ = rend_map.at("width").AsDouble();
            rend_set.height_ = rend_map.at("height").AsDouble();
            rend_set.padding_ = rend_map.at("padding").AsDouble();
            rend_set.line_width_ = rend_map.at("line_width").AsDouble();
            rend_set.stop_radius_ = rend_map.at("stop_radius").AsDouble();
            
            rend_set.bus_label_font_size_ = rend_map.at("bus_label_font_size").AsInt();
            
            if (rend_map.at("bus_label_offset").IsArray()) {
                bus_lab_offset = rend_map.at("bus_label_offset").AsArray();
                rend_set.bus_label_offset_ = std::make_pair(bus_lab_offset[0].AsDouble(),
                                                            bus_lab_offset[1].AsDouble());
            }
            
            rend_set.stop_label_font_size_ = rend_map.at("stop_label_font_size").AsInt();
 
            if (rend_map.at("stop_label_offset").IsArray()) {
                stop_lab_offset = rend_map.at("stop_label_offset").AsArray();
                rend_set.stop_label_offset_ = std::make_pair(stop_lab_offset[0].AsDouble(),
                                                             stop_lab_offset[1].AsDouble());
            }
            
            if (rend_map.at("underlayer_color").IsString()) {
                rend_set.underlayer_color_ = svg::Color(rend_map.at("underlayer_color").AsString());
            } else if (rend_map.at("underlayer_color").IsArray()) {
                arr_color = rend_map.at("underlayer_color").AsArray();
                red_ = arr_color[0].AsInt();
                green_ = arr_color[1].AsInt();
                blue_ = arr_color[2].AsInt();
 
                if(arr_color.size() == 4){
                    opacity_ = arr_color[3].AsDouble();
                    rend_set.underlayer_color_ = svg::Color(svg::Rgba(red_, 
                                                                      green_, 
                                                                      blue_, 
                                                                      opacity_));
                } else if (arr_color.size() == 3) {
                    rend_set.underlayer_color_ = svg::Color(svg::Rgb(red_, 
                                                                     green_, 
                                                                     blue_));
                }
                
            }
 
            rend_set.underlayer_width_ = rend_map.at("underlayer_width").AsDouble();
 
            if (rend_map.at("color_palette").IsArray()) {
                arr_palette = rend_map.at("color_palette").AsArray();
                
                for (Node color_palette : arr_palette) {
                    
                    if (color_palette.IsString()) {
                        rend_set.color_palette_.push_back(svg::Color(color_palette.AsString()));
                    } else if (color_palette.IsArray()) {
                        arr_color = color_palette.AsArray();
                        red_ = arr_color[0].AsInt();
                        green_ = arr_color[1].AsInt();
                        blue_ = arr_color[2].AsInt();
 
                        if (arr_color.size() == 4) {
                            opacity_ = arr_color[3].AsDouble();
                            rend_set.color_palette_.push_back(svg::Color(svg::Rgba(red_, 
                                                                                   green_, 
                                                                                   blue_, 
                                                                                   opacity_)));
                        } else if (arr_color.size() == 3) {
                            rend_set.color_palette_.push_back(svg::Color(svg::Rgb(red_, 
                                                                                  green_, 
                                                                                  blue_)));
                        }
                    }
                }
            }            
        } catch(...) {
            std::cout << "unable to parsse init settings";
        }
        
    } else {
        std::cout << "render_settings is not map";
    }
}
    
void JSONReader::ParseNode(const Node& root, TransportCatalogue& catalogue, std::vector<StatRequest>& stat_request, map_renderer::RenderSettings& render_settings){ 
    Dict root_dictionary;
    
    if (root.IsDict()) {
        root_dictionary = root.AsDict();
        
        try {          
            ParseNodeBase(root_dictionary.at("base_requests"), catalogue);
        } catch(...) {
            std::cout << "base_requests is empty";
        }
        
        try {
            ParseNodeStat(root_dictionary.at("stat_requests"), stat_request);
        } catch(...) {
            std::cout << "stat_requests is empty";
        }
        
        try {
            ParseNodeRender(root_dictionary.at("render_settings"), render_settings);
        } catch(...) {
            std::cout << "render_settings is empty";
        }
        
    } else {
        std::cout << "root is not map";
    }
}
    
void JSONReader::Parse(TransportCatalogue& catalogue, std::vector<StatRequest>& stat_request, map_renderer::RenderSettings& render_settings) {  
    ParseNode(document_.GetRoot(), 
               catalogue, 
               stat_request,
               render_settings);
}
    
}//end namespace json
}//end namespace detail
}//end namespace transport_catalogue
