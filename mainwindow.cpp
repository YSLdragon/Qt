#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFile"
#include "QDebug"
#include "QTimer"
#include "QDateTime"
#include "QPainter"
#include "QDir"

int jj=0;
int aa=0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /*cpu*/
    int i=0;
    for(;i<120;i++)cpu_utility[i]=0;//初始化cpu利用率数组
    cpu_time=0;
    cpu_idle_time=0;

     i=0;
    for(;i<120;i++)mem_utility[i]=0;//初始化mem利用率数组
    i=0;
    for(;i<120;i++)swap_utility[i]=0;//初始化swap利用率数组

    get_hostname(); //获取主机名
    get_version();//获取系统版本号
    get_cpu_model();//获取cpu版本

    //计算cpu利用率
    QTimer *timer1 = new QTimer(this);
    connect( timer1, SIGNAL(timeout()), this, SLOT(get_cpu_utility()));//每一秒调用一次
    timer1->start( 1000); // 1秒单触发定时器

    //获取系统时间和运行时长
    QTimer *timer2 = new QTimer(this);
    connect( timer2, SIGNAL(timeout()), this, SLOT(get_sys_time()));
    timer2->start(1000); // 1秒单触发定时器


   //计算内存使用率
    QTimer *timer3 = new QTimer(this);
    connect( timer3, SIGNAL(timeout()), this, SLOT(get_mem_utility()));
    timer3->start(1000); // 1秒单触发定时器

    //获取进程信息
   for(int i=0;i<1000;i++)pid_cpu_time[i]=0;
    cpu_totaltime=0;
    //设置表格
    ui->tableWidget->setColumnCount(6);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    QStringList header;
    header<<"Pid"<<"Name"<<"Priority"<<"nice"<<"CPU"<<"Mem";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setColumnWidth(5,30);
    QTimer *timer4 = new QTimer(this);
    connect(timer4,SIGNAL(timeout()), this, SLOT(update_data()));
    timer4->start(3000); // 3秒单触发定时

}

void MainWindow::get_hostname()//获取主机名
{

    QFile hostname_file("/proc/self/root/etc/hostname");//
    if (!hostname_file.open(QIODevice::ReadOnly | QIODevice::Text))
            qDebug()<<"文件打开失败(获取主机名)";
    QTextStream in_hostname(&hostname_file);
    QString hostname= in_hostname.readLine();//读取第零行
    ui->lineEdit_5->setText(hostname);//显示获取的主机名
}

void MainWindow::get_version()//获取系统版本号
{
    QFile version_file("/proc/version");
    if (!version_file.open(QIODevice::ReadOnly | QIODevice::Text))qDebug()<<"文件打开失败(系统版本号)";
    QTextStream in_version(&version_file);
    QString version= in_version.readLine();
    char  *ch1,*ch2;
    QByteArray ba=version.toLatin1();//转化为字节数组
    ch1=ba.data();
    ch1=strtok(ch1," ");
    ch2=strtok(NULL," ");
    ch2=strtok(NULL," ");
    QString str1=QString("%1").arg(ch1);
    QString str2=QString("%1").arg(ch2);
    ui->lineEdit_6->setText(str1);
    ui->lineEdit_7->setText(str2);
}

