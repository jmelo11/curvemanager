
#include <curvemanager/schemas/forwardratesrequest.hpp>
#include <qlp/schemas/commonschemas.hpp>

namespace QuantLibParser
{

    template <>
    void Schema<ForwardRatesRequest>::initSchema() {
        json base = R"({
            "title": "Discounts Request Schema",
            "type": "object",            
            "properties": {
                "CURVE": {
                    "type": "string"
                }                
            },
            "required": ["REFDATE", "CURVE", "DATES"]              
        })"_json;

        base["properties"]["FREQUENCY"]   = frequencySchema;
        base["properties"]["COMPOUNDING"] = compoundingSchema;
        base["properties"]["DAYCOUNTER"]  = dayCounterSchema;
        base["properties"]["REFDATE"]     = dateSchema;

        json dates                  = R"({
            "type": "array",
            "items": {
                "type": "array",
                "maxItems": 2                
            }
        })"_json;
        dates["items"]["items"]     = dateSchema;
        base["properties"]["DATES"] = dates;
        mySchema_                   = base;
    };

    template <>
    void Schema<ForwardRatesRequest>::initDefaultValues() {
        myDefaultValues_["DAYCOUNTER"]  = "ACT360";
        myDefaultValues_["COMPOUNDING"] = "SIMPLE";
        myDefaultValues_["FREQUENCY"]   = "ANNUAL";
    };

}  // namespace QuantLibParser
