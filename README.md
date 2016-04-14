# Pontoon

Yet another bridge. Republishes Imges from ROS to RSB and en/decodes RSB images.

## How do I get set up? ###

    > git clone https://bitbucket.org/vrichter/pontoon.git
    > mkdir -p pontoon/build && cd pontoon/build
    > cmake .. && make

## Applications

### pontoon-encode-images

Can be used to listen to raw images on one scope and publish an encoded version of them to another.
RSB uri syntax is supported.

### pontoon-decode-images

Can be used to listen to encoded images on one scope and publish a raw version of them to another.
RSB uri syntax is supported.

### pontoon-send-image

Can be used to read a single image from a file and publish is as rst::vision::Image or
rst::vision::EncodedImage.

### pontoon-write-images

Can be used to write rst::vision::Image or rst::vision::EncodedImage and write from RSB into files.

### pontoon-image-bridge

Can be used to listen to images on a ROS-topic and publish them via RSB.

## Thanks / 3rd party software

[RSB](https://code.cor-lab.de/projects/rsb "Robotics Service Bus")
[ROS](http://www.ros.org/ "Robot Operating System")
[OpenCV](http://www.ros.org/ "Open Source Computer Vision Library")
[Boost](http://www.boost.org/ "Boost C++ Libraries")
[ZLIB](http://www.zlib.net/ "zlib")

## Copyright

GNU LESSER GENERAL PUBLIC LICENSE

This project may be used under the terms of the GNU Lesser General
Public License version 3.0 as published by the
Free Software Foundation and appearing in the file LICENSE.LGPL
included in the packaging of this project.  Please review the
following information to ensure the license requirements will
be met: http://www.gnu.org/licenses/lgpl-3.0.txt