void MainWindow::get_cpu_model()//获取cpu相关信息
{
    QFile cpuinfo_file("/proc/cpuinfo");
    if (!cpuinfo_file.open(QIODevice::ReadOnly | QIODevice::Text))qDebug()<<"文件打开失败(获取cpu版本)";
    QTextStream in_cpumodel(&cpuinfo_file);
    QString cpu_model;
    for(int i=0;i<2;i++)cpu_model=in_cpumodel.readLine();//读到第2行的CPU类型
    char  *ch1,*ch2,*ch3,*ch4;
    QByteArray ba=cpu_model.toLatin1();
    ch1=ba.data();
    strtok(ch1,":");
    ch1=strtok(NULL,":");
    ui->lineEdit_3->setText(ch1);
    for(int i=0;i<3;i++)cpu_model=in_cpumodel.readLine();//读到第5行的CPU名称
    ba=cpu_model.toLatin1();
    ch2=ba.data();
    strtok(ch2,":");
    ch2=strtok(NULL,":");
    ui->lineEdit_2->setText(ch2);
    for(int i=0;i<3;i++)cpu_model=in_cpumodel.readLine()+" HZ";//读到第8行的CPU频率
    ba=cpu_model.toLatin1();
    ch3=ba.data();
    strtok(ch3,":");
    ch3=strtok(NULL,":");
    ui->lineEdit_4->setText(ch3);

    cpu_model=in_cpumodel.readLine();//读到第9行的CPU缓存
    ba=cpu_model.toLatin1();
    ch4=ba.data();
    strtok(ch4,":");
    ch4=strtok(NULL,":");
    ui->lineEdit_8->setText(ch4);
}

void MainWindow::get_sys_time()
{
    //获取cpu已运行时间
    QFile file("/proc/uptime");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))return;
    QTextStream in(&file);
    QString line = in.readLine();//第一行首个字符串代表运行时间
    char  *ch;
    QByteArray ba=line.toLatin1();
    ch=ba.data();
    int time=atof(ch);//转化为浮点型
    QString str1=QString::number(time%60);
    time=time/60;
    QString str2=QString::number(time%60);
    QString str3=QString::number(time/60);
    QString str=str3+"时"+str2+"分"+str1+"秒";
    ui->lineEdit_9->setText(str);
    //获取系统时间
    ui->lineEdit_10->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd"));
}

void MainWindow::get_cpu_utility()
{
    char *user,*nice,*system,*idle,*iowait,*irq,*softirq,*stealstolen,*guest;
    cpu_time1=cpu_time;//cpu_time为上一次计算所得的cpu总时间
    cpu_idle_time1=cpu_idle_time;//cpu_idle_time为上一次计算所得的cpu总空闲时间

    QFile stat_file("/proc/stat");
    if (!stat_file.open(QIODevice::ReadOnly | QIODevice::Text)) qDebug()<<"文件打开失败(获取cpu信息)";
    QTextStream in_cpu(&stat_file);//转换为文本方式读取文件
    QString cpu_info= in_cpu.readLine();//逐行读取
    char  *ch;
    QByteArray ba=cpu_info.toLatin1();//获得该字符串的latin1值,以便下一步转换为char*
    ch=ba.data();

    strtok(ch," ");//按空格分割字符串,ch为第一个空格前的字符串,即"cpu"
    user=strtok(NULL," ");
    nice=strtok(NULL," ");
    system=strtok(NULL," ");
    idle=strtok(NULL," ");
    iowait=strtok(NULL," ");
    irq=strtok(NULL," ");
    softirq=strtok(NULL," ");
    stealstolen=strtok(NULL," ");
    guest=strtok(NULL," ");

    //用atof()函数将字符串转为数值后进行计算
    cpu_time=atof(user)+atof(nice)+atof(system)+atof(idle)+atof(iowait)+atof(irq)+atof(softirq)+atof(stealstolen)+atof(guest);
    cpu_idle_time=atof(idle);
    cpu_used=100*((cpu_time-cpu_time1)-(cpu_idle_time-cpu_idle_time1))/(cpu_time-cpu_time1);//计算利用率,此处乘100与下一行连接"%"对应
    ui->progressBar->setValue(cpu_used);//在界面对应控件中显示

    for(int k=0;k<119;k++)cpu_utility[k]=cpu_utility[k+1];//刷新间隔是1秒，所以每次向前推进1秒
    cpu_utility[119]=cpu_used;

    //制作cpu使用率变化图
    QPixmap pix(650,200);
    QPainter painter(&pix);
    pix.fill(Qt::blue);
    QPen pen;
    pen.setColor(Qt::white);
    painter.setPen(pen);
    for(int i=1;i<5;i++)
    {
        painter.drawLine(0,i*50,600,i*50);
        painter.drawText(0,200-i*50,QString::number(25*i)+"%");
    }
    painter.drawText(0,0,QString::number(100)+"%");
    for(int i=0;i<12;i++)painter.drawText(50*i,200,QString::number(10*i));
    painter.drawText(600,200,"单位/秒");
    QPen pen1;
    pen1.setColor(Qt::red);
    pen1.setWidth(3);
    painter.setPen(pen1);
    for(int j=0;j<119;j++)painter.drawLine(5*j,200-2*cpu_utility[j],5*(j+1),200-2*cpu_utility[j+1]);

    ui->label_23->setPixmap(pix);
    ui->label_23->setScaledContents(true);
}

