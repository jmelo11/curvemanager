/*
 * Created on Sat Oct 29 2022
 *
 * Jose Melo - 2022
 */

#include <curvemanager/curvemanager.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11_json/pybind11_json.hpp>

namespace py = pybind11;

using namespace CurveManager;
using namespace std;

PYBIND11_MODULE(curvemanager, m) {
    m.doc() = "CurveManager for python";  // optional module docstring

    py::class_<MarketStore>(m, "MarketStore")
        .def(py::init<>())
        .def("allCurves", &MarketStore::allCurves)
        .def("bootstrapResults", &MarketStore::bootstrapResults)
        .def("discountRequest", &MarketStore::discountRequest)
        .def("zeroRateRequest", &MarketStore::zeroRateRequest)
        .def("forwardRateRequest", &MarketStore::forwardRateRequest);

    py::class_<CurveBuilder>(m, "CurveBuilder")
        .def(py::init<json, MarketStore&>(), py::arg("data"), py::arg("marketStore"))
        .def("build", &CurveBuilder::build)
        .def("updateQuotes", &CurveBuilder::updateQuotes, py::arg("prices"));
}
