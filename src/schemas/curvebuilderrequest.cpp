#include <curvemanager/schemas/curvebuilderrequest.hpp>

namespace QuantLibParser
{
    template <>
    void Schema<CurveBuilderRequest>::initSchema() {
        mySchema_ = readJSONFile("curvebuilder.request.schema.json");
    };

    template <>
    void Schema<CurveBuilderRequest>::initDefaultValues(){};

}  // namespace QuantLibParser
