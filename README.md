TPS Informer linux
==========
TPS Informer linux version its console application for download and show internet statistic usage from personal cabinet (for TPS UZ users)

### Otput
```
$ tpsinfo
Использовано: 1780.14 Мб (222.52 Мб/день)
Остаток: 5219.86 Мб (237.27 Мб/день)
Баланс: $1.26 / Курс: 2388.25
К оплате (1-го): 56697.05
```

### Complilation \ Install
Get sources:
```
wget https://github.com/bestlion/tpscabinet/archive/tpscabinet1.0.tar.gz
```
Unpack:
```
tar -zxvf tpscabinet1.0.tar.gz
```
Compile
```
cd tpscabinet-linux-1.0
sudo apt-get update
sudo apt-get install libcurl4-gnutls-dev
gcc -o tpsinfo tpsinfo.c -lcurl
```

### Usage
```
./tpsinfo [login] [password]
```
Optionally you may hardcode your login password in "tpsinfo.h" file:
```
#define TPSLOGIN "jetXXXX" << jetXXXX tps login here
#define TPSPASSWORD "XXXX" << XXXX your tps password here
```
and recompile code after this
then you may start util without any parameters:
```
./tpsinfo
```
optionally if you like to gets info everywhere you may copy binnary tpsinfo to sbin folder:
```
sudo mv tpsinfo /sbin
```
so after that for get info you should just write:
```
tpsinfo
```

### License
Copyright (c) 2014 Yuri Egorichev  
[The MIT License (MIT)](LICENSE)
