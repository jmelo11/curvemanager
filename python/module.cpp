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

#define SchemaWithoutMaker(name)                                      \
    py::class_<Schema<name>>(m, #name)                                \
        .def(py::init<>())                                            \
        .def("validate", &Schema<name>::validate)                     \
        .def("isValid", &Schema<name>::isValid)                       \
        .def("schema", &Schema<name>::schema)                         \
        .def("addDefaultValue", &Schema<name>::addDefaultValue)       \
        .def("removeDefaultValue", &Schema<name>::removeDefaultValue) \
        .def("schema", &Schema<name>::schema)                         \
        .def("addRequired", &Schema<name>::addRequired)               \
        .def("removeRequired", &Schema<name>::removeRequired)


PYBIND11_MODULE(CurveManager, m) {
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

    // requests
    SchemaWithoutMaker(DiscountFactorsRequest);
    SchemaWithoutMaker(ForwardRatesRequest);
    SchemaWithoutMaker(ZeroRatesRequests);
    
}
