
#include <curvemanager/schemas/updatequoterequest.hpp>

namespace QuantLibParser
{
    template <>
    void Schema<UpdateQuoteRequest>::initSchema() {
        json base = R"({
            "title": "Update Quotes Request Schema",
            "type": "array",
            "items": [
                {
                    "type": "object",
                    "properties": {
                        "NAME": {
                            "type": "string"
                        },
						"VALUE": {
                            "type": "number"
                        }					
                    },
                    "required": ["NAME", "VALUE"]
                }
            ]
        })"_json;

        mySchema_ = base;
    };

    template <>
    void Schema<UpdateQuoteRequest>::initDefaultValues(){};

}  // namespace QuantLibParser
