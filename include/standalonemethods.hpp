#pragma once
#include <jsontoobject.hpp>

namespace CurveManager {

	static auto makeDiscountCurve(const json& params) {
		const json& nodeData = params.at("NODES");

		std::vector<std::pair<Date, double>> nodes;

		for (const auto& [date, value] : nodeData.items()) {
			Date d = parse<Date>(date);
			nodes.push_back({ d, value });
		}
		std::sort(nodes.begin(), nodes.end());
		std::vector<Date>nodeDates;
		std::vector<double>nodeDFs;
		for (const auto& node : nodes) {
			nodeDates.push_back(node.first);
			nodeDFs.push_back(node.second);
		}

		SETVAR(params, DayCounter, DAYCOUNTER);

		Settings::instance().evaluationDate() = nodeDates[0];
		auto curve = boost::make_shared<InterpolatedDiscountCurve<Cubic>>(nodeDates, nodeDFs, DAYCOUNTER);
		curve->enableExtrapolation();
		curve->unregisterWithAll();
		return curve;
	}

	static std::vector<double> zeroRates(const json& params) {
		const json& curveData = params.at("CURVE");
		auto curve = makeDiscountCurve(curveData);
		std::vector<double> zeroRates;

		const json& dates = params.at("DATES");

		SETVAR(params, DayCounter, DAYCOUNTER, "ACT360");
		SETVAR(params, Compounding, COMPOUNDING, "SIMPLE");
		SETVAR(params, Frequency, FREQUENCY, "ANNUAL");

		for (const auto& date : dates) {
			Date qlDate = parse<Date>(date);
			zeroRates.push_back(curve->zeroRate(qlDate, DAYCOUNTER, COMPOUNDING, FREQUENCY).rate());
		}
		return zeroRates;
	};

	static std::vector<double> forwardRates(const json& params) {

		const json& curveData = params.at("CURVE");
		auto curve = makeDiscountCurve(curveData);

		std::vector<double> forwardRates;

		SETVAR(params, DayCounter, DAYCOUNTER);
		SETVAR(params, Compounding, COMPOUNDING);
		SETVAR(params, Frequency, FREQUENCY);

		const json& dates = params.at("DATES");

		for (const auto& pair : dates) {
			Date date1 = parse<Date>(pair[0]);
			Date date2 = parse<Date>(pair[1]);
			forwardRates.push_back(curve->forwardRate(date1, date2, DAYCOUNTER, COMPOUNDING, FREQUENCY).rate());
		}
		return forwardRates;
	};

	static std::vector<double> discounts(const json& params) {
		const json& curveData = params.at("CURVE");
		auto curve = makeDiscountCurve(curveData);

		std::vector<double> dfs;

		const json& dates = params.at("DATES");

		for (const auto& date : dates) {
			Date qlDate = parse<Date>(date);
			dfs.push_back(curve->discount(qlDate));
		}
		return dfs;
	};
}