#include <curvemanager/schemas/zeroratesrequest.hpp>

namespace QuantLibParser
{

    template <>
    void Schema<ZeroRatesRequests>::initSchema() {
        mySchema_ = readJSONFile("zerorates.request.schema.json");
    };

    template <>
    void Schema<ZeroRatesRequests>::initDefaultValues() {
        myDefaultValues_["dayCounter"]  = "Act360";
        myDefaultValues_["compounding"] = "Simple";
        myDefaultValues_["frequency"]   = "Annual";
    };

}  // namespace QuantLibParser
