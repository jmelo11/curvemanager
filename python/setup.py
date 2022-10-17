# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup
import os
from pathlib import Path
__version__ = "1.0.0"

BASE_DIR = Path(__file__).absolute().parent.parent.parent.parent.resolve()

ext_modules = [
    Pybind11Extension("curvemanager",
        ["module.cpp"],
        include_dirs=[str(BASE_DIR / 'curvemanager/curvemanager'),                                            
                      os.environ['QLE_PATH'],                                        
                      os.environ['QL_PATH'],                  
                      os.environ['BOOST_PATH'],
                      str(BASE_DIR / 'repos/boost'),
                      str(BASE_DIR / 'repos/json/include'),
                      str(BASE_DIR / 'repos/pybind11_json/include')],
        library_dirs=[os.environ['QLE_PATH'] + '/lib',                      
                      os.environ['QL_PATH'] + '/lib',                      
                      os.environ['BOOST_PATH'] + '/libs',                      
                      str(BASE_DIR / 'curvemanager/build/x64/Release'),],
        libraries=['QuantLib-x64-mt', 'QuantExt-x64-mt','curvemanager'],
        # Example: passing in the version to the compiled code
        define_macros = [('VERSION_INFO', __version__)]       
        ),
]

setup(
    name="curvemanager",
    version=__version__,
    author="Itau",
    author_email="jose.melo@itau.cl",
    description="curvemanager using pybind11",
    ext_modules=ext_modules,    
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.7",
)