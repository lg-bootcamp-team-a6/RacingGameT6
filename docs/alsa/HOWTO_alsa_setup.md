# alsa-lib 설치
여기까진 최초 1회만 실행

```
mkdir ~/work/alsa
cd ~/work/alsa
tar jxvf ~/res/alsa/alsa-lib-1.1.6.tar.bz2
cd alsa-lib-1.1.6
CC=aarch64-linux-gnu-gcc ./configure --host=arm-linux --prefix=/home/user/work/alsa/install/
make
make install
```

아래는 보드 셋업할 때마다 실행 필요
```
sudo cp -arf ~/work/alsa/install /nfsroot/alsa-lib
```

# alsa-utils 설치
```
cd ~/work/alsa
tar jxvf ~/res/alsa/alsa-utils-1.1.6.tar.bz2
cd alsa-utils-1.1.6
CC=aarch64-linux-gnu-gcc ./configure --prefix=/home/user/work/alsa/install/ --host=arm-linux --with-alsa-inc-prefix=/home/user/work/alsa/install/include --with-alsa-prefix=/home/user/work/alsa/install/lib --disable-alsamixer --disable-xmlto --disable-nls
make
sudo cp aplay/aplay /nfsroot
sudo cp amixer/amixer /nfsroot
```

