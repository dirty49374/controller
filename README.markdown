Dirty49374's fork of Kiibohd Controller
=======================================

* added macro recording feature
* added led animation, videos available on [youtube](https://www.youtube.com/watch?v=oWXFSwU79Tk)
* corresponding kll files are [here](https://github.com/dirty49374/kiibohd-dev-vagrant)

Some of my ideas
================
* It would be nice to support boolean capability function to extend triggers.
  * Condition_inputAbsent_capability( ms: 2 ) ==> returns true if there was no input within givin milliseconds.
* support concrete API for input/output and API daisy chaining with intercepting mechanism.
  * orig_usbCodeSend = intercept(usbCodeSend, myUsbCodeSend);
  * void myUsbCodeSend(...) { if (...) { return my_return; } else { return *orig_usbCodeSend(...); } }
* for led animation, both direct led access API and 2d screen API have to be supported.
  * set_pixel(scan_code, 255) or set_screen_rect(10, 10, 20, 20, 255)
* storing intermediate data of running macro instance
* It's better to have some kind of bi-directional usb protocol to exchange information of current status.
  * for example, keyboard can change layer automatically if OS or active windows changed on PC.
* support conditional evaluation of key press
  * sometimes, it is easy to implement send keystroke and evalute just before next key press/release or timeout.

The Kiibohd Controller
======================

This is the main Kiibohd Firmware.
In general, this should be the **only** git repo you need to clone.
The [KLL](https://github.com/kiibohd/kll) git repo is automatically cloned during the build process.

Please refer to the [KLL](https://github.com/kiibohd/kll) repo or [kiibohd.com](http://kiibohd.com) for details on the KLL (Keyboard Layout Language) Spec.

[![](https://travis-ci.org/kiibohd/controller.svg?branch=master)](https://travis-ci.org/kiibohd/controller)

[![Visit our IRC channel](https://kiwiirc.com/buttons/irc.freenode.net/input.club.png)](https://kiwiirc.com/client/irc.freenode.net/#input.club)



Official Keyboards
------------------

* Infinity 60%
* Infinity 60% LED
* Infinity Ergodox
* WhiteFox
* K-Type (coming soon)


The Kiibohd firmware supports a lot of other keyboards, but these are more obscure/custom/lesser known.



Compilation
-----------

Compilation is possible and tested on Windows/Linux/Mac.
Linux is the easiest using this [VM](https://s3.amazonaws.com/configurator-assets/ArchLinux_kiibohd_2015-02-13.tar.gz).

For most people refer [here](https://github.com/kiibohd/controller/tree/master/Keyboards).

For the full compilation details, please refer to the [wiki](https://github.com/kiibohd/controller/wiki).



Supported Microcontrollers
--------------------------

* Teensy 2.0 (Partial)
* Teensy 2.0++
* Teensy 3.0
* Teensy 3.1/3.2
* mk20dx128vlf5
* mk20dx256vlh7


Adding support for more microcontrollers is possible.
Some considerations for minimum specs:

* ~8  kB of SRAM
* ~25 kB of Flash

It's possible to port chips with lower specs, but will be more effort and have fewer features.



Contributions
-------------

Contributions welcome!

* Bug reports
* Documentation and Wiki editing
* Patches (including new features)



Licensing
---------

Licensing is done on a per-file basis.
Some of the source code is from [PJRC/Teensy](http://pjrc.com), other source code is from the [McHck Project](https://mchck.org).
Code written specifically for the Kiibohd Controller use the following licenses:

* MIT
* GPLv3
* Public Domain



Contact
-------

If you really need to get a hold of HaaTa, email is best: `haata@kiibohd.com`
