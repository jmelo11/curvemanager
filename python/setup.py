# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup
from pathlib import Path
from sys import platform

__version__ = "1.0.0"

BASE_DIR = Path(__file__).absolute().parent.resolve()

# reorder this
if platform == "win32":
    LIB_DIR = Path('C:/Users/bloomberg/Desktop/Desarrollo/builds')
    folders = ['QuantLib', 'QuantExt', 'curvemanager', 'quantlibparser',
               'nlohmann_json_schema_validator', 'nlohmann_json', 'pybind11', 'pybind11_json']
    include_dirs = [str(LIB_DIR / folder / 'include') for folder in folders]
    include_dirs += [str(LIB_DIR / 'boost'), str(LIB_DIR / 'include')]
    library_dirs = [str(LIB_DIR / folder / 'lib') for folder in folders]
    library_dirs += [str(LIB_DIR / 'lib')]

    libraries = ['QuantLib-x64-mt', 'QuantExt-x64-mt', 'CurveManager',
                 'QuantLibParser', 'nlohmann_json_schema_validator']
    extra_compile_args = ['-std=c++20']

elif platform == "linux" or platform == "linux2":
    LIB_DIR = Path('builds')
    include_dirs = [str(LIB_DIR / 'include')]
    library_dirs = [str(LIB_DIR / 'lib')]
    libraries = ['QuantLib', 'QuantExt', 'QuantLibParser', 'CurveManager',
                 'nlohmann_json_schema_validator']
    extra_compile_args = ['-std=c++20']

else:
    LIB_DIR = Path('/Users/josemelo/Desktop/dev/builds')
    include_dirs = [str(LIB_DIR / 'include')]
    include_dirs += ['/opt/homebrew/opt/boost/include']
    library_dirs = [str(LIB_DIR / 'lib')]
    library_dirs += ['/opt/homebrew/opt/boost/lib']
    libraries = ['QuantLib', 'QuantExt', 'QuantLibParser', 'CurveManager',
                 'nlohmann_json_schema_validator']
    extra_compile_args = ['-std=c++20', '-arch', 'arm64']


ext_modules = [
    Pybind11Extension("CurveManager",
                      ["module.cpp"],
                      include_dirs=include_dirs,
                      library_dirs=library_dirs,
                      libraries=libraries,
                      define_macros=[('VERSION_INFO', __version__)],
                      extra_compile_args=extra_compile_args,
                      language="c++20"
                      ),
]

setup(
    name="CurveManager",
    version=__version__,
    author="Jose Melo",
    author_email="jmelo@live.cl",
    description="CurveManager using pybind11",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.7",
)
