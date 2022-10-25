FROM quantlibparser:latest

COPY . /curvemanager

WORKDIR /curvemanager/build.curvemanager
#RUN apt update && apt install vim -y
RUN cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20
RUN make -j ${num_cores} && make install 

WORKDIR /curvemanager/python
RUN pip3 install pybind11
RUN python3 setup.py build && python setup.py install

WORKDIR /
CMD bash