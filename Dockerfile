FROM quantlibparser:latest

COPY . /curvemanager

WORKDIR /curvemanager/build.curvemanager
RUN apt update && apt install vim -y
RUN cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20
RUN make -j ${num_cores} && make install && ldconfig

WORKDIR /curvemanager/python
RUN pip3 install pybind11 && python3 setup.py install
RUN cd / && rm -rf curvemanager && ldconfig

WORKDIR /
CMD bash