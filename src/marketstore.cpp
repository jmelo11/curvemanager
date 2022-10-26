#include <curvemanager/marketstore.hpp>

namespace CurveManager {
	
	MarketStore::MarketStore() {};

	boost::shared_ptr<YieldTermStructure> MarketStore::getCurve(const std::string& name) const {
		if (hasCurve(name))
			return curveMap_.at(name);
		throw std::runtime_error("Curve not found: " + name);
	};

	boost::shared_ptr<IborIndex> MarketStore::getIndex(const std::string& name) const {
		if (hasIndex(name))
			return indexMap_.at(name);
		throw std::runtime_error("Index not found: " + name);
	}
	
	Handle<Quote>& MarketStore::getQuote(const std::string& ticker) {
		if (hasQuote(ticker))
			return quoteMap_.at(ticker);
		throw std::runtime_error("Index not found: " + ticker);
	}

	RelinkableHandle<YieldTermStructure>& MarketStore::getCurveHandle(const std::string& name) {
		if (hasCurveHandle(name))
			return curveHandleMap_.at(name);
		throw std::runtime_error("Curve handle not found: " + name);
	}

	bool MarketStore::hasCurve(const std::string& name) const {
		if (curveMap_.find(name) != curveMap_.end())
			return true;
		return false;
	}

	bool MarketStore::hasIndex(const std::string& name) const {
		if (indexMap_.find(name) != indexMap_.end())
			return true;
		return false;
	}
	bool MarketStore::hasCurveHandle(const std::string& name) const {
		return curveHandleMap_.find(name) != curveHandleMap_.end();
	}

	bool MarketStore::hasQuote(const std::string& ticker) const {
		return quoteMap_.find(ticker) != quoteMap_.end();
	}

	void MarketStore::addFixing(const std::string& name, const Date& date, double fixing) {
		indexMap_[name]->addFixing(date, fixing, true);
	}

	void MarketStore::addCurve(const std::string& name, boost::shared_ptr<YieldTermStructure>& curve) {
		curveMap_[name] = curve;
	}

	void MarketStore::addIndex(const std::string& name, boost::shared_ptr<IborIndex>& index) {
		indexMap_[name] = index;

	}

	void MarketStore::addQuote(const std::string& ticker, Handle<Quote>& handle) {
		quoteMap_.insert({ ticker, handle });
	}

	void MarketStore::addCurveHandle(const std::string& name, RelinkableHandle<YieldTermStructure>& handle) {
		curveHandleMap_.insert({ name, handle });
	}
	
	void MarketStore::freeze() {

		for (const auto& [name, curve] : curveMap_) {
			auto ptr = boost::dynamic_pointer_cast<PiecewiseYieldCurve<Discount, LogLinear>>(curve);
			if (ptr)
				ptr->freeze();
		}

	}

	void MarketStore::unfreeze() {
		for (const auto& [name, curve] : curveMap_) {
			auto ptr = boost::dynamic_pointer_cast<PiecewiseYieldCurve<Discount, LogLinear>>(curve);
			if (ptr)
				ptr->unfreeze();
		}
	}

	std::vector<std::string> MarketStore::allCurves() const {
		std::vector<std::string> names;
		for (const auto& [name, curve] : curveMap_)
			names.push_back(name);
		return names;
	}

	std::vector<std::string> MarketStore::allIndices() const {
		std::vector<std::string> names;
		for (const auto& [name, index] : indexMap_)
			names.push_back(name);
		return names;
	}

	json MarketStore::results(const std::vector<std::string>& dates) const {
		std::vector<Date> qlDates;
		json data;
		for (const auto& date : dates)
			qlDates.push_back(QuantLibParser::parse<Date>(date));
		
		auto curves = allCurves();
		for (const auto& curve : curves) {
			auto qlCurve = getCurve(curve);
			std::vector<double> values;
			for (const auto& date : qlDates) {
				values.push_back(qlCurve->discount(date));
			}

			data[curve]["DATES"] = dates;
			data[curve]["VALUES"] = values;
		}
		return data;
	}
}

