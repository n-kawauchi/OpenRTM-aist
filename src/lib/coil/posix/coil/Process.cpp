// -*- C++ -*-
/*!
 * @file  Process.h
 * @brief Process handling functions
 * @date  $Date$
 * @author Noriaki Ando <n-ando@aist.go.jp>
 *
 * Copyright (C) 2010
 *     Noriaki Ando
 *     Task-intelligence Research Group,
 *     Intelligent Systems Research Institute,
 *     National Institute of
 *         Advanced Industrial Science and Technology (AIST), Japan
 *     All rights reserved.
 *
 * $Id$
 *
 */

#ifndef COIL_PROCESS_H
#define COIL_PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <coil/stringutil.h>
#include <coil/File.h>
#include <regex>


namespace coil
{

  /*!
   * @if jp
   * @brief プロセスを起動する
   * @else
   * @brief Launching a process
   * @endif
   */
  int launch_shell(std::string command)
  {
    signal(SIGCHLD, SIG_IGN);

    pid_t pid;
    if((pid = fork()) < 0 )
      { // fork failed
        return -1; 
      }
    
    if (pid == 0) // I'm child process
      {
        //        signal(SIGCHLD, SIG_IGN);
        //        signal(SIGALRM, SIG_IGN);
        //        signal(SIGHUP , SIG_IGN);
        //        signal(SIGPIPE, SIG_IGN);
        //        signal(SIGTERM, SIG_IGN);
        setsid();
        //        close(0);
        //        close(1);
        //        close(2);
        //        open("/dev/null", O_RDWR);
        //        dup2(0, 1);
        //        dup2(0, 2);
        //        umask(0);

        const std::regex base_regex("((?:[^\\s\"\\\\]|\\\\.|\"(?:\\\\.|[^\\\\\"])*(?:\"|$))+)");
        std::sregex_iterator words_begin = std::sregex_iterator(command.begin(), command.end(), base_regex);
        std::sregex_iterator words_end = std::sregex_iterator();

        coil::vstring vstr;

        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::smatch match = *i;
            std::string match_str = replaceString(match.str(), "\"", "");
            vstr.push_back(match_str);
        }
        
        Argv argv(vstr);

        execvp(vstr.front().c_str(), argv.get());
        
        return -1;
      }
    return 0;
  }

  int daemon(int nochdir, int noclose)
  {
    return daemon(nochdir, noclose);
  }

  int create_process(std::string command, std::vector<std::string> &out)
  {
    FILE* fd;
    out.clear();
    if ((fd = popen(command.c_str(), "r")) == NULL)
      {
        //std::cerr << "popen faild" << std::endl;
        return -1;
      }
    do
      {
        char str[512];
        fgets(str, 512, fd);
        std::string line(str);
        if (0 < line.size())
          line.erase(line.size() - 1);
        out.push_back(line);
      } while (!feof(fd));

    return 0;
  }

}; // namespace coil
#endif // COIL_PROCESS_H
