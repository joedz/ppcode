#include "log_appender.h"
#include "logger.h"

#include <iostream>
#include <fstream>


namespace ppcode {


    /*******************************************************************
     * 父类 Appender
     *******************************************************************/
    Appender::Appender()
    :m_hasFormatter(false)
    {
    }

    void Appender::setFormatter(LogFormatter::ptr value){
        MutexType::Lock mylock(m_mutex);

        m_formatter = value;
        if(m_formatter) {
            m_hasFormatter = true;
        } else {
            m_hasFormatter = false;
        }
    }

    LogFormatter::ptr Appender::getFormatter(){
        MutexType::Lock mylock(m_mutex);
        return m_formatter;
    }


    /*******************************************************************
     * 输出到控制台  ConsoleAppender类
     *******************************************************************/

    void ConsoleAppender::log(Logger::ptr logger, LogEvent::ptr event){
        if(isAppender(event->getLevel())) {
            MutexType::Lock mylock(m_mutex);
            m_formatter->format(std::cout, event);
            // getFormatter()->format(std::cout, event);  remember 这里会死锁
        }
    }


    /*******************************************************************
     * 输出到文件  FileAppender类
     *******************************************************************/

    FileAppender::FileAppender(const std::string& file_name)
     :m_fileName(file_name)
    {
        if(m_logFile.is_open()) {
                m_logFile.close();
        }
        m_logFile.open(m_fileName, std::ios::app | std::ios::out);
    }

    void FileAppender::log(Logger::ptr logger, LogEvent::ptr event){
        if(isAppender(event->getLevel())){
            MutexType::Lock mylock(m_mutex);
            m_formatter->format(m_logFile, event);
            //getFormatter()->format(m_logFile, event);
        }
    }



    // void FileAppender::append(const std::string& line){
    //     // mutex
    //     if(!m_logFile.is_open()) {
    //         //std::cout << "FileAppender: log file is close" << std::endl;
    //         m_logFile.open(m_fileName, std::ios::app | std::ios::out);
    //     }
    //     m_logFile.write(line.c_str(), line.size());
    //     m_logFile.flush();
    // }
    

}