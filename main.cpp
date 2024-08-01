#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
 
using namespace std;
using namespace transport_catalogue;
using namespace map_renderer;
using namespace request_handler;
using namespace transport_catalogue::detail::json;
 
int main() {
    vector<StatRequest> stat_request;
    RenderSettings render_settings;
    TransportCatalogue catalogue;

    JSONReader json_reader;
    RequestHandler request_handler;
    /*
    Print(
        Document{
            Builder{}
            .StartDict()
                .Key("key1"s).Value(123)
                .Key("key2"s).Value("value2"s)
                .Key("key3"s).StartArray()
                    .Value(456)
                    .StartDict().EndDict()
                    .StartDict()
                        .Key(""s)
                        .Value(nullptr)
                    .EndDict()
                    .Value(""s)
                .EndArray()
            .EndDict()
            .Build()
        },
        cout
    );*/

    cout << endl;
    json_reader = JSONReader(cin);    
    json_reader.Parse(catalogue, stat_request, render_settings);
    
    request_handler = RequestHandler();    
    request_handler.ExecuteQueries(catalogue, stat_request, render_settings);
    transport_catalogue::detail::json::Print(request_handler.GetDocument(), cout);
}
