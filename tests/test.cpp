/*
 * Created on Sat Oct 29 2022
 *
 * Jose Melo - 2022
 */

#include "pch.h"
#include <fstream>
#include <curvemanager/curvemanager.hpp>
#include <iostream>

using namespace CurveManager;

json readJSONFile(std::string filePath) {
    std::ifstream file(filePath);
    std::ostringstream tmp;
    tmp << file.rdbuf();
    std::string s = tmp.str();
    json data     = json::parse(s);
    return data;
}

TEST(PiecewiseCurveBuild, CurveManager) {
    json curveData = readJSONFile("json/piecewise.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    EXPECT_NO_THROW(builder.build());

    auto curve = store.getCurve("SOFR");
    EXPECT_NO_THROW(curve->discount(1));
}

TEST(PiecewiseCurveFullBuild, CurveManager) {
    json curveData = readJSONFile("json/piecewisefull.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    try {
        builder.build();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    EXPECT_NO_THROW(builder.build());
}

TEST(PiecewiseCurveFullBuild2, CurveManager) {
    json curveData = readJSONFile("json/piecewisefull2.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    try {
        builder.build();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    EXPECT_NO_THROW(builder.build());

    auto curves = store.allCurves();
    for (const auto& curve : curves) {
        try {
            std::cout << curve << ":\t" << store.getCurve(curve)->discount(1) << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }
}

TEST(FlatForwardCurveBuild, CurveManager) {
    json curveData = readJSONFile("json/flatforward.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    try {
        EXPECT_NO_THROW(builder.build());
    }
    catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    auto curve = store.getCurve("SOFR");
    EXPECT_NO_THROW(curve->discount(1));
}

TEST(DiscountCurveBuild, CurveManager) {
    json curveData = readJSONFile("json/discount.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);

    try {
        builder.build();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    auto curve = store.getCurve("SOFR");
    EXPECT_NO_THROW(curve->discount(1));
}

TEST(BootstrapResults, CurveManager) {
    json curveData = readJSONFile("json/discount.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);

    try {
        builder.build();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    EXPECT_NO_THROW(store.bootstrapResults(););
}

TEST(UpdateQuotes, CurveManager) {
    json curveData = readJSONFile("json/piecewise.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    builder.build();

    json quoteData = R"([
		{
	"NAME": "USOSFR2Z",
	"VALUE": 0.03
		}
	])"_json;

    EXPECT_ANY_THROW(builder.updateQuotes(quoteData));

    quoteData[0]["NAME"] = "USOSFR2Z CURNCY";

    EXPECT_NO_THROW(builder.updateQuotes(quoteData));

    auto curve = store.getCurve("SOFR");
    EXPECT_NO_THROW(curve->discount(1));
}

TEST(DiscountFactorRequests, CurveManager) {
    json curveData = readJSONFile("json/discount.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    builder.build();
    json request = R"({"REFDATE":"28082022", "DATES":["29012026"], "CURVE":"SOFR"})"_json;

    EXPECT_NO_THROW(store.discountRequest(request));
}

TEST(ZeroRatesRequest, CurveManager) {
    json curveData = readJSONFile("json/discount.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    builder.build();
    json request = R"({"REFDATE":"28082022","CURVE":"SOFR","DAYCOUNTER":"ACT360","COMPOUNDING":"SIMPLE","DATES":["24022023"]})"_json;

    EXPECT_NO_THROW(store.zeroRateRequest(request));
}

TEST(ForwardRatesRequests, CurveManager) {
    json curveData = readJSONFile("json/discount.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    builder.build();
    json request = R"({
		"REFDATE":"28082022",
		"CURVE":"SOFR",
		"DATES":[["09032023","14042023"],["14042023","21052023"]],
		"COMPOUNDING":"SIMPLE",
		"FREQUENCY":"ANNUAL"
	})"_json;
    try {
        store.forwardRateRequest(request);
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    EXPECT_NO_THROW(store.forwardRateRequest(request));
}