void MainWindow::get_mem_utility()
{
    QFile meminfo_file("/proc/meminfo");
    if (!meminfo_file.open(QIODevice::ReadOnly | QIODevice::Text))qDebug()<<"文件打开失败";
    QTextStream in_memsize(&meminfo_file);
    /*内存利用率*/
    QString memfree,memtotal,swapfree,swaptotal;
    memtotal=in_memsize.readLine().simplified();//第一行是内存的总大小
    memfree=in_memsize.readLine().simplified();//第二行是内存的空闲部分大小

    char  *ch,*ch1;
    QByteArray ba1,ba;
    ba=memtotal.toLatin1();
    ba1=memfree.toLatin1();
    ch=ba.data();
    ch1=ba1.data();

    strtok(ch,":");
    ch=strtok(NULL,":");
    ui->label_17->setText(ch);

    strtok(ch1,":");
    ch1=strtok(NULL,":");
    ui->label_16->setText(ch1);


    mem_used=(100*(atof(ch)-atof(ch1))/atof(ch));
    ui->progressBar_2->setValue(mem_used);

    for(int k=0;k<119;k++)mem_utility[k]=mem_utility[k+1];//刷新间隔是1秒，所以每次向前推进1秒
    mem_utility[119]=mem_used;

    //制作memory使用率变化图
    QPixmap pix(650,200);
    QPainter painter(&pix);
    pix.fill(Qt::blue);
    QPen pen;
    pen.setColor(Qt::white);
    painter.setPen(pen);
    for(int i=1;i<5;i++)
    {
        painter.drawLine(0,i*50,600,i*50);
        painter.drawText(0,200-i*50,QString::number(25*i)+"%");
    }
    painter.drawText(0,0,QString::number(100)+"%");
    for(int i=0;i<12;i++)painter.drawText(50*i,200,QString::number(10*i));
    painter.drawText(600,200,"单位/秒");
    QPen pen1;
    pen1.setColor(Qt::red);
    pen1.setWidth(3);
    painter.setPen(pen1);
    for(int j=0;j<119;j++)painter.drawLine(5*j,200-2*mem_utility[j],5*(j+1),200-2*mem_utility[j+1]);

    ui->label_24->setPixmap(pix);
    ui->label_24->setScaledContents(true);

    /*交换分区利用率*/
    for(int i=0;i<17;i++) swaptotal=in_memsize.readLine().simplified();
    swapfree=in_memsize.readLine().simplified();

    ba=swaptotal.toLatin1();
    ba1=swapfree.toLatin1();
    ch=ba.data();
    ch1=ba1.data();

    strtok(ch,":");
    ch=strtok(NULL,":");
    ui->label_22->setText(ch);

    strtok(ch1,":");
    ch1=strtok(NULL,":");
    ui->label_21->setText(ch1);

    swap_used=(100*(atof(ch)-atof(ch1))/atof(ch));

    ui->progressBar_3->setValue(swap_used);

    for(int k=0;k<119;k++)swap_utility[k]=swap_utility[k+1];//刷新间隔是1秒，所以每次向前推进1秒
    swap_utility[119]=swap_used;

    //制作swap使用率变化图
    QPixmap pix1(650,200);
    QPainter painter1(&pix1);
    pix1.fill(Qt::blue);
    QPen pen2;
    pen2.setColor(Qt::white);
    painter1.setPen(pen2);
    for(int i=1;i<5;i++)
    {
        painter1.drawLine(0,i*50,600,i*50);
        painter1.drawText(0,200-i*50,QString::number(25*i)+"%");
    }
    painter1.drawText(0,0,QString::number(100)+"%");
    for(int i=0;i<12;i++)painter1.drawText(50*i,200,QString::number(10*i));
    painter1.drawText(600,200,"单位/秒");
    QPen pen3;
    pen3.setColor(Qt::red);
    pen3.setWidth(3);
    painter1.setPen(pen3);
    for(int j=0;j<119;j++)painter1.drawLine(5*j,200-2*swap_utility[j],5*(j+1),200-2*swap_utility[j+1]);

    ui->label_25->setPixmap(pix1);
    ui->label_25->setScaledContents(true);
}

