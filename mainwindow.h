#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QPushButton *load;
    QPushButton *encode;
    QPushButton *decode;
    QTableWidget *table;
    QMap<char, int> characters;
    QByteArray originalFileInput;
    qint64 originalFileSize;

    int nRows;
    int nCols;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void loadClicked();
    void encodeClicked();
    void decodeClicked();
};
#endif // MAINWINDOW_H
