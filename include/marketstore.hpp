#pragma once
#include <map>
#include <string>
#include <unordered_map>

#include <ql/quote.hpp>
#include <ql/handle.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/indexes/iborindex.hpp>

namespace CurveManager {
	using namespace QuantLib;
	using LogLinearPiecewiseCurve = PiecewiseYieldCurve<Discount, LogLinear>;

	class MarketStore {

	public:
		MarketStore();

		bool hasCurveHandle(const std::string& name) const;
		bool hasCurve(const std::string& name) const;
		bool hasIndex(const std::string& name) const;
		bool hasQuote(const std::string& ticker) const;

		boost::shared_ptr<YieldTermStructure>getCurve(const std::string& name) const;
		boost::shared_ptr<IborIndex>getIndex(const std::string& name) const;
		Handle<Quote>& getQuote(const std::string& ticker);
		RelinkableHandle<YieldTermStructure>& getCurveHandle(const std::string& name);

		void addFixing(const std::string& name, const Date& date, double fixing);
		void addCurve(const std::string& name, boost::shared_ptr<YieldTermStructure>& curve);
		void addIndex(const std::string& name, boost::shared_ptr<IborIndex>& index);
		void addQuote(const std::string& ticker, Handle<Quote>& handle);
		void addCurveHandle(const std::string& name, RelinkableHandle<YieldTermStructure>& handle);

		void freeze();
		void unfreeze();

		std::vector<std::string> allCurves() const;
		std::vector<std::string> allIndices() const;

	private:

		std::unordered_map<std::string, boost::shared_ptr<YieldTermStructure>> curveMap_;
		std::unordered_map<std::string, RelinkableHandle<YieldTermStructure>> curveHandleMap_;
		std::unordered_map<std::string, boost::shared_ptr<IborIndex>> indexMap_;
		std::unordered_map<std::string, Handle<Quote>> quoteMap_;
	};

}