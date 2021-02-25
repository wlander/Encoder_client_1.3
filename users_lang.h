#ifndef USERS_LANG_H
#define USERS_LANG_H
#include <QObject>



class Users_Lang : public QObject
{
    Q_OBJECT
public:

    enum lang_user{
        Disconnected,
        Stoped,
        Asked_connection,
        Connection_OK,
        SD_not_activated,
        SD_activated,
        SD_writing_stoped,
        SD_writing_running,
        Connect,
        Disconnect,
        Check_connection,
        Start,
        Stop,
        Write,
        Files_in_SD,
        Samples_in_cur_file,
        Total_data_in_SD,
        Clear_SD,
        Auto,
        Signal,
        Control,
        Max_lng_usr
    };

    explicit Users_Lang(QObject *parent = nullptr);
    ~Users_Lang();
	QString get_p(int n);
	int set_lang(QString lang);
	
signals:
	
public slots:

public:

const QString lang_ru[Max_lng_usr] = {"Отключено!", "Остановлено!", "Связь  - ?", "Связь - ОК",
        "SD не активирована!", "SD активирована!", "Запись на SD остановлена!", "Запись на SD запущена!", "Подключить",
        "Отключить", "Проверка связи", "Старт", "Стоп", "Запись", "Файлов на SD: ", "Сэмплов в тек. файле: ", "Всего сэмплов на SD: ",
        "Очистить SD", "Авто", "Сигнал", "Контроль"};

const QString lang_en[Max_lng_usr] = {"Disconnected!", "Stoped!", "Connection  - ?", "Connection - OK",
        "SD not activated!", "SD activated!", "SD writing stoped!", "SD writing running", "Connect",
        "Disconnect", "Check connection", "Start", "Stop", "Write", "Files in SD: ", "Samples in cur file: ", "Total data in SD: ",
        "Clear SD", "Auto", "Signal", "Control"};

private:
	char lng;
};

#endif
