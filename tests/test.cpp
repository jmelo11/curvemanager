#include "pch.h"
#include <curvemanager.hpp>

using namespace CurveManager;


TEST(DiscountCurve, CurveManager) {
	json curveData = {};
	MarketStore store;
	CurveBuilder build(curveData, store);
	auto curve = store.getCurve("SOFR");
		
}
TEST(FlatForwardCurve, CurveManager) {}
TEST(PiecewiseCurve, CurveManager) {}
TEST(MultiCurve, CurveManager) {}