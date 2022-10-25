#pragma once

#include <iostream>
#include <map>
#include <stdexcept>

#include <curvemanager/marketstore.hpp>
#include <curvemanager/jsontoobject.hpp>

namespace CurveManager {
	
	class CurveBuilder {
	public:
		CurveBuilder(const json& data, MarketStore& marketStore);
		
		void build();
		
		void updateQuotes(const json& prices);
				
	private:
		
		void preprocessData();

		void buildCurve(const std::string& name, const json& curve);
		boost::shared_ptr<YieldTermStructure> buildDiscountCurve(const std::string& name, const json& curve);
		boost::shared_ptr<YieldTermStructure> buildFlatForwardCurve(const std::string& name, const json& curve);
		boost::shared_ptr<YieldTermStructure> buildPiecewiseCurve(const std::string& name, const json& curve);

		std::vector<boost::shared_ptr<RateHelper>> buildRateHelpers(const json& rateHelperVector, const std::string& currentCurve);

		boost::shared_ptr<IborIndex> buildIndex(const std::string& name);

		json data_;
		MarketStore& marketStore_;
		std::unordered_map<std::string, json> curveConfigs_;
		std::unordered_map<std::string, json> indexConfigs_;
	};

}

