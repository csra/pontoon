/********************************************************************
**                                                                 **
** File   : app/rsb-server.cpp                                     **
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

#include "io/rst/Informer.h"
#include <boost/program_options.hpp>
#include <boost/make_shared.hpp>
#include <rst/generic/Value.pb.h>
#include <iostream>
#include <mutex>

int main(int argc, char **argv) {
  boost::program_options::variables_map program_options;

  std::string description =
      "This application creates an rsb informer and blocks.";
  std::stringstream description_text;
  description_text << description << "\n\n"
                   << "Allowed options";
  boost::program_options::options_description desc(description_text.str());
  desc.add_options()("help,h", "produce help message");

  desc.add_options()("uri,u",
                     boost::program_options::value<std::string>()->default_value("/rsb_server"),
                     "The informers uri to use.");

  desc.add_options()("message,m",
                     boost::program_options::value<std::string>()->default_value("Hello World!"),
                     "The message to send on startup.");
  ;

  try {
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc),
        program_options);
    boost::program_options::notify(program_options);

    std::stringstream arguments;
    for (int i = 0; i < argc; ++i) {
      arguments << argv[i] << " ";
    }
    std::cerr << "Program started with line: " << arguments.str() << std::endl;

    if (program_options.count("help")) {
      std::cout << desc << "\n";
      return 1;
    }

  } catch (boost::program_options::error &e) {
    std::stringstream arguments;
    for (int i = 0; i < argc; ++i) {
      arguments << argv[i] << " ";
    }
    std::cerr << "Could not parse program options: " << e.what();
    std::cerr << "\n\n" << desc << "\n";
    return 1;
  }

  const std::string uri = program_options["uri"].as<std::string>();

  auto rsb_informer = pontoon::io::rst::Informer<std::string>(uri);
  rsb_informer.publish(boost::make_shared<std::string>(program_options["message"].as<std::string>()),{});

  std::cout << "Ready. Blocking for ever. " << std::endl;
  std::mutex mutex;
  mutex.lock();
  mutex.lock();

}
