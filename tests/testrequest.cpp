#include <curvemanager/schemas/all.hpp>
#include "pch.hpp"

namespace QLP = QuantLibParser;
using json    = nlohmann::json;

TEST(Requests, CurveBuilderRequest) {
    json data = R"({
		"refDate":"2022-08-24",
		"curves": [{
			"curveName": "ICP_ICAP",			
			"curveConfig": {
				"curveType": "FlatForward",
				"rate": 0.01
			}
		}],
		"indexes":[]
	})"_json;
    QLP::Schema<QLP::CurveBuilderRequest> schema;
    EXPECT_NO_THROW(schema.validate(data));
}

TEST(Requests, UpdateQuoteRequest) {
    json data = R"([		
		{
			"ticker": "CLP CURRENCY",
			"value": 800
		}		
	])"_json;

    QLP::Schema<QLP::UpdateQuoteRequest> schema;
    EXPECT_NO_THROW(schema.validate(data));
}

TEST(Requests, DiscountFactorsRequest) {
    json data = R"({
		"refDate":"2022-08-24",
		"curve":"ICP_ICAP",		
		"dates":["2022-08-24"]
	})"_json;

    QLP::Schema<QLP::DiscountFactorsRequest> schema;
    EXPECT_NO_THROW(schema.validate(data));
}

TEST(Requests, ZeroRatesRequests) {
    json data = R"({
		"refDate":"2022-08-24",
		"curve":"ICP_ICAP",
		"dayCounter":"Act360",
		"compounding":"Simple",
		"dates":["2022-08-24"]
	})"_json;

    QLP::Schema<QLP::ZeroRatesRequests> schema;
    EXPECT_NO_THROW(schema.validate(data));
}

TEST(Requests, ForwardRatesRequest) {
    json data = R"({
		"refDate":"2022-08-24",
		"curve":"ICP_ICAP",
		"dayCounter":"Act360",
		"compounding":"Simple",
		"dates":[{
			"startDate":"2022-08-24",
			"endDate":"2022-08-24"
		}]
	})"_json;

    QLP::Schema<QLP::ForwardRatesRequest> schema;
    EXPECT_NO_THROW(schema.validate(data));
}