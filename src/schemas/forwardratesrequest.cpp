#include <curvemanager/schemas/forwardratesrequest.hpp>

namespace QuantLibParser
{

    template <>
    void Schema<ForwardRatesRequest>::initSchema() {
        mySchema_ = readJSONFile("forwardrates.request.schema.json");
    };

    template <>
    void Schema<ForwardRatesRequest>::initDefaultValues() {
        myDefaultValues_["dayCounter"]  = "Act360";
        myDefaultValues_["compounding"] = "Simple";
        myDefaultValues_["frequency"]   = "Annual";
    };

}  // namespace QuantLibParser
