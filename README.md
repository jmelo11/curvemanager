# CurveManager #

Herramienta para poder utilizar el sistema de curvas/bootstrap de QuantLib junto con JSON

## Instalación ##

Ejemplo de instalacion con cmake:

    mkdir build 
    cd build
    cmake .. -DCMAKE_PREFIX_PATH='C:\Users\bloomberg\Desktop\Desarrollo\builds' -DCMAKE_INSTALL_PREFIX='C:\Users\bloomberg\Desktop\Desarrollo\builds\curvemanager' -DBoost_INCLUDE_DIR="C:/Users/bloomberg/Desktop/Desarrollo/builds/boost"
    cmake --build . --config Release --target install