void MainWindow::update_data()
{
    QDir *dir=new QDir("/proc");
    QStringList filter;
    QList<QFileInfo> *filelist=new QList<QFileInfo>(dir->entryInfoList(filter));
    get_mem_totalsize();
    get_cpu_totaltime();

     //获取进程信息
    int sum=filelist->count();
    QString filename;
    countNum=0;

    for(int i=0;i<sum;i++)
    {
        filename=filelist->at(i).fileName();
        if(filename!="."&&filename!=".."&&filename.toInt()!=0)countNum++;
    }

    ui->tableWidget->setRowCount(countNum);

    //获取进程信息
    char *cpuinfo_file,*meminfo_file;
    QString stat_file,status_file;
    QByteArray ba,ba1;

    for( jj=0;jj<countNum;jj++)
    {
         filename=filelist->at(jj).fileName();
         stat_file="/proc/"+filename+"/stat";
         status_file="/proc/"+filename+"/status";
         ba=stat_file.toLatin1();
         ba1=status_file.toLatin1();
         cpuinfo_file=ba.data();
         meminfo_file=ba1.data();
         if(filename!="."&&filename!=".."&&filename.toInt()!=0)
         {

             get_pid_cpuinfo(cpuinfo_file);
             get_pid_meminfo(meminfo_file);
             QTableWidgetItem *item=new QTableWidgetItem(filename);
             item->setTextAlignment(Qt::AlignCenter);
             QTableWidgetItem *item1=new QTableWidgetItem(pid_name.simplified());
             item->setTextAlignment(Qt::AlignCenter);
             QTableWidgetItem *item2=new QTableWidgetItem(priority);
             item->setTextAlignment(Qt::AlignCenter);
             QTableWidgetItem *item3=new QTableWidgetItem(nice);
             item->setTextAlignment(Qt::AlignCenter);
             QTableWidgetItem *item4=new QTableWidgetItem(QString::number(pid_cpu_utility,'f',2)+"%");
             item->setTextAlignment(Qt::AlignCenter);
             QTableWidgetItem *item5=new QTableWidgetItem(QString::number(pid_mem_utility,'f',2)+"%");
             item->setTextAlignment(Qt::AlignCenter);
             ui->tableWidget->setItem(jj-2,0,item);
             ui->tableWidget->setItem(jj-2,1,item1);
             ui->tableWidget->setItem(jj-2,2,item2);
             ui->tableWidget->setItem(jj-2,3,item3);
             ui->tableWidget->setItem(jj-2,4,item4);
             ui->tableWidget->setItem(jj-2,5,item5);
         }
    }
}


