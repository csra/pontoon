/********************************************************************
**                                                                 **
** File   : src/convert/ConvertRstRosImage.cpp                     **
** Authors: Viktor Richter                                         **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
********************************************************************/

#include <convert/ConvertRstRosImage.h>

#include <sensor_msgs/image_encodings.h>

#include <utils/Exception.h>

namespace {

  int rstChannels(const sensor_msgs::ImageConstPtr& src){
    return sensor_msgs::image_encodings::numChannels(src->encoding);
  }

  rst::vision::Image_Depth rstDepth(const sensor_msgs::ImageConstPtr& src){
    switch (sensor_msgs::image_encodings::bitDepth(src->encoding)) {
      case 8:
        return rst::vision::Image::DEPTH_8U;
      case 16:
        return rst::vision::Image::DEPTH_16U;
      case 32:
        return rst::vision::Image::DEPTH_32F;
      default:
        throw Exception(std::string("Cannot match channel depth from '")
                               + src->encoding + std::string("' to rst-depth."));
    }
  }

  bool startsWith(const std::string& string, const std::string& prefix){
    return 0 == string.compare(0,prefix.size(),prefix);
  }

  rst::vision::Image_ColorMode rstColorMode(const sensor_msgs::ImageConstPtr& src){
    if(startsWith(src->encoding,"mono")) return rst::vision::Image::COLOR_GRAYSCALE;
    if(startsWith(src->encoding,"rgb"))  return rst::vision::Image::COLOR_RGB;
    if(startsWith(src->encoding,"bgr"))  return rst::vision::Image::COLOR_BGR;
    if(startsWith(src->encoding,"bgr"))  return rst::vision::Image::COLOR_BGR;
    if(src->encoding == sensor_msgs::image_encodings::YUV422) return rst::vision::Image::COLOR_YUV422;
    throw Exception(std::string("Cannot match color mode from '"
                                       + src->encoding + std::string("' to rst-color-mode.")));
  }

  rst::vision::Image_DataOrder rstDataOrder(const sensor_msgs::ImageConstPtr& src){
    return rst::vision::Image::DATA_INTERLEAVED;
  }

}

boost::shared_ptr<rst::vision::Image> ConvertRstRosImage::convert(const sensor_msgs::ImageConstPtr src){
  typedef rst::vision::Image RstImage;
  typedef boost::shared_ptr<RstImage> RstImagePtr;
  typedef sensor_msgs::ImageConstPtr RosImagePtr;

  RstImagePtr image = RstImagePtr(new RstImage());

  image->set_width(src->width);
  image->set_height(src->height);
  image->set_channels(rstChannels(src));
  image->set_depth(rstDepth(src));
  image->set_color_mode(rstColorMode(src));
  image->set_data_order(rstDataOrder(src)); // i think it is always interleaved
  image->mutable_data()->resize(src->step * src->height);
  std::copy(src->data.begin(),src->data.end(),image->mutable_data()->begin());

  return image;
}

sensor_msgs::ImageConstPtr convert::ConvertRstRosImage::convert(boost::shared_ptr<rst::vision::Image> src){
  throw utils::Exception("Function not implemented.");
}
