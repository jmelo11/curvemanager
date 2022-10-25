#pragma once

#include "pch.h"
#include <curvemanager/curvemanager.hpp>
#include <fstream>
#include <iostream>

using namespace CurveManager;

json readJSONFile(std::string filePath) {
	std::ifstream file(filePath);
	std::ostringstream tmp;
	tmp << file.rdbuf();
	std::string s = tmp.str();
	json data = json::parse(s);
	return data;
}

TEST(PiecewiseCurve, CurveManager) {
	json curveData = readJSONFile("piecewise.json");
	MarketStore store;
	CurveBuilder builder(curveData, store);
	EXPECT_NO_THROW(builder.build());
	
	auto curve = store.getCurve("SOFR");			
	EXPECT_NO_THROW(curve->discount(1));
}

TEST(FlatForwardCurve, CurveManager) {
	json curveData = readJSONFile("flatforward.json");
	MarketStore store;
	CurveBuilder builder(curveData, store);
	try
	{
		EXPECT_NO_THROW(builder.build());
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << "\n";
	}

	auto curve = store.getCurve("SOFR");
	EXPECT_NO_THROW(curve->discount(1));
}

TEST(DiscountCurve, CurveManager) {
	json curveData = readJSONFile("discount.json");
	MarketStore store;
	CurveBuilder builder(curveData, store);
	
	try
	{
		builder.build();		
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << "\n";
	}

	auto curve = store.getCurve("SOFR");
	EXPECT_NO_THROW(curve->discount(1));
}

TEST(Results, CurveManager) {
	json curveData = readJSONFile("discount.json");
	MarketStore store;
	CurveBuilder builder(curveData, store);

	try
	{
		builder.build();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << "\n";
	}

	std::vector<std::string> dates = { "28082022", "25022023", "25022025" };
	json results = store.results(dates);
	EXPECT_NO_THROW(store.results(dates));
}