void MainWindow::get_cpu_totaltime()//获取cpu总时间
{
    cpu_totaltime1=cpu_totaltime;
    char *user,*nice,*system,*idle,*iowait,*irq,*softirq,*stealstolen,*guest;
    QFile stat_file("/proc/stat");

    if (!stat_file.open(QIODevice::ReadOnly | QIODevice::Text))qDebug()<<"文件打开失败(获取cpu信息)";
    QTextStream in_cpu(&stat_file);
    QString cpu_info= in_cpu.readLine();
    char  *ch;
    QByteArray ba=cpu_info.toLatin1();
    ch=ba.data();
    strtok(ch," ");
    user=strtok(NULL," ");
    nice=strtok(NULL," ");
    system=strtok(NULL," ");
    idle=strtok(NULL," ");
    iowait=strtok(NULL," ");
    irq=strtok(NULL," ");
    softirq=strtok(NULL," ");
    stealstolen=strtok(NULL," ");
    guest=strtok(NULL," ");
    cpu_totaltime=atof(user)+atof(nice)+atof(system)+atof(idle)+atof(iowait)+atof(irq)+atof(softirq)+atof(stealstolen)+atof(guest);
}

void MainWindow::get_mem_totalsize()//获取内存容量
{
    QFile meminfo_file("/proc/meminfo");
    if (!meminfo_file.open(QIODevice::ReadOnly | QIODevice::Text))qDebug()<<"文件打开失败(获取主机名)";
    QTextStream in_memsize(&meminfo_file);
    QString memtotal=in_memsize.readLine();
    char  *ch;
    QByteArray ba;
    ba=memtotal.toLatin1();
    ch=ba.data();
    strtok(ch,":");
    ch=strtok(NULL,":");
    mem_totalsize=atof(ch);
}


void MainWindow::get_pid_cpuinfo(char *cpuinfo_file)//获取进程cpu信息
{
    pid_cpu_time1[jj]=pid_cpu_time[jj];
    char *utime,*stime,*cutime,*cstime,*pri,*nic;
    QFile stat_file(cpuinfo_file);
    if (!stat_file.open(QIODevice::ReadOnly | QIODevice::Text))qDebug()<<"文件打开失败(获取cpu信息)";
    QTextStream in_cpu(&stat_file);
    QString cpu_info= in_cpu.readLine();//读取文件
    QByteArray ba=cpu_info.toLatin1();
    char  *ch=ba.data();
    strtok(ch," ");
    for(int i=0;i<12;i++)strtok(NULL," ");
    utime=strtok(NULL," ");
    stime=strtok(NULL," ");
    cutime=strtok(NULL," ");
    cstime=strtok(NULL," ");
    pri=strtok(NULL," ");
    nic=strtok(NULL," ");
    priority=QString::number(atof(pri));
    nice=QString::number(atof(nic));
    pid_cpu_time[jj]=atof(utime)+atof(stime)+atof(cutime)+atof(cstime);//获取进程时间
    pid_cpu_utility=100*(pid_cpu_time[jj]-pid_cpu_time1[jj])/(cpu_totaltime-cpu_totaltime1);
}

void MainWindow::get_pid_meminfo(char *meminfo_file)//获取内存信息
{
    QFile status_file(meminfo_file);
    if (!status_file.open(QIODevice::ReadOnly | QIODevice::Text))qDebug()<<"文件打开失败(获取cpu信息)";
    QTextStream in_mem(&status_file);
    pid_name=in_mem.readLine();//获取进程名
    pid_name=pid_name.section(':',1,1);
    if(aa==0){qDebug()<<pid_name;aa++;}
    if(aa==1){qDebug()<<pid_name;aa++;}
    if(aa==2){qDebug()<<pid_name;aa++;}
    for(int i=0;i<21;i++)mem_size=in_mem.readLine();//读取进程实际占用的内存大小
    QByteArray ba=mem_size.toLatin1();
    char  *ch=ba.data();
    strtok(ch,":");
    ch=strtok(NULL,":");
    pid_mem_utility=100*atof(ch)/mem_totalsize;
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    system("poweroff");
}

void MainWindow::on_pushButton_2_clicked()
{
    QTableWidgetItem *item=ui->tableWidget->currentItem();
    QString pro=item->text();
    pro=pro.section('\t',0,0);
    system("kill "+pro.toLatin1());
}
