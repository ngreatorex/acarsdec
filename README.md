Acarsdec is a multi-channels acars decoder with built-in rtl_sdr front end.

The 2.x version is a complete rewrite, very few code line remains from 1.x

Features :

 - new and improved msk demodulator
 - up to four channels decoded simultaneously
 - multithreaded, sse2 optimized 
 - error detection AND correction (correct all one error and some two errors)
 - input from sound file (.wav) , alsa sound card or software defined radio (SRD) via a rtl dongle (http://sdr.osmocom.org/trac/wiki/rtl-sdr) 

The 4 channels decoding is particularly useful with the rtl dongle. It allows to directly listen simultaneously to 4 different frequencies , with a very low cost hardware.


Usage :
 acarsdec  [-v] [-o lv] [-g gain] [-A] -a alsapcmdevice  | -f sndfile | -r rtldevicenumber  f1 [f2] [f3] [f4] 

 -v :			verbose
 -o lv :		output format : 0 one line by msg., 1 full (default) 
 -A :			don't display uplink messages (ie : only aircraft messages)
 -g gain :		set rtl preamp gain in tenth of db. By default use AGC (ie -g 90 for +9db)
 -p ppm :		set rtl ppm frequency correction
 -f sndfile :		decode from sound file (ie: a .wav file)
 -a alsapcmdevice :	decode from soundcard input alsapcmdevice (ie: hw:0,0)
 -r rtldevicenumber f1 [f2] [f3] [f4] :		decode from rtl dongle number rtldevicenumber receiving at VHF frequencies f1 and optionaly f2 to f4 in Mhz (ie : -r 0 131.525 131.725 131.825 )

Examples :

Decoding from multichannel wav file :
acarsdec -v  -f test.wav 

Decoding from sound card with short output :
acarsdec -o 0 -a hw:0,0

Decoding donwlink messages from rtl dongle number 0 on 3 frequencies :
acarsdec -A -r 0 131.525 131.725 131.825


Compilation :
acarsdec must compile directly on any modern Intel Linux distrib.
It has been tested on x86_64 with fedora 19.
If you want to compile it on an other processor, undefine USE_SSE2 in acarsdec.h

It depends of 3 external libraries :
 - libsndfile for sound file input (rpm package libsndfile-devel on fedora)
 - libasound  for sound card input (rpm package alsa-lib-devel on fedora)
 - librtlsdr for software radio rtl dongle input (http://sdr.osmocom.org/trac/wiki/rtl-sdr)

If you don't have or don't need one of these libs, edit the #define at the beginning of acarsdec.h, to opt-out corresponding code and edit the LDLIBS setting in Makefile.

Testing :
acarsdec-2.x.tar include a test.wav file. It's a 4 channels audio file that contains 7 acars messages.

./acarsdec -o 0 -f test.wav
#2 .PH-BXR KL1681 5V S53A  
#4 .LN-DYY DY083J Q0 S46A  
#2 .LN-DYY DY083J Q0 S47A  
#1 .F-GTAE AF7728 H1 D65C  #DFB00000/V206,05,124,183,02,00,0
#1 .LN-DYY        _d       
#3 .G-DBCK BA031T _d S64A  
#3 .G-DBCK BA031T Q0 S63A  

Notes :
Include sse2 vectorized math functions by Julien Pommier (see sse_mathfun.h)
