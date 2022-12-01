#include <curvemanager/schemas/all.hpp>
#include "pch.hpp"

namespace QLP = QuantLibParser;
using json    = nlohmann::json;

TEST(Requests, CurveBuilderRequest) {
    json data = R"({
		"REFDATE":"28022022",
		"CURVES": [{}],
		"INDEXES":[{}]
	})"_json;
    QLP::Schema<QLP::CurveBuilderRequest> schema;
    EXPECT_NO_THROW(schema.validate(data));
}

TEST(Requests, UpdateQuoteRequest) {
    json data = R"([		
		{
			"NAME": "CLP CURRENCY",
			"VALUE": 800
		}		
	])"_json;

    QLP::Schema<QLP::UpdateQuoteRequest> schema;
    EXPECT_NO_THROW(schema.validate(data));
}

TEST(Requests, DiscountFactorsRequest) {
    json data = R"({
		"REFDATE":"24082022",
		"CURVE":"ICP_ICAP",		
		"DATES":["24022023"]
	})"_json;

    QLP::Schema<QLP::DiscountFactorsRequest> schema;
    EXPECT_NO_THROW(schema.validate(data));
}

TEST(Requests, ZeroRatesRequests) {
    json data = R"({
		"REFDATE":"24082022",
		"CURVE":"ICP_ICAP",
		"DAYCOUNTER":"ACT360",
		"COMPOUNDING":"SIMPLE",
		"DATES":["24022023"]
	})"_json;

    QLP::Schema<QLP::ZeroRatesRequests> schema;
    EXPECT_NO_THROW(schema.validate(data));
}

TEST(Requests, ForwardRatesRequest) {
    json data = R"({
		"REFDATE":"24082022",
		"CURVE":"ICP_ICAP",
		"DAYCOUNTER":"ACT360",
		"COMPOUNDING":"SIMPLE",
		"DATES":[["24022023","24022024"]]
	})"_json;

    QLP::Schema<QLP::ForwardRatesRequest> schema;
    EXPECT_NO_THROW(schema.validate(data));
}