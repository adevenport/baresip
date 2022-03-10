sudo apt -y install cmake libopencore-amrnb-dev libopencore-amrnb0 libsndfile1-dev ffmpeg openssl libasound2-dev
mkdir baresip-src && cd baresip-src
git clone https://github.com/adevenport/baresip.git && git clone https://github.com/baresip/re.git && git clone https://github.com/baresip/rem.git
cd re && git checkout 09c6fd27d6eefd483174ef9acb98ffd6437c1704 && make && sudo make install && sudo ldconfig && cd ..
cd rem && git checkout f2d386ecf508a1f5191d8f0b57251ec625efe92b && make && sudo make install && sudo ldconfig && cd ..
cd baresip && make && sudo make install && mkdir ~/.baresip && cp test-config ~/.baresip/config && cd ..
