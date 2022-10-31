# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup
from pathlib import Path
import os

__version__ = "1.0.0"

BASE_DIR = Path(__file__).absolute().parent.resolve()

if os.name == "nt":
    LIB_DIR = Path('C:/Users/bloomberg/Desktop/Desarrollo/builds')
    folders = ['QuantLib','QuantExt','curvemanager','quantlibparser','nlohmann_json_schema_validator', 'nlohmann_json', 'pybind11', 'pybind11_json']
    include_dirs = [str(LIB_DIR / folder / 'include') for folder in folders]
    include_dirs.append(str(LIB_DIR / 'boost'))
    library_dirs = [str(LIB_DIR / folder / 'lib') for folder in folders]
    libraries  = ['QuantLib-x64-mt','QuantExt-x64-mt','curvemanager','quantlibparser','nlohmann_json_schema_validator']
else:
    LIB_DIR = Path('/usr/local')
    include_dirs = [str(LIB_DIR / 'include')]
    include_dirs += ['/opt/homebrew/opt/boost/include']

    library_dirs = [str(LIB_DIR / 'lib')]
    library_dirs += ['/opt/homebrew/opt/boost/lib']

    libraries  = ['QuantLib','QuantExt','curvemanager','QuantLibParser','nlohmann_json_schema_validator']

ext_modules = [
    Pybind11Extension("curvemanager",
        ["module.cpp"],
        include_dirs=include_dirs,
        library_dirs=library_dirs,
        libraries=libraries,
        define_macros = [('VERSION_INFO', __version__)],
        compiler_args=['-std=c++20'],        
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