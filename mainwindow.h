#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void get_version();
    void get_hostname();
    void get_cpu_model();

    int cpu_time,cpu_time1;
    int cpu_idle_time,cpu_idle_time1;
    int cpu_used;
    int cpu_utility[120];//近120次计算所得的cpu利用率
    int mem_utility[120];//近120次计算所得的mem利用率
    int mem_used;
    int swap_utility[120];//近120次计算所得的swap利用率
    int swap_used;

    //PID
     int countNum;
     QString pid;
     QString pid_name;
     QString mem_size;
     QString priority,nice;
     int cpu_totaltime,cpu_totaltime1;
     int pid_cpu_time[1000],pid_cpu_time1[1000];
     int mem_totalsize;
     float pid_cpu_utility;
     float pid_mem_utility;
     void get_cpu_totaltime();
     void get_mem_totalsize();
     void get_pid_cpuinfo(char *cpuinfo_file);
     void get_pid_meminfo(char *meminfo_file);
    ~MainWindow();

private slots:
    void get_sys_time();
    void get_cpu_utility();
    void get_mem_utility();
    void update_data();
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
