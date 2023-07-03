#include "logger.h"
#include "version_def.h"

uint8_t g_APP_LOG_LEVEL = LOG_INFO; 
const char* g_log_level_str[] =
{
    "LOG_DEBUG",
    "LOG_INFO",
    "LOG_WARN",
    "LOG_ERROR",   
};

const char* log_dir_str = "dr_manager_log", *log_file_str = "dr_manager";
void writeLog()
{
    /*
    QFileInfo info(log_dir_str);
    QString path = info.absoluteFilePath();
    QDir dir(path);
    if(!dir.exists())
        dir.mkpath(path);
    QString date=QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString time=QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QFile file(QString(log_dir_str) + QString("/")
               + QString(log_file_str) + QString("_%1.log").arg(date));
    if(!file.open(QFile::WriteOnly | QFile::Append))
        return;
    QTextStream in(&file);
    QString str1, str2;
    str1 = "\t" + loc_str + "\t@" + APP_VER_STR + "\n";
    str2 = date + " " + time + " " + level_str + " " + msg + "\n";
    in << str1 << str2;
    file.flush();
    file.close();
*/
}
void receive_log()
{
}

bool start_log_thread()
{
    return true;
}
void end_log_thread()
{
}

