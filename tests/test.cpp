/*
 * Created on Sat Oct 29 2022
 *
 * Jose Melo - 2022
 */

#include <curvemanager/curvemanager.hpp>
#include "pch.hpp"
#include <fstream>
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

TEST(CurveManager, PiecewiseCurveBuildSingleCurve) {
    json curveData = readJSONFile("json/piecewise.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    EXPECT_NO_THROW(builder.build());
  
    auto f = [&](const std::string& curveName) {
        try {
            auto curve = store.getCurve(curveName);
            curve->discount(1);
        }
        catch (std::exception& e) {
            std::string error = "Error in curve " + curveName + ":\n" + e.what();
            throw std::runtime_error(error);
        }
    };

    for (const auto& curveName : store.allCurves()) {
        EXPECT_NO_THROW(f(curveName));
    }

    auto curveHandle = store.getCurveHandle("SOFR");
    EXPECT_FALSE(curveHandle.empty());
}

TEST(CurveManager, PiecewiseCurveBuildTwoCurves) {
    json curveData = readJSONFile("json/piecewisetwocurves.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    EXPECT_NO_THROW(builder.build());
    auto f = [&](const std::string& curveName) {
        try {
            auto curve = store.getCurve(curveName);
            curve->discount(1);
        }
        catch (std::exception& e) {
            std::string error = "Error in curve " + curveName + ":\n" + e.what();
            throw std::runtime_error(error);
        }
    };

    for (const auto& curveName : store.allCurves()) {
        auto curveHandle = store.getCurveHandle(curveName);
        EXPECT_FALSE(curveHandle.empty());
        EXPECT_NO_THROW(f(curveName));
    }
}

TEST(CurveManager, PiecewiseCurveFullBuild) {
    json curveData = readJSONFile("json/piecewisefull.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    EXPECT_NO_THROW(builder.build());
    auto f = [&](const std::string& curveName) {
        try {
            auto curve = store.getCurve(curveName);
            curve->discount(1);
        }
        catch (std::exception& e) {
            std::string error = "Error in curve " + curveName + ":\n" + e.what();
            throw std::runtime_error(error);
        }
    };

    for (const auto& curveName : store.allCurves()) {
        auto curveHandle = store.getCurveHandle(curveName);
        EXPECT_FALSE(curveHandle.empty());
        EXPECT_NO_THROW(f(curveName));
    }
}

TEST(CurveManager, PiecewiseCurveFullBuild2) {
    json curveData = readJSONFile("json/piecewisefull2.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    EXPECT_NO_THROW(builder.build());

    auto f = [&](const std::string& curveName) {
        try {
            auto curve = store.getCurve(curveName);
            curve->discount(1);
        }
        catch (std::exception& e) {
            std::string error = "Error in curve " + curveName + ":\n" + e.what();
            throw std::runtime_error(error);
        }
    };

    for (const auto& curveName : store.allCurves()) {
        EXPECT_NO_THROW(f(curveName));
    }
}

TEST(CurveManager, FlatForwardCurveBuild) {
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

TEST(CurveManager, DiscountCurveBuild) {
    json curveData = readJSONFile("json/discount.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    builder.build();

    auto curve = store.getCurve("SOFR");
    EXPECT_NO_THROW(curve->discount(1));
}

TEST(CurveManager, BootstrapResults) {
    json curveData = readJSONFile("json/discount.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    EXPECT_NO_THROW(store.bootstrapResults(););
}

TEST(CurveManager, UpdateQuotes) {
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

TEST(CurveManager, DiscountFactorRequests) {
    json curveData = readJSONFile("json/discount.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    builder.build();
    json request = R"({"REFDATE":"28082022", "DATES":["29012026"], "CURVE":"SOFR"})"_json;
    EXPECT_NO_THROW(store.discountRequest(request));
}

TEST(CurveManager, ZeroRatesRequest) {
    json curveData = readJSONFile("json/discount.json");
    MarketStore store;
    CurveBuilder builder(curveData, store);
    builder.build();
    json request = R"({
        "REFDATE":"28082022",
        "CURVE":"SOFR",
        "DAYCOUNTER":"ACT360",
        "COMPOUNDING":"SIMPLE",
        "FREQUENCY":"ANNUAL",
        "DATES":["24022023"]
    })"_json;

    EXPECT_NO_THROW(store.zeroRateRequest(request));
}

TEST(CurveManager, ForwardRatesRequests) {
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
    EXPECT_NO_THROW(store.forwardRateRequest(request));
}
