#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
 
namespace transport_catalogue {
namespace detail {
namespace json {
    
class JSONReader{
public:
    JSONReader() = default;    
    JSONReader(Document doc);
    JSONReader(std::istream& input);
    
    void ParseNodeBase(const Node& root, TransportCatalogue& catalogue);
    void ParseNodeStat(const Node& root, std::vector<StatRequest>& stat_request);
    void ParseNodeRender(const Node& node, map_renderer::RenderSettings& render_settings);
    void ParseNode(const Node& root, TransportCatalogue& catalogue, std::vector<StatRequest>& stat_request, map_renderer::RenderSettings& render_settings);
    void Parse(TransportCatalogue& catalogue, std::vector<StatRequest>& stat_request, map_renderer::RenderSettings& render_settings);
    
    Stop ParseNodeStop(Node& node);
    Bus ParseNodeBus(Node& node, TransportCatalogue& catalogue);
    std::vector<Distance> ParseNodeDistances(Node& node, TransportCatalogue& catalogue);
    
    
    
private:
    Document document_;
};
    
}//end namespace json
}//end namespace detail
}//end namespace transport_catalogue
